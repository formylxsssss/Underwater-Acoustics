#include "tim.h"
#include <stdint.h>
#include <stdlib.h>
#include "diff_signal.h"

// 初始化 DWT 微秒延迟功能
void DWT_Delay_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void DWT_Delay_us(uint32_t us)
{
    uint32_t cycles = (SystemCoreClock / 1000000L) * us;
    uint32_t start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < cycles)
        ;
}

void DiffSignal_Init(void)
{
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

    // 反相 PA7 对应的 TIM3_CH2
    TIM3->CCER &= ~TIM_CCER_CC2P;
    TIM3->CCER |= TIM_CCER_CC2P;

    DiffSignal_Stop();

    // 启用微秒延迟模块
    DWT_Delay_Init();
}

void DiffSignal_Send(uint16_t duration_us)
{
    uint32_t pulse = htim1.Init.Period / 2;

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pulse);

    DWT_Delay_us(duration_us);

    DiffSignal_Stop();
}

void DiffSignal_Stop(void)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
}

void DiffSignal_Senddata(uint8_t *data, uint16_t length)
{
    for (uint16_t i = 0; i < length; i++)
    {
        uint8_t byte = data[i];

        for (int bit = 7; bit >= 0; bit--)
        {
            if ((byte >> bit) & 0x01)
            {
                DiffSignal_Send(BIT_DURATION_US);
            }
            else
            {
                DWT_Delay_us(BIT_DURATION_US);
            }
        }
    }
}
