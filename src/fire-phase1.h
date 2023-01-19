#ifndef FIREPHASE1_H
#define FIREPHASE1_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "dtype.h"
#include "alloc.h"
#include "focalfuns.h"
#include "queue.h"
#include "spiral.h"
#include "vutils.h"
#include "fire-funs.h"


int firespread_phase1(int2u **INP, bands_t *bands, int nx, int ny, int nc, int season, args_t *args, fire_t *FIRE);
int phase1_directional_mask(int **dirmask, int distance);
int phase1_prepare(int2u **INP, bands_t *bands, int nx, int ny, int nc, int season, args_t *args, fire_t *FIRE, phase1_t *phase1);
int phase1_find_initial_seeds(int nx, int ny, int nc, args_t *args, fire_t *FIRE, phase1_t *phase1);
int phase1_enqueue_active_fires(int t, int nx, int ny, int nc, queue_t *fifo, args_t *args, fire_t *FIRE, phase1_t *phase1);
int phase1_trace_active_fires(int t, int nx, int ny, int nc, queue_t *fifo, args_t *args, fire_t *FIRE, phase1_t *phase1);
int phase1_find_next_seeds(int t, int nx, int ny, int nc, args_t *args, fire_t *FIRE, phase1_t *phase1);

#ifdef __cplusplus
}
#endif

#endif
