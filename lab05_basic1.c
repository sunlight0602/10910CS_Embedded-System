#include <msp430.h> // Specific device
#include <intrinsics.h> // Intrinsic functions
#define LED1 BIT0
#define LED2 BIT6
#define BUTTON BIT3

volatile unsigned int temp = 0;
volatile unsigned int cycle_LED = 0;
volatile unsigned int mode = 0;

volatile float LPM3_time = 0.0;
volatile float LPM0_time = 0.0;
volatile float ADC_time = 0.0;
volatile float v = 0.0;
volatile float c = 0.0;

// LPM3 only enables ACLK
// LPM0 enables DCO, SMCLK, ACLK

void main(void){

    WDTCTL = WDTPW|WDTHOLD;
    P1DIR |= LED1+LED2;

    // Timers:
    // timer0 for temperature
    TA0CTL |= MC_1|ID_0|TASSEL_1|TACLR; //up mode, ACLK
    BCSCTL3 |= LFXT1S_2; //enable VLO as ACLK src, 12kHz
    TA0CCR0 = 9601;

    TA0CCTL1 = OUTMOD_3; //TA0CCR1 set/reset
    TA0CCR1 = 9600; //TA0CCR1 OUT1 on time, 0.8sec
    TA0CCTL0 = CCIE; // Enable interrupt, somehow this is intefering the timer

    // timer1 for LED flash
    TA1CTL = MC_1|ID_0|TASSEL_1|TACLR; //up mode, ACLK
    BCSCTL3 |= LFXT1S_2; //enable VLO as ACLK src, 12kHz
    TA1CCR0 = 5999;
    TA1CCTL0 = CCIE;

    ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;
    ADC10CTL1 = INCH_10 + SHS_1; //sample&hold sc: TimerA.OUT1
    ADC10AE0 |= 0x10;
    ADC10CTL0 |= ENC;// + ADC10SC;

    _BIS_SR(GIE); //Enter interrupt
    __bis_SR_register(LPM3_bits + GIE); // Enter LPM3
    __enable_interrupt(); //Enable interrupts (intrinsic)

    for(;;){}

}

#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void){
    temp = ADC10MEM;
    v = temp*1.5/1023;
    c = (v - 0.986) / 0.00355;
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR(){
    //P1OUT |= LED2;
    //cycle_ADC++;
    ADC10CTL0 |= ENC; // sample once

    if(temp>760){
        if(mode==0){
            P1OUT &= ~LED1&~LED2;

            mode = 1;
            cycle_LED = 0;
            TA0CCR0 = 4801;
            TA0CCR1 = 4800;
            _BIC_SR(LPM3_EXIT);
            __bis_SR_register(LPM0_bits + GIE); // Enter LPM3
        }
    }
    else{
        if(mode==1){
            P1OUT &= ~LED1&~LED2;

            mode = 0;
            cycle_LED = 0;
            TA0CCR0 = 9601;
            TA0CCR1 = 9600;
            _BIC_SR(LPM0_EXIT);
            __bis_SR_register(LPM3_bits + GIE); // Enter LPM3
        }
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
            LPM3_time = LPM3_time + 0.5;
        }
        else if((cycle_LED==1)||(cycle_LED==3)){
            P1OUT &= ~LED1&~LED2;
            TA1CCR0 = 5999;
            cycle_LED++;
            LPM3_time = LPM3_time + 0.5;
        }
        else if(cycle_LED==4){
            P1OUT ^= LED2;
            TA1CCR0 = 10799;
            cycle_LED++;
            LPM3_time = LPM3_time + 0.9;
        }
        else if(cycle_LED==5){
            P1OUT &= ~LED1&~LED2;
            TA1CCR0 = 5999;
            cycle_LED = 0;
            LPM3_time = LPM3_time + 0.5;
        }
    }
    else if(mode==1){
        if(cycle_LED==0){
            P1OUT |= LED1+LED2;
            TA1CCR0 = 3600; //4199;
            cycle_LED++;
            LPM0_time  = LPM0_time + 0.3;
        }
        else if(cycle_LED==1){
            P1OUT &= ~LED1&~LED2;
            TA1CCR0 = 2400; //7799;
            cycle_LED = 0;
            LPM0_time  = LPM0_time + 0.2;
        }
    }

    TA1CTL &= ~TAIFG;  // Clear overflow flag
    TA1R = 0;
}
