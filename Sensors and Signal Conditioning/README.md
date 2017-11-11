# LAB 5 Sensors and Signal Conditioning

For this portion of lab 5, we will be exploring the Analog to Digital Converter (ADC) features and capabilities of the 5 launchpad boards / processors. We are tasked with designing around taking voltage changes from 3 different circuits that each have a different sensor at their core, converting these voltages to a HEX value, and then assigning meaning to these hex values in the form of light levels and temperature. Finally, the values must be transmitted via UART to be observed through realterm.

This readme file will be broken up into two main segments:
1.  Code design
2.  Circuit design

Before we get into the design aspects, let's look at the problem a little more closely.

## Sensors Used:
We will be taking voltage changes from 3 different sensors:
1.  A generic Photoresistor
  <img src="https://cdn.sparkfun.com//assets/parts/2/4/6/2/09088-02-L.jpg" width="100" height="120">
2.  <a href="http://www.ttelectronics.com/sites/default/files/download-files/OP800_830_SL%26WSL_Series_7.pdf">The OP805SL phototransistor</a>
<img src="https://na.suzohapp.com/images/o/OP805SL_V1.jpg" width="100" height="120">
3.  <a href="https://halckemy.s3.amazonaws.com/uploads/attachments/345863/lm35.pdf">The LM35 Temperature sensor</a>
<img src="https://www.geeker.co.nz/images/detailed/1/LM35.jpg" width="100" height="120">

Each of these sensors have material properties that allow more or less voltage to drop across them depending on light or temperature levels. We must design circuits and code such that we can get a linear relationship between the change in sensed phenomenon and voltage drop. Our method to solve this problem can be broken down as follows:
1.  Design circuits such that each circuit has a similar range of voltage changes so that code only needs to be written once.
2.  Write code such that the highest attainable voltage equates to all 1s in the ADC memory register, and the lowest attainable voltages equates to all 0s in the ADC memory register. 

A quick example: IF max voltage is 2.5V and 2.5V is read on the ADC, the ADC memory should store 0xFFF for a 12bit ADC memory register. Likewise if 0V is read, 0x000 should be stored.
#####     Note that 12 bit ADC memory registers were used when possible, however the G2553 only has a 10 bit ADC memory register.
3.  Write code that sends data through UART as it is being taken so that data isn't missed.


## Code
The code we will be looking at here is for the G2553, however the same logic is used for all other boards. Differences in register names will not be talked about, however functional differences will be. The code will be broken down into two sections: Data acquisition and data transmission.
### Data Acuisition Code
In order to take voltage readings we must first assign a GPIO pin as an ADC input pin:
```c
ADC10CTL1 = INCH_0;                       // input A0
ADC10AE0 |= 0x01;                         // P1.0 A0 ADC option select
```
This piece of code flips a bit in the ADC10CTL1 register that assigns P1.0 as the A0 ADC input. No PxSEL register is required, this assignment overwrites any PxSEL bits. The second line of code enables analog input on that pin. Now our P1.0 is ready for ADC. However, we must also set up the ADC.

ADC10 is the ADC available on the G2553. On all the processors, ADC12 was used. The difference is 2 more available bits on the ADCMEMx registers. ADCMEMx is the register that physically holds binary values that the voltage readings are converted to.
```c
ADC10CTL0 = ADC10SHT_2 + REF2_5V + ADC10ON + ADC10IE + REFON;
```
The above code flips bits in the ADC10CTL0 register (the ADC control register 0). We set ADC10SHT_2 which tells the ADC to sample and hold values for 16 ADC10CLKs. REF2_5V tells the ADC to use an internal 2.5V as high voltage reference. ADC10ON turns on the ADC. ADC10IE enables ADC10 interrupts. REFON tells the ADC to turn on internal reference voltage.

The above snippets of code are the entirety of the setup for the ADC. The following code is how we put the ADC to use:
First, inside a timer interrupt, we tell the ADC to sample and convert values every time CCR0 is reached. This gives us control on how often a sample is taken, as we can modify the settings of the timer.
```c
ADC10CTL0 |= ENC + ADC10SC;            // Sampling and conversion start
```

When a sample is taken, the ADC10 interrupt service routine is triggered due to our previous enabling of ADC10 interrupts. Within this ISR, we save the converted values stored in ADCMEM0 into integer variables to be transmitted:
```c
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
      msHex = ADC10MEM;                    // get adc12mem0 value
      lsHex = msHex;
      msHex = msHex >> 8;                   // shift to only hold top 4 bits
```
At this point, is important to mention that the UART transmit buffer can only hold 8 bits. However, for the G2553 we have 10 bits that need transmitting, and for the other boards we have 12 bits that need to be transmitted. The above code saves all of the bits into integers msHex and lsHex. Then, msHex is shifted right by 8 so that the most significant bits have been moved to the least significant bit positions. The values in msHex and lsHex will then be transmitted one after the other so that the full value stored in ADC10MEM0 is transmitted. An example may make this process a bit clearer:
```
ADC10MEM0 = 11 0011 1111.
msHex = 11 0011 1111 shifted right 8 bits = 00 0000 0011.
lsHex = 11 0011 1111.
Transmission 1 (8 bits only) = msHex (first 8 bits) = 0000 0011.
Transmission 2 (8 bits only) = lsHex (first 8 bitS) = 0011 1111.
Full transmission = 0000 0011 0011 1111 = ADC10MEM0.
The full value of ADC10MEM0 was transmitted.
```
So, now that we have good values stored to be transmitted, we want to transmit the most significant bits which are stored in msHex followed by the least significant bits stored in lsHex:
```c
      IE2 |= UCA0TXIE;                          // Enable USCI_A0 TX interrupt
      UCA0TXBUF = msHex;                        //send top 2 bits of ADC10MEM0

      while (!(IFG2&UCA0TXIFG));
      IE2 |= UCA0TXIE;                          // Enable USCI_A0 TX interrupt
      UCA0TXBUF = lsHex;                        //send bottom 8 bits of ADC10MEM0

      IE2 &= ~UCA0TXIE;                         // disable interrupts for Transmission
```
The above code transmits the value of ADC10MEM0 in two transmission chunks.

Issues were found and resolved between UART transmission and the timer that causes sampling and conversion to occur. It is important that the timer interrupt uses the same clock as the UART setup:
```c
void UARTSetup()
{
    P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2 ;                    // P1.1 = RXD, P1.2=TXD
    
    DCOCTL = 0;                               // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
    DCOCTL = CALDCO_1MHZ;
    BCSCTL2 &= ~(DIVS_3);                     // SMCLK = DCO = 1MHz
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 104;                            // 1MHz 9600
    UCA0BR1 = 0;                              // 1MHz 9600
    UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
}
void TimerA0setup()
{
      TA0CCTL0 = CCIE;                             // CCR0 interrupt enabled
      TA0CCR0 = 65000;                             //overflow roughly once per second
      TA0CTL = TASSEL_2 + MC_2 + ID_3;                  // SMclk, continuous mode, 8x divider
}
```
The above code shows that both UART and the timer use SMCLK. In the timer setup, it is worth noting that we used a clock divider and continuous mode so that the UART transmission would occur slower. Furthermore, in the UART setup, we use the TX and RX pins that connect to the emulation board, so that UART transmission can be done via USB to a computer.

## Hardware

<img src="https://cdn.discordapp.com/attachments/283426680582832128/378985635169435661/unknown.png">

The phototransistor is an NPN transistor where photons are used to control the base current. The more photons (light) to pass through into the base, creates more electron-hole pairs, this creates a current proportional to the intensity of the light. So in a sense, a phototransistor is a current source. 
To interact the phototransistor with our μc required us to convert the current generated by the phototransistor to a voltage. This was done using a transimpedance amplifier as depicted in [LABEL FIGURE NUMBER STEVE].  A nodal analysis at the inverting input reveals that 
V\_out=I\_c*R\_1. This implies that if a 1 ohm feedback resistor is used then we, can have a 1:1 ratio. This however is not applicable as the amount of current generated by the phototransistor would provide to too small of a voltage for the ADC to read. So, what must be done is use a larger value feedback resistor to be able to read the voltage; all the while keeping in mind that the true current is divided by that resistor value.

<img src="https://cdn.discordapp.com/attachments/283426680582832128/378990162501173249/unknown.png" width="100" height="120">

A photoresistor is a light-controlled variable resistor. The resistance of the photoresistor is inversely proportional the amount of light hitting it; this means that the more light hitting the resistor, the lower the resistance and inversely, the less light hitting the photoresistor, the higher the resistance. To read the resistance of the photoresistor on the μc required us to read the voltage across the it. This was done using a voltage divider circuit where the the photoresistor is the top resistor, this yields a V\_out proportional to the resistance of the photoresistor. 
The most difficult part was choosing the appropriate R-value to give us the full reading of the photoresistor. In ambient light, we measured a max photoresistance (Rp) of 36500 ohms, and when a cellphone flash was shined directly on it a minimum photoresistance of 100 ohms. These values required us to pick a Vin and R-value such that when Rp was 100, Vout was our chosen reference voltage of 2.5v and when Rp was 36500 ohms our Vout was approximately zero volts. After choosing a Vin of 3.3 volts, a trial and error procedure was run, with an R of 315 ohms providing our desired results.


The LM35 is a centigrade temperature sensor. This section of the was the easiest as the LM35 out is a voltage directly proportional to the temperature sensor. All that was required was to connect the Vout (pin 2) of the LM35DZ to the ADC input and provide a voltage V to pin one such that  V is within 4 and 20 volts, and lastly ground to pin 3.  The ground was provided by the msp430 so that the LM35DZ and msp430 shared the same ground, and V was provided by a breadboard power supply so that we could get 5 volts. The breadboard power supply was used because all of the development boards, for example, the G2, do not supply a 5-volt GPIO header. To calculate the temperature the linear equation V\_out=10mv*Temperature was used noting that the value read by the ADC is Vout. This equation was used because the LM35DZ datasheet specifies that linear relationship.
