#include "ADXL345.h"

// SPI读寄存器
static uint8_t ADXL345_ReadReg(uint8_t reg)
{
    uint8_t tx = reg | 0x80; // 设置读位
    uint8_t rx = 0;

    ADXL345_CS_LOW();
    
    uint8_t err_code =  HAL_SPI_Transmit(&hspi2, &tx, 1, HAL_MAX_DELAY);
    if(err_code != HAL_OK)
    {
        myprintf("adxl_readreg_transmit is error,code is %d\n",err_code);
    }
    err_code = HAL_SPI_Receive(&hspi2, &rx, 1, HAL_MAX_DELAY);
    if(err_code != HAL_OK)
    {
        myprintf("adxl_readreg_receive is error code is %d\n",err_code);
    }
    ADXL345_CS_HIGH();

    return rx;
}

// SPI写寄存器
static void ADXL345_WriteReg(uint8_t reg, uint8_t data)
{
    uint8_t tx[2] = {reg & 0x7F, data}; // 确保写位清零

    ADXL345_CS_LOW();
    uint8_t err_code =  HAL_SPI_Transmit(&hspi2, tx, 2, HAL_MAX_DELAY);
    if(err_code != HAL_OK)
    {
        myprintf("write_reg_spi_transmit error\n");
    }
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
void ADXL345_ReadXYZ(float *x, float *y, float *z)
{
    int16_t tmp_x,tmp_y,tmp_z =0x00;
    uint8_t tx = ADXL345_DATAX0 | 0xC0; // 多字节 + 读操作
    uint8_t buf[6];

    ADXL345_CS_LOW();
    HAL_SPI_Transmit(&hspi2, &tx, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi2, buf, 6, HAL_MAX_DELAY);
    ADXL345_CS_HIGH();

    tmp_x = (int16_t)((buf[1] << 8) | buf[0]);
    tmp_y = (int16_t)((buf[3] << 8) | buf[2]);
    tmp_z = (int16_t)((buf[5] << 8) | buf[4]);
    *x = tmp_x*0.0039f;
    *y = tmp_y*0.0039f;
    *z = tmp_z*0.0039f;

}
float ADXL345_ConvertToG(int16_t raw)
{
    return raw * 0.0039f;
}