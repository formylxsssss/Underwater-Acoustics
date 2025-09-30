#ifndef __USART3_DRV_H__
#define __USART3_DRV_H__

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* === 引脚默认使用 USART3 的“无重映射”：PB10=TX, PB11=RX ===
   如需改到 PC10/PC11（部分重映射）或 PD8/PD9（全重映射，100脚封装），
   在 usart3_drv.c 里启用对应的 __HAL_AFIO_REMAP_USART3_PARTIAL / _FULL 宏。 */
#ifndef UART3_TX_PORT
#define UART3_TX_PORT   GPIOB
#define UART3_TX_PIN    GPIO_PIN_10
#define UART3_RX_PORT   GPIOB
#define UART3_RX_PIN    GPIO_PIN_11
#define UART_BAUD_RARE   9600
#endif

/* ====== 对外 API ====== */
void    USART3_Driver_Init(uint32_t baudrate);
void uart_modbus_poll(uint8_t poll_status,void *call_back);
/* 发送 */
void    USART3_Write(const uint8_t *data, size_t len);
int USART3_WriteString(const char *s , size_t srting_len);
void    USART3_WriteByte(uint8_t b);

/* 接收（中断环形缓冲区） */
size_t  USART3_Available(void);               // 可读字节数
int     USART3_GetChar(void);                 // 无数据返回 -1
int     USART3_ReadByte(uint8_t *out);        // 成功返回1，无数据返回0
void    USART3_FlushRx(void);


#ifdef __cplusplus
}
#endif
#endif /* __USART3_DRV_H__ */
