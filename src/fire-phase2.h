#ifndef FIREPHASE2_H
#define FIREPHASE2_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "dtype.h"
#include "alloc.h"
#include "queue.h"
#include "spiral.h"
#include "vutils.h"
#include "fire-funs.h"


int firespread_phase2(int nx, int ny, int nc, args_t *args, fire_t *FIRE);
int phase2_subsegment(int nx, int ny, int nc, int *nsub, int *mintime, args_t *args, fire_t *FIRE, phase2_subobjects_t *phase2_subobj);
int phase2_seeds_for_subsegemts(int nx, int ny, int nsub, fire_t *FIRE, phase2_subobjects_t *phase2_subobj);
int phase2_reassign_invalid_patches(int nx, int ny, int nc, int *D, int *msub_invalid, args_t *args, fire_t *FIRE, phase2_subobjects_t *phase2_subobj, phase2_adjacent_t *phase2_adjobj);
int phase2_add_new_patches(int nx, int ny, int nc, int *mintime, args_t *args, fire_t *FIRE, phase2_subobjects_t *phase2_subobj);
int phase2_fix_dangling_subpatches(int nx, int ny, int nc, int nsub, int mintime, args_t *args, fire_t *FIRE, phase2_subobjects_t *phase2_subobj);

#ifdef __cplusplus
}
#endif

#endif
