#ifndef RS485_PORT_H
#define RS485_PORT_H

#include <stdbool.h>
#include "bridge_proto_cfg.h"

void RS485_Port_Init(void);
void RS485_Port_StartRxIt(void);
void RS485_Port_RxCpltCallback(UART_HandleTypeDef *huart);
void RS485_Port_ErrorCallback(UART_HandleTypeDef *huart);

bool RS485_Port_ReadByte(uint8_t *byte);
bool RS485_Port_SendBytes(const uint8_t *data, uint16_t len);

#endif /* RS485_PORT_H */
