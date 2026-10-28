#ifndef STM32F103_H
#define STM32F103_H

#include <stdint.h>

#define RCC  ((RCC_TypeDef *)0x40021000UL)
#define AFIO  ((AFIO_TypeDef *)0x40010000UL)
#define EXTI  ((EXTI_TypeDef *)0x40010400UL)
#define TIM1  ((TIM_TypeDef *)0x40012C00UL)
#define TIM2  ((TIM_TypeDef *)0x40000000UL)
#define TIM3  ((TIM_TypeDef *)0x40000400UL)
#define TIM4  ((TIM_TypeDef *)0x40000800UL)

/* RCC->APB1ENR 的定时器时钟使能位 */
#define RCC_APB1ENR_TIM2EN   (1UL << 0)
#define RCC_APB1ENR_TIM3EN   (1UL << 1)
#define RCC_APB1ENR_TIM4EN   (1UL << 2)

/* RCC->APB2ENR 的定时器时钟使能位 */
#define RCC_APB2ENR_TIM1EN   (1UL << 11)

/* APB2ENR 位定义 */
#define RCC_APB2ENR_IOPAEN    (1UL << 2)
#define RCC_APB2ENR_IOPBEN    (1UL << 3)
#define RCC_APB2ENR_IOPCEN    (1UL << 4)
#define RCC_APB2ENR_IOPDEN    (1UL << 5)
#define RCC_APB2ENR_IOPEEN    (1UL << 6)
#define RCC_APB2ENR_AFIOEN    (1UL << 0)

#define GPIOA  ((GPIO_TypeDef *)0x40010800UL)
#define GPIOB  ((GPIO_TypeDef *)0x40010C00UL)
#define GPIOC  ((GPIO_TypeDef *)0x40011000UL)
#define GPIOD  ((GPIO_TypeDef *)0x40011400UL)
#define GPIOE  ((GPIO_TypeDef *)0x40011800UL)

/* ---- 输出模式 ---- */
#define GPIO_CFG_OUT_PP_50MHZ   (0x3U)   /* MODE=11(50MHz) CNF=00(推挽) */
#define GPIO_CFG_OUT_PP_2MHZ    (0x2U)   /* MODE=10(2MHz)  CNF=00(推挽) */
#define GPIO_CFG_OUT_OD_50MHZ   (0x7U)   /* MODE=11        CNF=01(开漏) */
#define GPIO_CFG_AF_PP_50MHZ    (0xBU)   /* MODE=11        CNF=10(复用推挽)*/
#define GPIO_CFG_AF_OD_50MHZ    (0xFU)   /* MODE=11        CNF=11(复用开漏)*/

/* ---- 输入模式---- */
#define GPIO_CFG_IN_ANALOG      (0x0U)   /* CNF=00 模拟    */
#define GPIO_CFG_IN_FLOATING    (0x4U)   /* CNF=01 浮空    */
#define GPIO_CFG_IN_PULL        (0x8U)   /* CNF=10 上拉/下拉  */

/* ============================================================
 *  NVIC —— 嵌套向量中断控制器
 *  基地址 0xE000E100
 * ============================================================ */
#define NVIC_ISER0   (*(volatile uint32_t *)0xE000E100UL)   /* 使能 IRQ 0~31  */
#define NVIC_ISER1   (*(volatile uint32_t *)0xE000E104UL)   /* 使能 IRQ 32~63 */
#define NVIC_ICER0   (*(volatile uint32_t *)0xE000E180UL)   /* 关闭 IRQ 0~31  */
#define NVIC_ISER(idx) ((idx) < 32 ? (NVIC_ISER0 |= (1UL << (idx))) \
                                   : (NVIC_ISER1 |= (1UL << ((idx) - 32))))

/* 优先级寄存器：每个中断占 1 字节，只写入高 4 位有效 */
#define NVIC_IPR     ((volatile uint8_t  *)0xE000E400UL)

/* ============================================================
 *  中断号 —— 从 RM0008 向量表，只列常用的
 * ============================================================ */
#define IRQ_WWDG         0
#define IRQ_PVD          1
#define IRQ_TAMPER       2
#define IRQ_RTC          3
#define IRQ_FLASH        4
#define IRQ_RCC          5
#define IRQ_EXTI0        6
#define IRQ_EXTI1        7
#define IRQ_EXTI2        8
#define IRQ_EXTI3        9
#define IRQ_EXTI4        10
#define IRQ_DMA1_CH1     11
#define IRQ_ADC1_2       18
#define IRQ_EXTI9_5      23
#define IRQ_TIM1         25    
#define IRQ_TIM2         28
#define IRQ_TIM3         29
#define IRQ_TIM4         30
#define IRQ_I2C1_EV      31
#define IRQ_SPI1         35
#define IRQ_USART1       37
#define IRQ_USART2       38
#define IRQ_EXTI15_10    40

/* ============================================================
 *  RCC —— 复位与时钟控制（基地址 0x40021000）
 * ============================================================ */
typedef struct
{
    volatile uint32_t CR;        /* 0x00  时钟控制 */
    volatile uint32_t CFGR;      /* 0x04  时钟配置 */
    volatile uint32_t CIR;       /* 0x08  时钟中断 */
    volatile uint32_t APB2RSTR;  /* 0x0C  APB2 外设复位 */
    volatile uint32_t APB1RSTR;  /* 0x10  APB1 外设复位 */
    volatile uint32_t AHBENR;    /* 0x14  AHB 外设时钟使能 */
    volatile uint32_t APB2ENR;   /* 0x18  APB2 外设时钟使能 */
    volatile uint32_t APB1ENR;   /* 0x1C  APB1 外设时钟使能 */
    volatile uint32_t BDCR;      /* 0x20  备份域控制 */
    volatile uint32_t CSR;       /* 0x24  控制/状态 */
} RCC_TypeDef;

/* ============================================================
 *  AFIO —— 复用功能 IO（EXTI 线映射）
 * ============================================================ */
typedef struct
{
    volatile uint32_t EVCR;        /* 0x00 */
    volatile uint32_t MAPR;        /* 0x04 */
    volatile uint32_t EXTICR[4];   /* 0x08 ~ 0x14，EXTICR1~4 */
    volatile uint32_t RESERVED;    /* 0x18 */
    volatile uint32_t MAPR2;       /* 0x1C */
} AFIO_TypeDef;

/* ============================================================
 *  EXTI —— 外部中断/事件控制器
 * ============================================================ */
typedef struct
{
    volatile uint32_t IMR;     /* 0x00 中断屏蔽（1=允许产生中断） */
    volatile uint32_t EMR;     /* 0x04 事件屏蔽 */
    volatile uint32_t RTSR;    /* 0x08 上升沿触发选择 */
    volatile uint32_t FTSR;    /* 0x0C 下降沿触发选择 */
    volatile uint32_t SWIER;   /* 0x10 软件中断事件 */
    volatile uint32_t PR;      /* 0x14 挂起标志（★ 写 1 清除） */
} EXTI_TypeDef;

/* ============================================================
 *  TIM —— 通用定时器（TIM2/3/4）
 *  TIM2 基地址 0x40000000，TIM3 0x40000400，TIM4 0x40000800
 * ============================================================ */
typedef struct
{
    volatile uint32_t CR1;      /* 0x00 控制寄存器 1（CEN 在 bit0）*/
    volatile uint32_t CR2;      /* 0x04 */
    volatile uint32_t SMCR;     /* 0x08 从模式控制 */
    volatile uint32_t DIER;     /* 0x0C DMA/中断使能（UIE 在 bit0）*/
    volatile uint32_t SR;       /* 0x10 状态寄存器（UIF 在 bit0，★写0清）*/
    volatile uint32_t EGR;      /* 0x14 事件产生（UG 在 bit0）*/
    volatile uint32_t CCMR1;    /* 0x18 捕获/比较模式 1（CH1/CH2）*/
    volatile uint32_t CCMR2;    /* 0x1C 捕获/比较模式 2（CH3/CH4）*/
    volatile uint32_t CCER;     /* 0x20 捕获/比较使能 */
    volatile uint32_t CNT;      /* 0x24 计数器 */
    volatile uint32_t PSC;      /* 0x28 预分频器 */
    volatile uint32_t ARR;      /* 0x2C 自动重装载 */
    volatile uint32_t RCR;      /* 0x30 重复计数（高级定时器用）*/
    volatile uint32_t CCR1;     /* 0x34 捕获/比较 1（PWM 占空比）*/
    volatile uint32_t CCR2;     /* 0x38 */
    volatile uint32_t CCR3;     /* 0x3C */
    volatile uint32_t CCR4;     /* 0x40 */
	volatile uint32_t BDTR;		/* 0x44 */
} TIM_TypeDef;

/* ============================================================
 *  GPIO —— 通用输入输出（基地址 0x40010800 + 端口偏移）
 * ============================================================ */
typedef struct
{
    volatile uint32_t CRL;    /* 0x00  配置 P0~P7  */
    volatile uint32_t CRH;    /* 0x04  配置 P8~P15 */
    volatile uint32_t IDR;    /* 0x08  输入数据    */
    volatile uint32_t ODR;    /* 0x0C  输出数据    */
    volatile uint32_t BSRR;   /* 0x10  位设置/复位 */
    volatile uint32_t BRR;    /* 0x14  位复位      */
    volatile uint32_t LCKR;   /* 0x18  配置锁定    */
} GPIO_TypeDef;

/* ============================================================
 *  GPIO 操作（static inline：放头文件里，编译时内联，无函数调用开销）
 * ============================================================ */

/* 配置引脚模式和速度*/
static inline void gpio_config(GPIO_TypeDef *port, uint32_t pin, uint32_t cfg)
{
	if(pin>=16){return;}
    if(pin<8)
	{
		port->CRL &= ~(0x0FUL<<(pin*4));
		port->CRL |= ((cfg & 0x0FUL) << (pin*4));
	}
	else
	{
		port->CRH &= ~(0x0FUL<<((pin-8)*4));
		port->CRH |= ((cfg & 0x0FUL) << ((pin-8)*4));
	}
}

/* 置位：引脚输出高电平 */
static inline void gpio_set(GPIO_TypeDef *port,uint32_t pin)
{
	port->BSRR=(1UL<<pin);
}

/* 复位：引脚输出低电平 */
static inline void gpio_reset(GPIO_TypeDef *port,uint32_t pin)
{
	port->BSRR=(1UL<<(pin+16));
}

/* 翻转：高变低、低变高*/
static inline void gpio_toggle(GPIO_TypeDef *port,uint32_t pin)
{
	if(port->ODR&(1UL<<pin))
	{
		gpio_reset(port,pin);
	}
	else
	{
		gpio_set(port,pin);
	}
}

/* 读引脚电平：返回 0 或 1 */
static inline uint32_t gpio_read(GPIO_TypeDef *port, uint32_t pin)
{
    if((port->IDR&(1UL<<pin))==0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

#endif 
