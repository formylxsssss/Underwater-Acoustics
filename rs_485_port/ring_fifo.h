#ifndef RING_FIFO_H
#define RING_FIFO_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    volatile uint16_t head;
    volatile uint16_t tail;
    uint16_t size;
    uint8_t *buffer;
} RingFifo_t;

void RingFifo_Init(RingFifo_t *fifo, uint8_t *buffer, uint16_t size);
bool RingFifo_PushByte(RingFifo_t *fifo, uint8_t data);
bool RingFifo_PopByte(RingFifo_t *fifo, uint8_t *data);
uint16_t RingFifo_Count(const RingFifo_t *fifo);
void RingFifo_Clear(RingFifo_t *fifo);

#endif /* RING_FIFO_H */
