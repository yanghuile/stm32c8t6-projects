#ifndef EXTI_DRV_H
#define EXTI_DRV_H

#include <stdint.h>
#include "stm32f103.h"

typedef enum
{
    EXTI_EDGE_RISING  = 0,
    EXTI_EDGE_FALLING = 1,
    EXTI_EDGE_BOTH    = 2
} ExtiEdge_t;

int EXTI_Attach(GPIO_TypeDef *port, uint8_t pin, ExtiEdge_t edge, uint8_t prio);
uint32_t EXTI_GetCount(uint8_t pin);
void EXTI_ClearCount(uint8_t pin);
void EXTI_Enable(uint8_t pin);
void EXTI_Disable(uint8_t pin);

#endif
