#ifndef READ_H
#define READ_H

/** Geospatial Data Abstraction Library (GDAL) **/
#include "gdal.h"           // public (C callable) GDAL entry points

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "dtype.h"
#include "alloc.h"
#include "string.h"

int read_dataset(int *nx, int *ny, int *nc, bands_t *bands, double *geotran, int2u ***INP, char *proj, args_t *args);


#ifdef __cplusplus
}
#endif

#endif
