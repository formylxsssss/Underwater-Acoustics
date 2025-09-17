#include "tim.h"
#include <stdint.h>
#include <stdlib.h>
#include "diff_signal.h"
#include "diff_signal_rx.h"
#include "SEGGER_RTT.h"

// 初始化 DWT 微秒延迟功能
void DWT_Delay_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}
/**
 * @brief 使用dwt进行us级延时，已经验证过，没有问题；
 *
 * @param us
 */
void DWT_Delay_us(uint32_t us)
{
    uint32_t cycles = (SystemCoreClock / 1000000L) * us;
    uint32_t start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < cycles)
        ;
}

void DiffSignal_Init(void)
{
    DiffSignal_Stop();
    // 启用微秒延迟模块
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
    DWT_Delay_Init();
}

void DiffSignal_Send(uint16_t duration_us)
{
    uint32_t arr1   = __HAL_TIM_GET_AUTORELOAD(&htim1);
    uint32_t pulse1 = (arr1 + 1) / 2;                 // 50% 占空比

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse1); // CH1 与 CH1N 同时受 CCR1 控制

    DWT_Delay_us(duration_us);
    DiffSignal_Stop();
}

void DiffSignal_Stop(void)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);  // 置 0% 占空比；CH1/CH1N 同时静默
}

void DiffSignal_Senddata(uint8_t *data, uint16_t length)
{
    // 开启 TIM1 CH1 主路与互补路 CH1N
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

    for (uint16_t i = 0; i < length; i++)
    {
        uint8_t byte = data[i];

        // 起始位 0
        DWT_Delay_us(BIT_DURATION_US);

        for (int bit = 0; bit < 8; bit++)
        {
            if ((byte >> bit) & 0x01)
            {
                DiffSignal_Send(BIT_DURATION_US);   // 输出一个比特时间的载波（CH1 与 CH1N 互补）
            }
            else
            {
                DWT_Delay_us(BIT_DURATION_US);      // 空闲（0% 占空比）
            }
        }
        // 停止位 1
        DiffSignal_Send(BIT_DURATION_US);
    }

    // 全部发完后关闭互补与主通道
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
}
