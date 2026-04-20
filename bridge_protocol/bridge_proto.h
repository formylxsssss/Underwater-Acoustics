#ifndef __BRIDGE_PROTO_H
#define __BRIDGE_PROTO_H

#include "stm32f1xx_hal.h"
#include "bridge_proto_cfg.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t buf[BRIDGE_RS485_PARSER_BUF_SIZE];
    uint16_t len;
    bool in_frame;
} BridgeProtoParser_t;

typedef struct
{
    uint8_t addr;
    uint8_t cmd;
    uint8_t seq;
    uint8_t len;
    uint8_t payload[BRIDGE_MAX_PAYLOAD_LEN];
} BridgeProtoFrame_t;

void BridgeProto_ParserInit(BridgeProtoParser_t *parser);

bool BridgeProto_PushByte(BridgeProtoParser_t *parser,
                          uint8_t byte,
                          BridgeProtoFrame_t *out_frame,
                          uint8_t *out_error);

uint16_t BridgeProto_BuildEncodedFrame(uint8_t addr,
                                       uint8_t cmd,
                                       uint8_t seq,
                                       const uint8_t *payload,
                                       uint8_t payload_len,
                                       uint8_t *out_buf,
                                       uint16_t out_size);

uint16_t BridgeProto_CalcCrc16(const uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif