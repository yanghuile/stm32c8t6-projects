#include "stm32f103.h"
#include "timer.h"
#include "Delay.h"

#define LED_PIN 0

int main(void)
{
	RCC->APB2ENR |=RCC_APB2ENR_IOPAEN ;
	TIMER_Setup(TIM2,1,100);
	gpio_config (GPIOA,LED_PIN ,GPIO_CFG_AF_PP_50MHZ);
	TIMER_PwmStart(TIM2,TIM_CH1,0);
	
	uint32_t duty=0;
	
    while (1)
    {
		for(duty=0;duty <=99;duty++)
		{
			TIMER_PwmSetUs(TIM2,TIM_CH1 ,duty);
			Delay_ms(10);
		}
		for(duty=0;duty <=99;duty++)
		{
			TIMER_PwmSetUs(TIM2,TIM_CH1 ,99-duty);
			Delay_ms(10);
		}
    }
}
