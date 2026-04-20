#ifndef CHANNEL_CHANGE_H_
#define CHANNEL_CHANGE_H_
#include "stdint.h"


#define FREQ_12K         0x00
#define FREQ_14K      0x01

void change_local_channel(void);
void change_origin_channel(void);
void channel_init(void);
void change_rx_local_channel(void);
void send_data_change_channel(void);
void change_local_trans_freq(uint8_t freq);
void restore_send_data_change_channel(void);
#endif