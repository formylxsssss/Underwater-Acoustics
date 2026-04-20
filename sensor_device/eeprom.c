/* eeprom93lc66a.c */
#include "eeprom.h"
#include "diff_signal.h"
#include "SEGGER_RTT.h"
static void _Delay_us(void)
{
    // DWT_Delay_us(1);
    // 简单软件延时，需根据实际时钟微调
    
}

// 以下四个宏封装最底层引脚操作
#define CS_HIGH()    HAL_GPIO_WritePin(EEPROM_CS_GPIO_Port,  EEPROM_CS_Pin,   GPIO_PIN_SET)
#define CS_LOW()     HAL_GPIO_WritePin(EEPROM_CS_GPIO_Port,  EEPROM_CS_Pin,   GPIO_PIN_RESET)
#define CLK_HIGH()   HAL_GPIO_WritePin(EEPROM_CLK_GPIO_Port, EEPROM_CLK_Pin,  GPIO_PIN_SET)
#define CLK_LOW()    HAL_GPIO_WritePin(EEPROM_CLK_GPIO_Port, EEPROM_CLK_Pin,  GPIO_PIN_RESET)
#define DI_HIGH()    HAL_GPIO_WritePin(EEPROM_DI_GPIO_Port,  EEPROM_DI_Pin,   GPIO_PIN_SET)
#define DI_LOW()     HAL_GPIO_WritePin(EEPROM_DI_GPIO_Port,  EEPROM_DI_Pin,   GPIO_PIN_RESET)
#define DO_READ()    HAL_GPIO_ReadPin(EEPROM_DO_GPIO_Port,   EEPROM_DO_Pin)

/** @brief 初始化 GPIO */
void EEPROM_Init(void)
{
    GPIO_InitTypeDef GPIO_Init = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    // CS, CLK, DI 输出
    GPIO_Init.Pin   = EEPROM_CS_Pin | EEPROM_CLK_Pin | EEPROM_DI_Pin;
    GPIO_Init.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(EEPROM_CS_GPIO_Port, &GPIO_Init);
     HAL_GPIO_WritePin(EEPROM_DI_GPIO_Port,  EEPROM_DI_Pin,  GPIO_PIN_RESET);

    // DO 输入
    GPIO_Init.Pin  = EEPROM_DO_Pin;
    GPIO_Init.Mode = GPIO_MODE_INPUT;
    GPIO_Init.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(EEPROM_DO_GPIO_Port, &GPIO_Init);
    HAL_Delay(1);
    // 复位引脚状态
    CS_LOW();
    CLK_LOW();
    DI_LOW();
}

/** @brief 生成“Start”条件（SB=1） */
static void _StartCondition(void)
{
    CS_LOW();
    CLK_LOW();
    DI_HIGH();      // DI=1
    _Delay_us();
    CS_HIGH();      // CS 上升
    _Delay_us();
    CLK_HIGH();     // 首次 CLK 上升沿检测 SB
    _Delay_us();
    CLK_LOW();
    _Delay_us();
}

/** @brief 发送一位数据 */
static void _SendBit(uint8_t bit)
{
    if (bit) DI_HIGH(); else DI_LOW();
    _Delay_us();
    CLK_HIGH();
    _Delay_us();
    CLK_LOW();
    _Delay_us();
}

/** @brief 读取一位数据 */
static uint8_t _ReadBit(void)
{
    _Delay_us();
    CLK_HIGH();
    _Delay_us();
    uint8_t b = (DO_READ() == GPIO_PIN_SET) ? 1 : 0;
    CLK_LOW();
    _Delay_us();
    return b;
}

/** @brief 写使能：EWEN（SB=1, OPC=00, A8=1, A7=1） */
void EEPROM_WriteEnable(void)
{
    _StartCondition();
    // OPC=00
    _SendBit(0);
    _SendBit(0);
    // A8=1, A7=1, 其它地址位可随意
    _SendBit(1);
    _SendBit(1);
    for (int i = 6; i >= 0; i--) {
        _SendBit(0);
    }
    CS_LOW();        // 下降沿启动内部 EWEN
    HAL_Delay(1);    // 等待 > TCSL
}

/** @brief 写失能：EWDS（SB=1, OPC=00, A8=0, A7=0） */
void EEPROM_WriteDisable(void)
{
    _StartCondition();
    _SendBit(0);
    _SendBit(0);
    _SendBit(0);
    _SendBit(0);
    for (int i = 6; i >= 0; i--) {
        _SendBit(0);
    }
    CS_LOW();
    HAL_Delay(1);
}

/** @brief 写 1 字节（SB=1, OPC=01） */
void EEPROM_WriteByte(uint16_t address, uint8_t data)
{
    // 先使能写
    EEPROM_WriteEnable();

    _StartCondition();
    // OPC = 01
    _SendBit(0);
    _SendBit(1);
    // 地址 9 bits MSB first
    for (int i = 8; i >= 0; i--) {
        _SendBit((address >> i) & 0x01);
    }
    // 数据 8 bits MSB first
    for (int i = 7; i >= 0; i--) {
        _SendBit((data >> i) & 0x01);
    }
    CS_LOW();             // 下降沿启动自擦除+写操作
    HAL_Delay(6);         // TWC ≈ 6 ms :contentReference[oaicite:4]{index=4}
}

/** @brief 读 1 字节（SB=1, OPC=10） */
uint8_t EEPROM_ReadByte(uint16_t address)
{
    uint8_t result = 0;
    _StartCondition();
    // OPC = 10
    _SendBit(1);
    _SendBit(0);
    // 地址 9 bits MSB
    for (int i = 8; i >= 0; i--) {
        _SendBit((address >> i) & 0x01);
    }
    // 读取 8 bits
    for (int i = 7; i >= 0; i--) {
        result |= (_ReadBit() << i);
    }
    CS_LOW();             // 结束
    return result;
}


void EEPROM_WriteBuffer(uint16_t address, uint8_t *pData, uint16_t len)
{
    // 全部写之前，先使能写操作，降低 CS 切换开销
    if(address<EEPROM_MIN_ADDRESS||address>EEPROM_MAX_ADDRESS)
    {
        myprintf("address error\n");
        return;
    }
    EEPROM_WriteEnable();

    for (uint16_t i = 0; i < len; i++)
    {
        _StartCondition();
        // OPC = 01
        _SendBit(0);
        _SendBit(1);
        // 地址 9 bit
        for (int8_t b = 8; b >= 0; b--)
            _SendBit((address + i) >> b & 0x01);
        // 数据 8 bit
        for (int8_t b = 7; b >= 0; b--)
            _SendBit(pData[i] >> b & 0x01);
        CS_LOW();            // 下降沿触发写入
        HAL_Delay(6);        // TWC ≈ 6 ms
    }

    EEPROM_WriteDisable();
}

void EEPROM_ReadBuffer(uint16_t address, uint8_t *pData, uint16_t len)
{
      if(address<EEPROM_MIN_ADDRESS||address>EEPROM_MAX_ADDRESS)
    {
        myprintf("address error\n");
        return;
    }
    _StartCondition();
    // OPC = 10
    _SendBit(1);
    _SendBit(0);
    // 地址 9 bit
    for (int8_t b = 8; b >= 0; b--)
        _SendBit(address >> b & 0x01);

    // 连续读取 len 个字节
    for (uint16_t i = 0; i < len; i++)
    {
        uint8_t v = 0;
        for (int8_t b = 7; b >= 0; b--)
            v |= (_ReadBit() << b);
        pData[i] = v;
    }

    CS_LOW();  // 完成
}
