#ifndef __USART_H
#define __USART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern UART_HandleTypeDef huart3;

void USART3_Driver_Init(uint32_t baudrate);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H */
