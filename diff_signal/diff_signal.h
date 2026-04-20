#ifndef __DIFF_SIGNAL_H
#define __DIFF_SIGNAL_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f1xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DIFF_SIGNAL_TX_BITS_MAX    2048u

bool DiffSignal_Init(void);
bool DiffSignal_SetBitRate(uint32_t bps);
bool DiffSignal_SetBitPeriodUs(uint32_t bit_period_us);

uint32_t DiffSignal_GetBitRate(void);
uint32_t DiffSignal_GetBitPeriodUs(void);

bool DiffSignal_IsBusy(void);
void DiffSignal_Stop(void);

/* 新增：直接发送 bit 流，bits[i] 只能是 0 或 1 */
bool DiffSignal_SendBitStream(const uint8_t *bits, uint16_t bit_count);

/* 保留兼容接口 */
bool DiffSignal_SendByte(uint8_t data);
bool DiffSignal_SendBuffer(const uint8_t *buf, uint16_t len);

/* 给 TIM4 中断调用 */
void DiffSignal_TimerPeriodElapsedCallback(TIM_HandleTypeDef *htim);

/* 兼容你旧接口 */
void send_data(uint8_t data);

#ifdef __cplusplus
}
#endif

#endif