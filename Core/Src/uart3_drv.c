#include "uart3_drv.h"
#include <string.h>
#include "stm32f1xx_hal_uart.h"
#include "SEGGER_RTT.h"
/* ==== 句柄 ==== */
UART_HandleTypeDef huart3;

/* ==== RX 环形缓冲（根据需要可调大） ==== */
#ifndef UART3_RX_BUF_SIZE
#define UART3_RX_BUF_SIZE 512
#endif
static volatile uint8_t  rx_buf[UART3_RX_BUF_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

/* ===== 内部函数 ===== */
static void UART3_GPIO_Clock_NVIC_Init(void)
{
    /* 时钟 */
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();     // 默认 PB10/PB11
    __HAL_RCC_USART3_CLK_ENABLE();


    /* TX: PB10 AF-PP，高速 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin   = UART3_TX_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(UART3_TX_PORT, &GPIO_InitStruct);

    /* RX: PB11 输入浮空（F1 系列用输入+上/下拉由 ODR 决定；HAL 里用 Pull 参数模拟） */
    GPIO_InitStruct.Pin  = UART3_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;   // 如线较长、干扰大，可改为 GPIO_PULLUP
    HAL_GPIO_Init(UART3_RX_PORT, &GPIO_InitStruct);

    /* NVIC */
    HAL_NVIC_SetPriority(USART3_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
}

/* ===== 对外函数实现 ===== */

void USART3_Driver_Init(uint32_t baudrate)
{
    UART3_GPIO_Clock_NVIC_Init();

    huart3.Instance          = USART3;
    huart3.Init.BaudRate     = baudrate;
    huart3.Init.WordLength   = UART_WORDLENGTH_8B;
    huart3.Init.StopBits     = UART_STOPBITS_1;
    huart3.Init.Parity       = UART_PARITY_NONE;
    huart3.Init.Mode         = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart3) != HAL_OK)
    {
        /* 这里按需处理错误 */
        myprintf("uart_init_error\n");
    }
    /* 打开接收中断（RXNE + 错误） */
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_ERR);
}

/* 阻塞发送 */
void USART3_Write(const uint8_t *data, size_t len)
{
    if (!data || !len) return;
    HAL_UART_Transmit(&huart3, (uint8_t*)data, (uint16_t)len, HAL_MAX_DELAY);
}

int USART3_WriteString(const char *s , size_t srting_len)
{
    
    if (!s) return 0;
    USART3_Write((const uint8_t*)s, srting_len);
    return 1;
}

void USART3_WriteByte(uint8_t b)
{
    HAL_UART_Transmit(&huart3, &b, 1, HAL_MAX_DELAY);
}

/* 可读字节数 */
size_t USART3_Available(void)
{
    uint16_t head = rx_head;
    uint16_t tail = rx_tail;
    return (head >= tail) ? (head - tail) : (UART3_RX_BUF_SIZE - (tail - head));
}

/* 取 1 字节（没数据返回 -1） */
int USART3_GetChar(void)
{
    if (rx_head == rx_tail) return -1;
    uint8_t b = rx_buf[rx_tail];
    rx_tail = (uint16_t)((rx_tail + 1) % UART3_RX_BUF_SIZE);
    return (int)b;
}

/* 取 1 字节（成功1/无数据0） */
int USART3_ReadByte(uint8_t *out)
{
    if (!out) return 0;
    int c = USART3_GetChar();
    if (c < 0) return 0;
    *out = (uint8_t)c;
    return 1;
}

/* 清空 RX 缓冲 */
void USART3_FlushRx(void)
{
    rx_head = rx_tail = 0;
}

/* ===== 中断服务函数（放这里即可，启动文件里有弱定义，会被此处覆盖） ===== */
void USART3_IRQHandler(void)
{
    uint32_t isr = huart3.Instance->SR;   // SR 先读
    if (isr & (USART_SR_RXNE | USART_SR_ORE | USART_SR_NE | USART_SR_FE | USART_SR_PE))
    {
        uint8_t d = (uint8_t)(huart3.Instance->DR & 0xFF); // 读 DR 清 RXNE/错误
        uint16_t next = (uint16_t)((rx_head + 1) % UART3_RX_BUF_SIZE);
        if (next != rx_tail) {            // 缓冲未满
            rx_buf[rx_head] = d;
            rx_head = next;
        } else {
            /* 缓冲溢出：可在此丢弃、计数或覆盖旧数据 */
        }
    }

    /* TXE/TC 如需做非阻塞发送，可在此扩展 */
    /* 清其它可能的标志（一般读SR后读DR已清除常见错误位） */
}
