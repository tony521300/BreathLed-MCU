/**
 * @file    BreathLed.h
 * @brief   呼吸灯模块公共接口
 *
 * @details 纯算法型呼吸灯驱动模块，不依赖任何硬件外设。
 *          - 内部状态全部封装于 BreathLed.c 的文件级 static 变量，模块外部不可见；
 *          - 对外仅暴露三个函数，无全局变量、无动态内存分配、无浮点运算；
 *          - 模块只负责算法，输出"当前 tick 应当点亮还是熄灭"的电平决策，
 *            实际引脚驱动（PWM 外设 / GPIO / 移位寄存器等）由应用层实现。
 *
 * @author  CNN
 * @version 1.0.0
 * @date    2026-10-21
 *
 * @warning 使用前提：BreathLed_Process() 必须严格每 125us 调用一次。
 *          模块内部没有独立时基，tick 周期本身即为时间基准，
 *          调用频率偏移会等比例地拉伸整个呼吸波形。
 */
#ifndef __BREATH_LED_H__
#define __BREATH_LED_H__

/**
 * @defgroup BreathLed_Module 呼吸灯模块
 * @brief  硬件无关的呼吸灯（渐亮/渐暗）效果驱动
 * @{
 */

/**
 * @brief 呼吸灯 PWM 等级数（亮度分辨率）
 * @note  数值越大，亮度过渡越细腻，但 PWM 频率随之降低：
 *        - 16 -> 500Hz
 *        - 32 -> 250Hz（默认）
 *        - 64 -> 125Hz
 *        PWM 频率计算公式：f = 1 / (BREATH_LED_PWM_MAX × 125us)
 */
#define BREATH_LED_PWM_MAX 32  /* 32级 */

/**
 * @brief 呼吸渐变周期参数（以 tick 为单位）
 * @note  每一级亮度停留时间 = BREATH_LED_FADE_PERIOD × 125us；
 *        一个完整呼吸周期 = 64 × 每一级停留时间
 *        （0→31 渐亮 32 个 tick，31→0 渐暗 32 个 tick）。
 *
 *        参数对照表：
 *
 *          数值    每级变化时间    完整呼吸周期
 *          200     25.0ms          1.60s
 *          300     37.5ms          2.40s   （默认）
 *          400     50.0ms          3.20s
 *          500     62.5ms          4.00s
 *          600     75.0ms          4.80s
 *          700     87.5ms          5.60s
 *          800     100.0ms         6.40s
 *          900     112.5ms         7.20s
 *          1000    125.0ms         8.00s
 *
 * @note  早期版本注释中标注的"约 2.33s"仅统计了亮度发生变化的 62 级，
 *        遗漏了波峰与波谷处翻转 direction 标志所消耗的 2 个 tick，
 *        实际墙钟周期为 64 个 tick（参数 300 时即 2.40s）。
 */
#define BREATH_LED_FADE_PERIOD 300 /* 300 × 125us = 37.5ms/级 */

/**
 * @brief  初始化呼吸灯模块
 * @note   将内部全部状态（使能标志、PWM 计数、当前亮度、渐变方向、
 *         渐变计数）复位至上电初值。应在启用效果之前调用一次。
 */
void BreathLed_Init(void);

/**
 * @brief  开启或关闭呼吸灯效果
 * @param  enable 1：开启呼吸效果；0：关闭呼吸效果
 * @note   传入 0 时会同时复位内部状态，使下次开启时从最暗处重新渐亮。
 * @note   在已使能的情况下重复传入 1 是安全的，不会打断当前呼吸周期，
 *         因此可以在每个 tick 中无条件调用。
 */
void BreathLed_Enable(unsigned char enable);

/**
 * @brief  呼吸灯处理函数，每 125us 调用一次
 * @retval 1：当前 PWM 周期内 LED 应点亮
 * @retval 0：当前 PWM 周期内 LED 应熄灭
 * @note   本函数推进一个 tick 的状态机：PWM 计数、渐变亮度更新、
 *         Gamma 亮度映射，随后返回当前 tick 的输出电平。
 * @warning 必须在定时器中断（或等周期的调度任务）中严格按 125us 周期调用。
 */
unsigned char BreathLed_Process(void);

/** @} */

#endif /* __BREATH_LED_H__ */
