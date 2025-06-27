
#ifndef __RTU_MODEBUS_CRC_H__
#define __RTU_MODEBUS_CRC_H__
#include <stdint.h>
// CRC16 Modbus
uint16_t CRC16_Modbus(uint8_t *_pBuf, uint16_t _usLen);
uint16_t BEBufToUint16(uint8_t *_pBuf);
uint32_t floatToUint32(float value);
float Uint32ToFloat(uint32_t value);
#endif