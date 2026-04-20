#ifndef __UW_LINK_PROTO_H
#define __UW_LINK_PROTO_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UW_LINK_BIT_RATE_DEFAULT_BPS      100u
#define UW_LINK_BIT_PERIOD_US             10000u

#define UW_LINK_PREAMBLE_BITS             2u      /* 20ms = 2bit */
#define UW_LINK_TAIL_BITS                 2u      /* 20ms = 2bit */

#define UW_LINK_SYNC_BYTE                 0x55u
#define UW_LINK_BROADCAST_ID              0xFFu

#define UW_LINK_MAX_PAYLOAD_LEN           128u

uint16_t UW_Link_CalcCrc16(const uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif