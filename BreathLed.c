/**
 * @file    BreathLed.c
 * @brief   呼吸灯模块实现
 *
 * @details 纯算法型呼吸灯驱动模块，不依赖任何硬件外设。
 *          全部状态保存在本文件的文件级 static 变量中，模块外部不可见；
 *          应用层只需根据 BreathLed_Process() 的返回值驱动 GPIO 即可。
 *
 * @author  CNN
 * @version 1.0.0
 * @date    2026-10-21
 */
#include "BreathLed.h"

/**
 * @brief Gamma 亮度校正查找表（32 级）
 * @note  人眼对亮度的感知近似 Gamma 曲线（gamma ≈ 2.2），若直接使用线性
 *        亮度值，视觉上会出现"渐亮很快、渐暗很慢"的非线性观感。
 *        该表把线性亮度索引映射为补偿后的 PWM 占空比，使渐变在视觉上
 *        更接近线性。
 * @note  表中最大值为 31，因此最大占空比为 31/32 ≈ 96.9%，
 *        LED 不会达到 100% 全亮。若需真正全亮，可将末项改为 32 并把
 *        BREATH_LED_PWM_MAX 提升为 33，或把比较式改为 pwm_cnt <= pwm_value。
 */
static const unsigned char gamma_lut[BREATH_LED_PWM_MAX] = {
    0, 1, 1, 2, 2, 2, 3, 3,
    4, 4, 4, 5, 5, 6, 6, 7,
    8, 9, 10, 12, 13, 14, 16, 17,
    19, 20, 22, 24, 25, 27, 29, 31};

/**
 * @brief 呼吸灯内部状态
 * @note  全部为文件级 static 变量，模块对外不可见，无需调用者传入或回收。
 * @{
 */

/** @brief 使能标志：0 = 关闭，1 = 开启 */
static unsigned char breath_enable = 0;

/** @brief PWM 计数：0 ~ BREATH_LED_PWM_MAX-1，循环计数，决定当前 tick 是否点亮 */
static unsigned char pwm_cnt = 0;

/** @brief 当前亮度等级：0（最暗）~ BREATH_LED_PWM_MAX-1（最亮） */
static unsigned char bright = 0;

/** @brief 渐变方向：0 = 渐亮，1 = 渐暗 */
static unsigned char direction = 0;

/** @brief 渐变计数：累加至 BREATH_LED_FADE_PERIOD 后更新一次亮度等级 */
static unsigned int fade_cnt = 0;

/** @} */

/**
 * @brief 初始化呼吸灯
 * @note  将内部状态全部复位至上电初值，等待应用层调用 BreathLed_Enable(1) 开启。
 */
void BreathLed_Init(void)
{
    breath_enable = 0;
    pwm_cnt       = 0;
    bright        = 0;
    direction     = 0;
    fade_cnt      = 0;
}

/**
 * @brief 开启或关闭呼吸灯效果
 * @param  enable 1：开启呼吸效果；0：关闭呼吸效果
 * @note  关闭时一并复位内部状态，使下次开启时从最暗处重新渐亮。
 */
void BreathLed_Enable(unsigned char enable)
{
    breath_enable = enable ? 1 : 0;

    if (breath_enable == 0)
    {
        pwm_cnt   = 0;
        bright    = 0;
        direction = 0;
        fade_cnt  = 0;
    }
}

/**
 * @brief 呼吸灯处理函数，每 125us 调用一次
 * @return 当前 PWM 周期内 LED 的输出电平：1 = 点亮，0 = 熄灭
 * @note  处理流程：
 *        1. 未使能时直接返回 0（熄灭）并等待下一次开启；
 *        2. PWM 计数累加并在 32 处回绕，32 × 125us = 4ms，即 250Hz；
 *        3. 渐变计数累加，每满一个 BREATH_LED_FADE_PERIOD 更新一次亮度等级，
 *           到达最亮或最暗时翻转渐变方向；
 *        4. 经 Gamma 查找表映射出实际 PWM 值；
 *        5. 比较 PWM 计数与 PWM 值，返回本 tick 的输出电平。
 */
unsigned char BreathLed_Process(void)
{
    unsigned char pwm_value;

    /* 未使能时，关闭输出并等待下一次开启 */
    if (breath_enable == 0)
    {
        return 0;
    }

    /* 1. PWM计数：32 × 125us = 4ms，频率250Hz */
    pwm_cnt++;

    if (pwm_cnt >= BREATH_LED_PWM_MAX)
    {
        pwm_cnt = 0;
    }

    /* 2. 呼吸亮度变化 */
    fade_cnt++;

    if (fade_cnt >= BREATH_LED_FADE_PERIOD)
    {
        fade_cnt = 0;

        if (direction == 0)
        {
            /* 渐亮 */
            if (bright < BREATH_LED_PWM_MAX - 1)
            {
                bright++;
            }
            else
            {
                direction = 1;
            }
        }
        else
        {
            /* 渐暗 */
            if (bright > 0)
            {
                bright--;
            }
            else
            {
                direction = 0;
            }
        }
    }

    /* 3. Gamma亮度映射 */
    pwm_value = gamma_lut[bright];

    /* 4. 返回当前PWM输出状态，由应用层控制GPIO */
    return (pwm_cnt < pwm_value) ? 1 : 0;
}
