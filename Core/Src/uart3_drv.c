#include "uart3_drv.h"
#include "SEGGER_RTT.h"
#include "bridge_proto_cfg.h"
UART_HandleTypeDef huart3;
static void UART3_GPIO_Clock_NVIC_Init(void)
{
    /* 时钟 */
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();     // 默认 PB10/PB11
    __HAL_RCC_USART3_CLK_ENABLE();


    /* TX: PB10 AF-PP，高速 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin   = GPIO_PIN_10;
    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* RX: PB11 输入浮空（F1 系列用输入+上/下拉由 ODR 决定；HAL 里用 Pull 参数模拟） */
    GPIO_InitStruct.Pin  = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;   // 如线较长、干扰大，可改为 GPIO_PULLUP
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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