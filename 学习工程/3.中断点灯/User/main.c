/* ============================================================
 *  3.中断点灯 —— EXTI 外部中断（W8 Day3）
 *
 *  PA5 按键 → EXTI5 → 下降沿触发 → EXTI9_5_IRQHandler
 *  ISR 里翻转 PA12 的 LED
 * ============================================================ */

#include "stm32f103.h"
#include "led.h"
#include "key.h"
#include "Delay.h"

#define KEY_PIN  5

volatile uint32_t g_key_count = 0;
volatile uint8_t key_flag = 0;   // 中断标记

int main(void)
{
    LED_Init();                        
    KEY_Init();                        

	/* ---------- 开启AFIO时钟 ---------- */
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

    /* ---------- 把 EXTI5 映射到 PA5 ---------- */
    /* EXTI5 → EXTICR[1]（管 EXTI4~7），字段位置 (5 % 4) * 4 = 4 → bit[7:4] */
    AFIO->EXTICR[KEY_PIN / 4] &= ~(0x0FUL << ((KEY_PIN % 4) * 4));
    AFIO->EXTICR[KEY_PIN / 4] |=  (0x00UL << ((KEY_PIN % 4) * 4));   /* 0 = PA */

    /* ---------- EXTI5：下降沿触发 + 解屏蔽 ---------- */
    EXTI->FTSR |=  (1UL << KEY_PIN);     /* 下降沿触发 */
    EXTI->RTSR &= ~(1UL << KEY_PIN);     /* 关闭上升沿 */
    EXTI->IMR  |=  (1UL << KEY_PIN);     /* 解除中断屏蔽 */

    /* ---------- NVIC：设优先级 + 使能 ---------- */
    NVIC_IPR[IRQ_EXTI9_5 ]=(uint8_t)(2<<4);   /* 优先级 2 */
    NVIC_ISER0=(1UL<<IRQ_EXTI9_5 );

    while (1)
    {
		if(key_flag == 1)
        {
            Delay_ms(20);              /* 消抖放在主循环,中断里不能延时 */
            if(gpio_read(GPIOA, KEY_PIN) == 0)  /* 再次确认按键还处于按下 */
            {
                LED_Toggle();
                g_key_count++;
            }
            key_flag = 0;                /* 清除标志 */
        }
    }
}

/* ============================================================
 *  EXTI9_5 中断服务函数
 * ============================================================ */
void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR & (1UL << KEY_PIN))     /* 确认是 EXTI5 触发 */
    {
        key_flag =1;                     
		EXTI->PR = (1UL << KEY_PIN);     /* 清挂起标志：写 1 清除 */
    }
}
