#ifndef __UW_LINK_TX_H
#define __UW_LINK_TX_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool UW_LinkTx_Init(void);

/* device_id 是目标从机设备ID */
bool UW_LinkTx_Send(uint8_t device_id, const uint8_t *payload, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif