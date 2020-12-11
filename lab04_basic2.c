#include <intrinsics.h>
#include <msp430.h>
#define LED1 BIT0
#define LED2 BIT6
#define BUTTON BIT3

int adc[10];
int c_adc[10];
volatile unsigned int k = 0;
volatile unsigned int pressed = 0;
volatile unsigned int average = 0;
volatile unsigned int c_average = 0;
volatile unsigned int recorder = 0;
volatile unsigned int count_0 = 0, count_1 = 0, count_2 = 0;
volatile unsigned int mode = 0; //LED pattern: 0,1,2
volatile unsigned int status = 0; //status: 0,1
volatile unsigned int temp = 0; //temp: 0,1

int main(void){

    WDTCTL = WDTPW | WDTHOLD;
    P1DIR |= LED1+LED2;
    P1OUT |= BUTTON;

    P1REN |= BUTTON;

    // Timers
    TA1CTL = MC_1|ID_0|TASSEL_1|TACLR; // up mode, /4, ACLK, clear
    BCSCTL3 |= LFXT1S_2; // Enable VLO as MCLK/ACLK src, 12kHz
    TA1CCR0 = 5999; //299;
    TA1CCTL0 = CCIE;

    TA0CTL = MC_1|ID_0|TASSEL_1|TACLR; // up mode, /4, ACLK, clear
    TA0CCR0 = 1501;
    TA0CCTL1 = OUTMOD_3;
    TA0CCR1 = 1500; // 0.25 sec
    TA0CCTL0 = CCIE;

    // ADC
    ADC10CTL1 = SHS_1 + CONSEQ_2 + INCH_10;
    ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;
    //ADC10AE0 |= 0x02; //P1.1 ADC10 option select
    ADC10DTC1 = 4;
    ADC10SA = (int)adc;
    //ADC10CTL0 |= ENC + ADC10SC; //ADC10 Enable

    // Button
    P1IE |= BUTTON;
    P1IES |= BUTTON;
    P1IFG &= ~BUTTON;
    _BIS_SR(GIE);

    __enable_interrupt();
    for (;;) { }
}

#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void) {
    P1IES ^= BUTTON;

    pressed = !pressed;

    P1IFG &= ~BUTTON;
}

#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void){
    volatile int total = 0;

    ADC10SA = (int)adc;
    for(k=0; k<4; k++){
        total += adc[k];
        c_adc[k] = ((adc[k]*1.5/1023)-0.986)/0.00355;
    }

    average = total/4;
    c_average = ((average*1.5/1023)-0.986)/0.00355;

    if(average < 750){
        temp = 0;
    }
    else{
        temp = 1;
    }
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR (void){

    if(mode==0){ // pattern_1
        count_1 = 0;
        count_2 = 0;
        if((count_0==0)||(count_0==2)){
            TA1CCR0 = 5999;
            P1OUT &= ~LED1&~LED2;
            P1OUT |= LED1;
            count_0++;
        }
        else if((count_0==1)||(count_0==3)){
            P1OUT &= ~LED1&~LED2;
            count_0++;
        }
        else if(count_0==4){
            TA1CCR0 |= 10799;
            P1OUT |= LED2;
            count_0++;
        }
        else if(count_0==5){
            TA1CCR0 = 5999;
            P1OUT &= ~LED1&~LED2;
            count_0 = 0;
        }
    }
    else if(mode==1){ // pattern_2
        count_0 = 0;
        count_2 = 0;
        if(count_1==0){
            TA1CCR0 = 5999;
            P1OUT &= ~LED1&~LED2;
            P1OUT |= LED1+LED2;
            count_1++;
        }
        else if(count_1==1){
            P1OUT &= ~LED1&~LED2;
            count_1 = 0;
        }
    }
    else if(mode==2){ // pattern_3
        count_0 = 0;
        count_1 = 0;
        if(count_2==0){
            TA1CCR0 = 2400; // 0.2sec
            P1OUT &= ~LED1&~LED2;
            P1OUT |= LED1;
            count_2++;
        }
        else if(count_2==1){
            P1OUT &= ~LED1&~LED2;
            count_2 = 0;
        }
    }
    TA1CTL &= ~TAIFG;
    TA1CTL |= TACLR; //TA1R = 0; //????
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void) {

    if(status==0){ // Normal
        mode = 0;
        ADC10CTL0 &= ~ENC; // disable sampling
        ADC10CTL0 &= ~ADC10SC;

        if(pressed==1){
            recorder++;
            if(recorder>=40){ // 12000/1500*5
                status = 1;
            }
        }
        else{
            recorder = 0;
        }
    }
    else if(status==1){ // Measure
        ADC10CTL0 |= ENC + ADC10SC; // enable sampling

        if(temp==0){
            mode = 1; // LED pattern_2
        }
        else if(temp==1){
            mode = 2; // LED pattern_3
        }

        if(pressed==0){
            status = 0; // to Normal
        }
    }

    TA0CTL &= ~TAIFG;  // Clear overflow flag
    TA0CTL |= TACLR;
}
