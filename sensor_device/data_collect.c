#include "data_collect.h"
#include "stdint.h"
#include "string.h"
#include "stdlib.h"

DATA_GATHER all_data_gather = {0x00};


void set_local_dht22_data(float temp,float hum)
{
    all_data_gather.temperature = temp;
    all_data_gather.humidity = hum;
}

void get_local_dht22_data(float *temp,float *hum)
{
    *temp = all_data_gather.temperature;
    *hum = all_data_gather.humidity;
}

void set_local_adxl_345_data(float x_data,float y_data,float z_data)
{
    all_data_gather.x_data = x_data;
    all_data_gather.y_data = y_data;
    all_data_gather.z_data = z_data;
}

void get_local_adxl_345_data(float *x_data,float *y_data,float *z_data)
{
    *x_data = all_data_gather.x_data;
    *y_data = all_data_gather.y_data;
    *z_data = all_data_gather.z_data;

}

void set_local_power_data(uint8_t power_data )
{
    all_data_gather.power = power_data;
}
void get_local_power_data(uint8_t *power_data)
{
    *power_data = all_data_gather.power;
}