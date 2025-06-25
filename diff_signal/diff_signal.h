#ifndef __DIFF_SIGNAL_H
#define __DIFF_SIGNAL_H

#include "main.h"

#define BAUD_RATE 2400
#define BIT_DURATION_US (1000000 / BAUD_RATE)

void DiffSignal_Init(void);
void DiffSignal_Send(uint16_t duration_us);
void DiffSignal_Stop(void);
void DiffSignal_Senddata(uint8_t *data, uint16_t length);
void DWT_Delay_Init(void);
void DWT_Delay_us(uint32_t us);

#endif
