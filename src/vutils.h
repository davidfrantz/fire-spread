#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "dtype.h"

int    imaxoftwo(int x, int y);
int2u  imaxoftwo2u(int2u x, int2u y);
int2s  imaxoftwo2s(int2s x, int2s y);
int1u  imaxoftwo1u(int1u x, int1u y);
int1s  imaxoftwo1s(int1s x, int1s y);
float  fmaxoftwo(float x, float y);

int    iminoftwo(int x, int y);
int2u  iminoftwo2u(int2u x, int2u y);
int2s  iminoftwo2s(int2s x, int2s y);
int1u  iminoftwo1u(int1u x, int1u y);
int1s  iminoftwo1s(int1s x, int1s y);
float  fminoftwo(float x, float y);

int    imax(int *mat, int n);
int2u  imax2u(int2u *mat, int n);
int2s  imax2s(int2s *mat, int n);
int1u  imax1u(int1u *mat, int n);
int1s  imax1s(int1s *mat, int n);
float  floatmax(float *mat, int n);

int    imin(int *mat, int n);
int2u  imin2u(int2u *mat, int n);
int2s  imin2s(int2s *mat, int n);
int1u  imin1u(int1u *mat, int n);
int1s  imin1s(int1s *mat, int n);
float  floatmin(float *mat, int n);

// note that integer overflow may happen
int isum(int *x, int n);
int isum2u(int2u *x, int n);
int isum2s(int2s *x, int n);
int isum1u(int1u *x, int n);
int isum1s(int1s *x, int n);
float fsum(float *x, int n);
int bsum(bool *x, int n);

#ifdef __cplusplus
}
#endif

#endif
