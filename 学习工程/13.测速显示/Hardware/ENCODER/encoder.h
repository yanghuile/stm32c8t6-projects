#ifndef ENCODER_H
#define ENCODER_H

#include "stm32f103.h"

/* ============================================================
 *  编码器驱动（正交解码 + 测速）
 *
 *  依赖：
 *      - 调用方负责配置 GPIO（A/B 相为上拉输入）
 *      - 调用方负责提供固定周期的节拍，在节拍中断里调 ENCODER_Update()
 *
 *   ENCODER_Update() 必须【严格按固定周期】调用，
 *     否则 rpm 计算会错（PID 的 dt 也就错了）
 * ============================================================ */

/* 采样周期（ms）—— 必须能整除 1,500,000 */
#define ENCODER_SAMPLE_MS   10

/* 初始化
 *   tim      : 用哪个定时器（TIM2 / TIM3 / TIM4）
 *   lines    : 每圈脉冲数（EC11 = 20 格；电机 = 1000 线）
 *   gear_x10 : 减速比 × 10（1:1 传 10；1:10 传 100；1:7.5 传 75）
 * 返回 0 成功，-1 参数非法
 */
int   ENCODER_Init(TIM_TypeDef *tim, uint16_t lines, uint16_t gear_x10);

/* 由节拍中断调用（每 ENCODER_SAMPLE_MS 一次）*/
void  ENCODER_Update(void);

/* 累计位置（计数，正转 +，反转 -）*/
int32_t ENCODER_GetPosition(void);

/* 位置清零 */
void  ENCODER_ResetPosition(void);

/* 转速：单位 0.1 rpm（750 表示 75.0 rpm）*/
int32_t ENCODER_GetRpm10(void);

/* 转速：单位 计数/秒（原始值，调试用）*/
int32_t ENCODER_GetCountsPerSec(void);

#endif
