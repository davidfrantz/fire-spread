#include "read.h"


int read_dataset(int *nx, int *ny, int *nc, bands_t *bands, double *geotran, int2u ***INP, char *proj, args_t *args){
GDALDatasetH dataset = NULL;
GDALRasterBandH band = NULL;
char buffer[1024] = "\0";
char cy[5], cm[3];
FILE *fp = NULL;
int2u **input = NULL;
int ncol, nrow, ncell;
int b;




  if ((dataset = GDALOpen(args->finp, GA_ReadOnly)) == NULL){
        printf("unable to open %s\n", args->finp); return 1;}

  ncol  = GDALGetRasterXSize(dataset);
  nrow  = GDALGetRasterYSize(dataset);
  ncell = ncol*nrow;

  bands->n = GDALGetRasterCount(dataset);
  alloc((void**)&bands->years,   bands->n, sizeof(int));
  alloc((void**)&bands->months,  bands->n, sizeof(int));
  alloc((void**)&bands->seasons, bands->n, sizeof(int));


  if ((fp = fopen(args->fdat, "r")) == NULL){
    printf("Unable to open date file (%s)!\n", args->fdat); return 1; }

  for (b=0; b<bands->n; b++){

    if (fgets(buffer, 1024, fp) == NULL){
      printf("Unable to read from date file, line %d!\n", bands->n); return 1; }

    strncpy(cy, buffer, 4);
    strncpy(cm, buffer+4, 2);
    cy[4] = '\0'; bands->years[b]  = atoi(cy);
    cm[2] = '\0'; bands->months[b] = atoi(cm);

  }

  fclose(fp);

  
  GDALGetGeoTransform(dataset, geotran);
  copy_string(proj, 1024, GDALGetProjectionRef(dataset));


  alloc_2D((void***)&input, bands->n, ncell, sizeof(int2u));

  for (b=0; b<bands->n; b++){
    band = GDALGetRasterBand(dataset, b+1);
    if (GDALRasterIO(band, GF_Read, 0, 0, ncol, nrow, input[b], 
      ncol, nrow, GDT_UInt16, 0, 0) == CE_Failure){
      printf("could not read band #%d from %s.\n", b+1, args->finp); return 1;}
  }
  GDALClose(dataset);

	*INP = input;
	*nx = ncol;
	*ny = nrow;
	*nc = ncell;
	return 0;
}
