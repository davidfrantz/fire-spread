#ifndef FIREFUNS_H
#define FIREFUNS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "dtype.h"
#include "spiral.h"


int localize_fire_seeds(int nx, int ny, int nc, fire_t *FIRE);

#ifdef __cplusplus
}
#endif

#endif
