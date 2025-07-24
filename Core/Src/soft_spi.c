#include "soft_spi.h"
#include "diff_signal.h"

void SoftSPI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // CLK 和 CS
    GPIO_InitStruct.Pin = SPI_SOFT_SCK_PIN | SPI_SOFT_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(SPI_SOFT_PORT, &GPIO_InitStruct);

    // MOSI
    GPIO_InitStruct.Pin = SPI_SOFT_MOSI_PIN;
    HAL_GPIO_Init(SPI_SOFT_MOSI_PORT, &GPIO_InitStruct);

    // MISO
    GPIO_InitStruct.Pin = SPI_SOFT_MISO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(SPI_SOFT_PORT, &GPIO_InitStruct);

    // 默认状态
    HAL_GPIO_WritePin(SPI_SOFT_PORT, SPI_SOFT_SCK_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SPI_SOFT_PORT, SPI_SOFT_CS_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SPI_SOFT_MOSI_PORT, SPI_SOFT_MOSI_PIN, GPIO_PIN_RESET);
}

uint32_t SoftSPI_TransferBits(uint32_t data_out, uint8_t bitlen)
{
    uint32_t data_in = 0;

    for (int i = bitlen - 1; i >= 0; i--)
    {
        HAL_GPIO_WritePin(SPI_SOFT_MOSI_PORT, SPI_SOFT_MOSI_PIN,
                          (data_out >> i) & 0x01 ? GPIO_PIN_SET : GPIO_PIN_RESET);

        DWT_Delay_us(1);
        HAL_GPIO_WritePin(SPI_SOFT_PORT, SPI_SOFT_SCK_PIN, GPIO_PIN_SET);  // 上升沿
        DWT_Delay_us(1);

        data_in <<= 1;
        if (HAL_GPIO_ReadPin(SPI_SOFT_PORT, SPI_SOFT_MISO_PIN))
            data_in |= 1;

        HAL_GPIO_WritePin(SPI_SOFT_PORT, SPI_SOFT_SCK_PIN, GPIO_PIN_RESET); // 下降沿
         HAL_GPIO_WritePin(SPI_SOFT_MOSI_PORT, SPI_SOFT_MOSI_PIN,GPIO_PIN_RESET);

        DWT_Delay_us(1);
    }

    return data_in;
}
