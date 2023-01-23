#include "fire-phase2.h"


int firespread_phase2(int nx, int ny, int nc, args_t *args, fire_t *FIRE){
int nsub = 0, mintime = 1000;
phase2_subobjects_t phase2_subobj;


  alloc((void**)&phase2_subobj.SEGM, nc, sizeof(int));
  alloc((void**)&phase2_subobj.VALID, MAX_OBJECTS, sizeof(bool));
  alloc((void**)&phase2_subobj.MINTIME, MAX_OBJECTS, sizeof(int));

  phase2_subsegment(nx, ny, nc, &nsub, &mintime, args, FIRE, &phase2_subobj);

  alloc_2D((void***)&phase2_subobj.SEED, 2, nsub, sizeof(int));

  phase2_seeds_for_subsegemts(nx, ny, nsub, FIRE, &phase2_subobj);

  phase2_fix_dangling_subpatches(nx, ny, nc, nsub, mintime, args, FIRE, &phase2_subobj);

  free((void*)phase2_subobj.SEGM);
  free((void*)phase2_subobj.MINTIME);
  free((void*)phase2_subobj.VALID);
  free_2D((void**)phase2_subobj.SEED, 2);

  localize_fire_seeds(nx, ny, nc, FIRE);

  return 0;
}


int phase2_subsegment(int nx, int ny, int nc, int *nsub, int *mintime, args_t *args, fire_t *FIRE, phase2_subobjects_t *phase2_subobj){
int i, j, p, pnew, qi, qj, inew, jnew, ii, jj;
int id;
queue_t fifo;



  // initialize new queue
  create_queue(&fifo, args->queue_size);

  memset(FIRE->VISITED, 0, sizeof(bool)*nc);

  // sub-segment the segmentation
  for (i=0, p=0; i<ny; i++){
  for (j=0; j<nx; j++, p++){

    if (!FIRE->VISITED[p] && FIRE->OLD_BOOL[p]){

      (*nsub)++;
      id = FIRE->OLD_SEGM[p];

      // put the pixel in the queue
      if (enqueue(&fifo, i, j) != 0){
        printf("Failed to enqueue\n");
        exit(1);}

      // track the pixel
      while (dequeue(&fifo, &qi, &qj) == 0){

        pnew = xy_to_p(nx, qj, qi);
        if (FIRE->VISITED[pnew]) continue;

        // label this pixel
        phase2_subobj->SEGM[pnew] = *nsub;
        FIRE->VISITED[pnew]  = true;

        // if there is a seed in this patch, it is a valid patch
        if (FIRE->FIRE_SEED[pnew] > 0) phase2_subobj->VALID[*nsub-1] = true;

        // record the earliest pixel of this patch
        if (phase2_subobj->MINTIME[*nsub-1] == 0 || 
          FIRE->FIRE_TIME[pnew] < phase2_subobj->MINTIME[*nsub-1]){
          phase2_subobj->MINTIME[*nsub-1] = FIRE->FIRE_TIME[pnew];}

        // record the earliest pixel overall
        if (phase2_subobj->MINTIME[*nsub-1] < *mintime) *mintime = phase2_subobj->MINTIME[*nsub-1];

        // put the neighboring pixels in queue if they belong to the same patch
        for (ii=-1; ii<=1; ii++){
        for (jj=-1; jj<=1; jj++){

          if (!xyfocal_to_p(nx, ny, qj, qi, jj, ii, 
                    &jnew, &inew, &pnew)) continue;

          if (!FIRE->VISITED[pnew] && FIRE->OLD_BOOL[pnew] && FIRE->OLD_SEGM[pnew] == id){
            if (enqueue(&fifo, inew, jnew) != 0){
              printf("Failed to enqueue\n");
              exit(1);}
          }

        }
        }
      }

    } else FIRE->VISITED[p] = true;
  }
  }

  // free queue's memory
  destroy_queue(&fifo);


  return 0;
}


int phase2_seeds_for_subsegemts(int nx, int ny, int nsub, fire_t *FIRE, phase2_subobjects_t *phase2_subobj){
int i, j, p, o;
int subid;
double **SEEDCALC = NULL;


  alloc_2D((void***)&SEEDCALC, 3, nsub, sizeof(double));

  // calculate the potential seed points for every sub-patch
  for (i=0, p=0; i<ny; i++){
  for (j=0; j<nx; j++, p++){
    if (!FIRE->OLD_BOOL[p]) continue;
    subid = phase2_subobj->SEGM[p];
    if (phase2_subobj->MINTIME[subid-1] == FIRE->FIRE_TIME[p]){
      SEEDCALC[0][subid-1] += j;
      SEEDCALC[1][subid-1] += i;
      SEEDCALC[2][subid-1] += 1;
    }
  }
  }

  for (o=0; o<nsub; o++){
    phase2_subobj->SEED[0][o] = (int)round(SEEDCALC[0][o]/SEEDCALC[2][o]);
    phase2_subobj->SEED[1][o] = (int)round(SEEDCALC[1][o]/SEEDCALC[2][o]);
  }

  free_2D((void**)SEEDCALC, 3);


  return 0;
}

int phase2_reassign_invalid_patches(int nx, int ny, int nc, int *D, int *msub_invalid, args_t *args, fire_t *FIRE, phase2_subobjects_t *phase2_subobj, phase2_adjacent_t *phase2_adjobj){
int i, i0, i1, is, inew, ii, qi;
int j, j0, j1, js, jnew, jj, qj;
int p, pnew, t, d, adj, nadj;
int subid, bestid, oldid, nowid, nowsubid, neighborid;
double nowsum, nownum, nowmean, bestmean;
queue_t fifo;


  // initialize new queue
  create_queue(&fifo, args->queue_size);

  memset(FIRE->VISITED, 0, sizeof(bool)*nc);


  if (*D == 0){
    i0 = 0; i1 = ny-1; is = 1;
    j0 = 0; j1 = nx-1; js = 1;
    (*D)++;
  } else if (*D == 1){
    i0 = ny-1; i1 = 0; is = -1;
    j0 = nx-1; j1 = 0; js = -1;
    (*D)++;
  } else if (*D == 2){
    i0 = 0; i1 = ny-1; is = 1;
    j0 = nx-1; j1 = 0; js = -1;
    (*D)++;
  } else if (*D == 3){
    i0 = ny-1; i1 = 0; is = -1;
    j0 = 0; j1 = nx-1; js = 1;
    *D = 0;
  }

  for (i=i0; i!=i1; i+=is){
  for (j=j0; j!=j1; j+=js){

    p = nx*i+j; 

    if (!FIRE->VISITED[p] && FIRE->OLD_BOOL[p]){


      subid = phase2_subobj->SEGM[p];

      // if patch is valid, i.e. if it has a seed: do nothing for now
      if (phase2_subobj->VALID[subid-1]){ FIRE->VISITED[p] = true; continue;}

      // put the pixel in the queue
      if (enqueue(&fifo, i, j) != 0){
        printf("Failed to enqueue\n");
        exit(1);}

      nadj = 0;
      // track the pixel
      while (dequeue(&fifo, &qi, &qj) == 0){

        pnew = xy_to_p(nx, qj, qi);
        if (FIRE->VISITED[pnew]) continue;
        FIRE->VISITED[pnew] = true;

        for (ii=-1; ii<=1; ii++){
        for (jj=-1; jj<=1; jj++){

          if (!xyfocal_to_p(nx, ny, qj, qi, jj, ii, 
                  &jnew, &inew, &pnew)) continue;

          // if the pixel belongs to a neighboring patch
          if (FIRE->OLD_BOOL[pnew] && (neighborid = phase2_subobj->SEGM[pnew]) != subid){
            // if adjacent patch is valid: log it
            if (phase2_subobj->VALID[neighborid-1]){
              phase2_adjobj->SUBID[nadj] = neighborid;
              phase2_adjobj->ID[nadj]    = FIRE->OLD_SEGM[pnew];
              phase2_adjobj->TIME[nadj]  = abs(FIRE->FIRE_TIME[pnew]-FIRE->FIRE_TIME[nx*qi+qj]);
              nadj++;
            }
          } else if (!FIRE->VISITED[pnew] && FIRE->OLD_BOOL[pnew] && phase2_subobj->SEGM[pnew] == subid){
            // put the pixel in the queue
            if (enqueue(&fifo, inew, jnew) != 0){
              printf("Failed to enqueue\n"); exit(1);}
          }

        }
        }
      }

      // if no adjacent valid patch was found: continue
      if (nadj == 0) continue;

      for (adj=0; adj<nadj; adj++) phase2_adjobj->TODO[adj] = true;

      // find the patch with the lowest time difference
      bestmean = 10000000;
      while (bsum(phase2_adjobj->TODO, nadj) > 0){
        for (adj=0, nowid=0, nowsubid=0; adj<nadj; adj++){
          if (!phase2_adjobj->TODO[adj]) continue;
          if (nowsubid == 0){
            nowsubid = phase2_adjobj->SUBID[adj];
            nowid    = phase2_adjobj->ID[adj];
            nowsum   = phase2_adjobj->TIME[adj];
            nownum   = 1;
            phase2_adjobj->TODO[adj] = false;
          } else if (phase2_adjobj->SUBID[adj] == nowsubid){
            nowsum += phase2_adjobj->TIME[adj];
            nownum++;
            phase2_adjobj->TODO[adj] = false;
          }
        }
        if ((nowmean = nowsum/nownum) < bestmean){
          bestmean = nowmean;
          bestid = nowid;
        }

      }

      // if temporally closest patch is not close enough: continue
      if (bestmean > args->temp__dist*2) continue;

      // put the pixel in the queue
      if (enqueue(&fifo, i, j) != 0){
        printf("Failed to enqueue\n"); exit(1);}

      // track the patch again and re-assign
      while (dequeue(&fifo, &qi, &qj) == 0){

        pnew = xy_to_p(nx, qj, qi);

        // if we already re-assigned this pixel: continue
        if ((oldid = FIRE->OLD_SEGM[pnew]) == bestid) continue;

        // re-assign
        FIRE->OLD_SEGM[pnew] = bestid;

        // fire-time of this pixel
        t = FIRE->FIRE_TIME[pnew];

        // adjust the spread rates #1: remove from old patch
        d = get_direction(FIRE->OBJ_SEED[0][oldid-1], FIRE->OBJ_SEED[1][oldid-1], qj, qi);
        FIRE->OBJ_GAIN[t-1][0][oldid-1]--; // total spread rate
        FIRE->OBJ_GAIN[t-1][d][oldid-1]--; // directional spread rate

        // adjust the spread rates #2: add to new patch
        d = get_direction(FIRE->OBJ_SEED[0][bestid-1], FIRE->OBJ_SEED[1][bestid-1], qj, qi);
        FIRE->OBJ_GAIN[t-1][0][bestid-1]++; // total spread rate
        FIRE->OBJ_GAIN[t-1][d][bestid-1]++; // directional spread rate


        for (ii=-1; ii<=1; ii++){
        for (jj=-1; jj<=1; jj++){

          if (!xyfocal_to_p(nx, ny, qj, qi, jj, ii, 
                  &jnew, &inew, &pnew)) continue;

          if (FIRE->OLD_BOOL[pnew] && phase2_subobj->SEGM[pnew] == subid){
            // put the pixel in the queue
            if (enqueue(&fifo, inew, jnew) != 0){
              printf("Failed to enqueue\n"); exit(1);}
          }

        }
        }
      }

      // this patch is now valid
      phase2_subobj->VALID[subid-1] = true;
      (*msub_invalid)--;

    } else FIRE->VISITED[p] = true;

  }
  }

  // free queue's memory
  destroy_queue(&fifo);

  return 0;
}


int phase2_add_new_patches(int nx, int ny, int nc, int *mintime, args_t *args, fire_t *FIRE, phase2_subobjects_t *phase2_subobj){
int i, j, p, qi, qj, inew, jnew, pnew, ii, jj, d, t;
int subid, oldid, n_add = 0;
queue_t fifo;


  // initialize new queue
  create_queue(&fifo, args->queue_size);

  (*mintime)++; // increment the "oldest" pixels

  memset(FIRE->VISITED, 0, sizeof(bool)*nc);

  for (i=0, p=0; i<ny; i++){
  for (j=0; j<nx; j++, p++){

    if (FIRE->OLD_BOOL[p]){

      subid = phase2_subobj->SEGM[p];

      // if this patch is already valid: continue
      if (phase2_subobj->VALID[subid-1]) continue;

      // if old: add new seed
      if (phase2_subobj->MINTIME[subid-1] < *mintime){

        if (enqueue(&fifo, i, j) != 0){
          printf("Failed to enqueue\n"); exit(1);}

        FIRE->nfire++;  // increment total number of fire patches
        n_add++; // increment patches that were added in this iteration
        phase2_subobj->VALID[subid-1] = true; // this patch is now valid
        FIRE->OBJ_SEED[0][FIRE->nfire-1] = phase2_subobj->SEED[0][subid-1]; // create new seed
        FIRE->OBJ_SEED[1][FIRE->nfire-1] = phase2_subobj->SEED[1][subid-1]; // create new seed
        if (FIRE->nfire >= MAX_OBJECTS){
          printf("Critical failure: MAX_OBJECTS is set too low...\n"); exit(1);}
        FIRE->OBJ_ID[FIRE->nfire-1] = FIRE->nfire; // add newID
        FIRE->FIRE_SEED[nx*FIRE->OBJ_SEED[1][FIRE->nfire-1]+FIRE->OBJ_SEED[0][FIRE->nfire-1]] = FIRE->nfire; // draw new seed

        // track this patch
        while (dequeue(&fifo, &qi, &qj) == 0){

          pnew = xy_to_p(nx, qj, qi);
          if (FIRE->VISITED[pnew]) continue;
          FIRE->VISITED[pnew] = true;

          t = FIRE->FIRE_TIME[pnew];
          oldid = FIRE->OLD_SEGM[pnew];
          FIRE->OLD_SEGM[pnew] = FIRE->nfire; // re-assign

          // adjust the spread rates #1: remove from old patch
          d = get_direction(FIRE->OBJ_SEED[0][oldid-1], FIRE->OBJ_SEED[1][oldid-1], qj, qi);
          FIRE->OBJ_GAIN[t-1][0][oldid-1]--; // total spread rate
          FIRE->OBJ_GAIN[t-1][d][oldid-1]--; // directional spread rate

          // if newer than oldest fire in subpatch: add to directional spread
          if (t >= *mintime){
            d = get_direction(phase2_subobj->SEED[0][subid-1], phase2_subobj->SEED[1][subid-1], qj, qi);
            FIRE->OBJ_GAIN[t-1][d][FIRE->nfire-1]++; // directional spread rate
          }
          FIRE->OBJ_GAIN[t-1][0][FIRE->nfire-1]++; // total spread rate


          for (ii=-1; ii<=1; ii++){
          for (jj=-1; jj<=1; jj++){

            if (!xyfocal_to_p(nx, ny, qj, qi, jj, ii, 
                    &jnew, &inew, &pnew)) continue;

            if (!FIRE->VISITED[pnew] && FIRE->OLD_BOOL[pnew] && phase2_subobj->SEGM[pnew] == subid){
              // put the pixel in the queue
              if (enqueue(&fifo, inew, jnew) != 0){
                printf("Failed to enqueue\n"); exit(1);}
            }

          }
          }
        }
      
      }
    }
  }
  }

  if (args->v) printf("added %d new patches that were burnt before DOY %d\n", n_add, *mintime);
  destroy_queue(&fifo);

  return 0;
}


int phase2_fix_dangling_subpatches(int nx, int ny, int nc, int nsub, int mintime, args_t *args, fire_t *FIRE, phase2_subobjects_t *phase2_subobj){
phase2_adjacent_t phase2_adjobj;
int nsub_invalid = 0, msub_invalid = 0;
int iter = 0, D = 0;


  alloc((void**)&phase2_adjobj.ID, MAX_OBJECTS, sizeof(int));
  alloc((void**)&phase2_adjobj.SUBID, MAX_OBJECTS, sizeof(int));
  alloc((void**)&phase2_adjobj.TIME, MAX_OBJECTS, sizeof(int));
  alloc((void**)&phase2_adjobj.TODO, MAX_OBJECTS, sizeof(bool));

  // proceed until all sub-patches are valid!
  while ((nsub_invalid = nsub-bsum(phase2_subobj->VALID, nsub)) > 0 && mintime <= 366){
    iter++;

    // if both are still equal after one iteration: add new seeds
    msub_invalid = nsub_invalid;

    if (args->v) printf("invalid patches: %d; total patches: %d; iteration %d\n", nsub_invalid, nsub, iter);

    phase2_reassign_invalid_patches(nx, ny, nc, &D, &msub_invalid, args, FIRE, phase2_subobj, &phase2_adjobj);

    // if nothing could be re-assigned, add new seeds; start with oldest
    if (nsub_invalid == msub_invalid){
      if (phase2_add_new_patches(nx, ny, nc, &mintime, args, FIRE, phase2_subobj) != 0){
        printf("error in adding new patches\n");
        return 1;
      }
    }
    

  }

  free((void*)phase2_adjobj.ID);
  free((void*)phase2_adjobj.SUBID);
  free((void*)phase2_adjobj.TIME);
  free((void*)phase2_adjobj.TODO);


  return 0;
}

