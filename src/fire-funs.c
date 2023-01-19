
#include "fire-funs.h"

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

