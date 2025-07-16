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
   HAL_StatusTypeDef error =  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
   if(error != HAL_OK )
   {
     myprintf("tim11_pwm_start_error\n");
   }
     error = HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    if(error != HAL_OK)
    {
        myprintf("tim33_pwm_start_error\n");
    }
     

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
    // for (uint16_t i = 0; i < length; i++)
    // {
    //     uint8_t byte = data[i];

    //     // 起始位 0
    //     DWT_Delay_us(BIT_DURATION_US); 
 
    //     for (int bit = 0; bit < 8; bit++)
    //     {
    //         if ((byte >> bit) & 0x01)
    //         {
    //             DiffSignal_Send(BIT_DURATION_US);
    //         }
    //         else
    //         {
    //             DWT_Delay_us(BIT_DURATION_US);
    //         }
    //     }
    //     // 停止位 1
    //     DiffSignal_Send(BIT_DURATION_US);
    DiffSignal_Send(1000);
    DWT_Delay_us(100000);

    // }
}
