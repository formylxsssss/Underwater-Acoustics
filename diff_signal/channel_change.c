#include "stdint.h"
#include "stdbool.h"
#include "stm32f1xx_hal.h"
#include "channel_change.h"
#include "gpio.h"
#include "diff_signal.h"

/*
 * 说明：
 * 1. 保持原有接口名字不变
 * 2. 去掉 HAL_Delay(1)，避免 ms 级阻塞影响 100bps 发送节拍
 * 3. 对 74HCT257 这类逻辑器件，本体切换是 ns 级，不需要 1ms 等待
 * 4. 如果你后级模拟链路确实需要更长建立时间，后续再单独加
 */

/* 很短的保护延时，避免GPIO连续翻转过快。
   这里只做几个 NOP，不再用 HAL_Delay 或 DWT。 */
static inline void channel_short_delay(void)
{
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
}

/* 通用 74HCT257 切换函数
   /OE 低有效：
   - GPIO_PIN_SET   -> 禁止输出（高阻）
   - GPIO_PIN_RESET -> 使能输出

   S 选择端：
   - GPIO_PIN_RESET -> 选 A
   - GPIO_PIN_SET   -> 选 B
*/
static void channel_257_switch(GPIO_TypeDef *oe_port, uint16_t oe_pin,
                               GPIO_TypeDef *sel_port, uint16_t sel_pin,
                               GPIO_PinState sel_state)
{
    /* 1. 先禁止输出 */
    HAL_GPIO_WritePin(oe_port, oe_pin, GPIO_PIN_SET);
    channel_short_delay();

    /* 2. 切换选择端 */
    HAL_GPIO_WritePin(sel_port, sel_pin, sel_state);
    channel_short_delay();

    /* 3. 恢复输出 */
    HAL_GPIO_WritePin(oe_port, oe_pin, GPIO_PIN_RESET);
    channel_short_delay();
}

/* 频率选择更新
   保持你原来逻辑：
   PC8 先拉高 -> 更新 PA2 -> PC8 拉低
*/
static void freq_update_latch(GPIO_PinState freq_sel_state)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
    channel_short_delay();

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, freq_sel_state);
    channel_short_delay();

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
    channel_short_delay();
}

void channel_init(void)
{
    change_origin_channel();
}

/* 本地信号 = A 路，原始信号 = B 路
   若 S=0 选 A，S=1 选 B，则：
   本地通道 -> GPIO_PIN_RESET
*/
void change_local_channel(void)
{
    channel_257_switch(GPIOB, GPIO_PIN_15,
                       GPIOB, GPIO_PIN_14,
                       GPIO_PIN_RESET);
}

/* 原始通道 = B 路 */
void change_origin_channel(void)
{
    channel_257_switch(GPIOB, GPIO_PIN_15,
                       GPIOB, GPIO_PIN_14,
                       GPIO_PIN_SET);
}

void change_rx_local_channel(void)
{
    channel_257_switch(GPIOB, GPIO_PIN_13,
                       GPIOB, GPIO_PIN_12,
                       GPIO_PIN_SET);
}

/**
 * @brief 切换本地发送频率
 * 0 表示 12k
 * 1 表示 14k
 */
void change_local_trans_freq(uint8_t freq)
{
    if (freq == 0x00u)
    {
        /* 12k -> PA2 = 0 */
        freq_update_latch(GPIO_PIN_RESET);
    }
    else if (freq == 0x01u)
    {
        /* 14k -> PA2 = 1 */
        freq_update_latch(GPIO_PIN_SET);
    }
    else
    {
        /* 非法参数，直接返回 */
        return;
    }
}

void send_data_change_channel(void)
{
    change_local_channel();
    /* change_rx_local_channel(); */
}

void restore_send_data_change_channel(void)
{
    change_origin_channel();
}