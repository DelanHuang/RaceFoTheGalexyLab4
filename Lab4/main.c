/*
 * main_.c
 *
 *  Created on: Mar 3, 2021
 *
 */
#include "msp.h"

//header
void TimerA0_config();
void config_NVIC();
void gpio_config();

//declare global vars
int d1 = 0, d2 = 0, count = 0;
double dist;


void main(void) {

	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		//stop watchdog timer

	TimerA0_config();
	config_NVIC();

	gpio_config();

    TIMER_A0->CTL |= TIMER_A_CTL_MC__CONTINUOUS; //start TIMER_A0 in continuous mode

	while(1) {}

}

void TimerA0_config() {

	//TIMER_A0->CTL
    TIMER_A0->CTL &= ~TIMER_A_CTL_IE;     //disable interrupts TA_IE (TIMER_A0 is 0x0000)
	//TIMER_A0->CTL &= TIMER_A_CTL_MC__STOP;	//stop TIMER_A0 (TIMER_A0 is 0x0000)
    TIMER_A0->CTL |= TIMER_A_CTL_CLR;   	//clear TIMER_A0 (TIMER_A0 is 0x0004)

    TIMER_A0->CTL |= TIMER_A_CTL_SSEL__SMCLK; //SMCLK is source (TIMER_A0 is 0x0200)
    TIMER_A0->CTL &= ~TIMER_A_CTL_IFG;	//clear TAIFG bit (TIMER_A0 is 0x0200)

    //TIMER_A0->CCR
    TIMER_A0->CCR[0] = 30; //10us signal
    //TIMER_A0->CCR[2] = 0xFFFF;

    //TIMER_A0->CCTL[n]
    TIMER_A0->CCTL[1] &= ~TIMER_A_CCTLN_CAP; //CCTL[1] in compare mode (output)
    TIMER_A0->CCTL[1] |= TIMER_A_CCTLN_OUTMOD_4;	//P2.4 is our output mode (toggle/set)
    TIMER_A0->CCTL[2] |= TIMER_A_CCTLN_CAP;	//CCTL[2] in capture mode (input)

    TIMER_A0->CCTL[2] |=TIMER_A_CCTLN_SCS;		//synchronous with clock
    TIMER_A0->CCTL[2] |= TIMER_A_CCTLN_SCCI;	//synchronous capture input
    TIMER_A0->CCTL[2] |= TIMER_A_CCTLN_CM__BOTH;	//capture on both edges

    TIMER_A0->CCTL[1] |= TIMER_A_CCTLN_CCIE; //enable interrupts
    TIMER_A0->CCTL[2] |= TIMER_A_CCTLN_CCIE;
    //TIMER_A0->CCTL[2] &= ~TIMER_A_CCTLN_CCIS__VCC;  //CCIA

    TIMER_A0->CCTL[1] &= ~TIMER_A_CCTLN_CCIFG; //clear interrupt flags
    TIMER_A0->CCTL[2] &= ~TIMER_A_CCTLN_CCIFG;
}


void gpio_config() {

    P2->DIR |= BIT4;	//P2.4 is output
    P2->DIR &= ~BIT5;	//P2.5 is input

    P2->OUT |= BIT4;	//P2.4 is default high

    P2->IE |= BIT5;		//enable interrupt on BIT5
    P2->IES &= ~BIT5;	//P2.5 interrupt low to high transition

    P2->REN &= ~BIT4;    //P2.4, disable pullup/pulldown resistor
	P2->REN &= ~BIT5; 	//P2.5, disable pullup/pulldown resistor

    P2->SEL0 &= ~BIT4;	//P2.4 is GPIO
    P2->SEL1 &= ~BIT4;
    P2->SEL0 |= BIT5;	//P2.5 is Primary Module
	P2->SEL1 &= ~BIT5;

	P2->IFG &= 0x0;	//lower interrupt flags
}

void config_NVIC() {
    __NVIC_EnableIRQ(TA0_N_IRQn);
    __delay_cycles(3000000);	//also: asm nop ...
}

void TA0_N_IRQHandler() {
	/*
	 * TIMER_A0->IV == 2: In this case we lower P2.4 after a 10us period has occured.
	 *
	 * TIMER_A0->IV == 4:
	 * This interrupt is triggered by the ultrasonic sensor. When ECHO sends P2.5 HIGH,
	 * an initial time is recorded from TIMER_A0->CCR[1]. After ECHO sends P2.5 LOW (the next interrupt),
	 * the final time is recorded from TIMER_A0->CCR[1]. The calculation of`dist` gives the last
	 * recorded distance in meters. TIMER_A0 is then reset.
	 */
	__disable_irq();	//disable interrupts
	if(TIMER_A0->IV == 0x2){	//interrupt flag for CCTL[1] / P2.4

		P2->OUT ^= BIT4;							//toggle P2.4
		TIMER_A0->CTL &= ~TIMER_A_CTL_IFG;			//clear interrupt flags
	    TIMER_A0->CCTL[1] &= 0xFFFE;
	}

	else if(TIMER_A0->IV == 0x4){ //interrupt flag for CCTL[2] / P2.5

		count = count++ % 2;

		if(count % 2 == 0) {
			d1 = TIMER_A0->CCR[2];	//record init time
		}
		else {
			d2 = TIMER_A0->CCR[2];	//record final time
			dist = (d2-d1)*340 / 2;	//record meters
			TimerA0_config();					//reset TIMER_A0
		    TIMER_A0->CTL |= TIMER_A_CTL_MC__UP; //start TIMER_A0
		}

		TIMER_A0->CTL &= ~TIMER_A_CTL_IFG;			//clear interrupt flags
	    TIMER_A0->CCTL[2] &= ~TIMER_A_CCTLN_CCIFG;
	}

	__enable_irq();						//enable interrupts
}
