#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "types.h"

typedef struct Queue
{
    int front, rear, size;
    unsigned int capacity;
    tlv_request_t *array;
} queue_t;

queue_t* createQueue(unsigned int capacity)
{
    queue_t *queue = (queue_t*)malloc(sizeof(queue_t));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->array = (tlv_request_t *)malloc(queue->capacity * sizeof(tlv_request_t));
    return queue;
}

int isFull(queue_t *queue)
{
    return (queue->size == queue->capacity);
}

bool isEmpty(queue_t *queue)
{
    return (queue->size == 0);
}

int push(queue_t *queue, tlv_request_t request)
{
    if (isFull(queue))
        return 1;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = request;
    queue->size = queue->size + 1;
    return 0;
}

tlv_request_t pop(queue_t *queue)
{
    tlv_request_t request;
    if (isEmpty(queue))
        return request;
    request = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return request;
}
