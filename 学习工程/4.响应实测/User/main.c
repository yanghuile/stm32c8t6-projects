/* ============================================================
 *  4.响应实测 —— 测量中断延迟与 ISR 执行时间（W8 Day4）
 *
 *  硬件：
 *      PA0（推挽输出）── 杜邦线 ──► PA1（上拉输入，EXTI1）
 *      逻辑分析仪 CH0 = PA0（触发信号）
 *      逻辑分析仪 CH1 = PA12（ISR 打点）
 *      GND 共地
 *
 *  测量：
 *      CH0 下降沿 → CH1 变化 = 中断延迟 + ISR 执行时间
 *      g_isr_cycles = ISR 内部执行的 CPU 周期数
 * ============================================================ */

#include "stm32f103.h"
#include "Delay.h"

#define TRIG_PIN    0      /* PA0：触发输出 */
#define IN_PIN      1      /* PA1：EXTI 输入 */
#define MARK_PIN    12     /* PA12：ISR 打点输出 */

/* 调试观察用（Keil Watch 窗口） */
volatile uint32_t g_isr_cycles  = 0;    /* ISR 执行的周期数 */
volatile uint32_t g_isr_count   = 0;    /* ISR 被调用了几次 */

int main(void)
{
    /* ---------- GPIOA和AFIO时钟 ---------- */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;

    /* ---------- PA0：触发输出 ---------- */
    gpio_config(GPIOA, TRIG_PIN, GPIO_CFG_OUT_PP_50MHZ);
    gpio_set(GPIOA, TRIG_PIN);                  /* 初始高电平 */

    /* ---------- PA1：EXTI 输入 ---------- */
    gpio_set(GPIOA, IN_PIN);                    /* 先定上拉方向 */
    gpio_config(GPIOA, IN_PIN, GPIO_CFG_IN_PULL);

    /* ---------- PA12：打点输出 ---------- */
    gpio_config(GPIOA, MARK_PIN, GPIO_CFG_OUT_PP_50MHZ);
    gpio_reset(GPIOA, MARK_PIN);                /* 初始低电平 */

    /* ---------- EXTI1 ← PA1 ---------- */
    AFIO->EXTICR[IN_PIN / 4] &= ~(0x0FUL << ((IN_PIN % 4) * 4));
    AFIO->EXTICR[IN_PIN / 4] |=  (0x00UL << ((IN_PIN % 4) * 4));

    EXTI->FTSR |=  (1UL << IN_PIN);             /* 下降沿 */
    EXTI->RTSR &= ~(1UL << IN_PIN);
    EXTI->IMR  |=  (1UL << IN_PIN);

    NVIC_IPR[IRQ_EXTI1] = (uint8_t)(0 << 4);    /* 最高优先级 */
    NVIC_ISER0 = (1UL << IRQ_EXTI1);

    /* ---------- 启动 DWT 周期计数器 ---------- */
    CoreDebug_DEMCR |= (1UL << 24);             /* TRCENA */
    DWT_CYCCNT = 0;
    DWT_CTRL  |= 1UL;                           /* CYCCNTENA */

    /* ---------- 主循环：周期性产生下降沿 ---------- */
    while (1)
    {
        Delay_ms(10);                           /* 每 10ms 触发一次 */
        gpio_reset(GPIOA, TRIG_PIN);            /* ↓ 触发 EXTI1 */
        Delay_ms(1);
        gpio_set(GPIOA, TRIG_PIN);              /* ↑ 恢复高电平 */
    }
}


/* ============================================================
 *  EXTI1 中断服务函数（脉冲式）
 * ============================================================ */
void EXTI1_IRQHandler(void)
{
    uint32_t t0 = DWT_CYCCNT;                   /* 进 ISR 立刻读 */
	gpio_set(GPIOA, MARK_PIN);                 /* 进 ISR 立刻置高 */

    if (EXTI->PR & (1UL << IN_PIN))
    {
        EXTI->PR = (1UL << IN_PIN);             /* 清标志 */

        /* ★ 打开下面这两行之一，观察时间变化 */
        //实验 2：for (volatile uint32_t i = 0; i < 1000;  i++) { } 
        //实验 3：for (volatile uint32_t i = 0; i < 10000; i++) { } 

        g_isr_count++;
    }
	 gpio_reset(GPIOA, MARK_PIN);               /* 出 ISR 拉低 */
    g_isr_cycles = DWT_CYCCNT - t0;             /* 出 ISR 前再读 */
}
