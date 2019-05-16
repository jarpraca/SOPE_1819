#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "types.h"

struct Queue
{
    int front, rear, size;
    unsigned capacity;
    tlv_request_t *array;
};

struct Queue *createQueue(unsigned int capacity)
{
    struct Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->array = (tlv_request_t *)malloc(queue->capacity * sizeof(tlv_request_t));
    return queue;
}

int isFull(struct Queue *queue)
{
    return (queue->size == queue->capacity);
}

int isEmpty(struct Queue *queue)
{
    return (queue->size == 0);
}

int push(struct Queue *queue, tlv_request_t request)
{
    if (isFull(queue))
        return 1;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = request;
    queue->size = queue->size + 1;
    return 0;
}

tlv_request_t pop(struct Queue *queue)
{
    if (isEmpty(queue))
        return;
    tlv_request_t request = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return request;
}
