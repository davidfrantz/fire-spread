#include "fire-phase1.h"


int firespread_phase1(int2u **INP, bands_t *bands, int nx, int ny, int nc, int season, args_t *args, fire_t *FIRE){
int t;
queue_t fifo;
phase1_t phase1;


    alloc((void**)&phase1.NOW_BOOL,  nc, sizeof(bool));
    alloc((void**)&phase1.NOW_SEGM,  nc, sizeof(int));
    alloc((void**)&phase1.FIRE_SEGM, nc, sizeof(int));


    if (phase1_prepare(INP, bands, nx, ny, nc, season, args, FIRE, &phase1) != 0){
      printf("error in preparing input\n");
      return 1;
    }

    if (phase1_find_initial_seeds(nx, ny, nc, args, FIRE, &phase1) != 0){
      printf("error in finding seeds\n");
      return 1;
    }


    // cycle through every remaining day
    for (t=2; t<=366; t++){

      // step one: put old and active fires in queue

      // initialize new queue
      create_queue(&fifo, args->queue_size);

      if (phase1_enqueue_active_fires(t, nx, ny, nc, &fifo, args, FIRE, &phase1) != 0){
        printf("error in enqueueing active fires\n");
        return 1;
      }

      // step two: trace the fire fronts
      if (phase1_trace_active_fires(t, nx, ny, nc, &fifo, args, FIRE, &phase1) != 0){
        printf("error in tracing fires\n");
        return 1;
      }

      // free queue's memory
      destroy_queue(&fifo);


      // step three: prepare to put new fires in queue
      if (phase1_find_next_seeds(t, nx, ny, nc, args, FIRE, &phase1) != 0){
        printf("error finding next seeds\n");
        return 1;
      }

    }

    free((void*)phase1.NOW_BOOL);
    free((void*)phase1.NOW_SEGM);
    free((void*)phase1.FIRE_SEGM);


    // move fire seed centroids to actual burnt pixels
    if (localize_fire_seeds(nx, ny, nc, FIRE) != 0){
      printf("error in localizing seeds\n");
      return 1;
    }


  return 0;
}


int phase1_directional_mask(int **dirmask, int distance){
int ii, jj, k;

  for (ii=-1*distance, k=0; ii<=distance; ii++){
  for (jj=-1*distance; jj<=distance; jj++, k++){

    if (ii == 0 && jj == 0) continue;

    if (ii <= 0) dirmask[0][k] = 1;
    if (ii >= 0) dirmask[1][k] = 1;
    if (jj <= 0) dirmask[2][k] = 1;
    if (jj >= 0) dirmask[3][k] = 1;
    if (ii <= jj) dirmask[4][k] = 1;
    if (ii >= jj) dirmask[5][k] = 1;
    if (ii <= -1*jj) dirmask[6][k] = 1;
    if (ii >= -1*jj) dirmask[7][k] = 1;
    if (ii <= 0 && abs(ii) >= abs(jj)) dirmask[8][k] = 1;
    if (ii >= 0 && abs(ii) >= abs(jj)) dirmask[9][k] = 1;
    if (jj <= 0 && abs(jj) >= abs(ii)) dirmask[10][k] = 1;
    if (jj >= 0 && abs(jj) >= abs(ii)) dirmask[11][k] = 1;
    if (ii <= 0 && jj <= 0) dirmask[12][k] = 1;
    if (ii <= 0 && jj >= 0) dirmask[13][k] = 1;
    if (ii >= 0 && jj <= 0) dirmask[14][k] = 1;
    if (ii >= 0 && jj >= 0) dirmask[15][k] = 1;

  }
  }

  return 0;
}


int phase1_prepare(int2u **INP, bands_t *bands, int nx, int ny, int nc, int season, args_t *args, fire_t *FIRE, phase1_t *phase1){
bool *MASK = NULL;
int  *COPY = NULL;
int  *HIST    = NULL;
int **dirmask;
int b, p, i, j, ii, jj, k;
int s[16], w, minw;
double u[16], minu, bstu;
int nseg=0;
int smoothsize;
  
  
  smoothsize = (args->smoothdist*2+1)*(args->smoothdist*2+1);
  alloc_2D((void***)&dirmask, 16, smoothsize, sizeof(int));
  phase1_directional_mask(dirmask, smoothsize);


  alloc((void**)&MASK, nc, sizeof(bool));
  alloc((void**)&COPY, nc, sizeof(int));

  for (b=0; b<bands->n; b++){
    if (bands->seasons[b] != season) continue;
    for (p=0; p<nc; p++){
      if (INP[b][p] > 0 && INP[b][p] < 367){
        if (FIRE->FIRE_TIME[p] == 0){
          FIRE->FIRE_TIME[p] = (int)INP[b][p] - bands->min_doy + 1;
          if (FIRE->FIRE_TIME[p] < 1) FIRE->FIRE_TIME[p]+=366;
        }
        MASK[p] = true;
      }
    }
  }


  /** delete all patches < maxsize pixels
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
  connectedcomponents(MASK, phase1->FIRE_SEGM, ny, nx);
  nseg = imax(phase1->FIRE_SEGM, nc);

  if (args->min___size > 0){
    alloc((void**)&HIST, nseg, sizeof(int));
    for (p=0; p<nc; p++){ // compute histogram
      if (MASK[p]) HIST[phase1->FIRE_SEGM[p]-1]++;
    }
    for (p=0; p<nc; p++){ // remove small patches
      if (MASK[p] && HIST[phase1->FIRE_SEGM[p]-1] < args->min___size){
        phase1->FIRE_SEGM[p] = 0;
        FIRE->FIRE_TIME[p] = 0;
        MASK[p] = false;
      }
    }
    free((void*)HIST);
  }


  for (i=0, p=0; i<ny; i++){
  for (j=0; j<nx; j++, p++){
    COPY[p] = FIRE->FIRE_TIME[p];
  }
  }


  for (i=0, p=0; i<ny; i++){
  for (j=0; j<nx; j++, p++){
    if (MASK[p]){
      for (w=0; w<16; w++) u[w] = s[w] = 0;
      for (ii=-1*args->smoothdist, k=0; ii<=args->smoothdist; ii++){
      for (jj=-1*args->smoothdist; jj<=args->smoothdist; jj++, k++){
        if (i+ii < 0 || i+ii >= ny || 
          j+jj < 0 || j+jj >= nx) continue;
        if (MASK[nx*(i+ii)+j+jj]){
          for (w=0; w<16; w++){
            if (dirmask[w][k] == 1){
              u[w] += COPY[nx*(i+ii)+j+jj];
              s[w]++;
            }
          }
        }
      }
      }
      for (w=0; w<16; w++){
        if (s[w] > 0) u[w]/=s[w];
      }


      minw = -1;
      minu = bstu = 10000;
      for (w=0; w<16; w++){
        if ((u[w]-COPY[p])*(u[w]-COPY[p]) < minu &&
          s[w] > 0){
          minw = w;
          minu = (u[w]-COPY[p])*(u[w]-COPY[p]);
          bstu = u[w];
        }
      }
      if (minw >= 0) FIRE->FIRE_TIME[p] = (int)round(bstu);

    }
  }
  }

  free((void*)COPY);
  free((void*)MASK);
  free_2D((void**)dirmask, 16);


  return 0;
}


int phase1_find_initial_seeds(int nx, int ny, int nc, args_t *args, fire_t *FIRE, phase1_t *phase1){
int p, i, j, t;
int id;
double sumi, sumj, sumk;


  // get the pixels that were burnt on the 1st date
  for (p=0, t=1; p<nc; p++){
    if (FIRE->FIRE_TIME[p] == t) FIRE->OLD_BOOL[p] = true; else FIRE->OLD_BOOL[p] = false;
    FIRE->OLD_SEGM[p] = 0;
  }

  connectedcomponents(FIRE->OLD_BOOL, FIRE->OLD_SEGM, ny, nx);
  FIRE->nfire = imax(FIRE->OLD_SEGM, nc);
  if (args->v) printf("number of patches in first date: %d\n", FIRE->nfire);
  if (args->v) printf("number of pixels  in first date: %d\n", bsum(FIRE->OLD_BOOL, nc));
  if (FIRE->nfire > 0){
    if (FIRE->nfire >= MAX_OBJECTS){
      printf("Critical failure: MAX_OBJECTS is set too low...\n"); exit(1);}
    for (id=0; id<FIRE->nfire; id++) FIRE->OBJ_ID[id] = id+1;
  }

  memset(&FIRE->VISITED, 0, sizeof(bool)*nc);


  // try to improve seeds
  for (i=0, p=0; i<ny; i++){
  for (j=0; j<nx; j++, p++){
    if (FIRE->OLD_BOOL[p] && !FIRE->VISITED[p]){
      FIRE->VISITED[p] = true;
      sumi = sumj = sumk = 0;
      radial_search_1(nx, ny, i, j, p, FIRE->OLD_SEGM[p], args->init__dist, &sumi, &sumj, &sumk, FIRE, phase1);
      if (FIRE->OBJ_SEED[0][FIRE->OLD_SEGM[p]-1] > 0){
        printf("failure\n"); } else FIRE->OBJ_SEED[0][FIRE->OLD_SEGM[p]-1] = (int) round(sumj/sumk);
      if (FIRE->OBJ_SEED[1][FIRE->OLD_SEGM[p]-1] > 0){ 
        printf("failure\n"); } else FIRE->OBJ_SEED[1][FIRE->OLD_SEGM[p]-1] = (int) round(sumi/sumk);
      FIRE->OBJ_GAIN[t-1][0][FIRE->OLD_SEGM[p]-1] = (int) sumk;
      FIRE->FIRE_SEED[nx*((int)round(sumi/sumk))+((int)round(sumj/sumk))] = FIRE->OLD_SEGM[p];
    }
  }
  }


  return 0;
}


int phase1_enqueue_active_fires(int t, int nx, int ny, int nc, queue_t *fifo, args_t *args, fire_t *FIRE, phase1_t *phase1){
int p;
int D = 0;
int i0, i1, is;
int j0, j1, js;
int i, ii, j, jj, inew, jnew;


  // get new fire pixels
  for (p=0; p<nc; p++){
    if (FIRE->FIRE_TIME[p] == t) phase1->NOW_BOOL[p] = true; else phase1->NOW_BOOL[p] = false;
    phase1->NOW_SEGM[p] = 0;
  }


  if (D==0){
    i0 = 0; i1 = ny-1; is = 1;
    j0 = 0; j1 = nx-1; js = 1;
    D++;
  } else if (D==1){
    i0 = ny-1; i1 = 0; is = -1;
    j0 = nx-1; j1 = 0; js = -1;
    D++;
  } else if (D==2){
    i0 = 0; i1 = ny-1; is = 1;
    j0 = nx-1; j1 = 0; js = -1;
    D++;
  } else if (D==3){
    i0 = ny-1; i1 = 0; is = -1;
    j0 = 0; j1 = nx-1; js = 1;
    D=0;
  }


  for (i=i0; i!=i1; i+=is){
  for (j=j0; j!=j1; j+=js){

    p = nx*i+j; 

    // if old and active
    if (FIRE->OLD_BOOL[p] && abs((t-FIRE->FIRE_TIME[p])) <= args->temp__dist){

      // if at edge of patch, put in queue
      for (ii=-1; ii<=1; ii++){
      for (jj=-1; jj<=1; jj++){
        
        if (ii==0 && jj==0) continue;
        inew = i+ii; jnew = j+jj;
        if (inew < 0 || inew >= ny || jnew < 0 || jnew >= nx) continue;
        
        if (!FIRE->OLD_BOOL[nx*inew+jnew]){
          if (enqueue(fifo, i, j) != 0){ // put in queue
            printf("Failed to enqueue\n");
            exit(1);
          }
          ii = jj = 10; // pixel wa already put in queue, proceed with next one
        }
      }
      }

    }

  }
  }


  return 0;
}


int phase1_trace_active_fires(int t, int nx, int ny, int nc, queue_t *fifo, args_t *args, fire_t *FIRE, phase1_t *phase1){
int id, pold, pnew;
int d, DD = 0;
int jj0, jj1, jjs;
int ii0, ii1, iis;
int ii, jj, inew, jnew, qi, qj;


  while (dequeue(fifo, &qi, &qj) == 0){

    pold = nx*qi+qj;

    if (DD==0){
      ii0 = -1*args->track_dist; ii1 = args->track_dist; iis = 1;
      jj0 = -1*args->track_dist; jj1 = args->track_dist; jjs = 1;
      DD++;
    } else if (DD==1){
      ii0 = args->track_dist; ii1 = -1*args->track_dist; iis = -1;
      jj0 = args->track_dist; jj1 = -1*args->track_dist; jjs = -1;
      DD++;
    } else if (DD==2){
      ii0 = -1*args->track_dist; ii1 = args->track_dist; iis = 1;
      jj0 = args->track_dist; jj1 = -1*args->track_dist; jjs = -1;
      DD++;
    } else if (DD==3){
      ii0 = args->track_dist; ii1 = -1*args->track_dist; iis = -1;
      jj0 = -1*args->track_dist; jj1 = args->track_dist; jjs = 1;
      DD=0;
    }

    for (ii=ii0; ii!=ii1; ii+=iis){
    for (jj=jj0; jj!=jj1; jj+=jjs){

      if (ii==0 && jj==0) continue;
      inew = qi+ii; jnew = qj+jj;
      if (inew < 0 || inew >= ny || jnew < 0 || jnew >= nx) continue;
      pnew = nx*inew+jnew;

      if (phase1->FIRE_SEGM[pnew] != phase1->FIRE_SEGM[pold]) continue;
      if (sqrt(ii*ii+jj*jj) > args->track_dist) continue;

      // if new fire around dequeued pixel:
      if (phase1->NOW_BOOL[pnew]){
        // add pixel to segmentation
        FIRE->OLD_SEGM[pnew] = FIRE->OLD_SEGM[pold];
        FIRE->OLD_BOOL[pnew] = true;
        // clear pixel from NOW
        phase1->NOW_BOOL[pnew] = false;

        id = FIRE->OLD_SEGM[pnew]-1;

        d = get_direction(FIRE->OBJ_SEED[0][id], FIRE->OBJ_SEED[1][id], jnew, inew);

        FIRE->OBJ_GAIN[t-1][0][id]++;
        FIRE->OBJ_GAIN[t-1][d][id]++;

        // put the pixel in the queue
        if (enqueue(fifo, inew, jnew) != 0){
          printf("Failed to enqueue\n");
          exit(1);
        }

      }

    }
    }


  }


  return 0;
}


int phase1_find_next_seeds(int t, int nx, int ny, int nc, args_t *args, fire_t *FIRE, phase1_t *phase1){
int p, i, j;
int id;
int n_add;
double sumi, sumj, sumk;


  connectedcomponents(phase1->NOW_BOOL, phase1->NOW_SEGM, ny, nx);
  n_add = imax(phase1->NOW_SEGM, nc);

  memset(&FIRE->VISITED, 0, sizeof(bool)*nc);

  // try to improve seeds
  for (i=0, p=0; i<ny; i++){
  for (j=0; j<nx; j++, p++){
    if (phase1->NOW_BOOL[p] && !FIRE->VISITED[p]){
      sumi = sumj = sumk = 0;
      FIRE->VISITED[p] = true;
      radial_search_2(nx, ny, i, j, p, phase1->NOW_SEGM[p], args->init__dist, &sumi, &sumj, &sumk, FIRE, phase1);
      if (FIRE->OBJ_SEED[0][phase1->NOW_SEGM[p]+FIRE->nfire-1] > 0){
        printf("hmm\n"); } else FIRE->OBJ_SEED[0][phase1->NOW_SEGM[p]+FIRE->nfire-1] = (int) round(sumj/sumk);
      if (FIRE->OBJ_SEED[1][phase1->NOW_SEGM[p]+FIRE->nfire-1] > 0){ 
        printf("hii\n"); } else FIRE->OBJ_SEED[1][phase1->NOW_SEGM[p]+FIRE->nfire-1] = (int) round(sumi/sumk);
      FIRE->OBJ_GAIN[t-1][0][phase1->NOW_SEGM[p]+FIRE->nfire-1] = (int) sumk;
      FIRE->FIRE_SEED[nx*((int)round(sumi/sumk))+((int)round(sumj/sumk))] = phase1->NOW_SEGM[p]+FIRE->nfire;
    }
  }
  }

  if (n_add>0){
    // add nfire to Object IDs and add to OLD_SEGM
    for (p=0; p<nc; p++){
      if (phase1->NOW_BOOL[p]){
        FIRE->OLD_SEGM[p] = phase1->NOW_SEGM[p]+FIRE->nfire;
        FIRE->OLD_BOOL[p] = true;
      }
    }
    if (FIRE->nfire+n_add >= MAX_OBJECTS){
      printf("Critical failure: MAX_OBJECTS is set too low...\n"); exit(1);}
    for (id=FIRE->nfire; id<(FIRE->nfire+n_add); id++) FIRE->OBJ_ID[id] = id+1;

    FIRE->nfire+=n_add;
  }
  if (args->v) printf("number of new patches at DOY %03d: %d, number of patches: %d\n", t, n_add, FIRE->nfire);


  return 0;
}

