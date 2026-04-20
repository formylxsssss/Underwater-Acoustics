#include "bridge_app.h"
#include "bridge_proto_cfg.h"
#include "rs485_port.h"
#include "bridge_user_hooks.h"
#include <string.h>
#include "SEGGER_RTT.h"
#include "bridge_proto.h"
typedef struct
{
    uint8_t len;
    uint8_t data[BRIDGE_MAX_PAYLOAD_LEN];
} BridgePacket_t;

typedef struct
{
    BridgePacket_t items[BRIDGE_ACOUSTIC_TX_QUEUE_DEPTH];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} BridgeTxQueue_t;

typedef struct
{
    BridgePacket_t items[BRIDGE_ACOUSTIC_RX_QUEUE_DEPTH];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} BridgeRxQueue_t;

typedef struct
{
    BridgeProtoParser_t parser;
    BridgeProtoFrame_t frame;
    uint8_t parse_error;

    BridgeTxQueue_t txq;
    BridgeRxQueue_t rxq;

    uint8_t local_addr;
    bool acoustic_tx_active;
    uint32_t acoustic_tx_start_ms;
} BridgeContext_t;

static BridgeContext_t s_bridge;
static uint8_t s_tx_frame_buf[BRIDGE_MAX_FRAME_ENCODED_LEN];

static void BridgeTxQueue_Init(BridgeTxQueue_t *q)
{
    q->head = 0u;
    q->tail = 0u;
    q->count = 0u;
}

static bool BridgeTxQueue_Push(BridgeTxQueue_t *q, const uint8_t *data, uint8_t len)
{
    BridgePacket_t *pkt;

    if ((len > BRIDGE_MAX_PAYLOAD_LEN) || (q->count >= BRIDGE_ACOUSTIC_TX_QUEUE_DEPTH))
    {
        return false;
    }
 
    pkt = &q->items[q->head];
    pkt->len = len;
    if ((len > 0u) && (data != NULL))
    {
        memcpy(pkt->data, data, len);
    }

    q->head = (uint8_t)((q->head + 1u) % BRIDGE_ACOUSTIC_TX_QUEUE_DEPTH);
    q->count++;
    return true;
}

static bool BridgeTxQueue_Peek(BridgeTxQueue_t *q, BridgePacket_t **pkt)
{
    if (q->count == 0u)
    {
        return false;
    }
    *pkt = &q->items[q->tail];
    return true;
}

static bool BridgeTxQueue_Pop(BridgeTxQueue_t *q)
{
    if (q->count == 0u)
    {
        return false;
    }
    q->tail = (uint8_t)((q->tail + 1u) % BRIDGE_ACOUSTIC_TX_QUEUE_DEPTH);
    q->count--;
    return true;
}

static void BridgeRxQueue_Init(BridgeRxQueue_t *q)
{
    q->head = 0u;
    q->tail = 0u;
    q->count = 0u;
}

static bool BridgeRxQueue_Push(BridgeRxQueue_t *q, const uint8_t *data, uint8_t len)
{
    BridgePacket_t *pkt;

    if ((len > BRIDGE_MAX_PAYLOAD_LEN) || (q->count >= BRIDGE_ACOUSTIC_RX_QUEUE_DEPTH))
    {
        return false;
    }

    pkt = &q->items[q->head];
    pkt->len = len;
    if ((len > 0u) && (data != NULL))
    {
        memcpy(pkt->data, data, len);
    }

    q->head = (uint8_t)((q->head + 1u) % BRIDGE_ACOUSTIC_RX_QUEUE_DEPTH);
    q->count++;
    return true;
}

static bool BridgeRxQueue_Pop(BridgeRxQueue_t *q, BridgePacket_t *pkt)
{
    if (q->count == 0u)
    {
        return false;
    }

    *pkt = q->items[q->tail];
    q->tail = (uint8_t)((q->tail + 1u) % BRIDGE_ACOUSTIC_RX_QUEUE_DEPTH);
    q->count--;
    return true;
}

static void Bridge_SendResponse(uint8_t dst_addr, uint8_t cmd, uint8_t seq, const uint8_t *payload, uint8_t len)
{
    uint16_t send_len;

    send_len = BridgeProto_BuildEncodedFrame(dst_addr, cmd, seq, payload, len, s_tx_frame_buf, sizeof(s_tx_frame_buf));
    if (send_len > 0u)
    {
        (void)RS485_Port_SendBytes(s_tx_frame_buf, send_len);
    }
}

static void Bridge_SendSimpleStatus(uint8_t dst_addr, uint8_t rsp_cmd, uint8_t seq, uint8_t code)
{
    uint8_t payload[4];
    uint8_t len = 1u;

    payload[0] = code;

#if BRIDGE_ENABLE_VERBOSE_STATUS
    payload[1] = s_bridge.txq.count;
    payload[2] = s_bridge.rxq.count;
    payload[3] = (uint8_t)(s_bridge.acoustic_tx_active ? 1u : 0u);
    len = 4u;
#endif

    Bridge_SendResponse(dst_addr, rsp_cmd, seq, payload, len);
}

static void Bridge_HandleFrame(const BridgeProtoFrame_t *frame)
{
    uint8_t rsp[BRIDGE_MAX_PAYLOAD_LEN];
    uint8_t rsp_len = 0u;
    BridgePacket_t rx_pkt;
    bool need_reply;

    BridgeHook_OnValid485Frame(frame->cmd, frame->seq, frame->len);

    if ((frame->addr != s_bridge.local_addr) && (frame->addr != 0xFFu))
    {
        return;
    }

    /* 0xFF 作为广播地址时，只执行动作，不回包，避免485多机冲突 */
    need_reply = (frame->addr != 0xFFu);

    switch (frame->cmd)
    {
        case BRIDGE_CMD_PING:
            if (need_reply)
            {
                Bridge_SendSimpleStatus(frame->addr, BRIDGE_RSP_PONG, frame->seq, BRIDGE_ERR_NONE);
            }
            break;

        case BRIDGE_CMD_SEND_ACOUSTIC:
            if (BridgeTxQueue_Push(&s_bridge.txq, frame->payload, frame->len))
            {
                if (need_reply)
                {
                    Bridge_SendSimpleStatus(frame->addr, BRIDGE_RSP_ACK, frame->seq, BRIDGE_ERR_NONE);
                }
            }
            else
            {
                if (need_reply)
                {
                    Bridge_SendSimpleStatus(frame->addr, BRIDGE_RSP_BUSY, frame->seq, BRIDGE_ERR_QUEUE_FULL);
                }
            }
            break;

        case BRIDGE_CMD_GET_STATUS:
            rsp[0] = BRIDGE_ERR_NONE;
            rsp[1] = s_bridge.txq.count;
            rsp[2] = s_bridge.rxq.count;
            rsp[3] = (uint8_t)(s_bridge.acoustic_tx_active ? 1u : 0u);
            rsp[4] = (uint8_t)(BridgeHook_AcousticIsTxIdle() ? 1u : 0u);
            rsp_len = 5u;
            if (need_reply)
            {
                Bridge_SendResponse(frame->addr, BRIDGE_RSP_STATUS, frame->seq, rsp, rsp_len);
            }
            break;

        case BRIDGE_CMD_FETCH_RX:
            if (BridgeRxQueue_Pop(&s_bridge.rxq, &rx_pkt))
            {
                if (need_reply)
                {
                    Bridge_SendResponse(frame->addr, BRIDGE_RSP_RX_DATA, frame->seq, rx_pkt.data, rx_pkt.len);
                }
            }
            else
            {
                if (need_reply)
                {
                    Bridge_SendSimpleStatus(frame->addr, BRIDGE_RSP_NACK, frame->seq, BRIDGE_ERR_EMPTY);
                }
            }
            break;

        case BRIDGE_CMD_SET_PARAM:
        {
            uint8_t status = BRIDGE_RSP_NACK;
            BridgeHook_HandleSetParam(frame->payload, frame->len, rsp, &rsp_len, &status);
            if ((status != BRIDGE_RSP_ACK) && (status != BRIDGE_RSP_NACK) && (status != BRIDGE_RSP_BUSY))
            {
                status = BRIDGE_RSP_NACK;
                rsp[0] = BRIDGE_ERR_PARAM;
                rsp_len = 1u;
            }
            if (need_reply)
            {
                Bridge_SendResponse(frame->addr, status, frame->seq, rsp, rsp_len);
            }
            break;
        }

        default:
            if (need_reply)
            {
                Bridge_SendSimpleStatus(frame->addr, BRIDGE_RSP_NACK, frame->seq, BRIDGE_ERR_BAD_CMD);
            }
            break;
    }
}

static void Bridge_ProcessRs485Rx(void)
{
    uint8_t byte;
    while (RS485_Port_ReadByte(&byte))
    {
        bool got = BridgeProto_PushByte(&s_bridge.parser, byte, &s_bridge.frame, &s_bridge.parse_error);
        if (got)
        {
            Bridge_HandleFrame(&s_bridge.frame);
        }
    }
}

static void Bridge_ProcessAcousticTx(void)
{
    BridgePacket_t *pkt;
    uint32_t now = BridgeHook_GetMs();

    if (s_bridge.acoustic_tx_active)
    {
        if (BridgeHook_AcousticIsTxIdle())
        {
            s_bridge.acoustic_tx_active = false;
            (void)BridgeTxQueue_Pop(&s_bridge.txq);
        }
        else if ((now - s_bridge.acoustic_tx_start_ms) > BRIDGE_ACOUSTIC_TX_TIMEOUT_MS)
        {
            /* 超时后释放状态，避免死锁。
             * 是否丢弃当前队列头，由你决定；这里采取丢弃，防止一直堵塞。
             */
            s_bridge.acoustic_tx_active = false;
            (void)BridgeTxQueue_Pop(&s_bridge.txq);
        }
        return;
    }

    if (!BridgeHook_AcousticIsTxIdle())
    {
        return;
    }

    if (BridgeTxQueue_Peek(&s_bridge.txq, &pkt))
    {
        if (BridgeHook_AcousticStartTx(pkt->data, pkt->len))
        {
            s_bridge.acoustic_tx_active = true;
            s_bridge.acoustic_tx_start_ms = now;
        }
    }
}

void BridgeApp_Init(void)
{
    memset(&s_bridge, 0, sizeof(s_bridge));
    s_bridge.local_addr = BRIDGE_DEVICE_ADDR;

    BridgeProto_ParserInit(&s_bridge.parser);
    BridgeTxQueue_Init(&s_bridge.txq);
    BridgeRxQueue_Init(&s_bridge.rxq);

    RS485_Port_Init();
    RS485_Port_StartRxIt();
    myprintf("hhhhh\n");
}

void BridgeApp_Process(void)
{
    Bridge_ProcessRs485Rx();
    Bridge_ProcessAcousticTx();
}

bool BridgeApp_PushAcousticRxPacket(const uint8_t *data, uint8_t len)
{
    return BridgeRxQueue_Push(&s_bridge.rxq, data, len);
}

void BridgeApp_UartRxCpltCallback(UART_HandleTypeDef *huart)
{
    RS485_Port_RxCpltCallback(huart);
}

void BridgeApp_UartErrorCallback(UART_HandleTypeDef *huart)
{
    RS485_Port_ErrorCallback(huart);
}
