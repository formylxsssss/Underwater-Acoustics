#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "diff_signal.h"
#include "main.h"
#include "Custcom_Pin.h"
uint8_t DHT22_Read(void)
{
    uint8_t i, data = 0;
    for (i = 0; i < 8; i++)
    {
        while (HAL_GPIO_ReadPin(DHT22_PIN_PORT, DHT22_PIN) == GPIO_PIN_RESET)
            ;             // 等待高电平
        DWT_Delay_us(40); // 延时40us
        if (HAL_GPIO_ReadPin(DHT22_PIN_PORT, DHT22_PIN) == GPIO_PIN_SET)
        {
            data |= (1 << (7 - i)); // 如果是高电平，则设置当前位
        }
        while (HAL_GPIO_ReadPin(DHT22_PIN_PORT, DHT22_PIN) == GPIO_PIN_SET)
            ; // 等待低电平
    }

    return data;
}