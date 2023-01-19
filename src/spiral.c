#include "spiral.h"

bool xyfocal_to_p(int nx, int ny, int x, int y, int xx, int yy, int *xnew, int *ynew, int *pnew){
  if (xx == 0 && yy == 0) return false;
  *xnew = x+xx; *ynew = y+yy;
  if (*xnew < 0 || *xnew >= nx || *ynew < 0 || *ynew >= ny) return false;
  *pnew = nx*(*ynew)+(*xnew);
  return true;
}


int xy_to_p(int nx, int x, int y){
  return nx*y+x;
}


bool spiral(int *rx, int *ry, int hnx, int hny, int i, int *x, int *y, int *dx, int *dy){
/** This utility function calculates kernel positions for loops that require spiral
    behaviour from the central pixel in anti-clockwise direction towards the edge of 
    the kernel.
    Anticpated from: http://stackoverflow.com/questions/398299/looping-in-a-spiral

    Example 1: given a 3x3 kernel the positions are:
    (0, 0) (1, 0) (1, 1) (0, 1) (-1, 1) (-1, 0) (-1, -1) (0, -1) (1, -1)
    Example 2: given a 5x3 kernel the positions are:
    (0, 0) (1, 0) (1, 1) (0, 1) (-1, 1) (-1, 0) (-1, -1) (0, -1) (1, -1) 
    (2, -1) (2, 0) (2, 1) (-2, 1) (-2, 0) (-2, -1)

    Kernels can be non-square. Missing rows/cols are "jumped".
**/

/** how to use this:
    // It is important that the iterators need to be initialized!

    bool in;  // is position in kernel?
    int x, y; // returned position

    int ix, iy;   // position  iterator
    int idx, idy; // direction iterator
    int i, maxi;  // index of position, maximum number of positions

    nx = 3, ny = 5; // size of kernel
    if (nx >= ny) maxi = nx*nx; else maxi = ny*ny; // set maxi to square of largest dimension

    // initiate iterators to these values!
    ix = iy = idx = 0;
    idy = -1;

    // loop over all indices
    for(i=0; i<maxi; i++){
        if ((in = spiral(&x, &y, nx/2, ny/2, i, &ix, &iy, &idx, &idy))){
            printf("(%d,%d)", x, y);
            // DO SOMETHING
        }
    }
    printf("\n");
**/

int tmp;
bool ok;

  if ((-hnx <= *x) && (*x <= hnx) && (-hny <= *y) && (*y <= hny)){
    *rx = *x;
    *ry = *y;
    ok = true;
  } else ok = false;
  if( (*x == *y) || ((*x < 0) && (*x == -*y)) || ((*x > 0) && (*x == 1-*y))){
    tmp = *dx;
    *dx = -*dy;
    *dy = tmp;
  }
  *x += *dx;
  *y += *dy;
  return ok;

}

int radial_search_1(int nx, int ny, int i, int j, int pold, int v_center, int dist, 
          double *sumi, double *sumj, double *sumk, fire_t *FIRE, phase1_t *phase1){
int ii, jj, inew, jnew, pnew;

//printf("v: %d, sumi: %.0f, sumj: %.0f, sumk: %.0f\n", v_center, *sumi, *sumj, *sumk);

  *sumi+=i;
  *sumj+=j;
  *sumk+=1;

  for (ii=-1*dist; ii<=dist; ii++){
  for (jj=-1*dist; jj<=dist; jj++){

    inew = i+ii; jnew = j+jj;
    if (inew < 0 || inew >= ny || jnew < 0 || jnew >= nx) continue;
    pnew = nx*inew+jnew;

    if (!FIRE->OLD_BOOL[pnew]) continue;
    if (FIRE->VISITED[pnew]) continue;
    if (phase1->FIRE_SEGM[pnew] != phase1->FIRE_SEGM[pold]) continue;
    if (sqrt(ii*ii+jj*jj) > dist) continue;

    FIRE->OLD_SEGM[pnew] = v_center;
    FIRE->VISITED[pnew] = true;

    // keep searching from here
    radial_search_1(nx, ny, inew, jnew, pnew, v_center, dist, sumi, sumj, sumk, FIRE, phase1);

  }
  }

  return 0;
}

int radial_search_2(int nx, int ny, int i, int j, int pold, int v_center, int dist,
          double *sumi, double *sumj, double *sumk, fire_t *FIRE, phase1_t *phase1){
int ii, jj, inew, jnew, pnew;

//printf("v: %d, sumi: %.0f, sumj: %.0f, sumk: %.0f\n", v_center, *sumi, *sumj, *sumk);

  *sumi+=i;
  *sumj+=j;
  *sumk+=1;

  for (ii=-1*dist; ii<=dist; ii++){
  for (jj=-1*dist; jj<=dist; jj++){

    inew = i+ii; jnew = j+jj;
    if (inew < 0 || inew >= ny || jnew < 0 || jnew >= nx) continue;
    pnew = nx*inew+jnew;

    if (!phase1->NOW_BOOL[pnew]) continue;
    if (FIRE->VISITED[pnew]) continue;
    if (phase1->FIRE_SEGM[pnew] != phase1->FIRE_SEGM[nx*(i)+j]) continue;
    if (sqrt(ii*ii+jj*jj) > dist) continue;

    phase1->NOW_SEGM[pnew] = v_center;
    FIRE->VISITED[pnew] = true;
    radial_search_2(nx, ny, inew, jnew, pnew, v_center, dist, sumi, sumj, sumk, FIRE, phase1);

  }
  }

  return 0;
}


int get_direction(int xseed, int yseed, int x, int y){
int d;
float bearing;

  bearing = 90-_R2D_CONV_*atan2(yseed-y, x-xseed);
  if (bearing < 0) bearing += 360;

  if (bearing > 360){
    printf("bearing > 360° !?!\n"); exit(1);
  } else if (bearing >= 337.5) { d = 1; // N
  } else if (bearing >= 292.5) { d = 8; // NW
  } else if (bearing >= 247.5) { d = 7; // W
  } else if (bearing >= 202.5) { d = 6; // SW
  } else if (bearing >= 157.5) { d = 5; // S
  } else if (bearing >= 112.5) { d = 4; // SE
  } else if (bearing >=  67.5) { d = 3; // E
  } else if (bearing >=  22.5) { d = 2; // NE
  } else if (bearing >=   0.0) { d = 1; // N
  } else if (bearing <    0.0) {
    printf("bearing < 0° !?!\n"); exit(1);}

  return d;
}
