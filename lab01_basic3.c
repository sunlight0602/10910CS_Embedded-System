#include <msp430.h> 
#define RED BIT0
#define GREEN BIT6
#define BUTTON BIT3

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    P1DIR |= RED+GREEN; // DIR is telling the pin whether is input or output, for both bit to output LED, both bit should be 1
    P1REN |= BUTTON;
    P1OUT |= BUTTON;

    P1DIR = RED;
    P1OUT ^= RED;
    volatile unsigned int i = 20000;
    do i--;
    while(i != 0);

    for(;;){

        P1OUT |= BUTTON;
        if((P1IN & BUTTON)==0){
            P1DIR |= RED+GREEN;
            P1OUT = RED+GREEN;
            i = 20000;
            do i--;
            while(i != 0);

            P1DIR = 0;
            P1OUT = 0;
            i = 10000;
            do i--;
            while( i!=0 );
        }
        else{
            P1DIR = RED;
            P1OUT ^= RED;
            i = 20000;
            do i--;
            while(i != 0);

            P1DIR = 0;
            P1OUT = 0;
            i = 10000;
            do i--;
            while( i!=0 );

            P1DIR = RED;
            P1OUT ^= RED;
            i = 20000;
            do i--;
            while(i != 0);

            P1DIR = 0;
            P1OUT = 0;
            i = 10000;
            do i--;
            while( i!=0 );

            P1DIR = GREEN;
            P1OUT ^= GREEN;
            i = 40000;
            do i--;
            while(i != 0);

            P1DIR = 0;
            P1OUT = 0;
            i = 10000;
            do i--;
            while( i!=0 );
        }
    }

    return 0;
}
