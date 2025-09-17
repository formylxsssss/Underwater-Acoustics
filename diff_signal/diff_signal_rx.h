#ifndef __DIFF_SIGNAL_RX_H
#define __DIFF_SIGNAL_RX_H

#include <stdint.h>
#include "main.h"
#include "diff_signal.h"
#include "stm32f1xx_hal_adc.h"
/// 初始化 PA1 EXTI 中断、TIM2定时器 和 ADC 软件触发
void DiffSignal_RX_Init(void);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);
/// 接收到一个完整字节后回调（自行在此处理）
void DiffSignal_Frame_Handler(uint8_t data);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
#define BIT_DURATION_US   833u   // 与发送端保持一致
extern ADC_HandleTypeDef hadc1;
#endif // __DIFF_SIGNAL_RX_H
     