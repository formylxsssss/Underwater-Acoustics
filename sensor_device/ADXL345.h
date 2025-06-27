#ifndef __ADXL345_H
#define __ADXL345_H

#include "stm32f1xx_hal.h"

// 使用外部提供的 SPI 句柄
extern SPI_HandleTypeDef hspi2;

// ADXL345连接的CS引脚（如PB12）
#define ADXL345_CS_GPIO GPIOB
#define ADXL345_CS_PIN GPIO_PIN_12

#define ADXL345_CS_LOW() HAL_GPIO_WritePin(ADXL345_CS_GPIO, ADXL345_CS_PIN, GPIO_PIN_RESET)
#define ADXL345_CS_HIGH() HAL_GPIO_WritePin(ADXL345_CS_GPIO, ADXL345_CS_PIN, GPIO_PIN_SET)

// ADXL345寄存器定义
#define ADXL345_DEVID 0x00
#define ADXL345_POWER_CTL 0x2D
#define ADXL345_DATA_FORMAT 0x31
#define ADXL345_BW_RATE 0x2C
#define ADXL345_DATAX0 0x32

// 函数接口
uint8_t ADXL345_ReadID(void);
void ADXL345_Init(void);
void ADXL345_ReadXYZ(int16_t *x, int16_t *y, int16_t *z);
float ADXL345_ConvertToG(int16_t raw);
#endif
