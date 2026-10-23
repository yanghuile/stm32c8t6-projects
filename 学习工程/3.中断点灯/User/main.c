#include "stm32f103.h"
#include "led.h"
#include "key.h"
#include "exti.h"
#include "Delay.h"

#define KEY_PIN  5

int main(void)
{
    LED_Init();
	KEY_Init();
    EXTI_Attach(GPIOA, KEY_PIN, EXTI_EDGE_FALLING, 2);   

    while (1)
	{
		if (EXTI_GetCount(KEY_PIN) > 0)
		{
			Delay_ms(20);                          /* 先等抖动过去 */

			if (gpio_read(GPIOA, KEY_PIN) == 0)    /* 确实按着 */
			{
				LED_Toggle();
				while (gpio_read(GPIOA, KEY_PIN) == 0) { }   /* 等松手 */
			}

			EXTI_ClearCount(KEY_PIN);              /* 最后清零，吞掉抖动边沿 */
		}
	}
}
