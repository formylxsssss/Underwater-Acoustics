/* dht22.c */
#include "DHT22.h"  // CMSIS，提供 DWT 结构定义
#include "diff_signal.h"
#include "SEGGER_RTT.h"
// 延时 us 微秒
static void Delay_us(uint32_t us)
{
    DWT_Delay_us(us);
}

// 切换为输出模式
static void DHT22_Pin_Output(void)
{
    GPIO_InitTypeDef GPIO_Init = {0};
    GPIO_Init.Pin   = DHT22_Pin;
    GPIO_Init.Mode  = GPIO_MODE_OUTPUT_OD;
    GPIO_Init.Pull  = GPIO_NOPULL;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT22_GPIO_Port, &GPIO_Init);
}
// 切换为输入模式
static void DHT22_Pin_Input(void)
{
    GPIO_InitTypeDef GPIO_Init = {0};
    GPIO_Init.Pin  = DHT22_Pin;
    GPIO_Init.Mode = GPIO_MODE_INPUT;
    GPIO_Init.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DHT22_GPIO_Port, &GPIO_Init);
}

/**
 * @brief  初始化，调用一次即可
 */
void DHT22_Init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    DHT22_Pin_Output();
    HAL_GPIO_WritePin(DHT22_GPIO_Port, DHT22_Pin, GPIO_PIN_SET);
}

/**
 * @brief  发送启动脉冲：拉低 >500us，然后拉高 20–40us
 */
static void DHT22_StartSignal(void)
{
    DHT22_Pin_Output();
    HAL_GPIO_WritePin(DHT22_GPIO_Port, DHT22_Pin, GPIO_PIN_RESET);
    Delay_us(800);      // 500–1000us 起始信号 :contentReference[oaicite:2]{index=2}
    HAL_GPIO_WritePin(DHT22_GPIO_Port, DHT22_Pin, GPIO_PIN_SET);
    Delay_us(30);       // 等待 20–40us
    DHT22_Pin_Input();
}

/**
 * @brief  检测传感器应答信号
 * @return 1 = 有响应，0 = 无响应
 */
static uint8_t DHT22_CheckResponse(void)
{
    uint32_t t = 0;
    // 等待拉低（≈80us）
    while (HAL_GPIO_ReadPin(DHT22_GPIO_Port, DHT22_Pin) == GPIO_PIN_SET)
    {
        if (++t > 100) return 0;
        Delay_us(1);
    }
    t = 0;
    // 等待拉高（≈80us）
    while (HAL_GPIO_ReadPin(DHT22_GPIO_Port, DHT22_Pin) == GPIO_PIN_RESET)
    {
        if (++t > 100) return 0;
        Delay_us(1);
    }
    return 1;
}

/**
 * @brief  读取 1 bit
 */
static uint8_t DHT22_ReadBit(void)
{
    uint8_t bit = 0;
    uint32_t t = 0;

    // 等待低电平结束 (~50us)
    while (HAL_GPIO_ReadPin(DHT22_GPIO_Port, DHT22_Pin) == GPIO_PIN_RESET)
        ;
    // 计时高电平长度
    while (HAL_GPIO_ReadPin(DHT22_GPIO_Port, DHT22_Pin) == GPIO_PIN_SET)
    {
        if (++t > 100) break;
        Delay_us(1);
    }
    // 高电平持续时间 > 40us 判为“1”
    if (t > 20) bit = 1;
    myprintf("t is %d\n",t);
    return bit;
}

/**
 * @brief  读取 1 字节
 */
static uint8_t DHT22_ReadByte(void)
{
    uint8_t byte = 0;
    for (int8_t i = 7; i >= 0; i--)
    {
        byte <<= 1;
        byte |= DHT22_ReadBit();
    }
    return byte;
}

/**
 * @brief  公有接口：读取温度和湿度
 */
uint8_t DHT22_ReadData(float *temperature, float *humidity)
{
    uint8_t data[5] = {0};

    DHT22_StartSignal();
    if (!DHT22_CheckResponse()) return 1;

    // 依次读取 5 个字节：湿度高、湿度低、温度高、温度低、校验
    for (uint8_t i = 0; i < 5; i++)
    {
        data[i] = DHT22_ReadByte();
        // myprintf("read data %x\n",data[i]);
    }

    // CRC 校验
    if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF))
        return 2;

    // 解析湿度（16-bit -> 10×%RH）
    uint16_t rawH = (data[0] << 8) | data[1];
    *humidity    = rawH * 0.1f;

    // 解析温度（16-bit，最高位符号）
    uint16_t rawT = (data[2] << 8) | data[3];
    if (rawT & 0x8000)
    {
        rawT &= 0x7FFF;
        *temperature = -(rawT * 0.1f);
    }
    else
    {
        *temperature = rawT * 0.1f;
    }

    return 0;
}
