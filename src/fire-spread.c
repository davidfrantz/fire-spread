#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "math.h"

#include <omp.h>

#include "cpl_conv.h"       // various convenience functions for CPL
#include "gdal.h"           // public (C callable) GDAL entry points
#include "cpl_string.h"

#include "dtype.h"
#include "dtype.h"
#include "alloc.h"
#include "angle.h"
#include "date.h"
#include "spiral.h"
#include "write.h"
#include "queue.h"
#include "focalfuns.h"
#include "string.h"
#include "vutils.h"
#include "fire-phase1.h"
#include "fire-phase2.h"


void month_minimum(int2u **INP, int nc, bands_t *bands){
off_t count_per_month[13], min_count;
int p, b, m;

  for (m=0; m<13; m++) count_per_month[m] = 0;

  for (b=0; b<bands->n; b++){

    for (p=0; p<nc; p++){
      if (INP[b][p] > 0 && INP[b][p] < 367){
        count_per_month[bands->months[b]]++;
      } 
    }

  }

  min_count = count_per_month[1];
  bands->min_month = 1;
  for (m=2; m<13; m++){
    if (count_per_month[m] < min_count){
      min_count = count_per_month[m];
      bands->min_month = m;
    }
  }

  bands->min_doy = md2doy(bands->min_month, 1);


  for (m=1; m<13; m++) printf("burned pixels in month %02d: %lu\n", m, count_per_month[m]);
  printf("Burning minimum detected in month %02d, DOY %d\n", bands->min_month, bands->min_doy);

  return;
}


void sort_into_seasons(bands_t *bands){
int b;
int season = -1;
bool error = false;


  // make sure that there are no gaps
  for (b=1; b<bands->n; b++){
    
    if ((bands->years[b]  - bands->years[b-1])  == 0 &&
        (bands->months[b] - bands->months[b-1]) != 1){
          error = true;
                printf("1.\n");
    } else if ((bands->years[b]  - bands->years[b-1])  ==   1 &&
               (bands->months[b] - bands->months[b-1]) != -11){
          error = true;
                printf("2.\n");
    } else if ((bands->years[b] - bands->years[b-1]) != 0 &&
               (bands->years[b] - bands->years[b-1]) != 1){
          error = true;
          printf("3.\n");
    }

  }

  if (error){ 
    printf("bands/months are not gap-free, fix input data..\n");
    exit(1);
  }


  // label seasons
  for (b=0; b<bands->n; b++){

    if (bands->months[b] == bands->min_month) season++;

    bands->seasons[b] = season;

  }
  bands->nseasons = season;


  printf("%04d %02d %02d\n", bands->years[0], bands->months[0], bands->seasons[0]);
  for (b=1; b<bands->n; b++){
    if (bands->seasons[b] > bands->seasons[b-1]){
      printf("%04d %02d %02d\n", bands->years[b], bands->months[b], bands->seasons[b]);
    }
  } 
  printf("processing of %02d seasons.\n", bands->nseasons);

  return;
}



int main( int argc, char *argv[] ){

args_t args;



int i, ii, j, jj, inew, jnew;
int b, t, t0, t1, p; 
int id, k;
int ntotal=0;

FILE *fp = NULL;
char fname[1024];


int2u **INP = NULL;
fire_t FIRE;

char buffer[1024] = "\0", cy[5], cm[3];
int S;
bands_t bands;
int nx, ny, nc;

GDALDatasetH dataset = NULL;
GDALRasterBandH band = NULL;
double geotran[6];
char proj[1024];



  args.n = 14;
  if (argc != args.n){ //argv[0] is program name
    printf( "usage: %s input-stack input-dates output-dir basename\n", argv[0]);
    printf("                init-searchdist track-searchdist\n");
    printf("                temp-dist density-dist min-size smooth-dist\n");
    printf("                ncpu queue-size verbose\n");
    exit(0);
  }



  // INPUT VARIABLES
  copy_string(args.finp, 1024, argv[1]);
  copy_string(args.fdat, 1024, argv[2]);
  copy_string(args.dout, 1024, argv[3]);
  copy_string(args.bout, 1024, argv[4]);
  args.init__dist = atoi(argv[5]);   // 10
  args.track_dist = atoi(argv[6]);   // 15
  args.temp__dist = atoi(argv[7]);   // 5
  args.densi_dist = atoi(argv[8]);   // 10
  args.min___size = atoi(argv[9]);   // 5
  args.smoothdist = atoi(argv[10]); // 3
  args.ncpu       = atoi(argv[11]); // ?
  args.queue_size = atoi(argv[12]); // ?
  if        (strcmp(argv[13], "q") == 0){ args.v = false;
  } else if (strcmp(argv[13], "v") == 0){ args.v = true;
  } else { printf("error: last option must be q or v\n"); exit(1);}

  if (args.init__dist < 2){ printf("init-searchdist must be > 1.\n"); exit(1);}
  if (args.track_dist < 2){ printf("track-searchdist must be > 1.\n"); exit(1);}
  if (args.temp__dist < 1){ printf("temp-dist must be > 0.\n"); exit(1);}
  if (args.densi_dist < 1){ printf("density-dist must be > 1.\n"); exit(1);}
  if (args.min___size < 0){ printf("min-size must be >= 0.\n"); exit(1);}
  if (args.smoothdist < 0){ printf("smooth-dist must be >= 0.\n"); exit(1);}


  GDALAllRegister();
  OGRRegisterAll();




  /** read data
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/

  if ((dataset = GDALOpen(args.finp, GA_ReadOnly)) == NULL){
        printf("unable to open %s\n", args.finp); exit(1);}

  nx = GDALGetRasterXSize(dataset);
  ny = GDALGetRasterYSize(dataset);
  nc = nx*ny;

  bands.n = GDALGetRasterCount(dataset);
  alloc((void**)&bands.years,   bands.n, sizeof(int));
  alloc((void**)&bands.months,  bands.n, sizeof(int));
  alloc((void**)&bands.seasons, bands.n, sizeof(int));


  if ((fp = fopen(args.fdat, "r")) == NULL){
    printf("Unable to open date file (%s)!\n", args.fdat); exit(1); }

  for (b=0; b<bands.n; b++){

    if (fgets(buffer, 1024, fp) == NULL){
      printf("Unable to read from date file, line %d!\n", bands.n); exit(1); }

    strncpy(cy, buffer, 4);
    strncpy(cm, buffer+4, 2);
    cy[4] = '\0'; bands.years[b]  = atoi(cy);
    cm[2] = '\0'; bands.months[b] = atoi(cm);

  }

  fclose(fp);


  
  GDALGetGeoTransform(dataset, geotran);
  copy_string(proj, 1024, GDALGetProjectionRef(dataset));


  alloc_2D((void***)&INP, bands.n, nc, sizeof(int2u));

  for (b=0; b<bands.n; b++){
    band = GDALGetRasterBand(dataset, b+1);
    if (GDALRasterIO(band, GF_Read, 0, 0, nx, ny, INP[b], 
      nx, ny, GDT_UInt16, 0, 0) == CE_Failure){
      printf("could not read band #%d from %s.\n", b+1, args.finp); exit(1);}
  }
  GDALClose(dataset);




  month_minimum(INP, nc, &bands);
  sort_into_seasons(&bands);

  omp_set_num_threads(args.ncpu);


  #pragma omp parallel default(none) private(b,p,i,j,ii,jj,inew,jnew,t,t0,t1,id,k,FIRE,fp,fname) firstprivate(ntotal) shared(INP,bands,nx,ny,nc,proj,geotran,args)
  {

  #pragma omp for schedule(dynamic,1)
  for (S=bands.seasons[0]; S<=bands.seasons[bands.n-1]; S++){

    alloc((void**)&FIRE.OLD_BOOL, nc, sizeof(bool));
    alloc((void**)&FIRE.OLD_SEGM, nc, sizeof(int));

    alloc((void**)&FIRE.FIRE_TIME, nc, sizeof(int));
    alloc((void**)&FIRE.FIRE_SEED, nc, sizeof(int));

    alloc((void**)&FIRE.VISITED, nc, sizeof(bool));

    alloc((void**)&FIRE.OBJ_ID, MAX_OBJECTS, sizeof(int));
    alloc_2D((void***)&FIRE.OBJ_SEED, 2, MAX_OBJECTS, sizeof(int));
    alloc_3D((void****)&FIRE.OBJ_GAIN, 366, 9, MAX_OBJECTS, sizeof(int));


    if (firespread_phase1(INP, &bands, nx, ny, nc, S, &args, &FIRE) != 0){
      printf("error in phase 1 for season %d\n", S);
      exit(1);
    }

    if (firespread_phase2(nx, ny, nc, &args, &FIRE) != 0){
      printf("error in phase 1 for season %d\n", S);
      exit(1);
    }


    free((void*)FIRE.VISITED);




    alloc((void**)&FIRE.FIRE_DENSITY, nc, sizeof(int));

    // *** COMPUTE FIRE DENSITY **************************************
    // ***************************************************************
    for (i=0, p=0; i<ny; i++){
    for (j=0; j<nx; j++, p++){
      k = 0;
      for (ii=-1*args.densi_dist; ii<=args.densi_dist; ii++){
      for (jj=-1*args.densi_dist; jj<=args.densi_dist; jj++){
        inew = i+ii; jnew = j+jj;
        if (inew < 0 || inew >= ny || jnew < 0 || jnew >= nx) continue;
        if (FIRE.FIRE_SEED[nx*inew+jnew] < 1 || sqrt(ii*ii+jj*jj) > args.densi_dist) continue;
        k++;
      }
      }
      FIRE.FIRE_DENSITY[p] = k;
    }
    }

    // *** COMPUTE THE LIFETIME OF EACH FIRE *************************
    // ***************************************************************
    alloc((void**)&FIRE.OBJ_LIFETIME, FIRE.nfire, sizeof(int));
    alloc((void**)&FIRE.OBJ_STARTTIME, FIRE.nfire, sizeof(int));
    for (id=0; id<FIRE.nfire; id++){
      for (t=0, t0=0, t1=0; t<366; t++){

        if (FIRE.OBJ_GAIN[t][0][id] == 0){
          continue;
        } else if (FIRE.OBJ_GAIN[t][0][id] > 0){
          if (t0 == 0){
            t0 = t;
          } else {
            t1 = t;
          }
        }

      }
      if (t1 == 0) t1 = t0;
      FIRE.OBJ_LIFETIME[id]  = t1-t0+1;
      FIRE.OBJ_STARTTIME[id] = t0+1;
    }

    // *** COMPUTE THE TOTAL SIZE OF EACH FIRE ***********************
    // ***************************************************************
    alloc((void**)&FIRE.FIRE_HIST, FIRE.nfire, sizeof(int));
    
    for (p=0; p<nc; p++){
      if (FIRE.OLD_BOOL[p]) FIRE.FIRE_HIST[FIRE.OLD_SEGM[p]-1]++;
    }
    for (id=0; id<FIRE.nfire; id++) if (FIRE.FIRE_HIST[id] > 0) ntotal++;
    if (args.v) printf("total patches new: %d\n", ntotal);


    // write basic shapefile
     sprintf(fname, "%s/fire-spread_%s_season-%02d.gpkg", args.dout, args.bout, S);
    vector_write(fname, geotran, proj, S, &bands, &FIRE);

    // write basic table
     sprintf(fname, "%s/fire-spread_%s_season-%02d.csv", args.dout, args.bout, S);
    basic_write(fname, geotran, proj, S, &bands, &FIRE);

    // write extended table
    sprintf(fname, "%s/fire-spread_%s_season-%02d_extended.csv", args.dout, args.bout, S);
    extended_write(fname, geotran, proj, S, &bands, &FIRE);

    // write raster image
    sprintf(fname, "%s/fire-spread_%s_season-%02d.tif", args.dout, args.bout, S);
    raster_write(fname, nx, ny, geotran, proj, &FIRE);


    free((void*)FIRE.OLD_BOOL);
    free((void*)FIRE.OLD_SEGM);

    free((void*)FIRE.FIRE_TIME);
    free((void*)FIRE.FIRE_SEED);
    free((void*)FIRE.FIRE_HIST);
    free((void*)FIRE.FIRE_DENSITY);

    free((void*)FIRE.OBJ_ID);
    free((void*)FIRE.OBJ_LIFETIME);
    free((void*)FIRE.OBJ_STARTTIME);
    free_2D((void**)FIRE.OBJ_SEED, 2);
    free_3D((void***)FIRE.OBJ_GAIN, 366, 9);

  }

  }

  
  free_2D((void**)INP, bands.n);

  return 0;
}

