/* ============================================================
 *  M 法 vs T 法 对比实测（W10 Day4）
 *
 *  硬件：
 *      PA0（TIM2_CH1）── 杜邦线 ── PA6（TIM3_CH1）  ← 信号自产自测
 *      PA5 ── 按键（切换 4 档频率）
 *
 *  TIM2：PWM 输出已知频率（4 档：10Hz / 100Hz / 1kHz / 10kHz）
 *  TIM3：输入捕获，同时做 T 法（测周期）和 M 法（数脉冲）
 *  TIM4：100ms 节拍，给 M 法提供固定窗口
 *
 *  按一下按键切换下一档，观察 T/M 两种方法的读数差异
 * ============================================================ */

#include "stm32f103.h"
#include "timer.h"
#include "key.h"
#include "Delay.h"

/* ---------- 4 档测试频率（周期 μs）---------- */
static const uint32_t k_period_us[4] = { 100000, 10000, 1000, 100 };   /* 10Hz / 100Hz / 1kHz / 10kHz */
//static const uint32_t k_period_us[4] = { 76922, 10000, 1000, 100 };       /* 实验C */
static uint8_t g_idx = 0;

/* ---------- 输入捕获（T 法用）---------- */
static volatile uint32_t g_overflow   = 0;    /* CNT 溢出次数 */
static volatile uint32_t g_cap_val    = 0;    /* 捕获到的 CCR1 */
static volatile uint32_t g_cap_ovf    = 0;    /* 捕获那一刻的溢出快照 */
static volatile uint8_t  g_cap_flag   = 0;

/* ---------- M 法用 ---------- */
static volatile uint32_t g_cap_cnt    = 0;    /* 捕获次数（M 法计数）*/

/* ---------- 100ms 节拍 ---------- */
static volatile uint8_t  g_tick_100ms = 0;

/* ---------- 结果（Watch 里观察）---------- */
static volatile uint32_t g_true_freq  = 0;    /* 设定的理论频率 */
static volatile uint32_t g_t_period   = 0;    /* T 法：周期 μs */
static volatile uint32_t g_t_freq     = 0;    /* T 法：频率 Hz */
static volatile uint32_t g_m_freq     = 0;    /* M 法：频率 Hz */

int main(void)
{
    /* ---------- 1. 时钟 ---------- */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    KEY_Init();

    /* ---------- 2. TIM2_CH1（PA0）：已知频率信号源 ---------- */
    gpio_config(GPIOA, 0, GPIO_CFG_AF_PP_50MHZ);
    TIMER_Setup(TIM2, 2, k_period_us[0]);                  /* 2μs/计数 */
    TIMER_PwmStart(TIM2, TIM_CH1, k_period_us[0] / 2);     /* 50% 占空比 */
    g_true_freq = 1000000UL / k_period_us[0];

    /* ---------- 3. TIM3_CH1（PA6）：输入捕获 ---------- */
    gpio_config(GPIOA, 6, GPIO_CFG_IN_FLOATING);           /* 输入捕获用浮空 */
//	if (TIMER_Setup(TIM3, 256, 0x10000) != 0)          /* 粗捕获：256μs/tick   实验B */
//	{
//		while (1) { gpio_toggle(GPIOB, 15);                 /* 故障快闪 */
//		Delay_ms(100); }  
//	}                                                          
    TIMER_Setup(TIM3, 1, 0x10000);                          /* 1μs/计数，ARR=65535 */

    TIM3->CCMR1 = 0;
    TIM3->CCMR1 |= (0x01UL);                               /* CC1S = 01：输入 TI1 */
    TIM3->CCMR1 |= (0x03UL << 4);                          /* IC1F = 0011：8 采样滤波 */

    TIM3->CCER &= ~(1UL << 1);                             /* CC1P = 0：上升沿 */
    TIM3->CCER |=  (1UL << 0);                             /* CC1E = 1：使能捕获 */

    TIM3->DIER |= (1UL << 0) | (1UL << 1);                 /* UIE + CC1IE */
    NVIC_IPR[IRQ_TIM3] = (uint8_t)(3 << 4);
    NVIC_ISER0 = (1UL << IRQ_TIM3);                        /* IRQ 29 */

    TIM3->CNT  = 0;
    TIM3->CR1 |= (1UL << 0);                               /* CEN */

    /* ---------- 4. TIM4：100ms 节拍（给 M 法窗口）---------- */
    TIMER_Setup(TIM4, 2, 100000);                          /* 2μs/计数，100ms */
    TIMER_EnableUpdateIrq(TIM4, 2);
    TIMER_Start(TIM4);

    /* ---------- 5. 主循环 ---------- */
    uint32_t last_cap = 0;
    uint32_t last_ovf = 0;

    while (1)
    {
        /* ---------- 按键：切换频率档 ---------- */
        if (KEY_Scan() == 1)
        {
            g_idx = (uint8_t)((g_idx + 1) & 0x03);

            TIMER_Stop(TIM2);
            TIMER_Setup(TIM2, 2, k_period_us[g_idx]);
            TIMER_PwmStart(TIM2, TIM_CH1, k_period_us[g_idx] / 2);

            /* 换频率后清空 T 法状态，避免跨频率算错 */
            last_cap = 0;
            last_ovf = 0;
            g_overflow = 0;
            __disable_irq();
            g_cap_cnt = 0;
            __enable_irq();

            g_t_freq   = 0;
            g_t_period = 0;
            g_m_freq   = 0;
            g_true_freq = 1000000UL / k_period_us[g_idx];
        }

        /* ============ T 法计算 ============ */
		if (g_cap_flag)
		{
			g_cap_flag = 0;

			uint32_t cap = g_cap_val;
			uint32_t ovf = g_cap_ovf;

			/* ★ 从寄存器反推，不写死任何魔数 */
			uint32_t tick_us = (TIM3->PSC + 1) / 72;      /* 1 / 256 */
			uint32_t span    = TIM3->ARR + 1;             /* 65536 / 256 */

			uint32_t ovf_delta = ovf - last_ovf;
			uint32_t ticks     = ovf_delta * span + (cap - last_cap);

			last_cap = cap;
			last_ovf = ovf;

			/* ★ 两条除零保护都要有 */
			if (ticks != 0 && tick_us != 0)
			{
				uint32_t period_us = ticks * tick_us;

				if (period_us != 0)
				{
					g_t_period = period_us;
					g_t_freq   = 1000000UL / period_us;
				}
			}
		}

        /* ---------- M 法：每 100ms 结算一次 ---------- */
        if (g_tick_100ms)
        {
            g_tick_100ms = 0;

            uint32_t n = g_cap_cnt;
            g_cap_cnt -= n;                         /* 用减法，保留读取后新增的 */
            g_m_freq = n * 10;                      /* 100ms 窗口 → ×10 得每秒 */
        }
    }
}

/* ---------- TIM3 中断：捕获 + 溢出 ---------- */
void TIM3_IRQHandler(void)
{
    if (TIM3->SR & (1UL << 1))                      /* CC1IF 捕获 */
    {
        TIM3->SR = (uint16_t)~(1UL << 1);
        g_cap_val = TIM3->CCR1;                     /* 时间戳 */
        g_cap_ovf = g_overflow;                     /* ★ 同一时刻的快照 */
        g_cap_flag = 1;
        g_cap_cnt++;                                /* ★ M 法计数 */
    }

    if (TIM3->SR & (1UL << 0))                      /* UIF 溢出 */
    {
        TIM3->SR = (uint16_t)~(1UL << 0);
        g_overflow++;
    }
}

/* ---------- TIM4 中断：100ms 节拍 ---------- */
void TIM4_IRQHandler(void)
{
    if (TIM4->SR & (1UL << 0))
    {
        TIM4->SR = (uint16_t)~(1UL << 0);
        g_tick_100ms = 1;
    }
}
