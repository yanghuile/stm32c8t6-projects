#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "stm32f103.h"

typedef enum
{
	TIM_CH1=1,
	TIM_CH2=2,
	TIM_CH3=3,
	TIM_CH4=4
}TimCh_t;

int TIMER_Setup(TIM_TypeDef *tim, uint32_t tick_us, uint32_t period_us);
void TIMER_EnableUpdateIrq(TIM_TypeDef *tim, uint8_t prio);
void TIMER_PwmStart(TIM_TypeDef *tim, TimCh_t ch, uint32_t width_us);
void TIMER_PwmSetUs(TIM_TypeDef *tim, TimCh_t ch, uint32_t width_us);

void TIMER_Start(TIM_TypeDef *tim);
void TIMER_Stop(TIM_TypeDef *tim);

#endif
