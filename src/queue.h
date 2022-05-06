#ifndef QUEUE_H
#define QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>

typedef struct queue
{
  int length;
  struct queue_elmt *first, *last;
} queue_t;


queue_t* create_queue();
void destroy_queue(queue_t* q);

int enqueue(queue_t* q, int row, int col);
int dequeue(queue_t* q, int* row, int* col);

#ifdef __cplusplus
}
#endif

#endif
