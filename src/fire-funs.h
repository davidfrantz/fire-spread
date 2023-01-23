#ifndef FIREFUNS_H
#define FIREFUNS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "dtype.h"
#include "date.h"
#include "spiral.h"


int month_minimum(int2u **INP, int nc, bands_t *bands);
int sort_into_seasons(bands_t *bands, args_t *args);
int localize_fire_seeds(int nx, int ny, int nc, fire_t *FIRE);
int fire_density(int nx, int ny, args_t *args, fire_t *FIRE);
int fire_lifetime(fire_t *FIRE);
int fire_size(int nc, args_t *args, fire_t *FIRE);

#ifdef __cplusplus
}
#endif

#endif
