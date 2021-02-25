#include "pwm.h"
#include "msp.h"

/*
   Math for dis
   - 3MHz clock == 3*10^6 ticks/sec
   - high == tick count when signal is returned to pin 2.5
   - low == tick count when signal is sent from pin 2.4
   - to get time from ticks we do ticks/clock == ticks/(3*10^6)
   - equations are given in microseconds so we convert seconds to microseconds, (ticks/(3*10^6))*10^6 == ticks/3
 */


void dis(float high, float low){
    //ECHO TO DISTANCE
   Tdiff = low - high; //difference in ticks
   time = Tdif/3  //convert ticks to microseconds
   dis_cm = time/58; //distance in centimeters
   dis_in = time/148; //distance in inches
}

void config_pwm_timer(void){
    TIMER_A0->CTL |= TIMER_A_CTL_IE; //Turns on interrupts for clock
    TIMER_A0->CTL |= TIMER_A_CTL_TASSEL_2; //Chooses SMCLK as input clk

    //SET DIVIDER VALUE
    TIMER_A0->CTL |= TIMER_A_CTL_ID__4 ; //DIVIDE BY MASK FOR ID = 4

    //SET CCR VALUE BASED TO TICKS
    TIMER_A0->CCR[0] = 0x001E; //10us signal
    TIMER_A0->CCR[1] = ; //60ms signal

}

void start_pwm(void){
    TIMER_A0->CCTL[1] |= TIMER_A_CCTLN_OUTMOD_7; //RESETS mode
    TIMER_A0->CCTL[1] &= TIMER_A_CCTLN_OUTMOD_4; //SETS OUTPUT MODE TO TOGGLE
    TIMER_A0->CTL |= TIMER_A_CTL_MC_1; //Sets count mode to "UP"
}

void stop_pwm(void){
    TIMER_A0->CTL &= TIMER_A_CTL_MC__STOP;
}

void config_pwm_gpio(void){
    P2->DIR |= BIT4; //sets p2.4 as output 000010000
    P2->OUT &= ~BIT4; // sets bit4 to 0

    P2->SEL0 |= BIT4; //0000100000
    P2->SEL1 &= ~BIT4;// 111101111

}

void config_switch_interrupt(void){
    P1->DIR &= ~BIT1; //sets p1.1 (push button) as input
    P1->REN |= BIT1; //sets up pullup resistor config
    P1->OUT |= BIT1; //Initially sets to 1 since it will be active low
    P1->IE |= BIT1; //Enables interrupt functionality for P1.1

    __NVIC_EnableIRQ(PORT1_IRQn);

}
