#ifndef OGR_H
#define OGR_H

/** Geospatial Data Abstraction Library (GDAL) **/
#include "gdal.h"           // public (C callable) GDAL entry points
#include "ogr_spatialref.h" // coordinate systems services
#include "ogr_api.h"        // OGR geometry and feature definition


#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>

#include "warp.h"

int ogr_create_field(char *name,  OGRFieldType datatype, int width, OGRLayerH *layer);
int ogr_write(char *fname, double *geotran, char *proj, int season, int nfire, int *OBJ_ID, int **OBJ_SEED, int *FIRE_HIST, int *OBJ_STARTTIME, int *OBJ_LIFETIME);

#ifdef __cplusplus
}
#endif

#endif
