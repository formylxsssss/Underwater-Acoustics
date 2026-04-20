#ifndef __UW_LINK_RX_H
#define __UW_LINK_RX_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UW_LINK_RX_PAYLOAD_MAX_LEN        128u

/* ========================= LM567 输入定义 ========================= */
/* PC0: 12kHz -> bit0 (匹配时低电平) */
#define UW_LINK_BIT0_GPIO_PORT            GPIOC
#define UW_LINK_BIT0_GPIO_PIN             GPIO_PIN_0

/* PC1: 14kHz -> bit1 (匹配时低电平) */
#define UW_LINK_BIT1_GPIO_PORT            GPIOC
#define UW_LINK_BIT1_GPIO_PIN             GPIO_PIN_1

/* ========================= 接收时序参数 ========================= */
/* 100bps -> 1bit = 10ms */
#define UW_LINK_RX_BIT_PERIOD_MS          10u

/* 超过这个时间没有再检测到有效音调，就认为本帧结束/失效 */
#define UW_LINK_RX_FRAME_IDLE_MS          30u

/* EXTI 触发后，等待一段补偿时间再开始真正的 bit 窗口
   用来补偿 LM567 锁定延时。可现场微调：2~4ms 常见。 */
#define UW_LINK_RX_START_COMP_MS          2u

/* ========================= 多次采样参数 ========================= */
/* 当前逻辑是 10ms 窗口内多次采样 + 多数判决，不是单次判断 */
#define UW_LINK_RX_MIN_MAJORITY           2u

/* “0 的重点判断时刻”，单位 ms，范围建议 0~9
   例如设置成 6，表示在每个 bit 窗口的第 6ms，额外重点观察 bit0。 */
#define UW_LINK_RX_ZERO_JUDGE_MS          4u

/* ========================= 接口 ========================= */

void UW_LinkRx_Init(uint8_t local_device_id);

/* 放在 HAL_GPIO_EXTI_Callback() 里调用 */
void UW_LinkRx_EXTI_Callback(uint16_t GPIO_Pin);

/* 放在 1ms 周期中断里调用，例如 TIM6 1ms 回调 */
void UW_LinkRx_1msTick(void);

bool UW_LinkRx_FrameReady(void);
bool UW_LinkRx_GetFrame(uint8_t *device_id, uint8_t *payload, uint16_t *len);

bool UW_LinkRx_IsReceiving(void);

#ifdef __cplusplus
}
#endif

#endif