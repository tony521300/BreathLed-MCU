#include "BreathLed.h"

unsigned char breath_open_flag = 0;
/* 每125us调用一次 */
void LED_Breath_Process(void)
{
    unsigned char led_on;

    /* 根据状态控制呼吸灯 */
    BreathLed_Enable(breath_open_flag);

    // 如果其他业务共用LED 直接返回 避免影响
    if (breath_open_flag == 0)
    {
        return;
    }
    /* 获取当前PWM输出状态 */
    led_on = BreathLed_Process();

    /* 1. 先关闭所有COM */
    // ll_gpio_set_bits(GPIOB, LL_GPIO_PIN_0);

    /* 2. 根据PWM状态控制SEG */
    if (led_on)
    {
        // 开
        // ll_gpio_set_bits(GPIOA, LL_GPIO_PIN_4 | LL_GPIO_PIN_5 | LL_GPIO_PIN_6);
    }
    else
    {
        // 关
        // ll_gpio_reset_bits(GPIOA, LL_GPIO_PIN_4 | LL_GPIO_PIN_5 | LL_GPIO_PIN_6);
    }
}

int main(void)
{
    /* 初始化 */
    BreathLed_Init();

    while (1)
    {
    }
}
