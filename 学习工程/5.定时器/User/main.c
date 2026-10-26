#include "stm32f103.h"
#include "led.h"

int main(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN ;
	
	TIM2->PSC=7199;
	TIM2->ARR=9999;
	TIM2->CNT=0;
	
	TIM2->EGR=1;
	
	TIM2->SR=(uint16_t )~1;
	
	TIM2->DIER |=1;
	
	TIM2->CR1|=1;
	
	NVIC_IPR[IRQ_TIM2] = (uint8_t)(3 << 4);
	NVIC_ISER0 |= (1UL << IRQ_TIM2);
	
	LED_Init ();
	
    while (1)
    {
       
    }
}

void TIM2_IRQHandler(void)
{
    if (TIM2->SR & 1)                    
    {
        TIM2->SR = (uint16_t)~1;         /* 写 0 清除，不用 &= */
        LED_Toggle();
    }
}
