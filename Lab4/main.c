#include "msp.h"
#include "pwm.h"

int d1 = 0, d2 = 0, count = 0;
double dist;

/**
 * main.c
 */


void main(void) {

	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	gpio_config();
	TimerA0_config();
	TimerA1_config();
	config_NVIC();

	TimerA2_config();
	TIMER_A2->CTL |= TIMER_A_CTL_MC__UP; //start TIMER_A2

	while(1) {}
}


void TimerA0_config() {
    TIMER_A0->CTL |= TIMER_A_CTL_CLR;   		//clear TIMER_A0
    TIMER_A0->CTL |= TIMER_A_CTL_SSEL__SMCLK; //SMCLK is source

    TIMER_A0->CCTL[2] |= TIMER_A_CCTLN_CAP;	//Capture mode
    //TIMER_A0->CCTL[2] |= TIMER_A_CCTLN_OUTMOD_7; //because why not?
    //TIMER_A0->CCTL[2] &= TIMER_A_CCTLN_OUTMOD_4; //set output toggle/reset
    TIMER_A0->CCTL[2] |= TIMER_A_CCTLN_CCIE; //enable interrupt
}

void TimerA1_config() {
	TIMER_A1->CTL |= TIMER_A_CTL_CLR;			//clear TIMER_A1
    TIMER_A1->CTL |= TIMER_A_CTL_SSEL__SMCLK; 	//SMCLK is source

    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CAP;	//Compare mode
    TIMER_A1->CCR[0] = 0x1E;             		//TICKS = 30 = 10UM
    TIMER_A1->CCTL[0] |= TIMER_A_CCTLN_OUTMOD_7; //because why not?
    TIMER_A1->CCTL[0] &= TIMER_A_CCTLN_OUTMOD_0; //set output mode
    TIMER_A1->CCTL[0] |= TIMER_A_CCTLN_CCIE; 	//enable interrupt
}

void TimerA2_config() {
	TIMER_A2->CTL |= TIMER_A_CTL_CLR;			//clear TIMER_A1
    TIMER_A2->CTL |= TIMER_A_CTL_SSEL__SMCLK; 	//SMCLK is source

    TIMER_A2->CCTL[0] &= ~TIMER_A_CCTLN_CAP;	//Compare mode
    TIMER_A2->CCR[0] = 0xAFC8;             		//TICKS = 45000 = 60ms
    TIMER_A1->CCTL[0] |= TIMER_A_CCTLN_OUTMOD_7; //because why not?
    TIMER_A1->CCTL[0] &= TIMER_A_CCTLN_OUTMOD_0; //set output mode
    TIMER_A2->CCTL[0] |= TIMER_A_CCTLN_CCIE; 	//enable interrupt

    /* set 2.4 HIGH, turn on TimerA1, TimerA1 turns P2.4 low, */
}

void config_NVIC() {
    __NVIC_EnableIRQ(TA0_N_IRQn);
    __NVIC_EnableIRQ(TA1_0_IRQn);
    __NVIC_EnableIRQ(TA2_0_IRQn);
}

void gpio_config() {
    P2->DIR |= BIT4;	//P2.4 is output
    P2->DIR &= ~BIT5;	//P2.5 is input

	P2->REN &= ~BIT5; 	//P2.5, disable pullup/pulldown resistor

    P2->SEL0 &= ~BIT4;	//P2.4 is GPIO
    P2->SEL1 &= ~BIT4;
    P2->SEL0 |= BIT5;	//P2.5 is Primary Module
	P2->SEL1 &= ~BIT5;
}

void TA0_N_IRQHandler() {
	/*
	 * This interrupt is triggered by the ultrasonic sensor. When ECHO goes HIGH,
	 * TIMER_A0 is started, and an initial time is recorded. After ECHO goes LOW,
	 * TIMER_A0 is stopped and the final time is recorded. `dist` gives the last
	 * recorded distance in meters.
	 */

	__disable_irq(); 					//disable IRQsd

	if(count) {
	    count = count++ % 2; //counter
	    TIMER_A0->CTL &= TIMER_A_CTL_MC__STOP; //stop TIMER_A0
		d2 = TIMER_A0->CCR[2];		//record final time
		dist = (d2-d1)*340 / 2;		//record meters
		TIMER_A0->R &= 0x0000;		//zero the R register
	}
	else {
	    count = count++ % 2; //counter
	    TIMER_A0->CTL |= TIMER_A_CTL_MC__CONTINUOUS; //start TIMER_A0
		d1 = TIMER_A0->CCR[2];	//record initial time
	}

    __enable_irq();					//enable IRQs
}

void TA1_0_IRQHandler() {
	__disable_irq(); 					//disable IRQs
	P2->OUT &= ~BIT4;				//set P2.4 LOW
	TIMER_A1->CTL &= TIMER_A_CTL_MC__STOP; //stop TIMER_A1
	TIMER_A1->R &= 0x0000;			//zero out the R register
    __enable_irq();					//enable IRQ

}

void TA2_0_IRQHandler() {
	__disable_irq(); 					//disable IRQs
	TimerA1_config();					//configure TIMER_A1
    P2->OUT |= BIT4;					//set P2.4 HIGH
	TIMER_A1->CTL |= TIMER_A_CTL_MC__UP; //start TIMER_A1
    TIMER_A2->CCTL[0] &= ~BIT0;			//lower interrupt flag
    __enable_irq();					//enable IRQs
}
