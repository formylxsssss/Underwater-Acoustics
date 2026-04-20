#include "bridge_user_hooks.h"
#include "bridge_proto_cfg.h"
#include "diff_signal.h"
#include "uw_link_tx.h"
#include "SEGGER_RTT.h"
/* 你可以在这里改成真实的从机设备ID */
#define UW_REMOTE_DEVICE_ID    0x01u

bool BridgeHook_AcousticIsTxIdle(void)
{
    return !DiffSignal_IsBusy();
}

bool BridgeHook_AcousticStartTx(const uint8_t *data, uint16_t len)
{
    for(int i =0;i<len;i++)
    {
        myprintf("data[%d]is %x\n",i,data[i]);
    }
    return UW_LinkTx_Send(UW_REMOTE_DEVICE_ID, data, (uint8_t)len);
}

void BridgeHook_OnValid485Frame(uint8_t cmd, uint8_t seq, uint8_t len)
{
    (void)cmd;
    (void)seq;
    (void)len;
}

uint32_t BridgeHook_GetMs(void)
{
    return HAL_GetTick();
}

void BridgeHook_HandleSetParam(const uint8_t *payload,
                               uint8_t payload_len,
                               uint8_t *out_rsp,
                               uint8_t *out_rsp_len,
                               uint8_t *out_status)
{
    (void)payload;
    (void)payload_len;

    if ((out_rsp == NULL) || (out_rsp_len == NULL) || (out_status == NULL))
    {
        return;
    }

    out_rsp[0] = BRIDGE_ERR_PARAM;
    *out_rsp_len = 1u;
    *out_status = BRIDGE_RSP_NACK;
}