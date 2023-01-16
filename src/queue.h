#ifndef QUEUE_H
#define QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include "alloc.h"

typedef struct {
  short *buf_x;
  short *buf_y;
  int head;
  int tail;
  int size;
} queue_t;

int create_queue(queue_t *q, int size);
void destroy_queue(queue_t* q);
int enqueue(queue_t *q, int x, int y);
int dequeue(queue_t *q, int *x, int *y);

#ifdef __cplusplus
}
#endif

#endif
