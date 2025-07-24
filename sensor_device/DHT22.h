#ifndef DHT22_H_
#define DHT22_H_


uint8_t DHT22_Read(float *temperature, float *humidity);
void DHT22_SetOutput(void);
void DHT22_SetInput(void);
#endif