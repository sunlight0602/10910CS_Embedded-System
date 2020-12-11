/*
Bit mask:
set a bit: P1OUT = P1OUT | BIT3
clear a bit: P1OUT &= ~BIT3
toggle a bit: P1OUT ˆ= BIT3
*/

#include <msp430.h> 

#define LED1 BIT0
#define LED2 BIT6
#define BUTTON BIT3

int main(void)
{
    volatile unsigned int reset_timer0A = 1; // true

    volatile unsigned int mode = 0; // mode 0:RRG, 1:both on/off
    volatile unsigned int pressed = 0; // button pressed 0:up, 1:down
    volatile unsigned int cycle_LED = 0; // LED cycle, 0:1st_red_on/both_on, 1:off, 2:2nd_red_on, 3:off, 4:green_on, 5:off
    volatile unsigned int cycle_BUT = 0; // BUTTON cycle

    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

    P1DIR |= LED1|LED2;
    P1DIR &= ~BUTTON;
    P1REN = BUTTON;

    P1OUT |= BUTTON;

    // Timer
    TA0CTL = MC_1|ID_0|TASSEL_1|TACLR; //Setup Timer0A: up mode, divide by 1, use ACLK, clear timer
    TA1CTL = MC_3|ID_3|TASSEL_2|TACLR; //Setup Timer1A: up/down mode, divide by 8, use SMCLK, clear timer
    BCSCTL2 &= ~SELS; // Source of SMCLK to be DCO
    DCOCTL |= CALDCO_1MHZ; // DCO frequency to 1MHz
    BCSCTL3 |= LFXT1S_2; // Enable VLO as MCLK/ACLK source
    // Timer1A: 12500 counts / sec, 250000 counts / 2sec

    /* After starting timer1A, timer0A will run simultaneously until timer1A's TAIFG is raised to 1
     * (TA1R finishes its cycle in up/down mode), then state will change to mode 1 (both on/off)*/
    for(;;){
        if(mode == 0){ // RRG
            if(reset_timer0A == 1){
                reset_timer0A = 0;

                if(cycle_LED==0 || cycle_LED==2){ // Red on
                    P1OUT |= LED1;
                    P1OUT &= ~LED2;
                    TA0CCR0 = 5999;
                }
                else if(cycle_LED == 4){ // Green on
                    P1OUT |= LED2;
                    P1OUT &= ~LED1;
                    TA0CCR0 = 10900;
                }
                else{ // All off
                    P1OUT &= ~LED1&~LED2;
                    TA0CCR0 = 5999;
                }
                //TA0CTL |= TACLR; // Reset TA0R to 0 manually, not necessary for TA0CTL already declared TACLR

                cycle_LED = (cycle_LED+1) % 6;
            }

            if( (P1IN & BUTTON)==0 ){ // if button down
                if(pressed == 0){ // if button is not pressed, set to pressed
                    pressed = 1;
                    cycle_BUT = 0;
                    TA1CCR0 = 24999; // Set timer1A range (start timer)
                    //TA1CTL |= TACLR;
                }
                else if( (TA1CTL & TAIFG)==1 ){ // if TAIFG is raised to 1 (TA1R reach TA1CCR0 * 2) (up/down mode)
                    TA1CTL &= ~TAIFG; // pull TAIFG back to 0 manually
                    cycle_BUT += 1;
                    if(cycle_BUT == 5){
                        pressed = 0;
                        reset_timer0A = 1;
                        cycle_LED = 0;

                        mode = 1;
                    }
                }
            }
            else{ // if button up
                pressed = 0;
                cycle_BUT = 0;
                //TA1CTL |= TACLR;
            }
        }
        else if(mode == 1){ // both on, off
            if(reset_timer0A == 1){
                reset_timer0A = 0;
                if(cycle_LED == 0){ // both on
                    P1OUT |= LED1|LED2;
                }
                else if(cycle_LED == 1){ // both off
                    P1OUT &= ~LED1&~LED2;
                }
                TA0CCR0 = 5999; // Set timer0A range
                //TA0CTL |= TACLR;

                cycle_LED = (cycle_LED + 1) % 2;
            }

            if(P1IN & BUTTON){ // if button up
                reset_timer0A = 1;
                cycle_LED = 0; // to 1st_red_on
                mode = 0;
            }
        }

        if( (TA0CTL & TAIFG)==1 ){
            TA0CTL &= ~TAIFG; // pull down flag (TAIFG)
            TA0CTL |= TACLR; // reset counter to 0 (TA0R)
            reset_timer0A = 1;
        }
    }
    return 0;
}
