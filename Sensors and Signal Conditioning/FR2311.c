//Lab 5: Sensors and Signal Conditioning
//Authors: Stefan Symczak & Matt Delengowski & Glenn Dawson
//Device: MSP430FR2311


#include <msp430.h>

  void GPIOSetup();
  void LEDSetup();
  void UARTSetup();
  void ADCSetup();
  void TimerB0setup();

  volatile unsigned int msHex;
  volatile unsigned int lsHex;

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT
  // Disable the GPIO power-on default high-impedance mode to activate
  // previously configured port settings
  PM5CTL0 &= ~LOCKLPM5;

  GPIOSetup();
  LEDSetup();
  UARTSetup();
  ADCSetup();
  TimerB0setup();


  __bis_SR_register(LPM0_bits + GIE);      // LPM0, General interrupt enabled.

}

#pragma vector = ADC_VECTOR
__interrupt void ADC_ISR(void)
{
  switch (__even_in_range(ADCIV,ADCIV_ADCIFG))
  {
    case ADCIV_NONE:       // Vector  0:  No interrupt
        break;
    case ADCIV_ADCOVIFG:
        break;
    case ADCIV_ADCTOVIFG:
        break;
    case ADCIV_ADCHIIFG:
        break;
    case ADCIV_ADCLOIFG:
        break;
    case ADCIV_ADCINIFG:
        break;
    case ADCIV_ADCIFG:                 // Vector 12:  ADCMEM0 Interrupt
      UCA0IE |= UCTXIE;                     // Enable TX Interrupts
      msHex = ADCMEM0;
      lsHex = ADCMEM0;
      msHex = msHex >> 8;
      UCA0TXBUF = msHex;                    //Send MSB of ADCMEM0
      UCA0TXBUF = lsHex;              // Send Remaining bytes of ADCMEM0
      UCA0IE &= ~UCTXIE;              // Disable TX Interrupts

      /*
      if (ADCMEM0 >= 0x8DD)               // ADCMEM = A10 > 0.5V?
        P1OUT |= BIT0;                      // P1.0 = 1
      else if(ADCMEM0 <= 0x555)
        P1OUT |= BIT0;
      else
        P1OUT &= ~BIT0;                     // P1.0 = 0
        */
        __bic_SR_register_on_exit(LPM0_bits); // Exit active CPU
      break;                                // Clear CPUOFF bit from 0(SR)

    default: break;
  }
}

#pragma vector=TIMER0_B0_VECTOR
__interrupt void Timer_B (void)
{
    ADCCTL0 |= ADCENC | ADCSC;         // Sampling and conversion start

    P2OUT ^= BIT0;
        __bis_SR_register(LPM0_bits + GIE);      // LPM0, ADC10_ISR will force exit
        __no_operation();                        // For debug only
}

void GPIOSetup()
{
      P1DIR &= ~BIT1;
      P1SEL1 |= BIT1;                           // Configure P1.1 for ADC (A1)
      P1SEL0 |= BIT1;

      P1SEL0 |= BIT6 | BIT7;                    // Primary Model Select TX and RX
      P1SEL1 &= ~(BIT6 | BIT7);
}

void LEDSetup()
{
    P1DIR |= BIT0;                          // Set P1.0 LED to Output
    P2DIR |= BIT0;                          // Set P2.0 LED to Output

    P1OUT &= ~BIT0;                         // Disable P1.0 LED
    P2OUT &= ~BIT0;                         // Disable P2.0 LED
}

void UARTSetup()
{

  //FLL = frequency locked loop
  __bis_SR_register(SCG0);                  // Disables FLL, no bit modulation
  CSCTL3 |= SELREF_1;                 // FLL references REFO...lock clock system to 32khz(REF0)
  CSCTL1 = DCOFTRIMEN_1 | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_3; //Enable Frequency Trim, Trim Bits 1 & 0, DC0 gets 8 Mhz Range
  CSCTL2 = FLLD_0 + 243;                    // Divide DC0 by 1 and add 243
  __delay_cycles(3);
  __bis_SR_register(SCG0);                  // Enable FLL for bit modulation

  // Configure USCI_A1 for UART mode
  UCA0CTLW0 = UCSWRST;                      // Put eUSCI in reset
  UCA0CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK
  // Baud Rate calculation
  // 8000000/(16*9600) = 52.083
  // Fractional portion = 0.083
  // User's Guide Table 21-4: UCBRSx = 0x04
  // UCBRFx = int ( (52.083-52)*16) = 1
  UCA0BR0 = 52;                             // 8000000/16/9600
  UCA0BR1 = 0x00;
  UCA0MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
  UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
  UCA0IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt

  FRCTL0 = FWPW | NWAITS_0;                // Removes Warning #10421-D, FWPW is FRAM password, NWAITS_0 adds 0 wait states to FRAM, changes nothing, simply removes warning
}

void ADCSetup()
{

      // Configure ADC
      ADCCTL0 |= ADCSHT_2 | ADCON;           //16 ADC Clk Cylces & Turn ADC On
      ADCCTL1 |= ADCSHP;                     // ADCCLK = MODOSC; sampling timer
      ADCCTL2 |= ADCRES;                  // 10-bit conversion results
      ADCIE |= ADCIE0;                    // Enable ADC conv complete interrupt
      ADCMCTL0 |= ADCINCH_1; // A1 ADC input select; Vref=AVCC

}

void TimerB0setup()
{
      TB0CCTL0 = CCIE;                             // CCR0 interrupt enabled
      TB0CCR0 = 11000;                             //overflow roughly once per second
      TB0CTL = TBSSEL_1 + MC_1;                  // Aclk, up mode
}
