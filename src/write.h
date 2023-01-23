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
int main_direction(int id, fire_t *FIRE);
int ogr_create_field(char *name,  OGRFieldType datatype, int width, OGRLayerH *layer);
int vector_write(char *fname, double *geotran, char *proj, int season, bands_t *bands, fire_t *FIRE);
int basic_write(char *fname, double *geotran, char *proj, int season, bands_t *bands, fire_t *FIRE);
int extended_write(char *fname, double *geotran, char *proj, int season, bands_t *bands, fire_t *FIRE);
int raster_write(char *fname, int nx, int ny, double *geotran, char *proj, fire_t *FIRE);

#ifdef __cplusplus
}
#endif

#endif
