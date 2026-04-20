#include "diff_signal.h"
#include "channel_change.h"
#include "tim.h"
#include <string.h>
#include "SEGGER_RTT.h"
/*
 * 当前水下链路约定：
 * bit = 0 -> 12kHz
 * bit = 1 -> 14kHz
 *
 * 这里假设：
 * change_local_trans_freq(0x00) -> 12kHz
 * change_local_trans_freq(0x01) -> 14kHz
 *
 * TIM4: bit 节拍定时器
 * TIM1/TIM8: 实际 PWM 输出
 */

typedef struct
{
    uint32_t bit_rate_bps;
    uint32_t bit_period_us;

    uint8_t  bit_buf[DIFF_SIGNAL_TX_BITS_MAX];
    uint16_t bit_count;
    uint16_t bit_index;

    bool busy;
    bool current_tone_valid;
    uint8_t current_bit;
} DiffSignalCtrl_t;

static DiffSignalCtrl_t s_diff =
{
    .bit_rate_bps = 100u,
    .bit_period_us = 10000u,
    .bit_count = 0u,
    .bit_index = 0u,
    .busy = false,
    .current_tone_valid = false,
    .current_bit = 0u
};

static uint32_t DiffSignal_GetTimClock(TIM_TypeDef *instance)
{
    uint32_t pclk;
    uint32_t timclk;

    if ((instance == TIM2) || (instance == TIM3) || (instance == TIM4))
    {
        pclk = HAL_RCC_GetPCLK1Freq();
        if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_HCLK_DIV1)
        {
            timclk = pclk * 2u;
        }
        else
        {
            timclk = pclk;
        }
    }
    else
    {
        pclk = HAL_RCC_GetPCLK2Freq();
        if ((RCC->CFGR & RCC_CFGR_PPRE2) != RCC_HCLK_DIV1)
        {
            timclk = pclk * 2u;
        }
        else
        {
            timclk = pclk;
        }
    }

    return timclk;
}

static bool DiffSignal_ReconfigBitTimer(uint32_t bit_period_us)
{
    uint32_t timclk;
    uint32_t prescaler;
    uint32_t period;

    if (bit_period_us == 0u)
    {
        return false;
    }

    timclk = DiffSignal_GetTimClock(htim4.Instance);

    /* TIM4 计数频率固定成 1MHz -> 1 tick = 1us */
    prescaler = (timclk / 1000000u);
    if (prescaler == 0u)
    {
        return false;
    }
    prescaler -= 1u;

    period = bit_period_us - 1u;
    if (period > 0xFFFFu)
    {
        return false;
    }

    __HAL_TIM_DISABLE(&htim4);
    __HAL_TIM_SET_PRESCALER(&htim4, prescaler);
    __HAL_TIM_SET_AUTORELOAD(&htim4, period);
    __HAL_TIM_SET_COUNTER(&htim4, 0u);
    __HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_UPDATE);

    return true;
}

static void DiffSignal_StopCurrentTone(void)
{
    (void)HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    (void)HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);

    (void)HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_2);
    (void)HAL_TIMEx_PWMN_Stop(&htim8, TIM_CHANNEL_2);

    s_diff.current_tone_valid = false;
}

static bool DiffSignal_StartToneForBit(uint8_t bit_val)
{
    HAL_StatusTypeDef ret1;
    HAL_StatusTypeDef ret2;

    if (bit_val != 0u)
    {
        /* bit=1 -> 14kHz */
        change_local_trans_freq(0x01u);

        ret1 = HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
        if (ret1 != HAL_OK)
        {
            return false;
        }

        ret2 = HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_2);
        if (ret2 != HAL_OK)
        {
            (void)HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_2);
            return false;
        }
    }
    else
    {
        /* bit=0 -> 12kHz */
        change_local_trans_freq(0x00u);

        ret1 = HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        if (ret1 != HAL_OK)
        {
            return false;
        }

        ret2 = HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
        if (ret2 != HAL_OK)
        {
            (void)HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
            return false;
        }
    }

    s_diff.current_tone_valid = true;
    s_diff.current_bit = bit_val;
    return true;
}

static bool DiffSignal_ApplyBit(uint8_t bit_val)
{
    if ((s_diff.current_tone_valid == true) && (s_diff.current_bit == bit_val))
    {
        return true;
    }

    DiffSignal_StopCurrentTone();
    return DiffSignal_StartToneForBit(bit_val);
}

bool DiffSignal_Init(void)
{
    s_diff.bit_rate_bps = 100u;
    s_diff.bit_period_us = 10000u;
    s_diff.bit_count = 0u;
    s_diff.bit_index = 0u;
    s_diff.busy = false;
    s_diff.current_tone_valid = false;
    s_diff.current_bit = 0u;

    DiffSignal_StopCurrentTone();
    return DiffSignal_ReconfigBitTimer(s_diff.bit_period_us);
}

bool DiffSignal_SetBitRate(uint32_t bps)
{
    if ((bps == 0u) || s_diff.busy)
    {
        return false;
    }

    s_diff.bit_rate_bps = bps;
    s_diff.bit_period_us = 1000000u / bps;

    if (s_diff.bit_period_us == 0u)
    {
        return false;
    }

    return DiffSignal_ReconfigBitTimer(s_diff.bit_period_us);
}

bool DiffSignal_SetBitPeriodUs(uint32_t bit_period_us)
{
    if ((bit_period_us == 0u) || s_diff.busy)
    {
        return false;
    }

    s_diff.bit_period_us = bit_period_us;
    s_diff.bit_rate_bps = 1000000u / bit_period_us;
    if (s_diff.bit_rate_bps == 0u)
    {
        s_diff.bit_rate_bps = 1u;
    }

    return DiffSignal_ReconfigBitTimer(bit_period_us);
}

uint32_t DiffSignal_GetBitRate(void)
{
    return s_diff.bit_rate_bps;
}

uint32_t DiffSignal_GetBitPeriodUs(void)
{
    return s_diff.bit_period_us;
}

bool DiffSignal_IsBusy(void)
{
    return s_diff.busy;
}

void DiffSignal_Stop(void)
{
    (void)HAL_TIM_Base_Stop_IT(&htim4);
    DiffSignal_StopCurrentTone();

    s_diff.busy = false;
    s_diff.bit_count = 0u;
    s_diff.bit_index = 0u;
}

bool DiffSignal_SendBitStream(const uint8_t *bits, uint16_t bit_count)
{
    if ((bits == NULL) || (bit_count == 0u))
    {
        return false;
    }

    if (bit_count > DIFF_SIGNAL_TX_BITS_MAX)
    {
        return false;
    }

    if (s_diff.busy)
    {
        return false;
    }

    memcpy(s_diff.bit_buf, bits, bit_count);
    s_diff.bit_count = bit_count;
    s_diff.bit_index = 0u;
    s_diff.busy = true;
    s_diff.current_tone_valid = false;

    if (!DiffSignal_ApplyBit(s_diff.bit_buf[0]))
    {
        DiffSignal_Stop();
        return false;
    }

    __HAL_TIM_SET_COUNTER(&htim4, 0u);
    __HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_UPDATE);

    if (HAL_TIM_Base_Start_IT(&htim4) != HAL_OK)
    {
        DiffSignal_Stop();
        return false;
    }

    return true;
}

bool DiffSignal_SendByte(uint8_t data)
{
    uint8_t bits[8];
    uint8_t i;

    for (i = 0u; i < 8u; i++)
    {
        bits[i] = (uint8_t)((data >> (7u - i)) & 0x01u);
    }

    return DiffSignal_SendBitStream(bits, 8u);
}

bool DiffSignal_SendBuffer(const uint8_t *buf, uint16_t len)
{
    uint8_t bits[DIFF_SIGNAL_TX_BITS_MAX];
    uint16_t bit_count = 0u;
    uint16_t i;
    uint8_t b;

    if ((buf == NULL) || (len == 0u))
    {
        return false;
    }

    if ((len * 8u) > DIFF_SIGNAL_TX_BITS_MAX)
    {
        return false;
    }

    for (i = 0u; i < len; i++)
    {
        for (b = 0u; b < 8u; b++)
        {
            bits[bit_count++] = (uint8_t)((buf[i] >> (7u - b)) & 0x01u);
        }
    }

    return DiffSignal_SendBitStream(bits, bit_count);
}

void DiffSignal_TimerPeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != TIM4)
    {
        return;
    }

    if (!s_diff.busy)
    {
        (void)HAL_TIM_Base_Stop_IT(&htim4);
        DiffSignal_StopCurrentTone();
        return;
    }

    s_diff.bit_index++;

    if (s_diff.bit_index >= s_diff.bit_count)
    {
        DiffSignal_Stop();
        return;
    }

    if (!DiffSignal_ApplyBit(s_diff.bit_buf[s_diff.bit_index]))
    {
        DiffSignal_Stop();
    }
}

void send_data(uint8_t data)
{
    (void)DiffSignal_SendByte(data);
}