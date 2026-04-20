#ifndef __BRIDGE_APP_H
#define __BRIDGE_APP_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void BridgeApp_Init(void);
void BridgeApp_Process(void);

bool BridgeApp_PushAcousticRxPacket(const uint8_t *data, uint8_t len);

void BridgeApp_UartRxCpltCallback(UART_HandleTypeDef *huart);
void BridgeApp_UartErrorCallback(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif