#ifndef SPIRAL_H
#define SPIRAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "math.h"
#include "dtype.h"
#include "angle.h"


bool xyfocal_to_p(int nx, int ny, int x, int y, int xx, int yy, int *xnew, int *ynew, int *pnew);
int xy_to_p(int nx, int x, int y);
bool spiral(int *rx, int *ry, int hnx, int hny, int i, int *x, int *y, int *dx, int *dy);
int radial_search_1(int nx, int ny, int i, int j, int pold, int v_center, int dist, 
          double *sumi, double *sumj, double *sumk, fire_t *FIRE, phase1_t *phase1);
int radial_search_2(int nx, int ny, int i, int j, int pold, int v_center, int dist,
          double *sumi, double *sumj, double *sumk, fire_t *FIRE, phase1_t *phase1);

int get_direction(int xseed, int yseed, int x, int y);


#ifdef __cplusplus
}
#endif

#endif
