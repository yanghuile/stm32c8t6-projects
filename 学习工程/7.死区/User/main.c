/* ============================================================
	验证死区
 * ============================================================ */

#include "stm32f103.h"
#include "timer.h"

int main(void)
{
	
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN ;
	
	gpio_config (GPIOA ,8,GPIO_CFG_AF_PP_50MHZ );
	gpio_config (GPIOB ,13,GPIO_CFG_AF_PP_50MHZ );
	
	TIMER_Setup(TIM1 ,1,1000);
	
	TIM1->CCMR1=(1UL<<3)|(6UL<<4);
	
	TIM1->CCER=(1UL << 0) | (1UL << 2);
	
	TIM1->BDTR=(1UL<<15)|72;
	
	TIM1->CCR1=500;
	
	TIM1->CR1=(1UL<<0)|(1UL<<7);
	
	//TIM1->SR=(uint16_t )~1;
	
    while (1)
    {
        
    }
}
