#ifndef BRIDGE_USER_HOOKS_H
#define BRIDGE_USER_HOOKS_H

#include <stdint.h>
#include <stdbool.h>

/* ========================= 你需要根据自己的水下通信驱动实现这些接口 ========================= */

/* 返回系统毫秒计时，默认可直接返回 HAL_GetTick() */
uint32_t BridgeHook_GetMs(void);

/* 当前水下发送器是否空闲：空闲返回 true，忙返回 false */
bool BridgeHook_AcousticIsTxIdle(void);

/* 启动一次水下发送（异步启动）。
 * 返回 true 表示底层已经接受发送请求；
 * 返回 false 表示当前底层不接受发送（忙/故障）。
 */
bool BridgeHook_AcousticStartTx(const uint8_t *data, uint16_t len);

/* 参数设置命令的用户扩展接口。
 * payload[0] 可以自行约定为 param_id。
 * out_rsp 里写回参数响应数据。
 * out_rsp_len 返回响应长度。
 * out_status 返回 ACK/NACK/BUSY 之一。
 */
void BridgeHook_HandleSetParam(const uint8_t *payload,
                               uint8_t len,
                               uint8_t *out_rsp,
                               uint8_t *out_rsp_len,
                               uint8_t *out_status);

/* 可选：当收到一帧合法485协议帧时调用，便于你打日志或示波定位 */
void BridgeHook_OnValid485Frame(uint8_t cmd, uint8_t seq, uint8_t len);

#endif /* BRIDGE_USER_HOOKS_H */
