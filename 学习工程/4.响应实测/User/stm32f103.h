#ifndef STM32F103_H
#define STM32F103_H

#include <stdint.h>

#define RCC  ((RCC_TypeDef *)0x40021000UL)
#define AFIO  ((AFIO_TypeDef *)0x40010000UL)
#define EXTI  ((EXTI_TypeDef *)0x40010400UL)

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
#define NVIC_ISER(idx) ((idx) < 32 ? (NVIC_ISER0 = (1UL << (idx))) \
                                   : (NVIC_ISER1 = (1UL << ((idx) - 32))))

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


/* ============================================================
 *  DWT / CoreDebug —— CPU 周期计数器（测 ISR 耗时用）
 * ============================================================ */
#define CoreDebug_DEMCR  (*(volatile uint32_t *)0xE000EDFCUL)
#define DWT_CTRL         (*(volatile uint32_t *)0xE0001000UL)
#define DWT_CYCCNT       (*(volatile uint32_t *)0xE0001004UL)

#endif 
