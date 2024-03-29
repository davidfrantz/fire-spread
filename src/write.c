#include "write.h"


date_t true_date(int relative_doy, int season, bands_t *bands, char formatted[], size_t size){
date_t date;
int b;


    for (b=0; b<bands->n; b++){
      if (season >= 0){
        if (bands->seasons[b] == season){
          date.year = bands->years[b];
          break;
        }
      } else {
        if (bands->seasons[b] == season+1){
          date.year = bands->years[b]-1;
          break;
        }
      }
    } 

    date.doy = relative_doy + bands->min_doy - 1;
    if (date.doy >= 366){
      date.doy -= 366;
      date.year++;
    } 

    date.month = doy2m(date.doy);
    date.day   = doy2d(date.doy);
    compact_date(date.year, date.month, date.day, formatted, size);

  return date;
}


int main_direction(int id, fire_t *FIRE){
int t, d, dmax = 0;
double num[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  for (t=0; t<365; t++){

    for (d=1; d<9; d++){ 
      num[d] += FIRE->OBJ_GAIN[t][d][id];
    }

  }


  for (d=1; d<9; d++){ 
    if (num[d] > num[d-1]) dmax = d;
  }


  return dmax;
}


int ogr_create_field(char const *name, OGRFieldType datatype, int width, OGRLayerH *layer){
OGRFieldDefnH field;

  field = OGR_Fld_Create(name, datatype);
  OGR_Fld_SetWidth(field, width);
  if (OGR_L_CreateField(*layer, field, TRUE) != OGRERR_NONE){
      printf("Error creating field.\n"); return 1;}
  OGR_Fld_Destroy(field);

  return 0;
}


int vector_write(char *fname, double *geotran, char *proj, int season, bands_t *bands, fire_t *FIRE){
GDALDriverH driver;
OGRSpatialReferenceH srs;
GDALDatasetH fp;
OGRLayerH layer;
OGRFeatureH feature;
OGRGeometryH point;
double map_x, map_y, lon, lat;
int id;
date_t date;
char datestring[1024];
char directions[9][16] = { "NA", "N", "NE", "E", "SE", "S", "SW", "W", "NW" };
int dir;


  if ((driver = OGRGetDriverByName("GPKG")) == NULL){
      fprintf(stderr, "Geopackage driver is not available.\n"); return 1;}

  if ((fp = OGR_Dr_CreateDataSource(driver, fname, NULL)) == NULL){
      printf("Error creating output file.\n"); return 1;}

  srs = OSRNewSpatialReference(NULL);
  OSRImportFromEPSG(srs, 4326);

  // create layer
  if ((layer = OGR_DS_CreateLayer(fp, "ignition-points", srs, 
          wkbPoint, NULL)) == NULL){
      printf("Error creating layer.\n"); return 1;}

  // add fields
  ogr_create_field("ID",             OFTInteger, 12, &layer);
  ogr_create_field("season",         OFTInteger, 12, &layer);
  ogr_create_field("doy_relative",   OFTInteger, 12, &layer);
  ogr_create_field("date",           OFTString,  12, &layer);
  ogr_create_field("year",           OFTString,  12, &layer);
  ogr_create_field("month",          OFTString,  12, &layer);
  ogr_create_field("day",            OFTString,  12, &layer);
  ogr_create_field("doy",            OFTString,  12, &layer);
  ogr_create_field("area",           OFTInteger, 12, &layer);
  ogr_create_field("lifetime",       OFTInteger, 12, &layer);
  ogr_create_field("main_direction", OFTString,  12, &layer);

  for (id=0; id<FIRE->nfire; id++){

    if (FIRE->FIRE_HIST[id] == 0) continue;

    date = true_date(FIRE->OBJ_STARTTIME[id], season, bands, datestring, 1024);
    dir = main_direction(id, FIRE);

    // create feature
    feature = OGR_F_Create(OGR_L_GetLayerDefn(layer));

    // set fields
    OGR_F_SetFieldInteger(feature, OGR_F_GetFieldIndex(feature, "ID"), FIRE->OBJ_ID[id]);
    OGR_F_SetFieldInteger(feature, OGR_F_GetFieldIndex(feature, "season"), season);
    OGR_F_SetFieldInteger(feature, OGR_F_GetFieldIndex(feature, "doy_relative"), FIRE->OBJ_STARTTIME[id]);
    OGR_F_SetFieldString(feature,  OGR_F_GetFieldIndex(feature, "date"),  datestring);
    OGR_F_SetFieldInteger(feature, OGR_F_GetFieldIndex(feature, "year"),  date.year);
    OGR_F_SetFieldInteger(feature, OGR_F_GetFieldIndex(feature, "month"), date.month);
    OGR_F_SetFieldInteger(feature, OGR_F_GetFieldIndex(feature, "day"),   date.day);
    OGR_F_SetFieldInteger(feature, OGR_F_GetFieldIndex(feature, "doy"),   date.doy);
    OGR_F_SetFieldInteger(feature, OGR_F_GetFieldIndex(feature, "area"), FIRE->FIRE_HIST[id]);
    OGR_F_SetFieldInteger(feature, OGR_F_GetFieldIndex(feature, "lifetime"), FIRE->OBJ_LIFETIME[id]);
    OGR_F_SetFieldString(feature,  OGR_F_GetFieldIndex(feature, "main_direction"), directions[dir]);

    map_x = geotran[0] + FIRE->OBJ_SEED[0][id]*geotran[1];
    map_y = geotran[3] + FIRE->OBJ_SEED[1][id]*geotran[5];

    //printf("map x/y: %f/%f\n", map_x, map_y);
    //printf("map proj: %s\n", proj);
    //printf("geo x/y: %f/%f\n", lon, lat);

    warp_any_to_geo(map_x, map_y, &lon, &lat, proj);


    // create local geometry
    point = OGR_G_CreateGeometry(wkbPoint);
    OGR_G_SetPoint_2D(point, 0, lon,  lat);
    OGR_F_SetGeometry(feature, point);
    OGR_G_DestroyGeometry(point);

    // create feature in the file
    if (OGR_L_CreateFeature(layer, feature) != OGRERR_NONE){
    printf("Error creating feature in file.\n"); return 1;}
    OGR_F_Destroy(feature);

  }

  OGR_DS_Destroy(fp);

  return 0;
}



int basic_write(char *fname, double *geotran, char *proj, int season, bands_t *bands, fire_t *FIRE){
OGRSpatialReferenceH srs;
FILE *fp = NULL;
double map_x, map_y, lon, lat;
int id;
date_t date;
char datestring[1024];
char directions[9][16] = { "NA", "N", "NE", "E", "SE", "S", "SW", "W", "NW" };
int dir;


  fp = fopen(fname, "w");

  srs = OSRNewSpatialReference(NULL);
  OSRImportFromEPSG(srs, 4326);

  fprintf(fp, "ID,season,doy_relative,date,year,month,day,doy,area,lifetime,main_direction,longitude,latitude\n");

  for (id=0; id<FIRE->nfire; id++){

    if (FIRE->FIRE_HIST[id] == 0) continue;

    date = true_date(FIRE->OBJ_STARTTIME[id], season, bands, datestring, 1024);
    dir = main_direction(id, FIRE);

    map_x = geotran[0] + FIRE->OBJ_SEED[0][id]*geotran[1];
    map_y = geotran[3] - FIRE->OBJ_SEED[1][id]*geotran[5];

    //printf("map x/y: %f/%f\n", map_x, map_y);
    //printf("map proj: %s\n", proj);
    //printf("geo x/y: %f/%f\n", lon, lat);

    warp_any_to_geo(map_x, map_y, &lon, &lat, proj);

    fprintf(fp, "%d,%d,%d,%s,%d,%d,%d,%d,%d,%d,%s,%f,%f\n", 
      FIRE->OBJ_ID[id],
      season,
      FIRE->OBJ_STARTTIME[id],
      datestring,
      date.year,
      date.month,
      date.day,
      date.doy,
      FIRE->FIRE_HIST[id],
      FIRE->OBJ_LIFETIME[id],
      directions[dir],
      lon,
      lat);

  }

  fclose(fp);

  return 0;
}


int extended_write(char *fname, double *geotran, char *proj, int season, bands_t *bands, fire_t *FIRE){
FILE *fp = NULL;
int id, t, d;
date_t date;
char datestring[1024];
char directions[9][16] = { "IGN", "N", "NE", "E", "SE", "S", "SW", "W", "NW" };
bool ignited;


  fp = fopen(fname, "w");

  fprintf(fp, "ID,spread_date,spread_year,spread_month,spread_day,spread_doy,spread_size,spread_direction\n");


  for (id=0; id<FIRE->nfire; id++){

    if (FIRE->FIRE_HIST[id] == 0) continue;

    ignited = false;

    for (t=0; t<365; t++){

      date = true_date(t+1, season, bands, datestring, 1024);

      if (!ignited && FIRE->OBJ_GAIN[t][0][id] > 0){
        fprintf(fp, "%d,%s,%d,%d,%d,%d,%d,%s\n", 
          FIRE->OBJ_ID[id], 
          datestring,
          date.year,
          date.month,
          date.day,
          date.doy,
          FIRE->OBJ_GAIN[t][0][id],
          directions[0]
        );
        ignited = true;
      }


      for (d=1; d<9; d++){ 

        if (FIRE->OBJ_GAIN[t][d][id] == 0) continue;

        fprintf(fp, "%d,%s,%d,%d,%d,%d,%d,%s\n", 
          FIRE->OBJ_ID[id], 
          datestring,
          date.year,
          date.month,
          date.day,
          date.doy,
          FIRE->OBJ_GAIN[t][d][id],
          directions[d]
        );

      }

    }


  }

  fclose(fp);

  return 0;
}


int raster_write(char *fname, int nx, int ny, double *geotran, char *proj, fire_t *FIRE){
GDALDatasetH file = NULL;
GDALRasterBandH band = NULL;
GDALDriverH driver = NULL;
char **options = NULL;


  options = CSLSetNameValue(options, "TILED", "YES");
  options = CSLSetNameValue(options, "COMPRESS", "LZW");
  options = CSLSetNameValue(options, "PREDICTOR", "2");
  options = CSLSetNameValue(options, "INTERLEAVE", "BAND");
  options = CSLSetNameValue(options, "BIGTIFF", "YES");

  if ((driver = GDALGetDriverByName("GTiff")) == NULL){
    printf("%s driver not found\n", "GTiff"); exit(1);}

  if ((file = GDALCreate(driver, fname, nx, ny, 4, GDT_Int32, options)) == NULL){
    printf("Error creating memory file %s. ", fname); exit(1);}

  band = GDALGetRasterBand(file, 1);
  if (GDALRasterIO(band, GF_Write, 0, 0, 
        nx, ny, FIRE->OLD_SEGM, 
        nx, ny, GDT_Int32, 0, 0) == CE_Failure){
        printf("Unable to write %s. ", fname); exit(1);}
  GDALSetDescription(band, "fire segmentation");
  GDALSetRasterNoDataValue(band, 0);

  band = GDALGetRasterBand(file, 2);
  if (GDALRasterIO(band, GF_Write, 0, 0, 
        nx, ny, FIRE->FIRE_SEED, 
        nx, ny, GDT_Int32, 0, 0) == CE_Failure){
        printf("Unable to write %s. ", fname); exit(1);}
  GDALSetDescription(band, "fire seeds");
  GDALSetRasterNoDataValue(band, 0);

  band = GDALGetRasterBand(file, 3);
  if (GDALRasterIO(band, GF_Write, 0, 0, 
        nx, ny, FIRE->FIRE_TIME, 
        nx, ny, GDT_Int32, 0, 0) == CE_Failure){
        printf("Unable to write %s. ", fname); exit(1);}
  GDALSetDescription(band, "fire timing");
  GDALSetRasterNoDataValue(band, 0);

  band = GDALGetRasterBand(file, 4);
  if (GDALRasterIO(band, GF_Write, 0, 0, 
        nx, ny, FIRE->FIRE_DENSITY, 
        nx, ny, GDT_Int32, 0, 0) == CE_Failure){
        printf("Unable to write %s. ", fname); exit(1);}
  GDALSetDescription(band, "fire density");
  GDALSetRasterNoDataValue(band, 0);

  #pragma omp critical
  {
    GDALSetGeoTransform(file, geotran);
    GDALSetProjection(file,   proj);
  }
  
  GDALClose(file);

  CSLDestroy(options);


  return 0;
}