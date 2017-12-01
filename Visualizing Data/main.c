//Lab 5: Visualizing Data
//Authors: Stefan Symczak & Matt Delengowski & Glenn Dawson
//Device: MSP430FR6989
#include <msp430.h>
#include <LCDDriver.h>

//functions to be used:
void ADCSetup();
void TimerA0setup();
void LCDsetup();
void ConvertToLCD();

//integer variables to be used:
int iones, itens, ihundreds, ithousands, ADCin = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    PM5CTL0 &= ~LOCKLPM5;       //Disable high impedance mode.
    ADCSetup();
    TimerA0setup();
    LCDsetup();
    while(1){   //loop forever
        ConvertToLCD(ADCin);    // function that converts ADC to a decimal number and displays on LCD
        __bis_SR_register(LPM0_bits + GIE);      //power saving.
    }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{

    ADC12CTL0 |= ADC12ENC | ADC12SC; // begin sampling and conversion

    __bic_SR_register_on_exit(LPM0_bits); //exits into LPM0
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
  switch (__even_in_range(ADC12IV, ADC12IV_ADC12RDYIFG))
  {
    case ADC12IV_NONE:        break;            // Vector  0:  No interrupt
    case ADC12IV_ADC12IFG0:                     // Vector 12:  ADC12MEM0 Interrupt
        ADCin = ADC12MEM0;                      // assign value of memory to manipulate in conversion function
        __bic_SR_register_on_exit(LPM0_bits);   // Exit active CPU
      break;                                    // Clear CPUOFF bit from 0(SR)

    default: break;
  }
}

void ADCSetup()
{
    // By default, REFMSTR=1 => REFCTL is used to configure the internal reference
      while(REFCTL0 & REFGENBUSY);              // If ref generator busy, WAIT
      REFCTL0 |= REFVSEL_2 | REFON;             // Select internal ref = 2.5V
                                                // Internal Reference ON

      // Configure ADC12
      ADC12CTL0  = ADC12ON | ADC12SHT0_8 | ADC12MSC;  // Sets up multiple sample conversion
      ADC12CTL0 = ADC12SHT0_2 | ADC12ON;
      ADC12CTL1 = ADC12SHP | ADC12CONSEQ_2;;         // ADCCLK = MODOSC; sampling timer
      ADC12CTL2 |= ADC12RES_2;                          // 12-bit conversion results
      ADC12IER0 |= ADC12IE0;                         // Enable ADC conv complete interrupt
      ADC12MCTL0 |= ADC12INCH_10 | ADC12VRSEL_1;    // A1 ADC input select; Vref=2.5V

      while(!(REFCTL0 & REFGENRDY));            // Wait for reference generator
                                                // to settle

}

void TimerA0setup()
{
      CSCTL0_H = CSKEY >> 8;                    // Unlock clock registers
      CSCTL1 = DCOFSEL_3 | DCORSEL;             // Set DCO to 8MHz
      CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
      CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
      CSCTL0_H = 0;                             // Lock CS registers
      TA0CCTL0 = CCIE;                             // CCR0 interrupt enabled
      TA0CCR0 = 6000;                             //overflow roughly once per second
      TA0CTL = TASSEL_1 + MC_1;                  // Aclk, up mode
}

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

void ConvertToLCD(int input)
{
      iones = (input%10);             //gets ones place value
      itens = ((input/10)%10);        //gets tens place value
      ihundreds = ((input/100)%10);   //gets hundreds place value
      ithousands = ((input/1000)%10); //gets thousands place value

      showChar(iones + '0', 4);         //the following four lines display values on LCD.
      showChar(itens + '0', 3);
      showChar(ihundreds + '0', 2);
      showChar(ithousands + '0', 1);
}
