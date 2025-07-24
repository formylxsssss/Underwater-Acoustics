#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "diff_signal.h"
#include "DHT22.h"
#include "main.h"
#include "Custcom_Pin.h"
#include "SEGGER_RTT.h"


uint8_t DHT22_Read(float *temperature, float *humidity)
{
    uint8_t data[5] = {0};

    // 启动信号 - 拉低18ms
    DHT22_SetOutput();
    HAL_GPIO_WritePin(DHT22_PIN_PORT, DHT22_PIN, GPIO_PIN_RESET);
    HAL_Delay(18);

    // 拉高延时30µs
    HAL_GPIO_WritePin(DHT22_PIN_PORT, DHT22_PIN, GPIO_PIN_SET);
    DWT_Delay_us(30);

    // 切换为输入，准备接收响应
    DHT22_SetInput();

    // 等待DHT22响应（低电平80us）
    uint32_t timeout = 0;
    while (HAL_GPIO_ReadPin(DHT22_PIN_PORT, DHT22_PIN)) {
        if (++timeout > 100) return 1;
        DWT_Delay_us(1);
    }

    // 等待响应结束
    timeout = 0;
    while (!HAL_GPIO_ReadPin(DHT22_PIN_PORT, DHT22_PIN)) {
        if (++timeout > 100) return 2;
        DWT_Delay_us(1);
    }

    timeout = 0;
    while (HAL_GPIO_ReadPin(DHT22_PIN_PORT, DHT22_PIN)) {
        if (++timeout > 100) return 3;
        DWT_Delay_us(1);
    }

    // 接收 40 位数据
    for (int i = 0; i < 40; i++) {
        // 等待开始位（低电平）
        timeout = 0;
        while (!HAL_GPIO_ReadPin(DHT22_PIN_PORT, DHT22_PIN)) {
            if (++timeout > 100) return 4;
            DWT_Delay_us(1);
        }

        // 等待高电平并延迟 ~40µs 进行采样
        DWT_Delay_us(40);
        if (HAL_GPIO_ReadPin(DHT22_PIN_PORT, DHT22_PIN)) {
            data[i / 8] |= (1 << (7 - (i % 8)));
        }

        // 等待高电平结束
        timeout = 0;
        while (HAL_GPIO_ReadPin(DHT22_PIN_PORT, DHT22_PIN)) {
            if (++timeout > 100) return 5;
            DWT_Delay_us(1);
        }
    }

    // 校验和
    uint8_t sum = data[0] + data[1] + data[2] + data[3];
    if (sum != data[4]) return 6;

    // 数据转换
    *humidity = ((data[0] << 8) | data[1]) / 10.0f;

    int16_t temp_raw = ((data[2] & 0x7F) << 8) | data[3];
    *temperature = temp_raw / 10.0f;
    if (data[2] & 0x80) *temperature = -*temperature;

    return 0;
}







void DHT22_SetOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT22_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT22_PIN_PORT, &GPIO_InitStruct);
}

// 输入模式
void DHT22_SetInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT22_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DHT22_PIN_PORT, &GPIO_InitStruct);
}