# W8 Day5 —— EXTI 驱动模块化 + ISR 规范

> 目标：把散在 `main.c` 里的中断配置收进一个驱动模块，
> 并总结 ISR 的编写规范。

---

## 一、为什么要拆模块

### 1.1 拆之前的 `main.c`

```c
int main(void)
{
    LED_Init();
    KEY_Init();

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
    AFIO->EXTICR[1] &= ~(0x0FUL << 4);
    AFIO->EXTICR[1] |=  (0x00UL << 4);
    EXTI->FTSR |=  (1UL << 5);
    EXTI->RTSR &= ~(1UL << 5);
    EXTI->IMR  |=  (1UL << 5);
    NVIC_IPR[23]  = (uint8_t)(2 << 4);
    NVIC_ISER0    = (1UL << 23);

    while (1) { ... }
}

void EXTI9_5_IRQHandler(void) { ... }
```

问题：

| # | 问题 |
|---|---|
| 1 | `main.c` 里混着 6 条寄存器操作，业务逻辑被淹没 |
| 2 | 只支持 EXTI5（引脚写死） |
| 3 | ISR 里写死"翻转 LED"，按键驱动知道了 LED 的存在 |
| 4 | 换引脚要改 6 处 |
| 5 | 换一个工程（比如 `4.响应实测`）要把这段代码再抄一遍 |

### 1.2 拆之后的 `main.c`

```c
int main(void)
{
    LED_Init();
    EXTI_Attach(GPIOA, KEY_PIN, EXTI_EDGE_FALLING, 2);   /* 一行 */

    while (1)
    {
        if (EXTI_GetCount(KEY_PIN) > 0)
        {
            EXTI_ClearCount(KEY_PIN);
            Delay_ms(20);
            if (gpio_read(GPIOA, KEY_PIN) == 0) LED_Toggle();
        }
    }
}
```
从「6 条寄存器 + 30 行 ISR」变成「1 行配置 + 10 行业务」。


---

## 二、接口设计

### 2.1 `Hardware/EXTI/exti.h`

```c
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

int      EXTI_Attach(GPIO_TypeDef *port, uint8_t pin, ExtiEdge_t edge, uint8_t prio);
uint32_t EXTI_GetCount(uint8_t pin);
void     EXTI_ClearCount(uint8_t pin);
void     EXTI_Enable(uint8_t pin);
void     EXTI_Disable(uint8_t pin);

#endif
```

### 2.2 设计要点

| 设计 | 理由 |
|---|---|
|`EXTI_Attach()` 一次配完（时钟/引脚/映射/边沿/NVIC） | 调用方不用记 6 步顺序，少一个忘记开 AFIO 时钟的坑 |
| 返回 `int`（0 成功，-1 参数非法） | 失败时调用方能知道，而不是静默出错 |
| `ExtiEdge_t` 枚举，不是裸 0/1/2 | 调用处 `EXTI_EDGE_FALLING` 比 `1` 可读 |
| 按 `pin` 号索引，不用传指针** | `EXTI_GetCount(5)` 比 `EXTI_GetCount(&g_key)` 简单 |
| 提供 `EXTI_Enable/Disable` | 消抖时"临时关掉自己"要用 |
| 计数器不是标志 | 见第四节 |

---

## 三、实现要点

### 3.1 时钟：别忘了 AFIO

```c
RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;      /* ★ EXTI 线映射必须 */
if (port == GPIOA) RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
```

AFIO 是外设，不写进 `EXTICR` 就不会生效 —— 这是 Day 3 踩过的坑，现在由驱动内部统一处理。

### 3.2 EXTICR 的位映射

```c
uint32_t shift = (pin % 4) * 4;
AFIO->EXTICR[pin / 4] &= ~(0x0FUL << shift);
AFIO->EXTICR[pin / 4] |=  ((uint32_t)port_code << shift);
```

| 公式 | 说明 |
|---|---|
| 寄存器下标 `pin / 4` | EXTICR1 管 EXTI0~3，EXTICR2 管 4~7，以此类推 |
| 字段位置 `(pin % 4) * 4` | 每条线占 4 位 |
| 端口编码 `A=0, B=1, C=2, D=3, E=4` | 写进字段 |

例：PA5 → 下标 1（EXTICR2）、位置 4 → bit[7:4]、值 0。

### 3.3 IRQ 号是算出来的，不是查表的

```c
uint32_t irq = (pin <= 4) ? (6 + pin) : ((pin <= 9) ? 23 : 40);
```

| 引脚范围 | IRQ 号 | 向量名 |
|---|---|---|
| pin 0~4 | 6~10（规律：`6 + pin`） | `EXTI0_IRQHandler` … `EXTI4_IRQHandler` |
| pin 5~9 | 23（共用） | `EXTI9_5_IRQHandler` |
| pin 10~15 | 40（共用） | `EXTI15_10_IRQHandler` |

⚠️ 这是 EXTI 最容易出错的地方**：pin 5~9 只有一个向量，
所以 ISR 里必须逐条判断是哪根线触发的。

### 3.4 ISR：循环判断而不是写死

```c
void EXTI9_5_IRQHandler(void)
{
    for (uint8_t pin = 5; pin <= 9; pin++)
    {
        if (EXTI->PR & (1UL << pin))
        {
            EXTI->PR = (1UL << pin);     /* 清标志：写 1 清除 */
            s_count[pin]++;
        }
    }
}
```

为什么用循环：5 条线共享一个向量，一次中断可能是任意一条触发的，
写死 `if (PR & (1<<5))` 的话，pin 6 触发就永远不会被处理。

为什么循环只到 9：`EXTI9_5` 向量只管 5~9，不能多清（会误清其他线的标志）。

---

## 四、关键设计：计数器 vs 布尔标志

### 4.1 布尔标志会丢事件

```c
volatile uint8_t key_flag = 0;          /* ❌ 会丢 */

void ISR(void) { key_flag = 1; }        /* 置 100 次也还是 1 */

while (1)
{
    if (key_flag) { key_flag = 0; ... }  /* 这期间发生的按键全丢 */
}
```

时序：

```
t=0    按第 1 次  → flag = 1
t=0.1  主循环看到 → 清 0 → 开始 Delay 20ms
t=5    按第 2 次  → flag = 1
t=20   Delay 结束 → 处理第 1 次
       此时 flag 已经是 1，会再处理一次 → 侥幸没丢
```

但换成这个顺序就丢了：

```
t=0    按第 1 次  → flag = 1
t=0.1  主循环看到 → 开始 Delay 20ms
t=20   Delay 结束 → 处理 → 然后清 flag
t=19   【Delay 期间按了第 2 次】→ flag 已经是 1，没有区别
t=20   清 flag = 0  ← 第 2 次被抹掉
```

结论：布尔标志只能表达"有没有事件"，不能表达"有几个事件"。

### 4.2 计数器保留事件个数

```c
static volatile uint32_t s_count[16];   /* ✅ */

void ISR(void) { s_count[pin]++; }      /* 每次 +1，一个不丢 */
```

32 位计数器的溢出时间：即使每秒中断 1000 次，也要 49 天才溢出。

### 4.3 但计数器带来新问题：抖动会重复计数

```
按下瞬间抖动产生 3 个下降沿 → s_count += 3 → LED 翻转 3 次 ❌
```

所以计数器和消抖必须配合，有三种做法：

| 方案 | 做法 | 优缺点 |
|---|---|---|
| A. 累计 + 最后清零 | 处理完再清零，抖动期的边沿一起丢掉 | 简单；但会丢"处理期间的真实按键" |
| B. 关中断窗口 | 处理前 `EXTI_Disable()`，处理后 `EXTI_Enable()` | 不阻塞；但窗口期的按下被硬件忽略 |
| C. 时间戳 | 记录上次事件时间，间隔 < 20ms 忽略 | 最专业；需要毫秒时基（W9 定时器） |

当前 `main.c` 用的是 A：

```c
if (EXTI_GetCount(KEY_PIN) > 0)
{
    EXTI_ClearCount(KEY_PIN);       /* 先清零 */
    Delay_ms(20);                    /* 抖动期 20ms 内 ISR 仍在累加 */
    if (gpio_read(...) == 0) LED_Toggle();
}
```

⚠️ 这个顺序有隐患：清零在延时之前，抖动边沿会在延时期间累加进来，
下一圈循环又会处理一次 → 可能重复翻转。

Day 3 的版本没这个问题，因为那时清零放在延时之后（把抖动边沿一起吞掉）。

正确的 A 写法：

```c
if (EXTI_GetCount(KEY_PIN) > 0)
{
    Delay_ms(20);                            /* 先等抖动过去 */

    if (gpio_read(GPIOA, KEY_PIN) == 0)      /* 确实按着 */
    {
        LED_Toggle();
        while (gpio_read(GPIOA, KEY_PIN) == 0) { }   /* 等松手 */
    }

    EXTI_ClearCount(KEY_PIN);                /* ★ 最后清零 */
}
```

---

## 五、ISR 编写七条规范

### 规范 1：短

有量化依据（Day 4 实测）：

| ISR 时长 | 主循环 1ms 延时的实际耗时 | 超时 |
|---|---|---|
| 1.2 μs | 1.001 ms | +0.1% |
| 168 μs | 1.168 ms | +16.8% |
| 1 668 μs | 2.668 ms | +166.8% |

判断标准：

| 控制周期 | ISR 预算 |
|---|---|
| 1 ms | **< 50 μs** |
| 100 μs | **< 5 μs** |
| 只做标志 + 清中断（推荐） | **< 1 μs** |

### 规范 2：不阻塞

```c
void ISR(void)
{
    Delay_ms(20);                      /* ❌ */
    while (gpio_read(...) == 0) { }    /* ❌ 可能等几秒 */
    while (!(USART1->SR & TXE)) { }    /* ❌ 等硬件标志 */
}
```

ISR 执行期间，同级和更低优先级的中断全部无法响应。

### 规范 3：不调用不可重入函数

| 函数 | 能否放 ISR | 原因 |
|---|---|---|
| `printf` | ❌ | 内部静态缓冲 + 可能阻塞 |
| `malloc` / `free` | ❌ | 堆管理器不可重入 |
| `sprintf` | ⚠️ | 目标在栈上能跑，但耗时长 |
| `memcpy` | ✅ | 纯内存操作 |
| `gpio_*` / `rb_put()` | ✅ | 只读写寄存器/数组 |

`printf` 的重入问题：

```
主循环：printf("hello") 执行到一半
   ↓ 中断
ISR：printf("key")
   ↓ 返回
主循环：继续输出 "hello" 的后半截  → 串口输出交错乱码
```

### 规范 4：必须清中断标志

Day 3 实验 B 已亲眼验证：不清 `EXTI->PR` → 程序卡在 ISR 里，主循环永远不执行。

⚠️ 不同外设的清除方式相反**：

| 外设 | 清除方式 |
|---|---|
| `EXTI->PR` | 写 1 清除 |
| `TIM->SR` | 写 0 清除（W9 会撞上） |
| `USART->SR` | 读 SR 再读 DR（序列清除） |

### 规范 5：共享变量必须 `volatile`

```c
volatile uint32_t s_count[16];      /* ★ */
```

没有 `volatile`：

```c
while (s_count[5] == 0) { }        /* 编译器：循环里没人改 → 只读一次 */
```

编译后：

```asm
LDR  r3, [s_count]      ; 只读一次
CMP  r3, #0
BEQ  dead_loop          ; 永远跳回，不再读内存
```

→ ISR 里改了，主循环也看不到。

### 规范 6：不做浮点运算

STM32F103 没有 FPU，`float` 是软件模拟：

| 运算 | 大约周期 |
|---|---|
| `a + b`（int） | 1 |
| `a * b`（float） | 50~200 |
| `sqrtf()` | 500~2000 |

**ISR 里做一次浮点乘法 ≈ 3~14 μs，直接吃掉预算。

### 规范 7：用计数器而不是标志（本日新增）

见第四节。

---

## 六、已知限制（模块当前的缺口）

| # | 限制 | 后果 | 将来怎么补 |
|---|---|---|---|
| 1 | 只实现了 `EXTI9_5_IRQHandler` | 用 pin 0~4 或 10~15 时会跳到启动文件的默认 `Default_Handler`（死循环）→ 程序卡死 | 补 `EXTI0~4_IRQHandler` 和 `EXTI15_10_IRQHandler` |
| 2 | `EXTI_Attach` 的返回值没被检查 | 传错端口时静默失败 | 调用处判断 `if (EXTI_Attach(...) != 0) { 错误处理 }` |
| 3 | 强依赖"上拉输入" | 不适合低电平有效的其他信号源 | 加一个 `pull` 参数 |
| 4 | 没有防重复 Attach | 同一 pin 配两次会重复开 NVIC（无害但浪费） | 加标志位 |
| 5 | ISR 里没有回调机制 | 所有动作都要主循环轮询计数 | 需要时加函数指针回调 |

第 1 条最危险 —— 现在这个模块只能用在 pin 5~9。

---

## 七、踩坑记录

| # | 坑 | 原因 | 解决 |
|---|---|---|---|
| 1 | `EXTI9_5_IRQn` 未定义 | 它在 `stm32f10x.h` 的 `IRQn_Type` 枚举里，不在 `core_cm3.h` | 自己定义 IRQ 号宏 + 直接写 NVIC 寄存器 |
| 2 | `core_cm3.h` 单独 include 报 `unknown type name 'IRQn_Type'` | 它只用不定义，需要设备头文件先提供 | 不用 CMSIS 函数，改为手写 NVIC 寄存器 |
| 3 | 计数器在 Keil Watch 里 `<cannot evaluate>` | 旧调试会话符号残留 | 用 Memory 窗口按固定地址看 |
| 4 | 清零放在延时之前 → 可能重复翻转 | 抖动边沿在延时期间累加 | 清零放到延时之后 + 等松手 |
