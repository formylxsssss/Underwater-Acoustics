#include "rs485_port.h"
#include "ring_fifo.h"
#include "SEGGER_RTT.h"
extern UART_HandleTypeDef BRIDGE_RS485_HUART;

static RingFifo_t s_rs485_rx_fifo;
static uint8_t s_rs485_rx_fifo_mem[BRIDGE_RS485_RX_FIFO_SIZE];
static uint8_t s_rx_byte;

static void RS485_Port_SetTxMode(void)
{
#if BRIDGE_RS485_DE_ACTIVE_HIGH
    HAL_GPIO_WritePin(BRIDGE_RS485_DE_GPIO_Port, BRIDGE_RS485_DE_Pin, GPIO_PIN_SET);
#else
    HAL_GPIO_WritePin(BRIDGE_RS485_DE_GPIO_Port, BRIDGE_RS485_DE_Pin, GPIO_PIN_RESET);
#endif
}

static void RS485_Port_SetRxMode(void)
{
#if BRIDGE_RS485_DE_ACTIVE_HIGH
    HAL_GPIO_WritePin(BRIDGE_RS485_DE_GPIO_Port, BRIDGE_RS485_DE_Pin, GPIO_PIN_RESET);
#else
    HAL_GPIO_WritePin(BRIDGE_RS485_DE_GPIO_Port, BRIDGE_RS485_DE_Pin, GPIO_PIN_SET);
#endif
}

void RS485_Port_Init(void)
{
    RingFifo_Init(&s_rs485_rx_fifo, s_rs485_rx_fifo_mem, sizeof(s_rs485_rx_fifo_mem));
    RS485_Port_SetRxMode();
}

void RS485_Port_StartRxIt(void)
{
    uint32_t error_code = 0x00;
    error_code = HAL_UART_Receive_IT(&BRIDGE_RS485_HUART, &s_rx_byte, 1u);
    if(error_code != HAL_OK)
    {
    myprintf("?????\n");

    }
}

void RS485_Port_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart != &BRIDGE_RS485_HUART)
    {
        return;
    }

    (void)RingFifo_PushByte(&s_rs485_rx_fifo, s_rx_byte);
    (void)HAL_UART_Receive_IT(&BRIDGE_RS485_HUART, &s_rx_byte, 1u);
}

void RS485_Port_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart != &BRIDGE_RS485_HUART)
    {
        return;
    }

    __HAL_UART_CLEAR_PEFLAG(&BRIDGE_RS485_HUART);
    __HAL_UART_CLEAR_FEFLAG(&BRIDGE_RS485_HUART);
    __HAL_UART_CLEAR_NEFLAG(&BRIDGE_RS485_HUART);
    __HAL_UART_CLEAR_OREFLAG(&BRIDGE_RS485_HUART);
    (void)HAL_UART_AbortReceive(&BRIDGE_RS485_HUART);
    (void)HAL_UART_Receive_IT(&BRIDGE_RS485_HUART, &s_rx_byte, 1u);
}

bool RS485_Port_ReadByte(uint8_t *byte)
{
     bool ok;

    if (byte == NULL)
    {
        myprintf("byte is null\n");
        return false;
    }

    ok = RingFifo_PopByte(&s_rs485_rx_fifo, byte);

    return ok;
}

bool RS485_Port_SendBytes(const uint8_t *data, uint16_t len)
{
    if ((data == NULL) || (len == 0u))
    {
        return false;
    }

    RS485_Port_SetTxMode();
    uint32_t error_code  = 0x00;
     error_code =   HAL_UART_Transmit(&BRIDGE_RS485_HUART, (uint8_t *)data, len, 1000);
    if(error_code!= HAL_OK)
    {
        myprintf("send_error%d\n",error_code);
        RS485_Port_SetRxMode();
        return false;

    }
    

    while (__HAL_UART_GET_FLAG(&BRIDGE_RS485_HUART, UART_FLAG_TC) == RESET)
    {
    }

    RS485_Port_SetRxMode();
    return true;
}
