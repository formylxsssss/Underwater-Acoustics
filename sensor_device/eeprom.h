/* eeprom93lc66a.h */
#ifndef __EEPROM93LC66A_H
#define __EEPROM93LC66A_H

#define EEPROM_MAX_ADDRESS   0x1FF
#define EEPROM_MIN_ADDRESS   0x000

#include "stm32f1xx_hal.h"
#include "stdint.h"
// 引脚定义（可根据实际接线改动）
#define EEPROM_CS_GPIO_Port    GPIOA
#define EEPROM_CS_Pin          GPIO_PIN_4
#define EEPROM_CLK_GPIO_Port   GPIOA
#define EEPROM_CLK_Pin         GPIO_PIN_5
#define EEPROM_DI_GPIO_Port    GPIOB
#define EEPROM_DI_Pin          GPIO_PIN_5
#define EEPROM_DO_GPIO_Port    GPIOA
#define EEPROM_DO_Pin          GPIO_PIN_6

/** 
 * @brief 初始化 GPIO 引脚（CS/CLK/DI：推挽输出，DO：输入） 
 */
void EEPROM_Init(void);

/** 
 * @brief 发送 “写使能” 指令（EWEN） 
 */
void EEPROM_WriteEnable(void);

/** 
 * @brief 发送 “写失能” 指令（EWDS） 
 */
void EEPROM_WriteDisable(void);

/** 
 * @brief 往指定地址写入 1 字节数据 
 * @param address 0~511（9 位地址） 
 * @param data    待写入的数据 
 */
void EEPROM_WriteByte(uint16_t address, uint8_t data);

/** 
 * @brief 从指定地址读取 1 字节数据 
 * @param address 0~511（9 位地址） 
 * @return        读出的数据 
 */
uint8_t EEPROM_ReadByte(uint16_t address);


void EEPROM_WriteBuffer(uint16_t address, uint8_t *pData, uint16_t len);

void EEPROM_ReadBuffer(uint16_t address, uint8_t *pData, uint16_t len);
#endif /* __EEPROM93LC66A_H */
