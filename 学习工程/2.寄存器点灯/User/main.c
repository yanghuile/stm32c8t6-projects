#include "stm32f103.h"
#include "Delay.h"

int main(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	//RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	
	gpio_config(GPIOA, 12, GPIO_CFG_OUT_PP_50MHZ);
	gpio_config(GPIOA, 5, GPIO_CFG_IN_PULL);
    //gpio_config(GPIOB, 13, GPIO_CFG_OUT_PP_50MHZ);
    //gpio_config(GPIOB, 15, GPIO_CFG_OUT_PP_50MHZ);
	
	gpio_set(GPIOA,5); //ODR的bit5置1 → PA5上拉输入

	gpio_set(GPIOA, 12);
    //gpio_set(GPIOB, 13);
    //gpio_set(GPIOB, 15);
	
    while (1)
    {
		//gpio_reset(GPIOB, 13);   Delay_ms(500);  gpio_set(GPIOB, 13);
        //gpio_reset(GPIOB, 15);   Delay_ms(500);  gpio_set(GPIOB, 15);
        //gpio_reset(GPIOA, 12);   Delay_ms(500);  gpio_set(GPIOA, 12);
		if((gpio_read(GPIOA,5))==0)
		{
			Delay_ms (20);
			if((gpio_read(GPIOA,5))==0)
			{
				gpio_toggle(GPIOA,12);
				while((gpio_read(GPIOA,5))==0){}
			}
		}
    }
}
