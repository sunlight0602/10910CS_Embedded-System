#include msp430.h  Specific device
#include intrinsics.h  Intrinsic functions
#define LED1 BIT0
#define LED2 BIT6
#define BUTTON BIT3

volatile unsigned int cycle_LED = 0;
volatile unsigned int pressed = 0;
volatile unsigned int cycle_BUT = 0;

volatile unsigned int start_timerA1 = 0;
volatile unsigned int reach_3sec = 0;
volatile unsigned int start_mode2 = 0;
volatile unsigned int cycle_counter = 0;

void main(void)
{
      WDTCTL = WDTPWWDTHOLD;
      P1OUT = BUTTON;
      P1DIR = LED1+LED2;
      P1REN = BUTTON; P1.4 pullup

       Timers
      TA0CTL = MC_1ID_0TASSEL_1TACLR; Setup Timer0A up mode, divide by 1, use ACLK, clear timer
      BCSCTL3 = LFXT1S_2; enable VLO as ACLK src
      TA0CCR0 = 5999;  Upper limit of count for TA0R
       TimerA0 12kHz

      TA1CTL = MC_1ID_3TASSEL_2TACLR; Setup Timer1A up mode, divide by 8, use SMCLK, clear timer
      BCSCTL2 &= ~SELS;  Source of SMCLK to be DCO
      DCOCTL = CALDCO_1MHZ;  DCO frequency to 1MHz
       TimerA1 1M8 Hz = 125000 Hz

      P1IE = BUTTON;interrupt enabled
      P1IES = BUTTON; P1.4 Hilo edge
      P1IFG &= ~BUTTON; P1.4 IFG cleared
      _BIS_SR(GIE); Enter interrupt

      TA0CCTL0 = CCIE;  Enable interrupts
      TA1CCTL0 = CCIE;  Enable interrupts
      __enable_interrupt();  Enable interrupts (intrinsic)

      for (;;) { }  Loop forever doing nothing
}

 Interrupt service routine for CCR0 of Timer0_A3
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR (void){
      if((cycle_LED==0)(cycle_LED==2)){
          P1OUT ^= LED1;
          TA0CCR0 = 5999;
          cycle_LED++;
      }
      else if((cycle_LED==1)(cycle_LED==3)){
          P1OUT &= ~LED1&~LED2;
          TA0CCR0 = 5999;
          cycle_LED++;
      }
      else if(cycle_LED==4){
          P1OUT ^= LED2;
          TA0CCR0 = 10799;
          cycle_LED++;
      }
      else if(cycle_LED==5){
          P1OUT &= ~LED1&~LED2;
          TA0CCR0 = 5999;
          cycle_LED = 0;
      }
      else if(cycle_LED==6){  Both on
          P1OUT = LED1+LED2;
          TA0CCR0 = 5999;
          cycle_LED++;
      }
      else if(cycle_LED==7){
          P1OUT &= ~LED1&~LED2;
          TA0CCR0 = 5999;
          cycle_LED = 6;
      }
}
#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR (void){
    if(start_timerA1==1){
        if((TA1CTL& TAIFG)==1){
            TA1CTL&=~TAIFG;
            TA1CTL = TACLR;

            cycle_BUT = cycle_BUT+1;
            if(cycle_BUT=30){
                reach_3sec = 1;
            }
        }
    }
    else{
        if(reach_3sec==1){
            if((TA1CTL& TAIFG)==1){
                TA1CTL&=~TAIFG;
                TA1CTL = TACLR;

                cycle_counter = cycle_counter+1;
                if(cycle_counter==cycle_BUT){
                    P1OUT &= ~LED1&~LED2;
                    cycle_LED = 0;

                    cycle_BUT = 0;  Reset
                    reach_3sec = 0;  Reset
                    cycle_counter = 0; Reset, not necessary
                }
            }
        }
        else{
            cycle_BUT = 0;  Reset
        }
    }
}

#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void){
    P1IES ^= BUTTON;
    if(pressed==0){
        pressed = 1;
        start_timerA1 = 1;

        TA1CCR0 = 12499; 124999 Initialize
        cycle_BUT = 0; Initialize
    }
    else if(pressed==1){
        pressed = 0;
        start_timerA1 = 0;

        if(reach_3sec==1){
            cycle_counter = 0; Initialize

            P1OUT &= ~LED1&~LED2;
            cycle_LED = 6;
            TA1CTL = TACLR;
        }
        else{
            cycle_BUT = 0;  Reset, not necessary
        }
    }
    P1IFG &= ~BUTTON;
}
