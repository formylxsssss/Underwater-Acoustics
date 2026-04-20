#include "uw_link_proto.h"

uint16_t UW_Link_CalcCrc16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFFu;
    uint16_t i;
    uint8_t j;

    for (i = 0u; i < len; i++)
    {
        crc ^= data[i];
        for (j = 0u; j < 8u; j++)
        {
            if ((crc & 0x0001u) != 0u)
            {
                crc = (uint16_t)((crc >> 1u) ^ 0xA001u);
            }
            else
            {
                crc >>= 1u;
            }
        }
    }

    return crc;
}