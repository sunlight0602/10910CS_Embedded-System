#include <msp430.h>
#define LED1 BIT0
#define LED2 BIT6
#define BUTTON BIT3

void main(){
    WDTCTL = WDTPW|WDTHOLD;

    P1DIR |= LED1+LED2;
    P1REN |= BUTTON;

    P1OUT = 0;
    P1OUT |= BUTTON;

    TA0CTL |= MC_1|ID_0|TASSEL_1|TACLR; //Setup Timer0_A: up mode, divide by 2^8, use ACLK, clear timer

    for(;;){

        if((P1IN & BUTTON)==0){
            // Both on
            TA0CCR0 = 5999;
            P1OUT |= LED1+LED2;
            while( !(TA0CTL & TAIFG) ){ // wait for time up

            }
            TA0CTL &= ~TAIFG; // clear overflow flag

            //All off
            P1OUT &= ~LED1;
            P1OUT &= ~LED2;
            while( !(TA0CTL & TAIFG) ){ // wait for time up

            }
            TA0CTL &= ~TAIFG; // clear overflow flag
        }
        else{
            //Red on
            TA0CCR0 = 5999;
            P1OUT ^= LED1; //toggle LED
            while( !(TA0CTL & TAIFG) ){ // wait for time up

            }
            TA0CTL &= ~TAIFG; // clear overflow flag
            //P1OUT ^= LED1; //toggle LED

            //All off
            P1OUT &= ~LED1;
            P1OUT &= ~LED2;
            while( !(TA0CTL & TAIFG) ){ // wait for time up

            }
            TA0CTL &= ~TAIFG; // clear overflow flag

            //Red on
            P1OUT ^= LED1; //toggle LED
            while( !(TA0CTL & TAIFG) ){ // wait for time up

            }
            TA0CTL &= ~TAIFG; // clear overflow flag

            //All off
            P1OUT &= ~LED1;
            P1OUT &= ~LED2;
            while( !(TA0CTL & TAIFG) ){ // wait for time up

            }
            TA0CTL &= ~TAIFG; // clear overflow flag

            //Green
            TA0CCR0 |= 10799;
            P1OUT ^= LED2; //toggle "Green" LED
            while( !(TA0CTL & TAIFG) ){ // wait for time up

            }
            TA0CTL &= ~TAIFG; // clear overflow flag

            //All off
            TA0CCR0 |= 5999;
            P1OUT &= ~LED1;
            P1OUT &= ~LED2;
            while( !(TA0CTL & TAIFG) ){ // wait for time up

            }
            TA0CTL &= ~TAIFG; // clear overflow flag

            P1OUT = 0;
            P1OUT |= BUTTON;
        }

    }
}
