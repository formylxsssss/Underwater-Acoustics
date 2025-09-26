/* soft_timer.c */

#include "soft_timer.h"
#include "SEGGER_RTT.h"
static SoftTimer_t timers[SOFT_TIMER_MAX];

void SoftTimer_Init(void)
{
    for (int i = 0; i < SOFT_TIMER_MAX; i++) {
        timers[i].active    = false;
        timers[i].reload_ms = 0;
        timers[i].counter   = 0;
        timers[i].cb        = NULL;
        timers[i].mode      = TIMER_MODE_PERIODIC;
    }
}

int8_t SoftTimer_Start(uint32_t period_ms, SoftTimer_Callback cb, SoftTimer_Mode mode)
{
    for (int i = 0; i < SOFT_TIMER_MAX; i++) {
        if (!timers[i].active) {
            timers[i].active    = true;
            timers[i].reload_ms = period_ms;
            timers[i].counter   = period_ms;
            timers[i].cb        = cb;
            timers[i].mode      = mode;
            return i;
        }
    }
    return -1; // 全部通道已占用
}

void SoftTimer_Stop(int8_t id)
{
    if (id >= 0 && id < SOFT_TIMER_MAX) {
        timers[id].active = false;
    }
    myprintf("stop_count\n");
}

void SoftTimer_Tick(void)
{
    for (int i = 0; i < SOFT_TIMER_MAX; i++) {
        if (!timers[i].active) continue;

        if (timers[i].counter == 0) {
            // 到期回调
            if (timers[i].cb) {
                timers[i].cb();
            }
            // 根据模式决定是否重载
            if (timers[i].mode == TIMER_MODE_PERIODIC) {
                timers[i].counter = timers[i].reload_ms;
            } else {
                // 单次模式，执行完后停止该定时器
                timers[i].active = false;
            }
        } else {
            timers[i].counter--;
        }
    }
}
