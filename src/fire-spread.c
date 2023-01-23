#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <omp.h>
#include "gdal.h"

#include "dtype.h"
#include "alloc.h"
#include "read.h"
#include "write.h"
#include "fire-funs.h"
#include "fire-phase1.h"
#include "fire-phase2.h"


int main( int argc, char *argv[] ){
args_t args;
char fname[1024];
int2u **INP = NULL;
bands_t bands;
double geotran[6];
char proj[1024];
int nx, ny, nc;
fire_t FIRE;
int season;


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
  args.smoothdist = atoi(argv[10]);  // 3
  args.ncpu       = atoi(argv[11]);  // ?
  args.queue_size = atoi(argv[12]);  // ?
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

  
  read_dataset(&nx, &ny, &nc, &bands, geotran, &INP, proj, &args);

  month_minimum(INP, nc, &bands);
  sort_into_seasons(&bands, &args);

  omp_set_num_threads(args.ncpu);


  #pragma omp parallel default(none) private(FIRE,fname) shared(INP,bands,nx,ny,nc,proj,geotran,args)
  {

  #pragma omp for schedule(dynamic,1)
  for (season=bands.seasons[0]; season<=bands.seasons[bands.n-1]; season++){

    FIRE.nfire = 0;
    alloc((void**)&FIRE.OLD_BOOL, nc, sizeof(bool));
    alloc((void**)&FIRE.OLD_SEGM, nc, sizeof(int));
    alloc((void**)&FIRE.FIRE_TIME, nc, sizeof(int));
    alloc((void**)&FIRE.FIRE_SEED, nc, sizeof(int));
    alloc((void**)&FIRE.VISITED, nc, sizeof(bool));
    alloc((void**)&FIRE.OBJ_ID, MAX_OBJECTS, sizeof(int));
    alloc_2D((void***)&FIRE.OBJ_SEED, 2, MAX_OBJECTS, sizeof(int));
    alloc_3D((void****)&FIRE.OBJ_GAIN, 366, 9, MAX_OBJECTS, sizeof(int));


    if (firespread_phase1(INP, &bands, nx, ny, nc, season, &args, &FIRE) != 0){
      printf("error in phase 1 for season %d\n", season);
      exit(1);
    }

/**
    if (firespread_phase2(nx, ny, nc, &args, &FIRE) != 0){
      printf("error in phase 1 for season %d\n", season);
      exit(1);
    }
**/


    alloc((void**)&FIRE.FIRE_DENSITY, nc, sizeof(int));
    alloc((void**)&FIRE.OBJ_LIFETIME, FIRE.nfire, sizeof(int));
    alloc((void**)&FIRE.OBJ_STARTTIME, FIRE.nfire, sizeof(int));
    alloc((void**)&FIRE.FIRE_HIST, FIRE.nfire, sizeof(int));


    fire_density(nx, ny, &args, &FIRE);
    fire_lifetime(&FIRE);
    fire_size(nc, &args, &FIRE);


    // write basic shapefile
     sprintf(fname, "%s/fire-spread_%s_season-%02d.gpkg", args.dout, args.bout, season);
    vector_write(fname, geotran, proj, season, &bands, &FIRE);

    // write basic table
     sprintf(fname, "%s/fire-spread_%s_season-%02d.csv", args.dout, args.bout, season);
    basic_write(fname, geotran, proj, season, &bands, &FIRE);

    // write extended table
    sprintf(fname, "%s/fire-spread_%s_season-%02d_extended.csv", args.dout, args.bout, season);
    extended_write(fname, geotran, proj, season, &bands, &FIRE);

    // write raster image
    sprintf(fname, "%s/fire-spread_%s_season-%02d.tif", args.dout, args.bout, season);
    raster_write(fname, nx, ny, geotran, proj, &FIRE);


    free((void*)FIRE.OLD_BOOL);
    free((void*)FIRE.OLD_SEGM);
    free((void*)FIRE.FIRE_TIME);
    free((void*)FIRE.FIRE_SEED);
    free((void*)FIRE.FIRE_HIST);
    free((void*)FIRE.FIRE_DENSITY);
    free((void*)FIRE.VISITED);
    free((void*)FIRE.OBJ_ID);
    free((void*)FIRE.OBJ_LIFETIME);
    free((void*)FIRE.OBJ_STARTTIME);
    free_2D((void**)FIRE.OBJ_SEED, 2);
    free_3D((void***)FIRE.OBJ_GAIN, 366, 9);

  }

  }

  
  free_2D((void**)INP, bands.n);
  free((void*)bands.years);
  free((void*)bands.months);
  free((void*)bands.seasons);

  return 0;
}

