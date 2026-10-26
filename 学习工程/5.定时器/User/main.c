#include "stm32f103.h"
#include "Delay.h"

#define LED_PIN 0

int main(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN ;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN ;
	
	gpio_config (GPIOA,LED_PIN,GPIO_CFG_AF_PP_50MHZ );
	
	TIM2->PSC=71;
	TIM2->ARR=99;
	TIM2->CNT=0;
	
	TIM2->CCMR1 =(6UL<<4)|(1UL<<3);
	TIM2->CCER|=1;
	
	TIM2->EGR=1;
	TIM2->SR &= ~(1U << 0);
	
	TIM2->CR1|=(1UL<<7)|1;
	
	uint32_t duty=0;
	
    while (1)
    {
		for(duty=0;duty <=99;duty++)
		{
			TIM2->CCR1=duty;
			Delay_ms(10);
		}
		for(duty=0;duty <=99;duty++)
		{
			TIM2->CCR1=99-duty;
			Delay_ms(10);
		}
    }
}


