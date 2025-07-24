#ifndef __EEPROM_H__
#define __EEPROM_H__

#include <stdint.h>

void EEPROM_Init(void);
void EEPROM_WriteByte(uint8_t address, uint8_t data);
uint8_t EEPROM_ReadByte(uint8_t address);
void EEPROM_EraseByte(uint8_t address);

#endif // __EEPROM_H__
