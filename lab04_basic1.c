#include <msp430.h> // Specific device
#include <intrinsics.h> // Intrinsic functions
#define LED1 BIT0
#define LED2 BIT6
#define BUTTON BIT3

void main(void){

    WDTCTL = WDTPW|WDTHOLD;
    P1DIR |= LED1+LED2;

    // Timers: timer0 for temperature; timer1 for LED flash
    TA0CTL |= MC_1|ID_3|TASSEL_2|TACLR; //up mode, SMCLK
    BCSCTL2 &= ~SELS; //Source of SMCLK to be DCO
    DCOCTL = CALDCO_1MHZ; //DCO frequency to 1MHz ?
    TA0CCR0 = 10001; //125000 = 1sec

    TA0CCTL1 = OUTMOD_3; //TA0CCR1 set/reset
    TA0CCR1 = 10000; //125000; //TA0CCR1 OUT1 on time
    TA0CCTL0 = CCIE; // Enable interrupt, somehow this is intefering the timer

    TA1CTL = MC_1|ID_0|TASSEL_1|TACLR; //up mode, ACLK
    BCSCTL3 |= LFXT1S_2; //enable VLO as ACLK src, 12kHz
    TA1CCR0 = 5999;
    TA1CCTL0 = CCIE;

    ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;
    ADC10CTL1 = INCH_10 + SHS_1; //sample&hold sc: TimerA.OUT1
    ADC10AE0 |= 0x10;
    //ADC10CTL0 |= ENC;// + ADC10SC;

    _BIS_SR(GIE); //Enter interrupt
    __enable_interrupt(); //Enable interrupts (intrinsic)

    for(;;){}

}

volatile unsigned int temp = 0;
volatile unsigned int cycle_LED = 0;
volatile unsigned int cycle_ADC = 0;
volatile unsigned int mode = 0;

#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void){
    temp = ADC10MEM;

    if(temp>760){
        if(mode==0){
            P1OUT &= ~LED1&~LED2;
            mode = 1;
            cycle_LED = 0;
        }
    }
    else{
        if(mode==1){
            P1OUT &= ~LED1&~LED2;
            mode = 0;
            cycle_LED = 0;
        }
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR(){
    //P1OUT |= LED2;
    cycle_ADC++;

    if(cycle_ADC==10){
        ADC10CTL0 |= ENC;
        cycle_ADC = 0;
    }

    TA0CTL |= TACLR;
    TA0CTL &= ~TAIFG;  // Clear overflow flag
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR(){
    //P1OUT |= LED2;
    if(mode==0){
        if((cycle_LED==0)||(cycle_LED==2)){
            P1OUT ^= LED1;
            TA1CCR0 = 5999;
            cycle_LED++;
        }
        else if((cycle_LED==1)||(cycle_LED==3)){
            P1OUT &= ~LED1&~LED2;
            TA1CCR0 = 5999;
            cycle_LED++;
        }
        else if(cycle_LED==4){
            P1OUT ^= LED2;
            TA1CCR0 = 10799;
            cycle_LED++;
        }
        else if(cycle_LED==5){
            P1OUT &= ~LED1&~LED2;
            TA1CCR0 = 5999;
            cycle_LED = 0;
        }
    }
    else if(mode==1){
        if(cycle_LED==0){
            P1OUT |= LED1+LED2;
            TA1CCR0 = 4199;
            cycle_LED++;
        }
        else if(cycle_LED==1){
            P1OUT &= ~LED1&~LED2;
            TA1CCR0 = 7799;
            cycle_LED = 0;
        }
    }

    TA1CTL &= ~TAIFG;  // Clear overflow flag
    TA1R = 0;
}
