#include "eeprom.h"
#include "soft_spi.h"
#include "diff_signal.h"
#include "stm32f1xx_hal.h"

// 手动生成 Start Bit + 剩余指令（bitlen - 1）
static void EEPROM_SendCommand(uint16_t cmd, uint8_t bitlen)
{
    // Start bit: CS = 1, DI = 1, 然后 CLK 上升沿
    HAL_GPIO_WritePin(SPI_SOFT_PORT, SPI_SOFT_CS_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SPI_SOFT_MOSI_PORT, SPI_SOFT_MOSI_PIN, GPIO_PIN_SET);
    DWT_Delay_us(1);
    HAL_GPIO_WritePin(SPI_SOFT_PORT, SPI_SOFT_SCK_PIN, GPIO_PIN_SET);
    DWT_Delay_us(1);
    HAL_GPIO_WritePin(SPI_SOFT_PORT, SPI_SOFT_SCK_PIN, GPIO_PIN_RESET);
    DWT_Delay_us(1);

    // 发送剩余指令位（不含 Start）
    SoftSPI_TransferBits(cmd & ((1 << (bitlen - 1)) - 1), bitlen - 1);

    // 停止位
    HAL_GPIO_WritePin(SPI_SOFT_PORT, SPI_SOFT_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SPI_SOFT_MOSI_PORT, SPI_SOFT_MOSI_PIN, GPIO_PIN_RESET);
    DWT_Delay_us(3);
}

void EEPROM_Init(void)
{
    SoftSPI_Init();
    EEPROM_EWEN();
}

void EEPROM_EWEN(void)
{
    // EWEN = Start(1) + 00 + 11 + 000000 = 0x9800，共 10 bit
    EEPROM_SendCommand(0x9800, 10);
}

void EEPROM_WriteByte(uint8_t address, uint8_t data)
{
    // WRITE = Start + 01 + address(6bit) + data(8bit)
    uint16_t cmd = (0b01 << 8) | (address << 2) | (data >> 6); // 高10bit部分
    HAL_GPIO_WritePin(SPI_SOFT_PORT, SPI_SOFT_CS_PIN, GPIO_PIN_SET);
    SoftSPI_TransferBits(0b1, 1);                      // Start bit
    SoftSPI_TransferBits((0b01 << 6) | address, 8);    // Opcode + Address
    SoftSPI_TransferBits(data, 8);                     // Data
    HAL_GPIO_WritePin(SPI_SOFT_PORT, SPI_SOFT_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SPI_SOFT_MOSI_PORT, SPI_SOFT_MOSI_PIN, GPIO_PIN_RESET);
    DWT_Delay_us(3);
}

uint8_t EEPROM_ReadByte(uint8_t address)
{
    uint8_t data;
    HAL_GPIO_WritePin(SPI_SOFT_PORT, SPI_SOFT_CS_PIN, GPIO_PIN_SET);
    SoftSPI_TransferBits(0b1, 1);                      // Start bit
    SoftSPI_TransferBits((0b10 << 6) | address, 8);    // READ + Address
    data = SoftSPI_TransferBits(0xFF, 8);              // Dummy to clock in data
    HAL_GPIO_WritePin(SPI_SOFT_PORT, SPI_SOFT_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SPI_SOFT_MOSI_PORT, SPI_SOFT_MOSI_PIN, GPIO_PIN_RESET);
    DWT_Delay_us(3);
    return data;
}

void EEPROM_EraseByte(uint8_t address)
{
    // ERASE = Start + 11 + address
    HAL_GPIO_WritePin(SPI_SOFT_PORT, SPI_SOFT_CS_PIN, GPIO_PIN_SET);
    SoftSPI_TransferBits(0b1, 1);                      // Start bit
    SoftSPI_TransferBits((0b00 << 6) | address, 8);    // ERASE + Address
    HAL_GPIO_WritePin(SPI_SOFT_PORT, SPI_SOFT_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SPI_SOFT_MOSI_PORT, SPI_SOFT_MOSI_PIN, GPIO_PIN_RESET);
    DWT_Delay_us(3);
}
