/* ============================================================
 *  编码器测速（W10 Day3）
 *
 *  硬件：
 *      EC11 A   → PA6（TIM3_CH1）
 *      EC11 B   → PA7（TIM3_CH2）
 *      EC11 VCC → 3.3V
 *      EC11 GND → GND
 *      LED      → PB15
 *
 *  TIM3：编码器模式（4 倍频），读 CNT 得位置
 *  TIM2：100ms 节拍中断，用来算速度
 *
 *  速度只在有转动时更新，停手后保持最后值（方便暂停观察）
 * ============================================================ */

#include "stm32f103.h"
#include "timer.h"

#define ENC_DETENTS_PER_REV    20                                /* EC11 一圏 20 格 */
#define ENC_COUNTS_PER_DETENT  4                                 /* 4 倍频 */
#define ENC_COUNTS_PER_REV     (ENC_DETENTS_PER_REV * ENC_COUNTS_PER_DETENT)

/* ---------- ISR 与主循环共享 ---------- */
static volatile uint8_t  g_tick_100ms = 0;

/* ---------- 结果（Watch 里观察）---------- */
static volatile int32_t  g_position      = 0;   /* 累计位置（计数）*/
static volatile int32_t  g_detents       = 0;   /* 累计格数 = position / 4 */
static volatile int32_t  g_speed_cps     = 0;   /* 最后非零速度：每秒计数 */
static volatile int32_t  g_speed_dps     = 0;   /* 最后非零速度：每秒格数 */
static volatile int32_t  g_speed_rpm     = 0;   /* 最后非零速度：转/分 */
static volatile int32_t  g_speed_peak    = 0;   /* 历史最快（每秒计数）*/

int main(void)
{
    /* ---------- 1. 时钟 ---------- */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    /* ---------- 2. LED（PB15）---------- */
	gpio_reset(GPIOB, 15);                       /*初始点亮*/
    gpio_config(GPIOB, 15, GPIO_CFG_OUT_PP_50MHZ);

    /* ---------- 3. 编码器引脚 ---------- */
    gpio_set(GPIOA, 6);
    gpio_set(GPIOA, 7);
    gpio_config(GPIOA, 6, GPIO_CFG_IN_PULL);
    gpio_config(GPIOA, 7, GPIO_CFG_IN_PULL);

    /* ---------- 4. TIM3 编码器模式（4 倍频）---------- */
    TIM3->PSC   = 0;
    TIM3->ARR   = 0xFFFF;
    TIM3->CCMR1 = 0;
    TIM3->CCMR1 |= (0x01UL);                  /* CC1S=01 */
    TIM3->CCMR1 |= (0x01UL << 8);             /* CC2S=01 */
    TIM3->CCMR1 |= (0x0FUL << 4);             /* IC1F */
    TIM3->CCMR1 |= (0x0FUL << 12);            /* IC2F */
    TIM3->SMCR  = 0x03UL;                     /* SMS=011 编码器模式3 */
    TIM3->CCER |= (1UL << 0);                 /* CC1E */
    TIM3->CCER |= (1UL << 4);                 /* CC2E */
    TIM3->CNT   = 0;
    TIM3->CR1  |= (1UL << 0);                 /* CEN */

    /* ---------- 5. TIM2 做 100ms 节拍 ---------- */
    TIMER_Setup(TIM2, 2, 100000);             /* 2μs/计数，周期 100ms */
    TIMER_EnableUpdateIrq(TIM2, 3);
    TIMER_Start(TIM2);

    /* ---------- 6. 主循环 ---------- */
    uint16_t last_cnt = (uint16_t)TIM3->CNT;
    int32_t  acc      = 0;

    while (1)
    {
        uint16_t now   = (uint16_t)TIM3->CNT;
        int16_t  delta = (int16_t)(now - last_cnt);
        last_cnt = now;

        g_position += delta;
        acc        += delta;

        if (delta != 0) gpio_toggle(GPIOB, 15);

        if (g_tick_100ms)
        {
            g_tick_100ms = 0;

            /* 只在这一个 100ms 内有转动时更新速度 */
            if (acc != 0)
            {
                g_speed_cps = acc * 10;                            /* ×10 → 每秒 */
                g_speed_dps = (acc * 10) / ENC_COUNTS_PER_DETENT;  /* ×10/4 → 格/秒 */
                g_speed_rpm = g_speed_cps * 60 / ENC_COUNTS_PER_REV;

                if (g_speed_cps > g_speed_peak)
                    g_speed_peak = g_speed_cps;
            }
            /* acc == 0（没转动）→ 速度保持上次的值，不清零 */

            g_detents = g_position / ENC_COUNTS_PER_DETENT;
            acc = 0;
        }
    }
}

/* ---------- TIM2 更新中断：只置标志 ---------- */
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & (1UL << 0))
    {
        TIM2->SR = (uint16_t)~(1UL << 0);
        g_tick_100ms = 1;
    }
}
