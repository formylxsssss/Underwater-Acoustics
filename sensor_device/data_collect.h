#ifndef DATA_COLLECT_H_
#define DATA_COLLECT_H_
#include "stdint.h"

typedef struct 
{
    float temperature;
    float humidity;
    float x_data;
    float y_data;
    float z_data;

}DATA_GATHER;



extern DATA_GATHER all_data_gather;

void set_local_dht22_data(float temp,float hum);
void get_local_dht22_data(float *temp,float *hum);
void set_local_adxl_345_data(float x_data,float y_data,float z_data);
void get_local_adxl_345_data(float *x_data,float *y_data,float *z_data);


#endif