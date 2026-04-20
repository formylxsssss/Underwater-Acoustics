#include "uw_link_tx.h"
#include "uw_link_proto.h"
#include "diff_signal.h"

static void UW_LinkTx_AppendByteMsbBits(uint8_t byte, uint8_t *bits, uint16_t *bit_index)
{
    uint8_t i;
    for (i = 0u; i < 8u; i++)
    {
        bits[(*bit_index)++] = (uint8_t)((byte >> (7u - i)) & 0x01u);
    }
}

bool UW_LinkTx_Init(void)
{
    return true;
}

bool UW_LinkTx_Send(uint8_t device_id, const uint8_t *payload, uint8_t len)
{
    uint8_t raw[2u + UW_LINK_MAX_PAYLOAD_LEN + 2u];
    uint16_t raw_len = 0u;
    uint16_t bit_index = 0u;
    uint8_t bits[2048];
    uint16_t crc;
    uint8_t i;

    if (((payload == NULL) && (len > 0u)) || (len > UW_LINK_MAX_PAYLOAD_LEN))
    {
        return false;
    }

    if (DiffSignal_IsBusy())
    {
        return false;
    }

    /* raw = DEVICE_ID | LEN | PAYLOAD | CRC_L | CRC_H */
    raw[raw_len++] = device_id;
    raw[raw_len++] = len;

    for (i = 0u; i < len; i++)
    {
        raw[raw_len++] = payload[i];
    }

    crc = UW_Link_CalcCrc16(raw, raw_len);
    raw[raw_len++] = (uint8_t)(crc & 0xFFu);
    raw[raw_len++] = (uint8_t)((crc >> 8u) & 0xFFu);

    /* 前导改成 4 个连续 1bit */
    for (i = 0u; i < UW_LINK_PREAMBLE_BITS; i++)
    {
        bits[bit_index++] = 1u;
    }

    /* SYNC = 0x55 */
    UW_LinkTx_AppendByteMsbBits(UW_LINK_SYNC_BYTE, bits, &bit_index);

    /* DEVICE_ID | LEN | PAYLOAD | CRC */
    for (i = 0u; i < raw_len; i++)
    {
        UW_LinkTx_AppendByteMsbBits(raw[i], bits, &bit_index);
    }

    /* 结束仍保持 2 个连续 0bit */
    for (i = 0u; i < UW_LINK_TAIL_BITS; i++)
    {
        bits[bit_index++] = 0u;
    }

    return DiffSignal_SendBitStream(bits, bit_index);
}