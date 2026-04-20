#include "ring_fifo.h"

void RingFifo_Init(RingFifo_t *fifo, uint8_t *buffer, uint16_t size)
{
    fifo->head = 0u;
    fifo->tail = 0u;
    fifo->size = size;
    fifo->buffer = buffer;
}

bool RingFifo_PushByte(RingFifo_t *fifo, uint8_t data)
{
    uint16_t next = (uint16_t)((fifo->head + 1u) % fifo->size);
    if (next == fifo->tail)
    {
        return false;
    }

    fifo->buffer[fifo->head] = data;
    fifo->head = next;
    return true;
}

bool RingFifo_PopByte(RingFifo_t *fifo, uint8_t *data)
{
    if (fifo->head == fifo->tail)
    {
        return false;
    }

    *data = fifo->buffer[fifo->tail];
    fifo->tail = (uint16_t)((fifo->tail + 1u) % fifo->size);
    return true;
}

uint16_t RingFifo_Count(const RingFifo_t *fifo)
{
    if (fifo->head >= fifo->tail)
    {
        return (uint16_t)(fifo->head - fifo->tail);
    }
    return (uint16_t)(fifo->size - fifo->tail + fifo->head);
}

void RingFifo_Clear(RingFifo_t *fifo)
{
    fifo->head = 0u;
    fifo->tail = 0u;
}
