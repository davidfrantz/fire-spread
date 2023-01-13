#ifndef WRITE_H
#define WRITE_H

/** Geospatial Data Abstraction Library (GDAL) **/
#include "gdal.h"           // public (C callable) GDAL entry points
#include "ogr_spatialref.h" // coordinate systems services
#include "ogr_api.h"        // OGR geometry and feature definition


#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>

#include "dtype.h"
#include "date.h"
#include "warp.h"

date_t true_date(int relative_doy, int season, bands_t *bands, char formatted[], size_t size);
int ogr_create_field(char *name,  OGRFieldType datatype, int width, OGRLayerH *layer);
int vector_write(char *fname, double *geotran, char *proj, int season, bands_t *bands, int nfire, int *OBJ_ID, int **OBJ_SEED, int *FIRE_HIST, int *OBJ_STARTTIME, int *OBJ_LIFETIME);
int basic_write(char *fname, double *geotran, char *proj, int season, bands_t *bands, int nfire, int *OBJ_ID, int **OBJ_SEED, int *FIRE_HIST, int *OBJ_STARTTIME, int *OBJ_LIFETIME);
int extended_write(char *fname, double *geotran, char *proj, int season, bands_t *bands, int nfire, int *OBJ_ID, int *FIRE_HIST, int ***OBJ_GAIN);

#ifdef __cplusplus
}
#endif

#endif
