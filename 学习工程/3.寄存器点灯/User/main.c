#include "stm32f103.h"
#include "Delay.h"

int main(void)
{
	RCC_APB2ENR |= RCC_APB2ENR_IOPAEN ;
	RCC_APB2ENR |= RCC_APB2ENR_IOPBEN ;
	//RCC_APB2ENR |= RCC_APB2ENR_IOPCEN ;
	GPIOA_CRH &= ~(0x0FU<<16);
	GPIOB_CRH &= ~(0x0FU<<28);
	GPIOB_CRH &= ~(0x0FU<<20);
	//GPIOC_CRH &= ~(0x0FU<<20);
	GPIOA_CRH |= (0x03U<<16);
	GPIOB_CRH |= (0x03U<<28);
	GPIOB_CRH |= (0x03U<<20);
	//GPIOC_CRH |= (0x03U<<20);
	GPIOB_BSRR = (1U<<13);
	GPIOB_BSRR = (1U<<15);
	GPIOA_BSRR = (1U<<12);
	
	//GPIOC_ODR &= ~(1U<<13);
    while (1)
    {
		GPIOB_BSRR = (1U<<(13+16));
		Delay_ms (500);
		GPIOB_BSRR = (1U<<13);
		GPIOB_BSRR = (1U<<(15+16));
		Delay_ms (500);
		GPIOB_BSRR = (1U<<15);
		GPIOA_BSRR = (1U<<(12+16));
		Delay_ms (500);
		GPIOA_BSRR = (1U<<12);
    }
}
