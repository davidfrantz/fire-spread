#ifndef DATE_H
#define DATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "math.h"

typedef struct {
  int ce, day, doy, month, year;
} date_t;

int md2doy(int m, int d);

#ifdef __cplusplus
}
#endif

#endif
