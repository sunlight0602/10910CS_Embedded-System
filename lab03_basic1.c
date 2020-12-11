#include <msp430.h>
#include <intrinsics.h>

#define LED1 BIT0
#define LED2 BIT6
#define BUTTON BIT3

int state = 0;
int pressed = 0;

void main(){
    WDTCTL = WDTPW|WDTHOLD;

    P1OUT = LED1+BUTTON;
    P1DIR |= LED1+LED2;
    P1REN |= BUTTON;

    P1IE |= BUTTON;
    P1IES |= BUTTON;
    P1IFG &= ~BUTTON;
    _BIS_SR(GIE);

    TA0CTL |= MC_1|ID_0|TASSEL_1|TACLR; //Setup Timer0_A: up mode, divide by 2^8, use ACLK, clear timer
    TA0CCR0 = 5999;

    TA0CCTL0 = CCIE; // Enable interrupt
    __enable_interrupt();

    for(;;){
    }
}

#pragma vector = TIMER0_A0_VECTOR // for TA0CCR0
__interrupt void Timer_A(){ // 0 1 2 3 4 5
    if(state==0){
        P1OUT ^= LED1;
        state++;
    }
    else if(state==1){
        P1OUT &= ~LED1&~LED2;
        state++;
    }
    else if(state==2){
        P1OUT ^= LED1;
        state++;
    }
    else if(state==3){
        P1OUT &= ~LED1&~LED2;
        state++;
    }
    else if(state==4){
        TA0CCR0 = 10799;
        P1OUT ^= LED2;
        state++;
    }
    else if(state==5){
        TA0CCR0 = 5999;
        P1OUT &= ~LED1&~LED2;
        state = 0;
    }
    else if(state==6){
        TA0CCR0 = 5999;
        P1OUT |= LED1+LED2;
        state++;
    }
    else if(state==7){
        TA0CCR0 = 5999;
        P1OUT &= ~LED1&~LED2;
        state = 6;
    }
}

#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void){
    P1IES ^= BUTTON;

    if(pressed==0){
        pressed = 1;
        state = 6;
    }
    else if(pressed==1){
        pressed = 0;
        state = 0;
    }

    P1IFG &= ~BUTTON;
}
