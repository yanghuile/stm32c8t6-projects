#ifndef STM32F103_H
#define STM32F103_H

#include <stdint.h>

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

#define RCC  ((RCC_TypeDef *)0x40021000UL)

/* APB2ENR 位定义 */
#define RCC_APB2ENR_IOPAEN    (1UL << 2)
#define RCC_APB2ENR_IOPBEN    (1UL << 3)
#define RCC_APB2ENR_IOPCEN    (1UL << 4)
#define RCC_APB2ENR_IOPDEN    (1UL << 5)
#define RCC_APB2ENR_IOPEEN    (1UL << 6)
#define RCC_APB2ENR_AFIOEN    (1UL << 0)

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

#endif 
