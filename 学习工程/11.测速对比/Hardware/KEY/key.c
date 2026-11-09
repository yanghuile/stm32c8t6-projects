#include "key.h"
#include "Delay.h"
#include "stm32f103.h"

#define KEY_PORT  GPIOA
#define KEY_PIN   5


void KEY_Init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN ;              //开启时钟
	
	gpio_set (KEY_PORT,KEY_PIN);                      //上拉
	gpio_config (KEY_PORT,KEY_PIN,GPIO_CFG_IN_PULL ); // 配置引脚模式和速度  
}

uint8_t KEY_Scan(void)
{
	if (gpio_read(KEY_PORT, KEY_PIN) != 0)
		return 0;                           /* 没按下 */

    Delay_ms(20);                           /* 消抖 */

    if (gpio_read(KEY_PORT, KEY_PIN) != 0)
        return 0;                           /* 是抖动，不算 */

    while (gpio_read(KEY_PORT, KEY_PIN) == 0) { }   /* 等松手 */
    return 1;                               /* 报告一次有效按下 */
}
