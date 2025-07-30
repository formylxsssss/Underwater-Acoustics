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






#endif