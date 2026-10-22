#include "led.h"
#include "stm32f103.h"

#define LED_PORT  GPIOA
#define LED_PIN   12

void LED_Init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN ;              //开启时钟
	
	gpio_config (LED_PORT,LED_PIN,GPIO_CFG_OUT_PP_50MHZ );
	LED_Off ();
}

void LED_On(void)
{
	gpio_reset (LED_PORT ,LED_PIN);
}

void LED_Off(void)
{
	gpio_set (LED_PORT ,LED_PIN );
}

void LED_Toggle(void)
{
	gpio_toggle(LED_PORT, LED_PIN); 
}
