#include "stm32f10x.h"                  // Device header
#include "led.h"

int main()
{
	LED_Init();
	GPIO_ResetBits (GPIOC,GPIO_Pin_13);
	while(1)
	{
		
	}
}
