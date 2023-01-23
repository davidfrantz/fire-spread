
#include "fire-funs.h"



int month_minimum(int2u **INP, int nc, bands_t *bands){
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

  return 0;
}


int sort_into_seasons(bands_t *bands, args_t *args){
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
    return 1;
  }


  // label seasons
  for (b=0; b<bands->n; b++){

    if (bands->months[b] == bands->min_month) season++;

    bands->seasons[b] = season;

  }
  bands->nseasons = season;


  if (args->v){
    printf("%04d %02d %02d\n", bands->years[0], bands->months[0], bands->seasons[0]);
    for (b=1; b<bands->n; b++){
      if (bands->seasons[b] > bands->seasons[b-1]){
        printf("%04d %02d %02d\n", bands->years[b], bands->months[b], bands->seasons[b]);
      }
    } 
  }
  printf("processing of %02d seasons.\n", bands->nseasons);


  return 0;
}


int localize_fire_seeds(int nx, int ny, int nc, fire_t *FIRE){
int p, i, j, ii, jj, inew, jnew, pnew; 
int seed, pspiral, maxp, px, py, pdx, pdy;
bool inkernel;


  for (i=0, p=0; i<ny; i++){
  for (j=0; j<nx; j++, p++){

    seed = FIRE->FIRE_SEED[p];
    if (seed > 0 && FIRE->OLD_SEGM[p] != seed){

      maxp = nx*nx;
      px = py = pdx = 0;
      pdy = -1;

      for (pspiral=0; pspiral<maxp; pspiral++){

        if ((inkernel = spiral(&jj, &ii, nx/2, nx/2, pspiral, &px, &py, &pdx, &pdy))){

          if (!xyfocal_to_p(nx, ny, j, i, jj, ii, 
                &jnew, &inew, &pnew)) continue;

          if (FIRE->OLD_BOOL[pnew] && FIRE->OLD_SEGM[pnew] == seed){
            FIRE->FIRE_SEED[p] = 0;
            FIRE->FIRE_SEED[pnew] = seed;
            FIRE->OBJ_SEED[0][seed-1] = jnew;
            FIRE->OBJ_SEED[1][seed-1] = inew;
            break;
          }

        }
      }

    }

  }
  }


  return 0;
}


int fire_density(int nx, int ny, args_t *args, fire_t *FIRE){
int i, j, p, ii, jj, inew, jnew;
int k;

  for (i=0, p=0; i<ny; i++){
  for (j=0; j<nx; j++, p++){
    k = 0;
    for (ii=-1*args->densi_dist; ii<=args->densi_dist; ii++){
    for (jj=-1*args->densi_dist; jj<=args->densi_dist; jj++){
      inew = i+ii; jnew = j+jj;
      if (inew < 0 || inew >= ny || jnew < 0 || jnew >= nx) continue;
      if (FIRE->FIRE_SEED[nx*inew+jnew] < 1 || sqrt(ii*ii+jj*jj) > args->densi_dist) continue;
      k++;
    }
    }
    FIRE->FIRE_DENSITY[p] = k;
  }
  }

  return 0;
}


int fire_lifetime(fire_t *FIRE){
int id, t, t0, t1;

  for (id=0; id<FIRE->nfire; id++){
    for (t=0, t0=0, t1=0; t<366; t++){

      if (FIRE->OBJ_GAIN[t][0][id] == 0){
        continue;
      } else if (FIRE->OBJ_GAIN[t][0][id] > 0){
        if (t0 == 0){
          t0 = t;
        } else {
          t1 = t;
        }
      }

    }
    if (t1 == 0) t1 = t0;
    FIRE->OBJ_LIFETIME[id]  = t1-t0+1;
    FIRE->OBJ_STARTTIME[id] = t0+1;
  }

  return 0;
}


int fire_size(int nc, args_t *args, fire_t *FIRE){
int p, id;
int n = 0;
    
  for (p=0; p<nc; p++){
    if (FIRE->OLD_BOOL[p]) FIRE->FIRE_HIST[FIRE->OLD_SEGM[p]-1]++;
  }
  for (id=0; id<FIRE->nfire; id++) if (FIRE->FIRE_HIST[id] > 0) n++;
  if (args->v) printf("total patches new: %d\n", n);

  return 0;
}
