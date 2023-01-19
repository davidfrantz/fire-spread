#include "queue.h"


/** This function creates a FIFO queue using a circular buffer. Free with 
+++ destroy_queue.
--- q:      queue
--- size:   size of the buffer
+++ Return: SUCCESS/FAILURE
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
int create_queue(queue_t *q, int size){

  q->head = 0;
  q->tail = 0;
  q->size = size;

  alloc((void**)&q->buf_x, size, sizeof(short)); // dies on failure
  alloc((void**)&q->buf_y, size, sizeof(short)); // dies on failure

  return 0;
}


/** This function frees a FIFO queue
--- q:      queue
+++ Return: void
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
void destroy_queue(queue_t *q){

  q->head = 0;
  q->tail = 0;
  q->size = 0;
  
  free((void*)q->buf_x); q->buf_x = NULL;
  free((void*)q->buf_y); q->buf_y = NULL;

  return;
}


/** This function puts an image coordinate to the FIFO queue
--- q:      queue
--- x:      column
--- y:      row
+++ Return: SUCCESS/FAILURE
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
int enqueue(queue_t *q, int x, int y){

  if ((q->head+1 == q->tail) ||
      (q->head+1 == q->size && q->tail == 0)){
    printf("This is not a royal queue. Not enough space.\n");
    return 1; // head runs into tail (buffer is full)...
  } else {
    q->buf_x[q->head] = (short)x;
    q->buf_y[q->head] = (short)y;
    q->head++; // step forward head
    if(q->head == q->size) q->head = 0; // circular buffer
  }

  return 0;
}


/** This function pulls an image coordinate from the FIFO queue
--- q:      queue
--- x:      column
--- y:      row
+++ Return: SUCCESS/FAILURE
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
int dequeue(queue_t *q, int *x, int *y){


  if (q->tail != q->head){ //see if any data is available
    *x = (int)q->buf_x[q->tail];
    *y = (int)q->buf_y[q->tail];
    q->tail++;  // step forward  tail
    if (q->tail == q->size) q->tail = 0; // circular buffer
  } else {
    return 1; // nothing in buffer
  }

  return 0;
}

