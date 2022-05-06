#include "queue.h"

struct queue_elmt
{
  int r, c;
  struct queue_elmt *prev, *next;
};


queue_t* create_queue()
{
  queue_t *q = (queue_t*) malloc(sizeof(queue_t));

  q->length = 0;
  q->first = NULL;
  q->last = NULL;

  return q;
}

void destroy_queue(queue_t* q)
{
  struct queue_elmt *el, *tmp;

  if (q->length != 0) {

    el = q->first;

    while (el->next != NULL) {
      tmp = el;
      el = el->next;
      free((void*)tmp);
    }

    free((void*)el);
  }

  free((void*)q);
}

int enqueue(queue_t* q, int row, int col)
{
  struct queue_elmt *el;

  if ((el = (struct queue_elmt*) malloc(sizeof(struct queue_elmt))) == NULL)
    return 0;

  el->r = row;
  el->c = col;
  el->prev = NULL;
  el->next = NULL;

  if (q->length == 0) {
    q->first = el;
    q->last = el;
  } else {
    q->last->next = el;
    el->prev = q->last;
    q->last = el;
  }

  q->length += 1;

  return 1;
}

int dequeue(queue_t* q, int* row, int* col)
{
  struct queue_elmt *el;

  if (q->length == 0) return 0;

  el = q->first;
  q->first = el->next;

  if (q->first == NULL) {
    q->last = NULL;
  } else {
    el->next->prev = NULL;
  }

  *row = el->r;
  *col = el->c;

  free((void*)el);

  q->length -= 1;

  return 1;
}
