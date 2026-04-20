#ifndef BRIDGE_PROTO_CFG_H
#define BRIDGE_PROTO_CFG_H

#include "stm32f1xx_hal.h"

/* ========================= 用户必须根据工程确认/修改的部分 ========================= */
/* 1) 485串口句柄：本工程固定为 USART3 -> huart3 */
#define BRIDGE_RS485_HUART                huart3

/* 2) 485收发方向控制引脚：本工程固定为 PB8 */
#define BRIDGE_RS485_DE_GPIO_Port         GPIOB
#define BRIDGE_RS485_DE_Pin               GPIO_PIN_8

/* 3) 如果高电平发送，则为1；如果低电平发送，则为0 */
#define BRIDGE_RS485_DE_ACTIVE_HIGH       1

/* 4) 本机485地址 */
#define BRIDGE_DEVICE_ADDR                0x01u

/* 5) 485串口默认波特率（仅影响 USART3 初始化，不需要和水下100bps一致） */
#define BRIDGE_RS485_BAUDRATE             9600u

/* ========================= 可选配置 ========================= */
#define BRIDGE_MAX_PAYLOAD_LEN            64u
#define BRIDGE_MAX_FRAME_RAW_LEN          (1u + 1u + 1u + 1u + BRIDGE_MAX_PAYLOAD_LEN + 2u)
#define BRIDGE_MAX_FRAME_ENCODED_LEN      (BRIDGE_MAX_FRAME_RAW_LEN + (BRIDGE_MAX_FRAME_RAW_LEN / 254u) + 3u + 2u)

#define BRIDGE_RS485_RX_FIFO_SIZE         256u
#define BRIDGE_RS485_PARSER_BUF_SIZE      128u

#define BRIDGE_ACOUSTIC_TX_QUEUE_DEPTH    8u
#define BRIDGE_ACOUSTIC_RX_QUEUE_DEPTH    8u

/* 485发送超时 */
#define BRIDGE_RS485_TX_TIMEOUT_MS        200u

/* 水下发送超时，仅用于防止状态机卡死 */
#define BRIDGE_ACOUSTIC_TX_TIMEOUT_MS     5000u

/* 是否开启485调试应答中的状态字节 */
#define BRIDGE_ENABLE_VERBOSE_STATUS      1

/* ========================= 协议命令定义 ========================= */
#define BRIDGE_CMD_SEND_ACOUSTIC          0x01u
#define BRIDGE_CMD_GET_STATUS             0x02u
#define BRIDGE_CMD_FETCH_RX               0x03u
#define BRIDGE_CMD_SET_PARAM              0x04u
#define BRIDGE_CMD_PING                   0x05u

#define BRIDGE_RSP_ACK                    0x80u
#define BRIDGE_RSP_NACK                   0x81u
#define BRIDGE_RSP_BUSY                   0x82u
#define BRIDGE_RSP_STATUS                 0x83u
#define BRIDGE_RSP_RX_DATA                0x84u
#define BRIDGE_RSP_PONG                   0x85u

/* ========================= 错误码定义 ========================= */
#define BRIDGE_ERR_NONE                   0x00u
#define BRIDGE_ERR_BAD_ADDR               0x01u
#define BRIDGE_ERR_BAD_CRC                0x02u
#define BRIDGE_ERR_BAD_LEN                0x03u
#define BRIDGE_ERR_BAD_CMD                0x04u
#define BRIDGE_ERR_QUEUE_FULL             0x05u
#define BRIDGE_ERR_PARAM                  0x06u
#define BRIDGE_ERR_EMPTY                  0x07u
#define BRIDGE_ERR_TX_BUSY                0x08u

#endif /* BRIDGE_PROTO_CFG_H */
