/* ============================================================
 *  测速 + 统计 + 滑动平均滤波（W10 Day6）
 *
 *  硬件：
 *      PA0（TIM2_CH1）── 杜邦线 ── PA6
 *      PA7 悬空（上拉恒高，不产生边沿）
 *      PA5 ── 按键（复位统计）
 *      LED  PB15
 *
 *  TIM2：输出 5128μs（194.97Hz）方波做信号源
 *  TIM3：编码器模式，数 A 相边沿
 *  TIM4：10ms 节拍 → 更新速度 + 滤波 + 统计
 *
 *  预期：
 *      dbg_raw 在 2250 / 3000 之间跳
 *      dbg_ma  稳定在约 2925
 *      理论值 2924
 * ============================================================ */

#include "stm32f103.h"
#include "timer.h"
#include "key.h"
#include "encoder.h"
#include "Delay.h"

#define MA_N     8          /* 滑动平均窗口长度 */
#define STAT_N   100        /* 统计窗口（100 × 10ms = 1 秒）*/

/* ---------- 调试观察变量 ---------- */
volatile int32_t  dbg_raw   = 0;    /* 原始瞬时转速 rpm10 */
volatile int32_t  dbg_ma    = 0;    /* 滑动平均后 */
volatile int32_t  dbg_min   = 0;    /* 上一秒最小值 */
volatile int32_t  dbg_max   = 0;    /* 上一秒最大值 */
volatile int32_t  dbg_avg   = 0;    /* 上一秒平均值 */
volatile uint32_t dbg_tick  = 0;    /* 节拍计数（每秒 +100）*/
volatile uint32_t dbg_n     = 0;    /* 样本累计 */

/* ---------- 滑动平均滤波器 ---------- */
static int32_t s_ma_buf[MA_N];
static uint8_t s_ma_idx = 0;
static int32_t s_ma_sum = 0;

static int32_t MA_Push(int32_t v)
{
    s_ma_sum -= s_ma_buf[s_ma_idx];
    s_ma_buf[s_ma_idx] = v;
    s_ma_sum += v;
    s_ma_idx = (uint8_t)((s_ma_idx + 1) % MA_N);
    return s_ma_sum / MA_N;
}

static void MA_Reset(void)
{
    for (uint8_t i = 0; i < MA_N; i++) s_ma_buf[i] = 0;
    s_ma_idx = 0;
    s_ma_sum = 0;
}

/* ---------- 统计（每秒结算一次）---------- */
static int32_t  s_st_min = 0, s_st_max = 0;
static int32_t  s_st_sum = 0;
static uint16_t s_st_cnt = 0;

static void STAT_Push(int32_t v)
{
    if (s_st_cnt == 0) { s_st_min = v; s_st_max = v; s_st_sum = 0; }

    if (v < s_st_min) s_st_min = v;
    if (v > s_st_max) s_st_max = v;
    s_st_sum += v;
    s_st_cnt++;

    if (s_st_cnt >= STAT_N)
    {
        dbg_min = s_st_min;
        dbg_max = s_st_max;
        dbg_avg = s_st_sum / STAT_N;
        s_st_cnt = 0;
    }
}

int main(void)
{
    /* ---------- 1. 时钟 ---------- */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    KEY_Init();

    /* ---------- 2. LED ---------- */
    gpio_config(GPIOB, 15, GPIO_CFG_OUT_PP_50MHZ);
    gpio_set(GPIOB, 15);

    /* ---------- 3. TIM2_CH1 (PA0)：固定频率信号源 ---------- */
    gpio_config(GPIOA, 0, GPIO_CFG_AF_PP_50MHZ);

    if (TIMER_Setup(TIM2, 1, 5128) != 0)        /* 1μs/计数，周期 5128μs */
    {
        while (1) { gpio_toggle(GPIOB, 15); Delay_ms(100); }
    }
    TIMER_PwmStart(TIM2, TIM_CH1, 2564);        /* 50% 占空比 */

    /* ---------- 4. 编码器（TIM3，A 相接 PA0 送来的方波）---------- */
    gpio_set(GPIOA, 6);
    gpio_set(GPIOA, 7);
    gpio_config(GPIOA, 6, GPIO_CFG_IN_PULL);
    gpio_config(GPIOA, 7, GPIO_CFG_IN_PULL);

    if (ENCODER_Init(TIM3, 20, 10) != 0)
    {
        while (1) { gpio_toggle(GPIOB, 15); Delay_ms(300); }
    }

    /* ---------- 5. TIM4：10ms 节拍 ---------- */
    if (TIMER_Setup(TIM4, 2, 10000) != 0)
    {
        while (1) { gpio_toggle(GPIOB, 15); Delay_ms(500); }
    }
    TIMER_EnableUpdateIrq(TIM4, 2);
    TIMER_Start(TIM4);

    MA_Reset();

    /* ---------- 6. 主循环：只做按键复位 ---------- */
    while (1)
    {
        if (KEY_Scan() == 1)
        {
            MA_Reset();
            s_st_cnt = 0;
            dbg_min = 0; dbg_max = 0; dbg_avg = 0;
        }
    }
}

/* ---------- TIM4 中断：严格 10ms ---------- */
void TIM4_IRQHandler(void)
{
    if (TIM4->SR & (1UL << 0))
    {
        TIM4->SR = (uint16_t)~(1UL << 0);

        ENCODER_Update();

        dbg_raw = ENCODER_GetRpm10();
        dbg_ma  = MA_Push(dbg_raw);
        STAT_Push(dbg_raw);

        dbg_tick++;
        dbg_n++;
    }
}
