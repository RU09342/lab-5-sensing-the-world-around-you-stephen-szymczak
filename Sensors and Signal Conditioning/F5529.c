//Lab 5: Sensors and Signal Conditioning
//Authors: Stefan Symczak & Matt Delengowski & Glenn Dawson
//Device: MSP430F5529


#include <msp430.h>

  void GPIOSetup();
  void LEDSetup();
  void UARTSetup();
  void ADCSetup();
  void TimerA0setup();

  volatile unsigned int msHex;
  volatile unsigned int lsHex;

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT


  GPIOSetup();
  LEDSetup();
  UARTSetup();
  ADCSetup();
  TimerA0setup();


  __bis_SR_register(GIE);      // LPM0, General interrupt enabled.

  while(1)
  {

  };

}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
  switch (__even_in_range(ADC12IV,34))
  {
    case ADC12IV_NONE:        break;        // Vector  0:  No interrupt
    case ADC12IV_ADC12IFG0:                 // Vector 12:  ADC12MEM0 Interrupt
      UCA1IE |= UCTXIE;                     // Enable TX Interrupts
      msHex = ADC12MEM0;
      lsHex = ADC12MEM0;
      msHex = msHex >> 8;
      UCA1TXBUF = lsHex;              // Send Remaining bytes of ADC12MEM0
      UCA1IE &= ~UCTXIE;              // Disable TX Interrupts
      UCA1TXBUF = msHex;                    //Send MSB of ADC12MEM0
      /*
      if (ADC12MEM0 >= 0x8DD)               // ADC12MEM = A10 > 0.5V?
        P1OUT |= BIT0;                      // P1.0 = 1
      else if(ADC12MEM0 <= 0x555)
        P1OUT |= BIT0;
      else
        P1OUT &= ~BIT0;                     // P1.0 = 0
        */
      break;                                // Clear CPUOFF bit from 0(SR)

    default: break;
  }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    ADC12CTL0 |= ADC12ENC | ADC12SC;         // Sampling and conversion start

    P4OUT ^= BIT7;
        __bis_SR_register(GIE);      // LPM0, ADC10_ISR will force exit
        __no_operation();                        // For debug only
}

void GPIOSetup()
{
      P6DIR &= ~BIT0;
      P6SEL |= BIT0;                           // Configure P6.0 for ADC

      P4SEL |= BIT4 | BIT5;                    // USCI_A1 UART operation P4.4 - TX P4.5 - RX
}

void LEDSetup()
{
    P1DIR |= BIT0;                          // Set P1.0 LED to Output
    P4DIR |= BIT7;                          // Set P4.7 LED to Output

    P1OUT &= ~BIT0;                         // Disable P1.0 LED
    P4OUT &= ~BIT7;                         // Disable P4.7 LED
}

void UARTSetup()
{

  // Configure USCI_A1 for UART mode
  UCA1CTLW0 = UCSWRST;                      // Put eUSCI in reset
  UCA1CTL1 |= UCSSEL__SMCLK;               // CLK = SMCLK
  // Baud Rate calculation
  // 8000000/(16*9600) = 52.083
  // Fractional portion = 0.083
  // User's Guide Table 21-4: UCBRSx = 0x04
  // UCBRFx = int ( (52.083-52)*16) = 1
  UCA1BR0 = 52;                             // 8000000/16/9600
  UCA1BR1 = 0x00;
  UCA1MCTL |= UCOS16 | UCBRF_1;
  UCA1CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
  UCA1IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt

}

void ADCSetup()
{

      // Configure ADC12
      ADC12CTL0 = ADC12SHT0_2 | ADC12ON | ADC12REFON | ADC12REF2_5V;
      ADC12CTL1 = ADC12SHP;                     // ADCCLK = MODOSC; sampling timer
      ADC12IE  = ADC12IE0;                    // Enable ADC conv complete interrupt
      ADC12MCTL0 |= ADC12SREF_0 | ADC12INCH_0; // A0 ADC input select, Vr+=Vref+ and Vr- = AVss

}

void TimerA0setup()
{
      TA0CCTL0 = CCIE;                             // CCR0 interrupt enabled
      TA0CCR0 = 11000;                             //overflow roughly once per second
      TA0CTL = TASSEL_2 + MC_2 + ID_3;                  // Aclk, up mode
}
