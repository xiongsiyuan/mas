/* --------------------------------
program:mas.h
revise date:2018.12.23
mcu:HR7P153P4SA 

 ----------------------------------*/
#ifndef _MAS_H_
#define _MAS_H_

#include "mas_op_mode.h"

//#define sw_load	PA2
#define led PA2
#define key	PA4
#define chrg PA5

#define LED_ON 1 
#define LED_OFF 0
#define KEY_PRESS 0
#define KEY_RELAX 1

//pwm
#define sw_timer_module T8P2E 
#define sw_pwm_module T8P1E
#define DISABLE 0
#define ENABLE 1

#define sw_pwm T8P1TRN
#define PWM_ON 0
#define PWM_OFF 1

#define pwm_period T8P1P
#define pwm_duty T8P1R
void pwm_set(unsigned char pwm_value)
	{
		if(pwm_value < pwm_period)
			pwm_duty = pwm_value;
			else pwm_duty = pwm_period;
	}
//pwm value
#ifdef PwmFreq_2K
#define PWM_LOW 50
#define PWM_MID 90
#define PWM_HIG 140
#endif

#ifdef PwmFreq_0K5
#define PWM_LOW 70
#define PWM_MID 110
#define PWM_HIG 200
#endif

#define PWM_NULL 0

//1=8ms
#define ONE_SECOND_COUNT 128//1s
#define ONE_SECOND_COUNT_MASK ONE_SECOND_COUNT-1

#define ONE_2_SECOND_COUNT (128 >> 1)//1s/(2^1)=0.5s
#define ONE_2_SECOND_COUNT_MASK ONE_2_SECOND_COUNT-1

#define ONE_4_SECOND_COUNT (128 >> 2)	//1s/(2^2) = 1/4 s
#define ONE_4_SECOND_COUNT_MASK ONE_4_SECOND_COUNT-1

#define TWO_SECOND_COUNT (128 << 1)//2s
#define TWO_SECOND_COUNT_MASK TWO_SECOND_COUNT-1

#define LED_FLICKER_COUNT	128
#define LED_FLICKER_COUNT_MASK 	LED_FLICKER_COUNT-1
#define LED_FLICKER_COUNT_END	(128 >> 2)	//1s/(2^2) = 1/4 s 

#define PULSE1_COUNT (128 >> 1)//1s/(2^1)=0.5s
#define PULSE1_COUNT_MASK PULSE1_COUNT-1
#define PULSE1_COUNT_END (128 >> 2)//1s/(2^1)=0.25s

#define PULSE2_COUNT (128 >> 0)//1s/(2^0)=1s
#define PULSE2_COUNT_MASK PULSE2_COUNT-1
#define PULSE2_COUNT_END 32

#define SHAKE_AVOID_DELAY 6//6*8ms=48ms
#define TIME_CHRG_SAD 20

//1= 1/4s
#define TIME_KLP 6//key long press last time=6*0.25=1.5s
#define TIME_SD 8//sleep delay time=10*0.25=2s


#endif