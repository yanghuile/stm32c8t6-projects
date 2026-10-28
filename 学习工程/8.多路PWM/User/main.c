/* ============================================================
 *  PWM多路输出
 *
 *  硬件接线：
 *  	PA0接舵机（TIM2—CH1），PA5接按键
 *		PA7接LED呼吸灯（TIM3-CH2）
 *  	PA8接直流电机（TIM1-CH1）
 * ============================================================ */
#include "stm32f103.h"
#include "timer.h"
#include "key.h"

#define SERVO_PIN 0
#define LED1_PIN  7
#define MOTOR_PIN 8
#define MOTOR_AIN1_PIN 0
#define MOTOR_AIN2_PIN 1

static volatile uint32_t g_ms = 0;
static uint32_t s_duty = 0;
static int32_t  s_step = 1;
static int32_t motor_duty = 30;
static int32_t  motor_step = 20;

int main(void)
{
	/*==========PWM驱动舵机配置=========*/
	KEY_Init ();
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;   
    gpio_config(GPIOA, SERVO_PIN, GPIO_CFG_AF_PP_50MHZ);
	TIMER_Setup(TIM2,1,20000);
	TIMER_PwmStart(TIM2 ,TIM_CH1,1500);
	uint8_t angle=90;
	int32_t step=45;
	
	/*==========PWM驱动呼吸灯配置=========*/
	gpio_config(GPIOA, LED1_PIN, GPIO_CFG_AF_PP_50MHZ);
	TIMER_Setup(TIM3,1,100);
	TIMER_PwmStart(TIM3 ,TIM_CH2,0);
	TIMER_Setup(TIM4, 1, 1000);      /* 1μs/计数，周期 10000μs = 10ms */
	TIMER_EnableUpdateIrq(TIM4, 3);   /* 开更新中断 */
	TIMER_Start(TIM4);
	uint32_t last_breathe = 0;
	
	/*==========PWM驱动直流电机配置=========*/
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	gpio_config (GPIOA,MOTOR_PIN ,GPIO_CFG_AF_PP_50MHZ );
	gpio_config (GPIOB,MOTOR_AIN1_PIN,GPIO_CFG_OUT_PP_50MHZ );
	gpio_config (GPIOB,MOTOR_AIN2_PIN,GPIO_CFG_OUT_PP_50MHZ );
	gpio_set(GPIOB ,MOTOR_AIN1_PIN);
	gpio_reset(GPIOB ,MOTOR_AIN2_PIN);
	TIMER_Setup(TIM1, 1, 50);          /* 1μs/计数，50μs 周期 = 20kHz */
	TIMER_PwmStart(TIM1, TIM_CH1, 30); /* CCR=25 → 50% 占空比 */
	//TIMER_PwmSetUs(TIM1, TIM_CH1, motor_duty);
	
	uint32_t last_motor=0;
	
    while (1)
    {
		/*========舵机：按键调角度=======*/
		if (KEY_Scan() == 1)
		{
			angle += step;

			if (angle >= 180) { angle = 180; step = -45; }   /* 到顶，反向 */
			if (angle <= 0)   { angle = 0;   step =  45; }   /* 到底，反向 */

			uint32_t pulse_us = 500 + angle * 2000 / 180;
			TIMER_PwmSetUs(TIM2, TIM_CH1, pulse_us);
		}	
		
		/*========呼吸灯：100ms一步=======*/
		if(g_ms-last_breathe >=10)
		{
			last_breathe +=10;
			if (g_ms - last_breathe >= 10) last_breathe = g_ms;   /* 落后超一拍，直接对齐 */
			s_duty += s_step;
			if(s_duty >= 99)
			{
				s_duty = 99;
				s_step = -1;
			}
			if(s_duty <= 0)
			{
				s_duty = 0;
				s_step = 1;
			}
			TIMER_PwmSetUs(TIM3, TIM_CH2, s_duty);
		}
		
		/*========直流电机：每2秒换一次速度=======*/
		if(g_ms - last_motor >=2000)
		{
			last_motor +=2000;
			if (g_ms - last_motor >= 2000) last_motor = g_ms;
			
			motor_duty += motor_step;
			if(motor_duty >= 50) {motor_duty = 50; motor_step = -20; }
			if(motor_duty <= 0)  {motor_duty = 0; motor_step = 20;  }
			
			TIMER_PwmSetUs(TIM1, TIM_CH1, motor_duty);
		}
    }
}

void TIM4_IRQHandler(void)
{
    if (TIM4->SR & 1) 
	{ 
		TIM4->SR = (uint16_t)~1; 
		g_ms++; 
	}
}
