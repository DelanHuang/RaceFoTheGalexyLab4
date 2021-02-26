#include "pwm.h"
#include "msp.h"




/**
 * main.c
 */

void config_pwm_timer(void){
    TIMER_A0->CTL |= TIMER_A_CTL_IE; //Turns on interrupts for clock
    TIMER_A0->CTL |= TIMER_A_CTL_TASSEL_2; //Chooses SMCLK as input clk

    //SET DIVIDER VALUE
    TIMER_A0->CTL |= TIMER_A_CTL_ID__1 ; //DIVIDE BY MASK FOR ID = 1

    //SET CCR VALUE BASED TO TICKS
    TIMER_A0->CCR[0] = TICKS ;//Limit value for clock

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

void PORT1_IRQHandler(void){
    __NVIC_DisableIRQ(PORT1_IRQn); //disable since we're in the interrupt

    if(P1->IFG & BIT1){
        P1->IFG &= ~BIT1;
    }
    while((P1->IN & BIT1) == 0x00){ //RESETS timer while P1.1 is being held
        stop_pwm(); //stops/reset
        TIMER_A0->R = 0; //sets counter register to 0
    }
    //re-initialize timer stuff and begin
    config_pwm_timer();
    start_pwm();

    __NVIC_EnableIRQ(PORT1_IRQn); //enable interrupt since we are about to exit handler
}

//void main(void)
//{
//    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer
//
//    stop_pwm();
//    config_switch_interrupt();
//    config_pwm_timer();
//    config_pwm_gpio();
//    start_pwm();
//    while(1);
//}




