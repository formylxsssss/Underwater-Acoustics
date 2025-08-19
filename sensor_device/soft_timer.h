/* soft_timer.h */

#ifndef __SOFT_TIMER_H__
#define __SOFT_TIMER_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"

#define SOFT_TIMER_MAX    8   // 最多同时支持 8 路软件定时器

typedef void (*SoftTimer_Callback)(void);

typedef enum {
    TIMER_MODE_ONE_SHOT = 0,   // 到期只执行一次，然后自动停止
    TIMER_MODE_PERIODIC         // 到期后自动重载，循环执行
} SoftTimer_Mode;

typedef struct {
    bool               active;       // 是否正在运行
    uint32_t           reload_ms;    // 重载周期（ms）
    uint32_t           counter;      // 当前倒计时值
    SoftTimer_Callback cb;           // 到期回调
    SoftTimer_Mode     mode;         // 单次 or 循环
} SoftTimer_t;

/**
 * @brief 初始化软件定时器模块（必须在 TIM6 启动前调用一次）
 */
void SoftTimer_Init(void);

/**
 * @brief 创建并启动一个软件定时器
 * @param period_ms  定时周期，单位 ms
 * @param cb         到期回调函数
 * @param mode       TIMER_MODE_ONE_SHOT 或 TIMER_MODE_PERIODIC
 * @return 定时器 ID（0 ～ SOFT_TIMER_MAX-1），返回 -1 表示已满
 */
int8_t SoftTimer_Start(uint32_t period_ms, SoftTimer_Callback cb, SoftTimer_Mode mode);

/**
 * @brief 停止并释放一个定时器
 * @param id 定时器 ID
 */
void SoftTimer_Stop(int8_t id);

/**
 * @brief 软件定时器节拍函数，需在 1ms 中断里调用一次
 */
void SoftTimer_Tick(void);

/* 方便封装：单次、周期定时器 */
static inline int8_t SoftTimer_StartOneShot(uint32_t ms, SoftTimer_Callback cb) {
    return SoftTimer_Start(ms, cb, TIMER_MODE_ONE_SHOT);
}
static inline int8_t SoftTimer_StartPeriodic(uint32_t ms, SoftTimer_Callback cb) {
    return SoftTimer_Start(ms, cb, TIMER_MODE_PERIODIC);
}

#endif // __SOFT_TIMER_H__
