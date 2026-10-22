# W8 Day3 —— EXTI 外部中断

## 一、中断链路
GPIO 电平变化 → AFIO 把 EXTI线 映射到端口 → EXTI 检测边沿 → NVIC 判优先级 → CPU 跳 ISR

## 二、6 步配置（按顺序）
1. RCC->APB2ENR: IOPAEN + AFIOEN（★ AFIO 容易忘）
2. GPIO 配输入模式（上拉）
3. AFIO->EXTICR 映射 EXTI线 → 端口
4. EXTI: FTSR/RTSR 选边沿，IMR 解屏蔽
5. NVIC: 设优先级 + 使能
6. ISR: 判断哪条线 → 清 PR → 做事

## 三、寄存器速查
| 寄存器 | 地址 | 作用 |
|---|---|---|
| AFIO->EXTICR[1] | 0x4001000C | EXTI4~7 映射（bit[7:4] = EXTI5）|
| EXTI->IMR | 0x40010400 | 中断屏蔽 |
| EXTI->FTSR | 0x4001040C | 下降沿触发 |
| EXTI->PR | 0x40010414 | 挂起标志（★写1清除）|

## 四、三个坑
1. 忘记清 PR → 反复进中断
2. 忘记开 AFIO 时钟 → 中断不触发
3. EXTI5~9 共用一个向量 → ISR 里要判断是哪条线

## 五、实验记录
| 实验 | 现象 |
|---|---|
| A 正常 | 按一下翻转一次 |
| B 不清 PR | 按下按键LED不亮 |

## 六、消抖：为什么必须在主循环
- ISR 里不能 Delay（阻塞会拖住所有低优先级中断）
- 我的做法：ISR 只置 volatile 标志，主循环 Delay(20) + 二次确认
- ★ 关键：清标志放在 Delay 之后，把抖动期的多个边沿合并成一次
- 代价：20ms 内的两次真实按下会被合并（人手做不到，无影响）

## 七、错误记录
- `EXTI9_5_IRQn` 不在 core_cm3.h 里（在 stm32f10x.h 的 IRQn_Type 枚举）
- core_cm3.h 单独 include 会报 unknown type name 'IRQn_Type'
- 解法：自己定义 IRQ 号 + 直接写 NVIC 寄存器（NVIC_ISER0 / NVIC_IPR）