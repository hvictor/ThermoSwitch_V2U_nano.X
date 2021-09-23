#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include "../include/os_queue.h"

/*
void OS_Queue_Init(OS_Queue *q)
{
	q->count = 0;
	q->head = q->a;
	q->tail = q->a;

	q->empty = 1;
	q->full = 0;
}

void OS_Queue_Reset(OS_Queue *q)
{
	q->count = 0;
	q->head = q->a;
	q->tail = q->a;

	q->empty = 1;
	q->full = 0;
}

int OS_Queue_Push(OS_Queue *q, void *data)
{
	if (q->full)
        return -1;

	q->tail->data = data;
	
	if (q->tail == &(q->a[OS_QUEUE_MAX - 1])) {
		q->tail = &(q->a[0]);
	}
	else {
		q->tail++;
	}
	
	q->count++;
    q->empty = 0;
    
    if (q->count == OS_QUEUE_MAX) {
        q->full = 1;
    }

	return 0;
}

int OS_Queue_Pull(OS_Queue *q, void **data_dest)
{
	if (q->empty)
        return -1;

	if (q->head == &(q->a[OS_QUEUE_MAX - 1])) {
		q->head = &(q->a[0]);
	}
	else {
		q->head++;
	}

	q->count--;
	q->full = 0;
	
    if (q->count == 0) {
        q->empty = 1;
    }
    
	return 0;
}
 */