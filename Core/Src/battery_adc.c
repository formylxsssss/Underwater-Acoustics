#include "battery_adc.h"

/* 模块私有：ADC1 句柄 */
static ADC_HandleTypeDef hadc1;

/* 打开时钟、配置 ADC 预分频（F1: ADCCLK ≤ 14MHz） */
static void BatteryADC_EnableClocks(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();

    /* PCLK2=72MHz → ADCCLK = PCLK2/6 = 12MHz（合规） */
    __HAL_RCC_ADC_CONFIG(RCC_ADCPCLK2_DIV6);
}

/* PA0 配置为模拟输入 */
static void BatteryADC_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin  = GPIO_PIN_0;        /* PA0 -> ADC1_IN0 */
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* ADC1 基本参数 + 通道配置 */
static void BatteryADC_ADC1_CoreInit(void)
{
    hadc1.Instance                      = ADC1;
    hadc1.Init.ScanConvMode             = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode       = DISABLE;
    hadc1.Init.DiscontinuousConvMode    = DISABLE;
    hadc1.Init.DataAlign                = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion          = 1;
    hadc1.Init.ExternalTrigConv         = ADC_SOFTWARE_START;
    HAL_ADC_Init(&hadc1);

    /* 校准（提升精度） */
    // HAL_ADCEx_Calibration_Start(&hadc1);

    /* 通道：ADC1_IN0（PA0），长采样时间以适配 ~9kΩ 源阻 */
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel      = ADC_CHANNEL_0;
    sConfig.Rank         = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

/*=================== 对外 API 实现 ===================*/
void BatteryADC_Init(void)
{
    BatteryADC_EnableClocks();
    BatteryADC_GPIO_Init();
    BatteryADC_ADC1_CoreInit();
}

uint16_t BatteryADC_ReadRawOnce(void)
{
    HAL_StatusTypeDef st;

    st = HAL_ADC_Start(&hadc1);
    if (st != HAL_OK) return 0;

    /* 轮询等待转换完成：给 10ms 超时足够 */
    st = HAL_ADC_PollForConversion(&hadc1, 10);
    if (st != HAL_OK)
    {
        HAL_ADC_Stop(&hadc1);
        return 0;
    }

    uint16_t raw = (uint16_t)HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return raw; 
}

uint16_t BatteryADC_ReadRawAverage(void)
{
    uint32_t acc = 0;
    for (uint32_t i = 0; i < BAT_ADC_AVG_TIMES; i++)
    {
        acc += BatteryADC_ReadRawOnce();
    }
    return (uint16_t)(acc / BAT_ADC_AVG_TIMES);
}

uint32_t BatteryADC_ReadPA0_mV(void)
{
    uint16_t raw = BatteryADC_ReadRawAverage();
    /* mV = raw * Vref / 4095，使用 64bit 避免中间溢出 */
    return (uint32_t)((uint64_t)raw * (uint64_t)ADC_VREF_MV / 4095ULL);
}

uint32_t BatteryADC_ReadBattery_mV(void)
{
    /* Vbat = Vpa0 * (RTOP + RBOT) / RBOT */
    uint32_t pa0_mV = BatteryADC_ReadPA0_mV();
    uint32_t num = (BAT_RTOP_OHM + BAT_RBOT_OHM);
    uint32_t den = BAT_RBOT_OHM;

    return (uint32_t)((uint64_t)pa0_mV * (uint64_t)num / (uint64_t)den);
}

uint8_t BatteryADC_ReadPercent(void)
{
    uint32_t vbat = BatteryADC_ReadBattery_mV();

    if (vbat <= BAT_VMIN_MV) return 0;
    if (vbat >= BAT_VMAX_MV) return 100;

    uint32_t pct = (uint32_t)((vbat - BAT_VMIN_MV) * 100UL
                              / (BAT_VMAX_MV - BAT_VMIN_MV));
    float real_data = pct *1.1628;
    return (uint8_t)real_data;
}
/*=====================================================*/
