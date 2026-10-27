#include "stm32f103.h"
#include "timer.h"
#include "led.h"
#include "key.h"
#include "Delay.h"

#define SERVO_PIN 0

int main(void)
{
	LED_Init ();
	KEY_Init ();
	
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;   
    gpio_config(GPIOA, SERVO_PIN, GPIO_CFG_AF_PP_50MHZ);
	
	if(TIMER_Setup(TIM2,1,20000)!=0)
	{
		while(1)
		{
			LED_Toggle ();
			Delay_ms(100);
		}
	}
	TIMER_PwmStart(TIM2 ,TIM_CH1,1500);
	
	uint8_t angle=90;
	int32_t step=45;
	
    while (1)
    {
		if (KEY_Scan() == 1)
		{
			angle += step;

			if (angle >= 180) { angle = 180; step = -45; }   /* 到顶，反向 */
			if (angle <= 0)   { angle = 0;   step =  45; }   /* 到底，反向 */

			uint32_t pulse_us = 500 + angle * 2000 / 180;
			TIMER_PwmSetUs(TIM2, TIM_CH1, pulse_us);
		}	
		
		
    }
}
