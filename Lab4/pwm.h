#ifndef PWM_H_
#define PWM_H_

#include "msp.h"

/*  MATH FOR TICKS: Where divider = 1
 * (3*10^6)/(2^0) = 3000000 |||| 300000*(500*10^-6) = 15 ticks ==> 0xF in hex
 * After running analysis with TICKS = 15, we noticed we were getting 94Khz which meant our pwm was running slightly slow. To counteract
 * this, we set TICKS = 14 = 0xE. We are guess this was due to the fact an entire tick period is required to reset the counter to 0.
 */
#define TICKS 0x927C

void config_pwm_timer(void);

void start_pwm(void);

void stop_pwm(void);

void config_pwm_gpio(void);

void config_switch_interrupt(void);


#endif /* PWM_H_ */
