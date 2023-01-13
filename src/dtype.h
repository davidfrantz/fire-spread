#ifndef DTYPE_H
#define DTYPE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int int4u;
typedef signed int int4s;
typedef unsigned short int int2u;
typedef signed short int int2s;
typedef unsigned char int1u;
typedef signed char int1s;


typedef struct {
  int n; // number of bands
  int *years; 
  int *months;
  int *seasons;
  int min_month;
  int min_doy;
  int nseasons;
} bands_t;


#ifdef __cplusplus
}
#endif

#endif
