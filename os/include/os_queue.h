#ifndef OS_QUEUE_H_
#define OS_QUEUE_H_

//#include <stdint.h>

#define OS_QUEUE_MAX 80

/*
static struct QueueObject
{
	void *data;
};

typedef struct
{
	volatile int count;
    uint8_t empty;
    uint8_t full;

	struct QueueObject *head;
	struct QueueObject *tail;

	struct QueueObject a[OS_QUEUE_MAX];
} OS_Queue;

void OS_Queue_Init(OS_Queue *q);
void OS_Queue_Reset(OS_Queue *q);
int OS_Queue_Push(OS_Queue *q, void *data);
int OS_Queue_Pull(OS_Queue *q, void **data_dest);
*/

#endif