// Sensors and Signal Conditioning G2553
// Authors: Stephen Szymczak & Matt Delengowski & Glenn Dawson

#include <msp430.h>
  void GPIOSetup();
  void LEDSetup();
  void UARTSetup();
  void ADCSetup();
  void TimerA0setup();
int msHex,lsHex = 0;
int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT


  GPIOSetup();
  LEDSetup();
  UARTSetup();
  ADCSetup();
  TimerA0setup();
  UCA0TXBUF = 0xFF;
  __bis_SR_register(GIE);
  while(1){
  }

}

#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
      msHex = ADC10MEM;                    // get adc12mem0 value
      lsHex = msHex;
      msHex = msHex >> 8;                   // shift to only hold top 4 bits
      while (!(IFG2&UCA0TXIFG));
      IE2 |= UCA0TXIE;                          // Enable USCI_A0 TX interrupt
      UCA0TXBUF = msHex;                    //send top 4 bits of mem0

      while (!(IFG2&UCA0TXIFG));
      IE2 |= UCA0TXIE;                          // Enable USCI_A0 TX interrupt
      UCA0TXBUF = lsHex;                 //send bottom 4 bits of mem0

      IE2 &= ~UCA0TXIE;
      msHex=0;

}
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{

        //__delay_cycles(420);                // Wait for ADC Ref to settle
        ADC10CTL0 |= ENC + ADC10SC;            // Sampling and conversion start

        __bis_SR_register(GIE);      // LPM0, ADC10_ISR will force exit
        __no_operation();                        // For debug only
}

void GPIOSetup()
{
    P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2 ;                    // P1.1 = RXD, P1.2=TXD
}

void LEDSetup()
{
    P1DIR |= BIT6;                          // Set P1.0 LED to Output
    P1OUT &= ~BIT6;                         // Disable P1.0 LED
}

void UARTSetup()
{
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

void ADCSetup()
{
      // Configure ADC12
      ADC10CTL0 = ADC10SHT_2 + REF2_5V + ADC10ON + ADC10IE + REFON; // ADC10ON, interrupt enabled
      ADC10CTL1 = INCH_0;                       // input A0
      ADC10AE0 |= 0x01;                         // PA.0 ADC option select
}

void TimerA0setup()
{
      TA0CCTL0 = CCIE;                             // CCR0 interrupt enabled
      TA0CCR0 = 50000;                             //overflow roughly once per second
      TA0CTL = TASSEL_2 + MC_2 + ID_3;                  // Aclk, up mode
}
