#ifndef BATTERY_ADC_H
#define BATTERY_ADC_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include "stm32f1xx_hal_adc_ex.h"

#ifdef __cplusplus
extern "C" {
#endif

/*================ 用户可按需修改的参数 ================*/
/* 参考电压（mV）。为提高精度，建议用万用表测 VDDA 后填写实测值 */
#ifndef ADC_VREF_MV
#define ADC_VREF_MV        (3299UL)
#endif

/* 分压电阻（欧姆）——电池正极侧为 RTOP，GND 侧为 RBOT */
#ifndef BAT_RTOP_OHM
#define BAT_RTOP_OHM       (100000UL)   /* 100kΩ */
#endif
#ifndef BAT_RBOT_OHM
#define BAT_RBOT_OHM       (10000UL)    /* 10kΩ  */
#endif

/* 采样平均次数（抗抖/降噪） */
#ifndef BAT_ADC_AVG_TIMES
#define BAT_ADC_AVG_TIMES  (32U)
#endif

/* 百分比量程（mV）：V<=VMIN→0%，V>=VMAX→100%（线性映射） */
#ifndef BAT_VMIN_MV
#define BAT_VMIN_MV        (12000UL)    /* 12.0 V -> 0%  */
#endif
#ifndef BAT_VMAX_MV
#define BAT_VMAX_MV        (16800UL)    /* 16.8 V -> 100% */
#endif
/*=====================================================*/

/* ========== 对外 API ========== */
/**
 * @brief 初始化 ADC1 与 PA0(ADC1_IN0)，设置时钟/通道/采样时间并执行校准
 */
void     BatteryADC_Init(void);

/**
 * @brief 读取一次原始 ADC 值（0..4095）
 */
uint16_t BatteryADC_ReadRawOnce(void);

/**
 * @brief 连续采样 BAT_ADC_AVG_TIMES 次并求平均后的原始值（0..4095）
 */
uint16_t BatteryADC_ReadRawAverage(void);

/**
 * @brief 读取分压点（PA0）电压（mV）
 */
uint32_t BatteryADC_ReadPA0_mV(void);

/**
 * @brief 换算得到电池端电压（mV）：Vbat = Vpa0 * (RTOP+RBOT)/RBOT
 */
uint32_t BatteryADC_ReadBattery_mV(void);

/**
 * @brief 读取电池电量百分比（0~100，线性映射并饱和）
 */
uint8_t  BatteryADC_ReadPercent(void);

#ifdef __cplusplus
}
#endif

#endif /* BATTERY_ADC_H */
