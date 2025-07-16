#include "ADXL345.h"

// SPI读寄存器
static uint8_t ADXL345_ReadReg(uint8_t reg)
{
    uint8_t tx = reg | 0x80; // 设置读位
    uint8_t rx = 0;

    ADXL345_CS_LOW();
    HAL_SPI_Transmit(&hspi2, &tx, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi2, &rx, 1, HAL_MAX_DELAY);
    ADXL345_CS_HIGH();

    return rx;
}

// SPI写寄存器
static void ADXL345_WriteReg(uint8_t reg, uint8_t data)
{
    uint8_t tx[2] = {reg & 0x7F, data}; // 确保写位清零

    ADXL345_CS_LOW();
    HAL_SPI_Transmit(&hspi2, tx, 2, HAL_MAX_DELAY);
    ADXL345_CS_HIGH();
}

// 读取设备ID（期望返回0xE5）
uint8_t ADXL345_ReadID(void)
{
    return ADXL345_ReadReg(ADXL345_DEVID);
}

// 初始化ADXL345
void ADXL345_Init(void)
{
    HAL_Delay(10); // 启动延迟

    ADXL345_WriteReg(ADXL345_BW_RATE, 0x0A);     // 设置输出速率为100Hz
    ADXL345_WriteReg(ADXL345_DATA_FORMAT, 0x0A); // 设置为 ±8g + 全分辨率
    ADXL345_WriteReg(ADXL345_POWER_CTL, 0x08);   // 进入测量模式
}

// 读取三轴数据
void ADXL345_ReadXYZ(int16_t *x, int16_t *y, int16_t *z)
{
    uint8_t tx = ADXL345_DATAX0 | 0xC0; // 多字节 + 读操作
    uint8_t buf[6];

    ADXL345_CS_LOW();
    HAL_SPI_Transmit(&hspi2, &tx, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi2, buf, 6, HAL_MAX_DELAY);
    ADXL345_CS_HIGH();

    *x = (int16_t)((buf[1] << 8) | buf[0]);
    *y = (int16_t)((buf[3] << 8) | buf[2]);
    *z = (int16_t)((buf[5] << 8) | buf[4]);
}
float ADXL345_ConvertToG(int16_t raw)
{
    return raw * 0.0039f;
}