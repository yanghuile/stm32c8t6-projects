#ifndef STM32F103_H
#define STM32F103_H

#include <stdint.h>

#define RCC_BASE     0x40021000
#define RCC_APB2ENR  *(volatile uint32_t *)(RCC_BASE+0x18)
#define RCC_APB2ENR_IOPAEN   (1<<2)
#define RCC_APB2ENR_IOPBEN   (1<<3)
#define RCC_APB2ENR_IOPCEN   (1<<4)

#define GPIOA_BASE   0x40010800	
#define GPIOA_CRL     *(volatile uint32_t *)(GPIOA_BASE+0x00)
#define GPIOA_CRH     *(volatile uint32_t *)(GPIOA_BASE+0x04)
#define GPIOA_IDR     *(volatile uint32_t *)(GPIOA_BASE+0x08)
#define GPIOA_ODR     *(volatile uint32_t *)(GPIOA_BASE+0x0c)
#define GPIOA_BSRR    *(volatile uint32_t *)(GPIOA_BASE+0x10)
#define GPIOA_BRR     *(volatile uint32_t *)(GPIOA_BASE+0x14)
#define GPIOA_LCKR    *(volatile uint32_t *)(GPIOA_BASE+0x18)

#define GPIOB_BASE   0x40010c00
#define GPIOB_CRL     *(volatile uint32_t *)(GPIOB_BASE+0x00)
#define GPIOB_CRH     *(volatile uint32_t *)(GPIOB_BASE+0x04)
#define GPIOB_IDR     *(volatile uint32_t *)(GPIOB_BASE+0x08)
#define GPIOB_ODR     *(volatile uint32_t *)(GPIOB_BASE+0x0c)
#define GPIOB_BSRR    *(volatile uint32_t *)(GPIOB_BASE+0x10)
#define GPIOB_BRR     *(volatile uint32_t *)(GPIOB_BASE+0x14)
#define GPIOB_LCKR    *(volatile uint32_t *)(GPIOB_BASE+0x18)

#define GPIOC_BASE   0x40011000
#define GPIOC_CRL     *(volatile uint32_t *)(GPIOC_BASE+0x00)
#define GPIOC_CRH     *(volatile uint32_t *)(GPIOC_BASE+0x04)
#define GPIOC_IDR     *(volatile uint32_t *)(GPIOC_BASE+0x08)
#define GPIOC_ODR     *(volatile uint32_t *)(GPIOC_BASE+0x0c)
#define GPIOC_BSRR    *(volatile uint32_t *)(GPIOC_BASE+0x10)
#define GPIOC_BRR     *(volatile uint32_t *)(GPIOC_BASE+0x14)
#define GPIOC_LCKR    *(volatile uint32_t *)(GPIOC_BASE+0x18)

#endif
