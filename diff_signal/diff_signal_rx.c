#include "diff_signal_rx.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_adc.h"
#include "stm32f1xx_it.h"
// 你的 DWT 延时初始化，保证 DWT->CYCCNT 可用
extern void DWT_Delay_Init(void);
extern void DWT_Delay_us(uint32_t us);

#define DIFF_PIN        GPIO_PIN_1
#define DIFF_PORT       GPIOA

#define DIFF_THRESHOLD  2000  // 如果你要比较ADC值，可改此阈值

TIM_HandleTypeDef htim2;
ADC_HandleTypeDef hadc1;
static uint8_t recv_byte;
static uint8_t bit_cnt;

//    ----------------- 初始化 -----------------
void DiffSignal_RX_Init(void)
{
    // 1) DWT 初始化
    DWT_Delay_Init();

    // 2) PA1 EXTI 下降沿触发（起始位）
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();
    GPIO_InitTypeDef gp = {0};
    gp.Pin  = DIFF_PIN;
    gp.Mode = GPIO_MODE_IT_FALLING;
    gp.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DIFF_PORT, &gp);
    HAL_NVIC_SetPriority(EXTI1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);

    // 3) TIM2 初始化，用于定时采样
    __HAL_RCC_TIM2_CLK_ENABLE();
    htim2.Instance           = TIM2;
    htim2.Init.Prescaler     = (SystemCoreClock/1000000) - 1; // 1MHz
    htim2.Init.CounterMode   = TIM_COUNTERMODE_UP;
    htim2.Init.Period        = BIT_DURATION_US - 1;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim2);
    HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    // 4) ADC1 初始化，软件触发模式，单次转换+中断
    __HAL_RCC_ADC1_CLK_ENABLE();
    hadc1.Instance                   = ADC1;
    hadc1.Init.ScanConvMode          = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode    = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;  // 软件触发
    hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion       = 1;
    HAL_ADC_Init(&hadc1);
    ADC_ChannelConfTypeDef cfg = {0};
    cfg.Channel      = ADC_CHANNEL_1; // PA1
    cfg.Rank         = ADC_REGULAR_RANK_1;
    cfg.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    HAL_ADC_ConfigChannel(&hadc1, &cfg);
    __HAL_ADC_ENABLE_IT(&hadc1, ADC_IT_EOC);
    HAL_NVIC_SetPriority(ADC1_2_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
}

// ----------------- EXTI 回调（起始位到达） -----------------
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin != DIFF_PIN) return;

    // 重置字节和位计数
    recv_byte = 0;
    bit_cnt   = 0;

    // 启动 TIM2，进入定时中断
    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
    HAL_TIM_Base_Start_IT(&htim2);
}


// ----------------- ADC 转换完成回调：收集一位 -----------------
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance != ADC1) return;

    uint16_t v = HAL_ADC_GetValue(hadc);
    uint8_t bit = (v > DIFF_THRESHOLD) ? 1 : 0;
    recv_byte |= (bit << bit_cnt);
    bit_cnt++;

    if (bit_cnt >= 8)
    {
        // 收满 8 位：停止定时，回调处理
        HAL_TIM_Base_Stop_IT(&htim2);
        DiffSignal_Frame_Handler(recv_byte);
    }
}

void DiffSignal_Frame_Handler(uint8_t byte)
{
    // TODO: 处理接收到的字节
}

__weak void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    // unused, do nothing
}