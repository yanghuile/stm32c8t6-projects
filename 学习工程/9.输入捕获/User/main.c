/* ============================================================
	输入捕获
 * ============================================================ */

#include "stm32f103.h"
#include "timer.h"

static volatile uint32_t g_cap_val, g_cap_ovf, g_cap_flag;
static volatile uint32_t g_overflow_cnt;
static uint32_t last_cap = 0, last_ovf = 0;

static volatile uint32_t g_period_us = 0;
static volatile uint32_t g_freq_hz   = 0;


int main(void)
{
	/* 先开时钟 */
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN ;
	
	// ========= TIM3-CH1 PA6 输入捕获（TI1，上升沿） =========
	gpio_config (GPIOA ,6,GPIO_CFG_IN_FLOATING  );
	TIMER_Setup(TIM3, 1, 0x10000);   // 1us计数，ARR=65535
	TIM3->CR1 |= (1UL << 0);
	TIM3->CCMR1 &= ~(0x000F);       // 清CC1S + IC1F滤波位
	TIM3->CCMR1 |=  (0x01);         // CC1S=01 映射TI1
	TIM3->CCMR1 |= (0x03 << 4);     // IC1F=0011 滤波，抑制毛刺
	TIM3->CCER &= ~(1UL<<1);        // CC1P=0 上升沿捕获
	TIM3->CCER |=  (1UL<<0);        // CC1E=1 通道捕获使能
	TIM3->DIER |= (1UL<<0)|(1UL<<1);
	NVIC_ISER(IRQ_TIM3 );
	NVIC_IPR [IRQ_TIM3 ]=0x40;
	
	// ========= TIM2-CH1 PA0 PWM 1kHz方波 =========
	gpio_config (GPIOA ,0,GPIO_CFG_AF_PP_50MHZ );
	TIMER_Setup(TIM2 ,10,100000);
	TIM2->CR1 |= (1UL << 0);
	TIM2->CCMR1 &= ~(7UL << 4);
	TIM2->CCMR1 |=  (6UL << 4);     // PWM模式1
	TIM2->CCMR1 &= ~(1UL<<3);       // OC1PE=0
	TIM2->CCER |= (1UL << 0);       // CC1E输出使能
	TIM2->CCR1=5000;
	
    while (1)
    {
        if(g_cap_flag ==1)
		{
			uint32_t cap = g_cap_val;
            uint32_t ovf = g_cap_ovf;           /* 用快照 */

            uint32_t ovf_delta = ovf - last_ovf;
            uint32_t delta = ovf_delta * 65536UL + (cap - last_cap);

            last_cap = cap;
            last_ovf = ovf;
			
			if (delta != 0)
            {
                g_period_us = delta;            /* ★ 存起来 */
                g_freq_hz   = 1000000UL / delta;
            }
		}
    }
}

void TIM3_IRQHandler(void)
{
	if(TIM3->SR&(1UL<<1))                  //CC1IF捕获中断
	{
		TIM3->SR =(uint16_t )~(1UL<<1);    //清CC1IF
		g_cap_val =TIM3->CCR1;
		g_cap_ovf =g_overflow_cnt ;
		g_cap_flag =1;
	}
	if(TIM3->SR & (1UL<<0))               //UIF溢出中断
	{
		TIM3->SR=(uint16_t )~(1UL<<0);    //清UIF
		g_overflow_cnt ++;
	}
}

