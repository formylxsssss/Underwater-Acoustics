#include "bridge_proto.h"
#include <string.h>
#include "SEGGER_RTT.h"
static uint16_t BridgeProto_CobsEncode(const uint8_t *input, uint16_t length, uint8_t *output, uint16_t out_size)
{
    uint16_t read_index = 0u;
    uint16_t write_index = 1u;
    uint16_t code_index = 0u;
    uint8_t code = 1u;

    if (out_size == 0u)
    {
        return 0u;
    }

    while (read_index < length)
    {
        if (write_index >= out_size)
        {
            return 0u;
        }

        if (input[read_index] == 0u)
        {
            output[code_index] = code;
            code = 1u;
            code_index = write_index++;
            read_index++;
        }
        else
        {
            output[write_index++] = input[read_index++];
            code++;
            if (code == 0xFFu)
            {
                output[code_index] = code;
                code = 1u;
                code_index = write_index++;
                if (write_index > out_size)
                {
                    return 0u;
                }
            }
        }
    }

    if (code_index >= out_size)
    {
        return 0u;
    }

    output[code_index] = code;
    return write_index;
}

static uint16_t BridgeProto_CobsDecode(const uint8_t *input, uint16_t length, uint8_t *output, uint16_t out_size)
{
    uint16_t read_index = 0u;
    uint16_t write_index = 0u;

    while (read_index < length)
    {
        uint8_t code = input[read_index++];
        uint8_t i;

        if (code == 0u)
        {
            return 0u;
        }

        for (i = 1u; i < code; i++)
        {
            if ((read_index >= length) || (write_index >= out_size))
            {
                return 0u;
            }
            output[write_index++] = input[read_index++];
        }

        if ((code != 0xFFu) && (read_index < length))
        {
            if (write_index >= out_size)
            {
                return 0u;
            }
            output[write_index++] = 0u;
        }
    }

    return write_index;
}

uint16_t BridgeProto_CalcCrc16(const uint8_t *data, uint16_t len)
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

void BridgeProto_ParserInit(BridgeProtoParser_t *parser)
{
    parser->len = 0u;
    parser->in_frame = false;
}

static bool BridgeProto_DecodeOne(const uint8_t *enc, uint16_t enc_len, BridgeProtoFrame_t *out_frame, uint8_t *out_error)
{
    uint8_t raw[BRIDGE_MAX_FRAME_RAW_LEN];
    uint16_t raw_len;
    uint16_t rx_crc;
    uint16_t calc_crc;

    raw_len = BridgeProto_CobsDecode(enc, enc_len, raw, sizeof(raw));
    if (raw_len < 6u)
    {
        *out_error = BRIDGE_ERR_BAD_LEN;
        return false;
    }

    out_frame->addr = raw[0];
    out_frame->cmd  = raw[1];
    out_frame->seq  = raw[2];
    out_frame->len  = raw[3];

    if (out_frame->len > BRIDGE_MAX_PAYLOAD_LEN)
    {
        *out_error = BRIDGE_ERR_BAD_LEN;
        return false;
    }

    if (raw_len != (uint16_t)(4u + out_frame->len + 2u))
    {
        *out_error = BRIDGE_ERR_BAD_LEN;
        return false;
    }

    rx_crc = (uint16_t)raw[raw_len - 2u] | ((uint16_t)raw[raw_len - 1u] << 8u);
    calc_crc = BridgeProto_CalcCrc16(raw, (uint16_t)(raw_len - 2u));
    if (rx_crc != calc_crc)
    {
        *out_error = BRIDGE_ERR_BAD_CRC;
        return false;
    }

    if (out_frame->len > 0u)
    {
        memcpy(out_frame->payload, &raw[4], out_frame->len);
    }

    *out_error = BRIDGE_ERR_NONE;
    return true;
}

bool BridgeProto_PushByte(BridgeProtoParser_t *parser,
                          uint8_t byte,
                          BridgeProtoFrame_t *out_frame,
                          uint8_t *out_error)
{
    if (byte == 0x00u)
    {
        if (parser->in_frame && parser->len > 0u)
        {
            bool ok = BridgeProto_DecodeOne(parser->buf, parser->len, out_frame, out_error);
            parser->len = 0u;
            parser->in_frame = true;
            return ok;
        }

        parser->len = 0u;
        parser->in_frame = true;
        return false;
    }

    if (!parser->in_frame)
    {
        return false;
    }

    if (parser->len >= sizeof(parser->buf))
    {
        parser->len = 0u;
        parser->in_frame = false;
        *out_error = BRIDGE_ERR_BAD_LEN;
        return false;
    }

    parser->buf[parser->len++] = byte;
    return false;
}

uint16_t BridgeProto_BuildEncodedFrame(uint8_t addr,
                                       uint8_t cmd,
                                       uint8_t seq,
                                       const uint8_t *payload,
                                       uint8_t payload_len,
                                       uint8_t *out_buf,
                                       uint16_t out_buf_size)
{
    uint8_t raw[BRIDGE_MAX_FRAME_RAW_LEN];
    uint16_t raw_len;
    uint16_t enc_len;
    uint16_t crc;

    if (payload_len > BRIDGE_MAX_PAYLOAD_LEN)
    {
        return 0u;
    }

    raw[0] = addr;
    raw[1] = cmd;
    raw[2] = seq;
    raw[3] = payload_len;

    if ((payload_len > 0u) && (payload != NULL))
    {
        memcpy(&raw[4], payload, payload_len);
    }

    raw_len = (uint16_t)(4u + payload_len);
    crc = BridgeProto_CalcCrc16(raw, raw_len);
    raw[raw_len++] = (uint8_t)(crc & 0xFFu);
    raw[raw_len++] = (uint8_t)((crc >> 8u) & 0xFFu);

    if (out_buf_size < 2u)
    {
        return 0u;
    }

    out_buf[0] = 0x00u;
    enc_len = BridgeProto_CobsEncode(raw, raw_len, &out_buf[1], (uint16_t)(out_buf_size - 2u));
    if (enc_len == 0u)
    {
        return 0u;
    }

    out_buf[1u + enc_len] = 0x00u;
    return (uint16_t)(enc_len + 2u);
}
