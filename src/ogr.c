#include "ogr.h"


int ogr_create_field(char const *name,  OGRFieldType datatype, int width, OGRLayerH *layer){
OGRFieldDefnH field;

  field = OGR_Fld_Create(name, datatype);
  OGR_Fld_SetWidth(field, width);
  if (OGR_L_CreateField(*layer, field, TRUE) != OGRERR_NONE){
      printf("Error creating field.\n"); return 1;}
  OGR_Fld_Destroy(field);

  return 0;
}


int ogr_write(char *fname, double *geotran, char *proj, int season, int nfire, int *OBJ_ID, int **OBJ_SEED, int *FIRE_HIST, int *OBJ_STARTTIME, int *OBJ_LIFETIME){
GDALDriverH driver;
OGRSpatialReferenceH srs;
GDALDatasetH fp;
OGRLayerH layer;
OGRFieldDefnH field;
OGRFeatureH feature;
OGRGeometryH point;
double map_x, map_y, lon, lat;
int id;


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

    // create feature
    feature = OGR_F_Create(OGR_L_GetLayerDefn(layer));

    // set fields
    OGR_F_SetFieldInteger(feature, OGR_F_GetFieldIndex(feature, "ID"), OBJ_ID[id]);
    OGR_F_SetFieldInteger(feature, OGR_F_GetFieldIndex(feature, "season"), season);

    OGR_F_SetFieldInteger(feature, OGR_F_GetFieldIndex(feature, "area"), FIRE_HIST[id]);
    OGR_F_SetFieldInteger(feature, OGR_F_GetFieldIndex(feature, "doy_relative"), OBJ_STARTTIME[id]);
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
