#ifndef DATE_H
#define DATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "math.h"

#include <stdio.h>   // core input and output functions
#include <stdlib.h>  // standard general utilities library


typedef struct {
  int ce, day, doy, month, year;
} date_t;

void compact_date(int y, int m, int d, char formatted[], size_t size);
int doy2m(int doy);
int doy2d(int doy);
int md2doy(int m, int d);


#ifdef __cplusplus
}
#endif

#endif
