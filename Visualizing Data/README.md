# Visualizing Data
Taking data from sensors is a big step in the right direction for making a useful embedded device. However, if the end user can't see the data in a useful way, then it is all for naught. For this portion of the lab we explore and implement the visualization of data on the LCD screen of the FR6989.

## Type of Data Being Acquired
The data being acquired is 12-bit chunks being fed in from the ADC. Our choice for high reference voltage on the ADC is 2.5V, so the sensor is arbitrary as long as it's output range is between 0-2.5V.

## Data Acquisition
Data acquisition is identical to the sensors portion of this lab, but in short we use a 12 bit ADC (A1) on the 6989. This value is stored in a variable ADCin. ADCin in the last portion of the lab would have been split into two 8 bit chunks to send over UART. However, for the purposes of this lab, that portion of the code was gutted and replaced with code to convert the value in ADCin to hex characters (0x000-0xFFF) and then displayed on the LCD.

## LCD Display
The meat of the code lies in our ```c void ConvertToLCD(); ``` and ```c void LCDsetup(); ``` functions. Both of these functions are reliant on the LCDDriver.h and LCDDriver.c files:

The code includes the LCDDriver file and declares our new functions:
```c
#include <LCDDriver.h>

//functions to be used:
void ADCSetup();
void TimerA0setup();
void LCDsetup();    //new function
void ConvertToLCD();//new function
```
Within the LCD setup function we set the pins for the LCD Display, initialize the LCD segments, setup the LCD clock synchronization, clear the LCD memory, and turn the LCD screen on:
```c
void LCDsetup(void)
{
      PJSEL0      = BIT4 | BIT5;                               // Sets pins for LCD Display
      LCDCPCTL0   = 0xFFFF;                                    // Initializes the LCD Segments
      LCDCPCTL1   = 0xFC3F;                                    // Initializes the LCD Segments
      LCDCPCTL2   = 0x0FFF;                                    // Initializes the LCD Segments
      LCDCCTL0    = LCDDIV__1 | LCDPRE__16 | LCD4MUX | LCDLP;
      LCDCVCTL    = VLCD_1 | VLCDREF_0 | LCDCPEN;
      LCDCCPCTL   = LCDCPCLKSYNC;                              // Clock synchronization enabled
      LCDCMEMCTL  = LCDCLRM;                                   // Clear LCD memory
      LCDCCTL0   |= LCDON;
}
```

Within the ADC to LCD visualization function we convert the raw binary to characters such that they can be processed by the LCDDriver files:

```c
void ConvertToLCD(int input)
{
      iones = (input%10);             //gets ones place value
      itens = ((input/10)%10);        //gets tens place value
      ihundreds = ((input/100)%10);   //gets hundreds place value
      ithousands = ((input/1000)%10); //gets thousands place value

      showChar(iones + '0', 4);         //the following four lines display values on LCD.
      showChar(itens + '0', 3);         //Worth noting that the + '0' are essential in properly setting the values as their proper       
      showChar(ihundreds + '0', 2);     //character number for display
      showChar(ithousands + '0', 1);
}
```

Finally, in the main function, we use a polling loop to constantly update the LCD screen as values from the ADC change:

```c
while(1){   //loop forever
        ConvertToLCD(ADCin);    // function that converts ADC to a decimal number and displays on LCD
        __bis_SR_register(LPM0_bits + GIE);      //power saving.
    }
```
