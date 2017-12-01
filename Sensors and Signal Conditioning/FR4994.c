// LAB 5 - ADC_UART_5994
// STEPHEN SZYMCZAK & MATT DELENGOWSKI & Glenn Dawson

#include <msp430.h>
  void GPIOSetup();
  void LEDSetup();
  void UARTSetup();
  void ADCSetup();
  void TimerA0setup();
int msHex = 0;
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
  TimerA0setup();

  __bis_SR_register(LPM0_bits + GIE);      // LPM0, General interrupt enabled.

}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
  switch (__even_in_range(ADC12IV, ADC12IV_ADC12RDYIFG))
  {
    case ADC12IV_NONE:        break;        // Vector  0:  No interrupt
    case ADC12IV_ADC12IFG0:                 // Vector 12:  ADC12MEM0 Interrupt
      msHex = ADC12MEM0;                    // acquire top 4 bits of mem0
      msHex = msHex >> 8;

      UCA0IE |= UCTXIE;
      UCA0TXBUF = msHex;                    //send top 4 bits of mem0
      UCA0TXBUF = ADC12MEM0;                //acquire lower 8 bits of mem0
      UCA0IE &= ~UCTXIE;                    //send lower 8 bits of mem0
      msHex=0;

       __bic_SR_register_on_exit(LPM0_bits); // Exit active CPU
      break;                                // Clear CPUOFF bit from 0(SR)

    default: break;
  }
}
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    ADC12CTL0 |= ADC12ENC | ADC12SC;             // Sampling and conversion start

    P1OUT ^= BIT1;  //confirms sampling is happening (not necessary)
        __bis_SR_register(LPM0_bits + GIE);      // LPM0, ADC10_ISR will force exit
        __no_operation();                        // For debug only
}

void GPIOSetup()
{
      //P1DIR &= ~BIT2;
      P4SEL1 |= BIT2;                           // Configure P4.2 for ADC
      P4SEL0 &= ~BIT2;

      P2SEL1 |= BIT0 | BIT1;                    // USCI_A0 UART operation
      P2SEL0 &= ~(BIT0 | BIT1);
}

void LEDSetup()
{
    P1DIR |= BIT0;                          // Set P1.0 LED to Output
    P1DIR |= BIT1;                          // Set P1.1 LED to Output

    P1OUT &= ~BIT0;                         // Disable P1.0 LED
    P1OUT &= ~BIT1;                         // Disable P1.1 LED
}

void UARTSetup()
{
    // Startup clock system with max DCO setting ~8MHz
  CSCTL0_H = CSKEY >> 8;                    // Unlock clock registers
  CSCTL1 = DCOFSEL_3 | DCORSEL;             // Set DCO to 8MHz
  CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
  CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
  CSCTL0_H = 0;                             // Lock CS registers

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
    // By default, REFMSTR=1 => REFCTL is used to configure the internal reference
      while(REFCTL0 & REFGENBUSY);              // If ref generator busy, WAIT
      REFCTL0 |= REFVSEL_2 | REFON;             // Select internal ref = 2.5V
                                                // Internal Reference ON

      // Configure ADC12
      ADC12CTL0 = ADC12SHT0_2 | ADC12ON;
      ADC12CTL1 = ADC12SHP;                     // ADCCLK = MODOSC; sampling timer
      ADC12CTL2 |= ADC12RES_2;                  // 12-bit conversion results
      ADC12IER0 |= ADC12IE0;                    // Enable ADC conv complete interrupt
      ADC12MCTL0 |= ADC12INCH_10 | ADC12VRSEL_1; // A4 ADC input select; Vref=2.5V

      while(!(REFCTL0 & REFGENRDY));            // Wait for reference generator
                                                // to settle
}

void TimerA0setup()
{
      TA0CCTL0 = CCIE;                             // CCR0 interrupt enabled
      TA0CCR0 = 11000;                             //overflow roughly once per second
      TA0CTL = TASSEL_1 + MC_1;                  // Aclk, up mode
}
