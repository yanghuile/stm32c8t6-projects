#include "exti.h"

static volatile uint32_t s_count[16];   /* 每个 pin 一个计数器 */

int EXTI_Attach(GPIO_TypeDef *port, uint8_t pin, ExtiEdge_t edge, uint8_t prio)
{
    if (pin > 15) return -1;

    /* 开时钟：AFIO + 对应 GPIO 口 */
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    if      (port == GPIOA) RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    else if (port == GPIOB) RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    else if (port == GPIOC) RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    else return -1;

    /* 引脚配为输入（上拉，按键接地）*/
    gpio_set(port, pin);
    gpio_config(port, pin, GPIO_CFG_IN_PULL);

    /* 把 EXTI 线映射到端口 */
    uint8_t port_code = 0;                       /* A=0, B=1, C=2 */
    if      (port == GPIOB) port_code = 1;
    else if (port == GPIOC) port_code = 2;

    uint32_t shift = (pin % 4) * 4;
    AFIO->EXTICR[pin / 4] &= ~(0x0FUL << shift);
    AFIO->EXTICR[pin / 4] |=  ((uint32_t)port_code << shift);

    /* 边沿选择 */
    if (edge == EXTI_EDGE_RISING || edge == EXTI_EDGE_BOTH)
        EXTI->RTSR |=  (1UL << pin);
    else
        EXTI->RTSR &= ~(1UL << pin);

    if (edge == EXTI_EDGE_FALLING || edge == EXTI_EDGE_BOTH)
        EXTI->FTSR |=  (1UL << pin);
    else
        EXTI->FTSR &= ~(1UL << pin);

    /* 解屏蔽 */
    EXTI->IMR |= (1UL << pin);

    /* NVIC：优先级 + 使能 */
    uint32_t irq = (pin <= 4) ? (6 + pin) : ((pin <= 9) ? 23 : 40);
    NVIC_IPR[irq] = (uint8_t)(prio << 4);
    if (irq < 32) NVIC_ISER0 = (1UL << irq);
    else          NVIC_ISER1 = (1UL << (irq - 32));

    return 0;
}

uint32_t EXTI_GetCount(uint8_t pin)   
{
	return (pin > 15) ? 0 : s_count[pin];
}

void  EXTI_ClearCount(uint8_t pin) 
{
	if (pin <= 15) s_count[pin] = 0; 
}

void EXTI_Enable(uint8_t pin)  
{
	if (pin <= 15) EXTI->IMR |=  (1UL << pin); 
}

void EXTI_Disable(uint8_t pin) 
{
	if (pin <= 15) EXTI->IMR &= ~(1UL << pin); 
}

/* ============================================================
 *  中断服务函数：EXTI9_5
 * ============================================================ */
void EXTI9_5_IRQHandler(void)
{
    for (uint8_t pin = 5; pin <= 9; pin++)
    {
        if (EXTI->PR & (1UL << pin))
        {
            EXTI->PR = (1UL << pin);     /* 清标志 */
            s_count[pin]++;               /* 计数，不是标志 */
        }
    }
}
