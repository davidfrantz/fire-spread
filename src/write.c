#include "write.h"


void true_date(int relative_doy, int season, bands_t *bands, char formatted[], size_t size){
int true_year, true_month, true_doy, true_day;
int b;


    for (b=0; b<bands->n; b++){
      if (bands->seasons[b] == season){
        true_year = bands->years[b];
        break;
      }
    } 

    true_doy = relative_doy + bands->min_doy - 1;
    if (true_doy >= 366){
      true_doy -= 366;
      true_year++;
    } 

    true_month = doy2m(true_doy);
    true_day   = doy2d(true_doy);
    compact_date(true_year, true_month, true_day, formatted, size);

  return;
}


int ogr_create_field(char const *name,  OGRFieldType datatype, int width, OGRLayerH *layer){
OGRFieldDefnH field;

  field = OGR_Fld_Create(name, datatype);
  OGR_Fld_SetWidth(field, width);
  if (OGR_L_CreateField(*layer, field, TRUE) != OGRERR_NONE){
      printf("Error creating field.\n"); return 1;}
  OGR_Fld_Destroy(field);

  return 0;
}


int vector_write(char *fname, double *geotran, char *proj, int season, bands_t *bands, int nfire, int *OBJ_ID, int **OBJ_SEED, int *FIRE_HIST, int *OBJ_STARTTIME, int *OBJ_LIFETIME){
GDALDriverH driver;
OGRSpatialReferenceH srs;
GDALDatasetH fp;
OGRLayerH layer;
OGRFeatureH feature;
OGRGeometryH point;
double map_x, map_y, lon, lat;
int id;
char datestring[1024];


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
  ogr_create_field("area",           OFTInteger, 12, &layer);
  ogr_create_field("lifetime",       OFTInteger, 12, &layer);
  ogr_create_field("main_direction", OFTString,  12, &layer);

  for (id=0; id<nfire; id++){

    if (FIRE_HIST[id] == 0) continue;

    true_date(OBJ_STARTTIME[id], season, bands, datestring, 1024);

    // create feature
    feature = OGR_F_Create(OGR_L_GetLayerDefn(layer));

    // set fields
    OGR_F_SetFieldInteger(feature, OGR_F_GetFieldIndex(feature, "ID"), OBJ_ID[id]);
    OGR_F_SetFieldInteger(feature, OGR_F_GetFieldIndex(feature, "season"), season);
    OGR_F_SetFieldInteger(feature, OGR_F_GetFieldIndex(feature, "doy_relative"), OBJ_STARTTIME[id]);
    OGR_F_SetFieldString(feature, OGR_F_GetFieldIndex(feature, "date"), datestring);
    OGR_F_SetFieldInteger(feature, OGR_F_GetFieldIndex(feature, "area"), FIRE_HIST[id]);
    OGR_F_SetFieldInteger(feature, OGR_F_GetFieldIndex(feature, "lifetime"), OBJ_LIFETIME[id]);

    map_x = geotran[0] + OBJ_SEED[0][id]*geotran[1];
    map_y = geotran[3] - OBJ_SEED[1][id]*geotran[5];

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



int basic_write(char *fname, double *geotran, char *proj, int season, bands_t *bands, int nfire, int *OBJ_ID, int **OBJ_SEED, int *FIRE_HIST, int *OBJ_STARTTIME, int *OBJ_LIFETIME){
OGRSpatialReferenceH srs;
FILE *fp = NULL;
double map_x, map_y, lon, lat;
int id;
char datestring[1024];


  fp = fopen(fname, "w");

  srs = OSRNewSpatialReference(NULL);
  OSRImportFromEPSG(srs, 4326);

  fprintf(fp, "ID,season,doy_relative,date,area,lifetime,main_direction,longitude,latitude\n");

  for (id=0; id<nfire; id++){

    if (FIRE_HIST[id] == 0) continue;

    true_date(OBJ_STARTTIME[id], season, bands, datestring, 1024);

    map_x = geotran[0] + OBJ_SEED[0][id]*geotran[1];
    map_y = geotran[3] - OBJ_SEED[1][id]*geotran[5];

    //printf("map x/y: %f/%f\n", map_x, map_y);
    //printf("map proj: %s\n", proj);
    //printf("geo x/y: %f/%f\n", lon, lat);

    warp_any_to_geo(map_x, map_y, &lon, &lat, proj);

    fprintf(fp, "%d,%d,%d,%s,%d,%d,%s,%f,%f\n", 
      OBJ_ID[id],
      season,
      OBJ_STARTTIME[id],
      datestring,
      FIRE_HIST[id],
      OBJ_LIFETIME[id],
      "TBD",
      lon,
      lat);

  }

  fclose(fp);

  return 0;
}


int extended_write(char *fname, double *geotran, char *proj, int season, bands_t *bands, int nfire, int *OBJ_ID, int *FIRE_HIST, int ***OBJ_GAIN){
FILE *fp = NULL;
int id, t, d;
char datestring[1024];
char directions[9][16] = { "IGN", "N", "NE", "E", "SE", "S", "SW", "W", "NW" };
bool ignited;


  fp = fopen(fname, "w");

  fprintf(fp, "ID,spread_date,spread_size,spread_direction\n");


  for (id=0; id<nfire; id++){

    if (FIRE_HIST[id] == 0) continue;

    ignited = false;

    for (t=0; t<365; t++){

      true_date(t+1, season, bands, datestring, 1024);

      if (!ignited && OBJ_GAIN[t][0][id] > 0){
        fprintf(fp, "%d,%s,%d,%s\n", 
          OBJ_ID[id], 
          datestring,
          OBJ_GAIN[t][0][id],
          directions[0]
        );
        ignited = true;
      }


      for (d=1; d<9; d++){ 

        if (OBJ_GAIN[t][d][id] == 0) continue;

        fprintf(fp, "%d,%s,%d,%s\n", 
          OBJ_ID[id], 
          datestring,
          OBJ_GAIN[t][d][id],
          directions[d]
        );

      }

    }


  }

  fclose(fp);

  return 0;
}
