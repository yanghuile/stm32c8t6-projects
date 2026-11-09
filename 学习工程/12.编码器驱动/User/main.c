/* ============================================================
 *  编码器驱动测试（W10 Day5）
 *
 *  硬件：
 *      EC11 A   → PA6（TIM3_CH1）
 *      EC11 B   → PA7（TIM3_CH2）
 *      EC11 VCC → 3.3V
 *      EC11 GND → GND
 *      LED      → PB15
 *
 *  TIM3：编码器模式（4 倍频）
 *  TIM4：10ms 节拍 → 调用 ENCODER_Update()
 * ============================================================ */

#include "stm32f103.h"
#include "timer.h"
#include "encoder.h"
#include "Delay.h"

/* ---------- ★ 调试用影子变量 ---------- */
volatile int32_t  dbg_pos        = 0;    /* 累计位置 */
volatile int32_t  dbg_rpm10      = 0;    /* 转速 0.1rpm */
volatile int32_t  dbg_cps        = 0;    /* 计数/秒 */
volatile uint32_t dbg_update_cnt = 0;    /* Update 调用次数：每秒应 +100 */

int main(void)
{
    /* ---------- 1. 时钟 ---------- */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    /* ---------- 2. LED ---------- */
    gpio_config(GPIOB, 15, GPIO_CFG_OUT_PP_50MHZ);
    gpio_set(GPIOB, 15);

    /* ---------- 3. 编码器引脚（上拉输入）---------- */
    gpio_set(GPIOA, 6);
    gpio_set(GPIOA, 7);
    gpio_config(GPIOA, 6, GPIO_CFG_IN_PULL);
    gpio_config(GPIOA, 7, GPIO_CFG_IN_PULL);

    /* ---------- 4. 编码器驱动：EC11 20 格/圈，无减速（gear_x10 = 10）---------- */
    if (ENCODER_Init(TIM3, 20, 10) != 0)
    {
        while (1) { gpio_toggle(GPIOB, 15); Delay_ms(100); }   /* 故障快闪 */
    }

    /* ---------- 5. TIM4：10ms 节拍 ---------- */
    if (TIMER_Setup(TIM4, 2, 10000) != 0)
    {
        while (1) { gpio_toggle(GPIOB, 15); Delay_ms(300); }
    }
    TIMER_EnableUpdateIrq(TIM4, 2);
    TIMER_Start(TIM4);

    /* ---------- 6. 主循环：只读数据 ---------- */
    while (1)
    {
        dbg_pos   = ENCODER_GetPosition();
        dbg_rpm10 = ENCODER_GetRpm10();
        dbg_cps   = ENCODER_GetCountsPerSec();

        if (dbg_cps != 0) gpio_toggle(GPIOB, 15);
    }
}

/* ---------- TIM4 中断：严格 10ms，在这里更新编码器 ---------- */
void TIM4_IRQHandler(void)
{
    if (TIM4->SR & (1UL << 0))
    {
        TIM4->SR = (uint16_t)~(1UL << 0);
        ENCODER_Update();          /* ★ 在 ISR 里调用，保证周期严格 */
        dbg_update_cnt++;          /* ★ 每秒应该 +100 */
    }
}
