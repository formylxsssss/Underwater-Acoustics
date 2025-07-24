#ifndef __SOFT_SPI_H__
#define __SOFT_SPI_H__

#include "stm32f1xx_hal.h"

// 引脚定义
#define SPI_SOFT_SCK_PIN      GPIO_PIN_5
#define SPI_SOFT_MOSI_PIN     GPIO_PIN_5
#define SPI_SOFT_MISO_PIN     GPIO_PIN_6
#define SPI_SOFT_CS_PIN       GPIO_PIN_4

#define SPI_SOFT_PORT         GPIOA
#define SPI_SOFT_MOSI_PORT    GPIOB

void SoftSPI_Init(void);
uint32_t SoftSPI_TransferBits(uint32_t data_out, uint8_t bitlen);

#endif
