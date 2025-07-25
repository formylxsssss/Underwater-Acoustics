/* dht22.h */
#ifndef __DHT22_H
#define __DHT22_H

#include "stm32f1xx_hal.h"
#include "stdint.h"
// DATA 引脚定义
#define DHT22_GPIO_Port    GPIOC
#define DHT22_Pin          GPIO_PIN_2

/**
 * @brief  初始化 DHT22 驱动（配置引脚，启用 DWT 延时）
 */
void DHT22_Init(void);

/**
 * @brief  读取一次温湿度数据
 * @param  temperature 指针，输出温度值，单位 ℃
 * @param  humidity    指针，输出湿度值，单位 %RH
 * @return 0 = 成功，其他 = 错误
 */
uint8_t DHT22_ReadData(float *temperature, float *humidity);

#endif /* __DHT22_H */
