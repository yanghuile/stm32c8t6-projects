#include "timer.h"
#include "stm32f103.h"

static uint32_t tick_of(TIM_TypeDef *tim);

/* ============================================================
 *  设置时基单元
 *    tick_us   : 1 个计数代表多少微秒（1 / 10 / 100 ...，必须能整除 72）
 *    period_us : 一个周期的微秒数（20000 = 20ms → 50Hz）
 *  返回：0 成功，-1 参数不可实现
 * ============================================================ */
int TIMER_Setup(TIM_TypeDef *tim, uint32_t tick_us, uint32_t period_us)
{
	uint32_t psc_val = 72 * tick_us - 1;
    if (tick_us == 0) return -1;                    /* 先挡住除零 */
	if (72 % tick_us != 0) return -1;
	if (period_us % tick_us != 0) return -1;
	if (period_us / tick_us > 0x10000) return -1;

    if(tim == TIM1)
    {
        RCC->APB2ENR |= RCC_APB2ENR_TIM1EN ;
    }
    else if(tim == TIM2)
    {
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN ;
    }
    else if(tim == TIM3)
    {
        RCC->APB1ENR |= RCC_APB1ENR_TIM3EN ;
    }
    else if(tim == TIM4)
    {
        RCC->APB1ENR |= RCC_APB1ENR_TIM4EN ;
    }

    tim->PSC = psc_val;
    tim->ARR = (period_us / tick_us) - 1;
    tim->CNT = 0;
	
    tim->EGR = 1;               // UG：载入影子寄存器
    tim->SR = (uint16_t)~1;      // 清除UIF更新标志
	
    return 0;
}

/* 使能更新中断（定时器溢出时进 ISR）*/
void TIMER_EnableUpdateIrq(TIM_TypeDef *tim, uint8_t prio)
{
	uint32_t irq;
	if      (tim == TIM1) irq = 25;
	else if (tim == TIM2) irq = 28;
	else if (tim == TIM3) irq = 29;
	else if (tim == TIM4) irq = 30;
	else return;    
	
	if(tim->SR&(1UL<<0))
	{
		tim->SR = (uint16_t)~1;
	}
	
	tim->DIER |= (1UL<<0);
	NVIC_IPR[irq] =(prio&0x0F) << 4;
	NVIC_ISER0 |= 1U << irq;
}

/* ============================================================
 *  启动一路 PWM
 *    调用前必须把对应引脚配成复用推挽（GPIO_CFG_AF_PP_50MHZ）
 *    width_us: 初始脉宽（微秒），必须 <= period_us
 * ============================================================ */
void TIMER_PwmStart(TIM_TypeDef *tim, TimCh_t ch, uint32_t width_us)
{
	uint32_t ccr_val = width_us / tick_of(tim);
    switch(ch)
    {
        case TIM_CH1:
            tim->CCR1 = ccr_val;
            // CCMR1: OC1M=110(PWM模式1)  | OC1PE=1 预装载
            tim->CCMR1 |= (6UL << 4) | (1UL << 3);
            tim->CCER  |= (1UL << 0);  // CC1E 通道输出使能
            break;
        case TIM_CH2:
            tim->CCR2 = ccr_val;
            // CCMR1: OC2M=110 | OC2PE=1
            tim->CCMR1 |= (6UL << 12) | (1UL << 11);
            tim->CCER  |= (1UL << 4);  // CC2E
            break;
        case TIM_CH3:
            tim->CCR3 = ccr_val;
            // CCMR2: OC3M=110 | OC3PE=1
            tim->CCMR2 |= (6UL << 4) | (1UL << 3);
            tim->CCER  |= (1UL << 8);  // CC3E
            break;
        case TIM_CH4:
            tim->CCR4 = ccr_val;
            // CCMR2: OC4M=110 | OC4PE=1
            tim->CCMR2 |= (6UL << 12) | (1UL << 11);
            tim->CCER  |= (1UL << 12); // CC4E
            break;
        default:
            return;
    }
	if (tim == TIM1) tim->BDTR |= (1UL << 15);    /* MOE */
    // 最后启动定时器计数 (CEN位 CR1 bit0)
    TIMER_Start(tim);
}

/* 运行中改脉宽（微秒）*/
void TIMER_PwmSetUs(TIM_TypeDef *tim, TimCh_t ch, uint32_t width_us)
{
	uint32_t ccr_val = width_us / tick_of(tim);
	switch(ch)
	{
		case TIM_CH1: tim->CCR1 = ccr_val; break;
		case TIM_CH2: tim->CCR2 = ccr_val; break;
		case TIM_CH3: tim->CCR3 = ccr_val; break;
		case TIM_CH4: tim->CCR4 = ccr_val; break;
		default: return;
	}
}

/* 启动计数器 */
void TIMER_Start(TIM_TypeDef *tim)
{
	tim->CR1 |= (1U << 0);
}

/* 停止计数器 */
void TIMER_Stop(TIM_TypeDef *tim)
{
	tim->CR1 &= ~(1U << 0);
}

static uint32_t tick_of(TIM_TypeDef *tim)
{
    uint32_t t= (tim->PSC + 1) / 72;     /* PSC=71 → 1μs；PSC=719 → 10μs */
	return (t == 0) ? 1 : t;
}
