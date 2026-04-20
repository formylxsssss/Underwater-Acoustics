#include "uw_link_rx.h"
#include "uw_link_proto.h"
#include <string.h>
#include "SEGGER_RTT.h"

typedef enum
{
    UW_TONE_NONE = 0,
    UW_TONE_BIT0,
    UW_TONE_BIT1,
    UW_TONE_ERROR
} UW_ToneState_t;

typedef enum
{
    UW_RX_IDLE = 0,
    UW_RX_START_COMP,
    UW_RX_RECV
} UW_RxState_t;

typedef enum
{
    UW_PARSE_WAIT_PREAMBLE = 0,
    UW_PARSE_WAIT_SYNC,
    UW_PARSE_READ_ID,
    UW_PARSE_READ_LEN,
    UW_PARSE_READ_PAYLOAD,
    UW_PARSE_READ_CRC_L,
    UW_PARSE_READ_CRC_H,
    UW_PARSE_WAIT_TAIL0_A,
    UW_PARSE_WAIT_TAIL0_B
} UW_ParseState_t;

typedef struct
{
    UW_RxState_t state;
    UW_ParseState_t parse_state;

    uint32_t tick_ms;
    uint32_t last_valid_tone_ms;

    uint8_t start_comp_ms_left;
    uint8_t bit_ms_index; /* 当前 bit 窗口内已经过去了多少 ms，范围 0~9 */

    /* 多次采样投票 */
    uint8_t bit0_votes;
    uint8_t bit1_votes;
    uint8_t err_votes;

    /* 0 的重点判断时刻命中记录 */
    uint8_t zero_judge_bit0_seen;
    uint8_t zero_judge_bit1_seen;

    /* 前导 1 计数 */
    uint8_t preamble_one_count;

    /* SYNC(0x55) 检测移位寄存器 */
    uint8_t shift_reg;
    uint8_t sync_bit_count;

    /* 当前组字节 */
    uint8_t current_byte;
    int8_t bit_index; /* 7 -> 0, MSB first */

    /* 当前正在接收的一帧 */
    uint8_t local_device_id;
    uint8_t rx_device_id;
    uint8_t rx_len;
    uint8_t rx_payload[UW_LINK_RX_PAYLOAD_MAX_LEN];
    uint8_t rx_payload_index;

    uint8_t crc_l;
    uint8_t crc_h;

    /* 已完成帧 */
    uint8_t ready_device_id;
    uint8_t ready_payload[UW_LINK_RX_PAYLOAD_MAX_LEN];
    uint16_t ready_len;
    bool frame_ready;
} UW_LinkRx_t;

static UW_LinkRx_t s_rx;

/* ========================= 内部辅助函数 ========================= */

static inline bool UW_LinkRx_IsSamplePoint(uint8_t bit_ms_index)
{
    /* 10ms 窗口内取 5 个采样点：2,4,5,6,8ms */
    return (bit_ms_index == 2u) ||
           (bit_ms_index == 4u) ||
           (bit_ms_index == 5u) ||
           (bit_ms_index == 6u) ||
           (bit_ms_index == 8u);
}

static UW_ToneState_t UW_LinkRx_ReadTone(void)
{
    GPIO_PinState pc0 = HAL_GPIO_ReadPin(UW_LINK_BIT0_GPIO_PORT, UW_LINK_BIT0_GPIO_PIN);
    GPIO_PinState pc1 = HAL_GPIO_ReadPin(UW_LINK_BIT1_GPIO_PORT, UW_LINK_BIT1_GPIO_PIN);

    /* LM567: 匹配到频率时输出低电平 */
    bool bit0_active = (pc0 == GPIO_PIN_RESET);
    bool bit1_active = (pc1 == GPIO_PIN_RESET);

    if (bit0_active && (!bit1_active))
    {
        return UW_TONE_BIT0;
    }
    else if (bit1_active && (!bit0_active))
    {
        return UW_TONE_BIT1;
    }
    else if ((!bit0_active) && (!bit1_active))
    {
        return UW_TONE_NONE;
    }
    else
    {
        return UW_TONE_ERROR;
    }
}

static void UW_LinkRx_ResetVotes(void)
{
    s_rx.bit0_votes = 0u;
    s_rx.bit1_votes = 0u;
    s_rx.err_votes = 0u;

    s_rx.zero_judge_bit0_seen = 0u;
    s_rx.zero_judge_bit1_seen = 0u;
}

static void UW_LinkRx_ResetByteAssembler(void)
{
    s_rx.current_byte = 0u;
    s_rx.bit_index = 7;
}

static void UW_LinkRx_ResetParser(void)
{
    s_rx.parse_state = UW_PARSE_WAIT_PREAMBLE;
    s_rx.preamble_one_count = 0u;
    s_rx.shift_reg = 0u;
    s_rx.sync_bit_count = 0u;

    s_rx.rx_device_id = 0u;
    s_rx.rx_len = 0u;
    s_rx.rx_payload_index = 0u;

    s_rx.crc_l = 0u;
    s_rx.crc_h = 0u;

    UW_LinkRx_ResetByteAssembler();
}

static void UW_LinkRx_Abort(void)
{
    s_rx.state = UW_RX_IDLE;
    s_rx.start_comp_ms_left = 0u;
    s_rx.bit_ms_index = 0u;

    UW_LinkRx_ResetVotes();
    UW_LinkRx_ResetParser();
}

static void UW_LinkRx_Finish(uint8_t device_id, const uint8_t *payload, uint8_t len)
{
    s_rx.ready_device_id = device_id;
    memcpy(s_rx.ready_payload, payload, len);
    s_rx.ready_len = len;
    s_rx.frame_ready = true;

    UW_LinkRx_Abort();
}

static bool UW_LinkRx_DecideBit(uint8_t *out_bit)
{
    /* 先判 1：优先按多数票 */
    if ((s_rx.bit1_votes >= UW_LINK_RX_MIN_MAJORITY) &&
        (s_rx.bit1_votes > s_rx.bit0_votes))
    {
        *out_bit = 1u;
        return true;
    }

    /* 再判 0：如果在“0重点判断时刻”命中 bit0，则优先偏向判 0 */
    if ((s_rx.zero_judge_bit0_seen != 0u) &&
        (s_rx.zero_judge_bit1_seen == 0u))
    {
        *out_bit = 0u;
        return true;
    }

    /* 最后退回到多数票判 0 */
    if ((s_rx.bit0_votes >= UW_LINK_RX_MIN_MAJORITY) &&
        (s_rx.bit0_votes >= s_rx.bit1_votes))
    {
        *out_bit = 0u;
        return true;
    }

    return false;
}

static bool UW_LinkRx_PushBitToByte(uint8_t bit, uint8_t *out_byte_ready, uint8_t *out_byte)
{
    if (bit != 0u)
    {
        s_rx.current_byte |= (uint8_t)(1u << s_rx.bit_index);
    }

    if (s_rx.bit_index == 0)
    {
        *out_byte = s_rx.current_byte;
        *out_byte_ready = 1u;
        UW_LinkRx_ResetByteAssembler();
        return true;
    }
    else
    {
        s_rx.bit_index--;
        *out_byte_ready = 0u;
        return true;
    }
}

static void UW_LinkRx_ProcessDecodedBit(uint8_t bit)
{
    uint8_t byte_ready = 0u;
    uint8_t byte_val = 0u;
    uint16_t crc_calc;
    uint8_t crc_buf[2u + UW_LINK_RX_PAYLOAD_MAX_LEN];

    switch (s_rx.parse_state)
    {
    case UW_PARSE_WAIT_PREAMBLE:
        if (bit == 1u)
        {
            s_rx.preamble_one_count++;
            if (s_rx.preamble_one_count >= UW_LINK_PREAMBLE_BITS)
            {
                s_rx.parse_state = UW_PARSE_WAIT_SYNC;
                s_rx.shift_reg = 0u;
                s_rx.sync_bit_count = 0u;
            }
        }
        else
        {
            s_rx.preamble_one_count = 0u;
        }
        break;

    case UW_PARSE_WAIT_SYNC:
        s_rx.shift_reg = (uint8_t)((s_rx.shift_reg << 1u) | bit);
        s_rx.sync_bit_count++;

        if (s_rx.sync_bit_count >= 8u)
        {
            if (s_rx.shift_reg == UW_LINK_SYNC_BYTE)
            {
                s_rx.parse_state = UW_PARSE_READ_ID;
                UW_LinkRx_ResetByteAssembler();
            }
            else
            {
                UW_LinkRx_Abort();
            }
        }
        break;

    case UW_PARSE_READ_ID:
        UW_LinkRx_PushBitToByte(bit, &byte_ready, &byte_val);
        if (byte_ready != 0u)
        {
            s_rx.rx_device_id = byte_val;

            if ((s_rx.rx_device_id == s_rx.local_device_id) ||
                (s_rx.rx_device_id == UW_LINK_BROADCAST_ID))
            {
                s_rx.parse_state = UW_PARSE_READ_LEN;
            }
            else
            {
                UW_LinkRx_Abort();
            }
        }
        break;

    case UW_PARSE_READ_LEN:
        UW_LinkRx_PushBitToByte(bit, &byte_ready, &byte_val);
        if (byte_ready != 0u)
        {
            s_rx.rx_len = byte_val;

            if (s_rx.rx_len > UW_LINK_RX_PAYLOAD_MAX_LEN)
            {
                UW_LinkRx_Abort();
            }
            else if (s_rx.rx_len == 0u)
            {
                s_rx.parse_state = UW_PARSE_READ_CRC_L;
            }
            else
            {
                s_rx.rx_payload_index = 0u;
                s_rx.parse_state = UW_PARSE_READ_PAYLOAD;
            }
        }
        break;

    case UW_PARSE_READ_PAYLOAD:
        UW_LinkRx_PushBitToByte(bit, &byte_ready, &byte_val);
        if (byte_ready != 0u)
        {
            s_rx.rx_payload[s_rx.rx_payload_index++] = byte_val;

            if (s_rx.rx_payload_index >= s_rx.rx_len)
            {
                s_rx.parse_state = UW_PARSE_READ_CRC_L;
            }
        }
        break;

    case UW_PARSE_READ_CRC_L:
        UW_LinkRx_PushBitToByte(bit, &byte_ready, &byte_val);
        if (byte_ready != 0u)
        {
            s_rx.crc_l = byte_val;
            s_rx.parse_state = UW_PARSE_READ_CRC_H;
        }
        break;

    case UW_PARSE_READ_CRC_H:
        UW_LinkRx_PushBitToByte(bit, &byte_ready, &byte_val);
        if (byte_ready != 0u)
        {
            s_rx.crc_h = byte_val;

            crc_buf[0] = s_rx.rx_device_id;
            crc_buf[1] = s_rx.rx_len;

            if (s_rx.rx_len > 0u)
            {
                memcpy(&crc_buf[2], s_rx.rx_payload, s_rx.rx_len);
            }

            crc_calc = UW_Link_CalcCrc16(crc_buf, (uint16_t)(2u + s_rx.rx_len));
            if (((uint8_t)(crc_calc & 0xFFu) == s_rx.crc_l) &&
                ((uint8_t)((crc_calc >> 8u) & 0xFFu) == s_rx.crc_h))
            {
                s_rx.parse_state = UW_PARSE_WAIT_TAIL0_A;
            }
            else
            {
                UW_LinkRx_Abort();
            }
        }
        break;

    case UW_PARSE_WAIT_TAIL0_A:
        if (bit == 0u)
        {
            s_rx.parse_state = UW_PARSE_WAIT_TAIL0_B;
        }
        else
        {
            UW_LinkRx_Abort();
        }
        break;

    case UW_PARSE_WAIT_TAIL0_B:
        if (bit == 0u)
        {
            UW_LinkRx_Finish(s_rx.rx_device_id, s_rx.rx_payload, s_rx.rx_len);
        }
        else
        {
            UW_LinkRx_Abort();
        }
        break;

    default:
        UW_LinkRx_Abort();
        break;
    }
}

/* ========================= 对外接口 ========================= */

void UW_LinkRx_Init(uint8_t local_device_id)
{
    memset(&s_rx, 0, sizeof(s_rx));
    s_rx.local_device_id = local_device_id;
    s_rx.state = UW_RX_IDLE;
    UW_LinkRx_ResetParser();
}

bool UW_LinkRx_IsReceiving(void)
{
    return (s_rx.state != UW_RX_IDLE);
}

void UW_LinkRx_EXTI_Callback(uint16_t GPIO_Pin)
{
    if ((GPIO_Pin != UW_LINK_BIT0_GPIO_PIN) &&
        (GPIO_Pin != UW_LINK_BIT1_GPIO_PIN))
    {
        return;
    }

    if (s_rx.state == UW_RX_IDLE)
    {
        UW_LinkRx_ResetParser();
        UW_LinkRx_ResetVotes();

        s_rx.state = UW_RX_START_COMP;
        s_rx.start_comp_ms_left = UW_LINK_RX_START_COMP_MS;
        s_rx.last_valid_tone_ms = s_rx.tick_ms;
        s_rx.bit_ms_index = 0u;
    }
}

void UW_LinkRx_1msTick(void)
{
    UW_ToneState_t tone;
    uint8_t decided_bit;

    s_rx.tick_ms++;

    if (s_rx.state == UW_RX_IDLE)
    {
        return;
    }

    tone = UW_LinkRx_ReadTone();

    /* 任意时刻检测到有效音调，都刷新活动时间 */
    if ((tone == UW_TONE_BIT0) || (tone == UW_TONE_BIT1))
    {
        s_rx.last_valid_tone_ms = s_rx.tick_ms;
    }

    /* 太久没看到有效音调，认为接收失效/结束 */
    if ((s_rx.tick_ms - s_rx.last_valid_tone_ms) >= UW_LINK_RX_FRAME_IDLE_MS)
    {
        if ((s_rx.parse_state == UW_PARSE_WAIT_TAIL0_A) ||
            (s_rx.parse_state == UW_PARSE_WAIT_TAIL0_B))
        {
            UW_LinkRx_Finish(s_rx.rx_device_id, s_rx.rx_payload, s_rx.rx_len);
        }
        else
        {
            UW_LinkRx_Abort();
        }
        return;
    }

    /* 起始补偿阶段 */
    if (s_rx.state == UW_RX_START_COMP)
    {
        if (s_rx.start_comp_ms_left > 0u)
        {
            s_rx.start_comp_ms_left--;
            return;
        }

        s_rx.state = UW_RX_RECV;
        s_rx.bit_ms_index = 0u;
        UW_LinkRx_ResetVotes();
    }

    if (s_rx.state != UW_RX_RECV)
    {
        return;
    }

    /* 正常多次采样 */
    if (UW_LinkRx_IsSamplePoint(s_rx.bit_ms_index))
    {
        if (tone == UW_TONE_BIT0)
        {
            s_rx.bit0_votes++;
        }
        else if (tone == UW_TONE_BIT1)
        {
            s_rx.bit1_votes++;
        }
        else if (tone == UW_TONE_ERROR)
        {
            s_rx.err_votes++;
        }
    }

    /* 在指定时刻额外记录一次“0 的重点判断状态” */
    if (s_rx.bit_ms_index == UW_LINK_RX_ZERO_JUDGE_MS)
    {
        if (tone == UW_TONE_BIT0)
        {
            s_rx.zero_judge_bit0_seen = 1u;
        }
        else if (tone == UW_TONE_BIT1)
        {
            s_rx.zero_judge_bit1_seen = 1u;
        }
    }

    s_rx.bit_ms_index++;

    if (s_rx.bit_ms_index >= UW_LINK_RX_BIT_PERIOD_MS)
    {
        s_rx.bit_ms_index = 0u;

        if (UW_LinkRx_DecideBit(&decided_bit))
        {
            UW_LinkRx_ProcessDecodedBit(decided_bit);
        }

        UW_LinkRx_ResetVotes();
    }
}

bool UW_LinkRx_FrameReady(void)
{
    return s_rx.frame_ready;
}

bool UW_LinkRx_GetFrame(uint8_t *device_id, uint8_t *payload, uint16_t *len)
{
    if ((!s_rx.frame_ready) || (device_id == NULL) || (payload == NULL) || (len == NULL))
    {
        return false;
    }

    *device_id = s_rx.ready_device_id;
    memcpy(payload, s_rx.ready_payload, s_rx.ready_len);
    *len = s_rx.ready_len;
    s_rx.frame_ready = false;

    return true;
}