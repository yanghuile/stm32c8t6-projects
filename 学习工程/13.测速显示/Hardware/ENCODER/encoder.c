#include "encoder.h"

/* ---------- 内部状态 ---------- */
static TIM_TypeDef *s_tim      = 0;
static uint16_t     s_lines    = 0;
static uint16_t     s_gear_x10 = 10;
static uint16_t     s_last_cnt = 0;
static int32_t      s_k        = 0;          /* 1500000 / ENCODER_SAMPLE_MS */
static int32_t      s_delta_max = 0;         /* 按 k 反算的阈值 */

static volatile int32_t s_position  = 0;
static volatile int32_t s_speed_cps = 0;
static volatile int32_t s_rpm10     = 0;

/* ============================================================
 *  初始化
 * ============================================================ */
int ENCODER_Init(TIM_TypeDef *tim, uint16_t lines, uint16_t gear_x10)
{
    /* ---------- 参数检查 ---------- */
    if (tim == 0)                return -1;
    if (lines == 0)              return -1;
    if (gear_x10 == 0)           return -1;      /* 防除零 */
    if (lines > 10000)           return -1;      /* 防溢出 */
    if (1500000UL % ENCODER_SAMPLE_MS != 0) return -1;   /* k 必须整除 */

    s_tim      = tim;
    s_lines    = lines;
    s_gear_x10 = gear_x10;
    s_k        = (int32_t)(1500000UL / ENCODER_SAMPLE_MS);
	s_delta_max = (int32_t)(2000000000L / s_k);      /* 留 7% 余量 */

        /* ---------- 配定时器为编码器模式（4 倍频）---------- */
    tim->PSC   = 0;                              /* 不分频 */
    tim->ARR   = 0xFFFF;                         /* 16 位全范围 */

    tim->CCMR1 = 0;
    tim->CCMR1 |= (0x01UL);                      /* CC1S = 01：IC1 → TI1 */
    tim->CCMR1 |= (0x01UL << 8);                 /* CC2S = 01：IC2 → TI2 */
    tim->CCMR1 |= (0x0FUL << 4);                 /* IC1F = 1111：强滤波 */
    tim->CCMR1 |= (0x0FUL << 12);                /* IC2F = 1111：强滤波 */

    tim->SMCR  = 0x03UL;                         /* SMS = 011：编码器模式 3（4 倍频）*/

    tim->CCER |= (1UL << 0);                     /* CC1E：使能 TI1 */
    tim->CCER |= (1UL << 4);                     /* CC2E：使能 TI2 */

    tim->CNT   = 0;
    tim->CR1  |= (1UL << 0);                     /* CEN */

    return 0;
}

/* ============================================================
 *  周期更新（由节拍中断调用）
 * ============================================================ */
void ENCODER_Update(void)
{
    if (s_tim == 0) return;                      /* 未初始化 */

    uint16_t now   = (uint16_t)s_tim->CNT;
    int16_t  delta = (int16_t)(now - s_last_cnt);   /* ★ 有符号差值处理回绕 */
    s_last_cnt = now;

    /* 异常保护：单次变化过大可能是溢出/干扰 */
    if (delta > s_delta_max || delta < -s_delta_max) return;

    /* ---------- 位置 ---------- */
    /* 位置饱和，防止长期运行溢出 */
	if (s_position > 2000000000L) s_position = 2000000000L;
	if (s_position < -2000000000L) s_position = -2000000000L;
	s_position += delta;

    /* ---------- 速度：计数/秒 ---------- */
    s_speed_cps = (int32_t)delta * (1000 / ENCODER_SAMPLE_MS);

    /* ---------- 速度：0.1 rpm ---------- */
    /* rpm10 = delta × (1500000/T) / (lines × gear_x10) */
    s_rpm10 = ((int32_t)delta * s_k) / ((int32_t)s_lines * s_gear_x10);
}

/* ============================================================
 *  读取接口
 * ============================================================ */
int32_t ENCODER_GetPosition(void)     { return s_position;  }
void    ENCODER_ResetPosition(void)   { s_position = 0;     }
int32_t ENCODER_GetRpm10(void)        { return s_rpm10;     }
int32_t ENCODER_GetCountsPerSec(void) { return s_speed_cps; }
