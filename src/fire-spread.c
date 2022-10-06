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
#include "alloc.h"
#include "angle.h"
#include "date.h"
#include "queue.h"
#include "focalfuns.h"
#include "string.h"
#include "vutils.h"

/**

#include "io.h"

#include "quantile.h"

**/

#define SUCCESS  0
#define FAILURE -1
#define MAX_OBJECTS 1000000


bool *OLD_BOOL = NULL; 
int  *OLD_SEGM = NULL;

bool *NOW_BOOL = NULL;
int  *NOW_SEGM = NULL;

int  *FIRE_SEGM = NULL;

bool *VISITED = NULL;

int nx, ny, nc, nb;

#pragma omp threadprivate(OLD_BOOL,OLD_SEGM,NOW_BOOL,NOW_SEGM,FIRE_SEGM,VISITED)

int xy_to_p(int x, int y){
	return nx*y+x;
}

bool xyfocal_to_p(int x, int y, int xx, int yy, int *xnew, int *ynew, int *pnew){
	if (xx == 0 && yy == 0) return false;
	*xnew = x+xx; *ynew = y+yy;
	if (*xnew < 0 || *xnew >= nx || *ynew < 0 || *ynew >= ny) return false;
	*pnew = nx*(*ynew)+(*xnew);
	return true;
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

int radial_search_1(int i, int j, int pold, int v_center, int dist, 
					double *sumi, double *sumj, double *sumk){
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

		if (!OLD_BOOL[pnew]) continue;
		if (VISITED[pnew]) continue;
		if (FIRE_SEGM[pnew] != FIRE_SEGM[pold]) continue;
		if (sqrt(ii*ii+jj*jj) > dist) continue;

		OLD_SEGM[pnew] = v_center;
		VISITED[pnew] = true;

		// keep searching from here
		radial_search_1(inew, jnew, pnew, v_center, dist, sumi, sumj, sumk);

	}
	}

	return SUCCESS;

}

int radial_search_2(int i, int j, int pold, int v_center, int dist,
					double *sumi, double *sumj, double *sumk){
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

		if (!NOW_BOOL[pnew]) continue;
		if (VISITED[pnew]) continue;
		if (FIRE_SEGM[pnew] != FIRE_SEGM[nx*(i)+j]) continue;
		if (sqrt(ii*ii+jj*jj) > dist) continue;

		NOW_SEGM[pnew] = v_center;
		VISITED[pnew] = true;
		radial_search_2(inew, jnew, pnew, v_center, dist, sumi, sumj, sumk);

	}
	}

	return SUCCESS;

}

int modal(int *arr, int nfire){
int count = 0;
int mod = arr[4];
int modCount = 0;
int i, j;
bool *valid;

	alloc((void**)&valid, nfire, sizeof(bool));

	for (j=0; j<nfire; j++){
		if (arr[j] == 0) valid[j] = false; else valid[j] = true;
	}

	for (j=0; j<nfire; j++){
		if (!valid[j]) continue;
		count = 0;
		for (i=0; i<nfire; i++){
			if (!valid[i]) continue;
			if (arr[j] == arr[i]){
				count += 1;
			}
		}
		if (count > modCount){
			mod = arr[j];
			modCount = count;
		}
	}

	free((void*)valid);
	return mod;

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

void reset_visited(){
int p;
	for (p=0; p<nc; p++) VISITED[p] = false;
}

int main( int argc, char *argv[] ){

	if (argc != 12){ //argv[0] is program name
		printf( "usage: %s input-stack input-dates output-dir basename\n", argv[0]);
		printf("  init-searchdist track-searchdist temp-dist density-dist max-size smooth-dist verbose\n");
		exit(0);
	}

int D = 0, DD = 0;
int p0, ps;
int i0, i1, is;
int j0, j1, js;
int jj0, jj1, jjs;
int ii0, ii1, iis;
float m;

int i, ii, j, jj, inew, jnew, qi, qj;
int b, d, t, t0, t1, p, pold, pnew, ok; 
int id, k, adj, bestid, oldid, subid, neighborid, nowid, nowsubid;
int nfire=0, addfire=0, nseg=0, nsub=0, nadj=0;
int nsub_invalid=0, msub_invalid=0, ntotal=0;
int iter=0, mintime=1000;
float *neighbor;
double sumi, sumj, sumk;
double nowsum, nownum, nowmean, bestmean;
bool scatter, near;
queue_t *fifo;
FILE *fp = NULL, *ft = NULL;
size_t res;
char fname[1024];

int seed, pspiral, maxp, px, py, pdx, pdy;
bool inkernel;

int2u **INP = NULL;

bool *FIRE_BOOL    = NULL;
int  *FIRE_TIME    = NULL;
int  *FIRE_SEED    = NULL;
int  *FIRE_DENSITY = NULL;
int  *FIRE_HIST    = NULL;
int  *FIRE_COPY    = NULL;

int   *OBJ_ID        = NULL;
int   *OBJ_LIFETIME  = NULL;
int   *OBJ_STARTTIME = NULL;
int  **OBJ_SEED      = NULL;
int ***OBJ_GAIN      = NULL;

bool *ADJ_TODO  = NULL;
int  *ADJ_ID    = NULL;
int  *ADJ_SUBID = NULL;
int  *ADJ_TIME  = NULL;

int     *SUB_SEGM        = NULL;
bool    *SUBOBJ_VALID    = NULL;
int     *SUBOBJ_MINTIME  = NULL;
int    **SUBOBJ_SEED     = NULL;
double **SUBOBJ_SEEDCALC = NULL;

char buffer[1024] = "\0", cy[5], cm[3];
int *yy = NULL, YY;
int *mm = NULL, m0;
int *ym = NULL, ym0;
int *season = NULL, S;
int doy0;
double *bm = NULL;

int smoothsize, **dirmask;
int s[16], w, minw;
double u[16], minu, bstu;
bool v;

GDALDatasetH dataset = NULL;
GDALRasterBandH band = NULL;
double geotran[6];
char proj[1024];
GDALDatasetH output_file = NULL;
GDALRasterBandH output_band = NULL;
GDALDriverH output_driver = NULL;
char **output_options = NULL;


char finp[1024];
char fdat[1024];
char dout[1024];
char bout[1024];
int init__dist, track_dist, temp__dist, densi_dist;
int max___size, smoothdist;

// INPUT VARIABLES
copy_string(finp, 1024, argv[1]);
copy_string(fdat, 1024, argv[2]);
copy_string(dout, 1024, argv[3]);
copy_string(bout, 1024, argv[4]);
init__dist = atoi(argv[5]);	// 10
track_dist = atoi(argv[6]);	// 15
temp__dist = atoi(argv[7]);	// 5
densi_dist = atoi(argv[8]);	// 10
max___size = atoi(argv[9]);	// 5
smoothdist = atoi(argv[10]);	// 3
if        (strcmp(argv[11], "q") == 0){ v = false;
} else if (strcmp(argv[11], "v") == 0){ v = true;
} else { printf("error: last option must be q or v\n"); exit(1);}

	if (init__dist < 2){ printf("init-searchdist must be > 1.\n"); exit(1);}
	if (track_dist < 2){ printf("track-searchdist must be > 1.\n"); exit(1);}
	if (temp__dist < 1){ printf("temp-dist must be > 0.\n"); exit(1);}
	if (densi_dist < 1){ printf("density-dist must be > 1.\n"); exit(1);}
	if (max___size < 0){ printf("max-size must be >= 0.\n"); exit(1);}
	if (smoothdist < 0){ printf("smooth-dist must be >= 0.\n"); exit(1);}


  	GDALAllRegister();


	smoothsize = (smoothdist*2+1)*(smoothdist*2+1);


	alloc_2D((void***)&dirmask, 16, smoothsize, sizeof(int));
	for (ii=-1*smoothdist, k=0; ii<=smoothdist; ii++){
	for (jj=-1*smoothdist; jj<=smoothdist; jj++, k++){

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


	/** read data
	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/

	if ((dataset = GDALOpen(finp, GA_ReadOnly)) == NULL){
        printf("unable to open %s\n", finp); exit(1);}

	nx = GDALGetRasterXSize(dataset);
	ny = GDALGetRasterYSize(dataset);
	nc = nx*ny;

	nb = GDALGetRasterCount(dataset);


	alloc((void**)&yy,     nb, sizeof(int));
	alloc((void**)&mm,     nb, sizeof(int));
	alloc((void**)&ym,     nb, sizeof(int));
	alloc((void**)&season, nb, sizeof(int));
	alloc((void**)&bm,     12, sizeof(double));


	if ((fp = fopen(fdat, "r")) == NULL){
		printf("Unable to open date file (%s)!\n", fdat); exit(1); }

	for (b=0; b<nb; b++){

		if (fgets(buffer, 1024, fp) == NULL){
			printf("Unable to read from date file, line %d!\n", nb+1); exit(1); }

		strncpy(cy, buffer, 4);
		strncpy(cm, buffer+4, 2);
		cy[4] = '\0'; yy[b] = atoi(cy);
		cm[2] = '\0'; mm[b] = atoi(cm);
		ym[b] = yy[b]*12 + mm[b];

	}

	fclose(fp);




	
	GDALGetGeoTransform(dataset, geotran);
	copy_string(proj, 1024, GDALGetProjectionRef(dataset));
	output_options = CSLSetNameValue(output_options, "TILED", "YES");
	output_options = CSLSetNameValue(output_options, "COMPRESS", "LZW");
	output_options = CSLSetNameValue(output_options, "PREDICTOR", "2");
	output_options = CSLSetNameValue(output_options, "INTERLEAVE", "BAND");
	output_options = CSLSetNameValue(output_options, "BIGTIFF", "YES");
	

	alloc_2D((void***)&INP, nb, nc, sizeof(int2u));

	for (b=0; b<nb; b++){
		band = GDALGetRasterBand(dataset, b+1);
		if (GDALRasterIO(band, GF_Read, 0, 0, nx, ny, INP[b], 
			nx, ny, GDT_UInt16, 0, 0) == CE_Failure){
			printf("could not read band #%d from %s.\n", b+1, finp); exit(1);}
		for (p=0; p<nc; p++){
			if (INP[b][p]>0 && INP[b][p] < 367) bm[mm[b]-1]++;
		}
		printf("%03d: %02d: %f\n", b, mm[b], bm[mm[b]-1]);
	}
	GDALClose(dataset);

	for (b=0; b<12; b++) printf("%02d: %f\n", b+1, bm[b]);

	m0 = 1;
	for (b=1; b<12; b++){
		if (bm[b]<bm[m0]) m0 = b+1;
	}
	doy0 = md2doy(m0, 1);
	printf("burning minimum detected in month %02d\n", m0);
	printf("DOYs are reported relative to DOY %03d\n", doy0);


	ym0 = yy[0]*12 + m0;

	for (b=0, k=0; b<nb; b++){

		if (ym[b] < ym0){
			season[b] = k;
		} else {
			k++;
			season[b] = k;
			ym0 += 12;
		}
	}

	for (b=0; b<nb; b++) printf("%04d %02d %02d\n", yy[b], mm[b], season[b]);
	printf("processing of %02d seasons.\n", season[nb-1]);

	omp_set_num_threads(20);
	//

	#pragma omp parallel default(none) private(w,minw,minu,bstu,p0,ps,i0,i1,is,j0,j1,js,jj0,jj1,jjs,ii0,ii1,iis,m,b,p,i,j,ii,jj,inew,jnew,qi,qj,d,t,t0,t1,pold,pnew,ok,id,adj,bestid,oldid,subid,neighborid,nowid,nowsubid,k,s,u,FIRE_BOOL,FIRE_TIME,FIRE_SEED,FIRE_COPY,OBJ_ID,OBJ_SEED,OBJ_GAIN,OBJ_LIFETIME,OBJ_STARTTIME,ADJ_ID,ADJ_SUBID,ADJ_TIME,ADJ_TODO,SUB_SEGM,SUBOBJ_VALID,SUBOBJ_MINTIME,SUBOBJ_SEED,SUBOBJ_SEEDCALC,fp,ft,sumi,sumj,sumk,nowsum,nownum,nowmean,bestmean,fname,seed,pspiral,maxp,px,py,pdx,pdy,inkernel,fifo,FIRE_HIST,FIRE_DENSITY,output_file,output_band,output_driver) firstprivate(nfire, addfire,nseg,nsub,nadj,nsub_invalid,msub_invalid,ntotal,iter,mintime,D,DD) shared(INP,season,nb,nx,ny,nc,argv,init__dist,track_dist,temp__dist,densi_dist,max___size,smoothdist,dirmask,v,doy0,proj,geotran,dout,bout,output_options)
	{

	#pragma omp for schedule(dynamic,1)
	for (S=season[0]; S<=season[nb-1]; S++){

		alloc((void**)&OLD_BOOL, nc, sizeof(bool));
		alloc((void**)&OLD_SEGM, nc, sizeof(int));
		alloc((void**)&NOW_BOOL, nc, sizeof(bool));
		alloc((void**)&NOW_SEGM, nc, sizeof(int));

		alloc((void**)&FIRE_BOOL, nc, sizeof(bool));
		alloc((void**)&FIRE_SEGM, nc, sizeof(int));
		alloc((void**)&FIRE_TIME, nc, sizeof(int));
		alloc((void**)&FIRE_SEED, nc, sizeof(int));

		alloc((void**)&FIRE_COPY, nc, sizeof(int));

		alloc((void**)&VISITED, nc, sizeof(bool));

		alloc((void**)&OBJ_ID, MAX_OBJECTS, sizeof(int));
		alloc_2D((void***)&OBJ_SEED, 2, MAX_OBJECTS, sizeof(int));
		alloc_3D((void****)&OBJ_GAIN, 366, 9, MAX_OBJECTS, sizeof(int));

		for (b=0; b<nb; b++){
			if (season[b] != S) continue;
			printf("season %d band %d\n", S, b);
			for (p=0; p<nc; p++){
				if (INP[b][p] > 0 && INP[b][p] < 367){
					if (FIRE_TIME[p] == 0){
						FIRE_TIME[p] = (int)INP[b][p] - doy0 + 1;
						if (FIRE_TIME[p] < 1) FIRE_TIME[p]+=366;
					}
					FIRE_BOOL[p] = true;
				}
			}
		}


		/** delete all patches < maxsize pixels
		+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
		connectedcomponents(FIRE_BOOL, FIRE_SEGM, ny, nx);
		nseg = imax(FIRE_SEGM, nc);

		if (max___size > 0){
			alloc((void**)&FIRE_HIST, nseg, sizeof(int));
			for (p=0; p<nc; p++){ // compute histogram
				if (FIRE_BOOL[p]) FIRE_HIST[FIRE_SEGM[p]-1]++;
			}
			for (p=0; p<nc; p++){ // remove small patches
				if (FIRE_BOOL[p] && FIRE_HIST[FIRE_SEGM[p]-1] < max___size){
					FIRE_SEGM[p] = 0;
					FIRE_TIME[p] = 0;
					FIRE_BOOL[p] = false;
				}
			}
			free((void*)FIRE_HIST);
		}


		for (i=0, p=0; i<ny; i++){
		for (j=0; j<nx; j++, p++){
			FIRE_COPY[p] = FIRE_TIME[p];
		}
		}


		for (i=0, p=0; i<ny; i++){
		for (j=0; j<nx; j++, p++){
			if (FIRE_BOOL[p]){
				for (w=0; w<16; w++) u[w] = s[w] = 0;
				for (ii=-1*smoothdist, k=0; ii<=smoothdist; ii++){
				for (jj=-1*smoothdist; jj<=smoothdist; jj++, k++){
					if (i+ii < 0 || i+ii >= ny || 
						j+jj < 0 || j+jj >= nx) continue;
					if (FIRE_BOOL[nx*(i+ii)+j+jj]){
						for (w=0; w<16; w++){
							if (dirmask[w][k] == 1){
								u[w] += FIRE_COPY[nx*(i+ii)+j+jj];
								s[w]++;
							}
						}
					}
				}
				}
				for (w=0; w<16; w++){
					if (s[w] > 0) u[w]/=s[w];
				}
				if (i==4850 && j==3467){

					for (ii=-1*smoothdist, k=0; ii<=smoothdist; ii++){
					for (jj=-1*smoothdist; jj<=smoothdist; jj++, k++){
						printf("%3d ", FIRE_COPY[nx*(i+ii)+j+jj]);
					}
					printf("\n");
					}


					printf("%d\n", FIRE_COPY[p]);
					for (w=0; w<16; w++) printf("%f %d\n", u[w], s[w]);
				}

				minw = -1;
				minu = bstu = 10000;
				for (w=0; w<16; w++){
					if ((u[w]-FIRE_COPY[p])*(u[w]-FIRE_COPY[p]) < minu &&
						s[w] > 0){
						minw = w;
						minu = (u[w]-FIRE_COPY[p])*(u[w]-FIRE_COPY[p]);
						bstu = u[w];
					}
				}
				if (minw >= 0) FIRE_TIME[p] = (int)round(bstu);

			}
		}
		}

		free((void*)FIRE_COPY);



		free((void*)FIRE_BOOL);


		/** start tracking!
		+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/

		// get the pixels that were burnt on the 1st date
		for (p=0, t=1; p<nc; p++){
			if (FIRE_TIME[p] == t) OLD_BOOL[p] = true; else OLD_BOOL[p] = false;
			OLD_SEGM[p] = 0;
		}

		connectedcomponents(OLD_BOOL, OLD_SEGM, ny, nx);
		nfire = imax(OLD_SEGM, nc);
		if (v) printf("number of patches in first date: %d\n", nfire);
		if (v) printf("number of pixels  in first date: %d\n", bsum(OLD_BOOL, nc));
		if (nfire > 0){
			if (nfire >= MAX_OBJECTS){
				printf("Critical failure: MAX_OBJECTS is set too low...\n"); exit(1);}
			for (id=0; id<nfire; id++) OBJ_ID[id] = id+1;
		}

		reset_visited();

		// try to improve seeds
		for (i=0, p=0; i<ny; i++){
		for (j=0; j<nx; j++, p++){
			if (OLD_BOOL[p] && !VISITED[p]){
				VISITED[p] = true;
				sumi = sumj = sumk = 0;
				radial_search_1(i, j, p, OLD_SEGM[p], init__dist, &sumi, &sumj, &sumk);
				if (OBJ_SEED[0][OLD_SEGM[p]-1] > 0){
					printf("failure\n"); } else OBJ_SEED[0][OLD_SEGM[p]-1] = (int) round(sumj/sumk);
				if (OBJ_SEED[1][OLD_SEGM[p]-1] > 0){ 
					printf("failure\n"); } else OBJ_SEED[1][OLD_SEGM[p]-1] = (int) round(sumi/sumk);
				OBJ_GAIN[t-1][0][OLD_SEGM[p]-1] = (int) sumk;
				FIRE_SEED[nx*((int)round(sumi/sumk))+((int)round(sumj/sumk))] = OLD_SEGM[p];
			}
		}
		}


		// cycle through every remaining day
		for (t=2; t<=366; t++){

			// *** STEP ONE: PUT OLD AND ACTIVE FIRES IN QUEUE ***************
			// ***************************************************************

			// initialize new queue
			if ((fifo = create_queue()) == NULL){
				printf("failed to create new queue!\n"); exit(1);}

			// get new fire pixels
			for (p=0; p<nc; p++){
				if (FIRE_TIME[p] == t) NOW_BOOL[p] = true; else NOW_BOOL[p] = false;
				NOW_SEGM[p] = 0;
			}

	//D = 0;
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
	//		D = D * -1;
	//		printf("i: %d %d %d j: %d %d %d p: %d %d\n",
	//			i0, i1, is, j0, j1, js, p0, ps);

			for (i=i0; i!=i1; i+=is){
			for (j=j0; j!=j1; j+=js){

			p = nx*i+j; 
	//printf("%d %d %d\n", i, j, p);
	//		for (i=0, p=0; i<ny; i++){
	//		for (j=0; j<nx; j++, p++){

				// if old and active
				if (OLD_BOOL[p] && abs((t-FIRE_TIME[p])) <= temp__dist){

					// if at edge of patch, put in queue
					for (ii=-1; ii<=1; ii++){
					for (jj=-1; jj<=1; jj++){
						
						if (ii==0 && jj==0) continue;
						inew = i+ii; jnew = j+jj;
						if (inew < 0 || inew >= ny || jnew < 0 || jnew >= nx) continue;
						
						if (!OLD_BOOL[nx*inew+jnew]){
							if (!(ok = enqueue(fifo, i, j))){ // put in queue
								printf("Failed to allocate enqueue memory\n");
								exit(1);
							}
							ii = jj = 10; // pixel wa already put in queue, proceed with next one
						}
					}
					}

				}

			}
			}
			//if (v) printf("queue length: %d\n", fifo->length);


			// *** STEP TWO: TRACE THE FIRE FRONTS ***************************
			// ***************************************************************

			while (dequeue(fifo, &qi, &qj)){

				pold = nx*qi+qj;
	//DD = 0;
				if (DD==0){
					ii0 = -1*track_dist; ii1 = track_dist; iis = 1;
					jj0 = -1*track_dist; jj1 = track_dist; jjs = 1;
					DD++;
				} else if (DD==1){
					ii0 = track_dist; ii1 = -1*track_dist; iis = -1;
					jj0 = track_dist; jj1 = -1*track_dist; jjs = -1;
					DD++;
				} else if (DD==2){
					ii0 = -1*track_dist; ii1 = track_dist; iis = 1;
					jj0 = track_dist; jj1 = -1*track_dist; jjs = -1;
					DD++;
				} else if (DD==3){
					ii0 = track_dist; ii1 = -1*track_dist; iis = -1;
					jj0 = -1*track_dist; jj1 = track_dist; jjs = 1;
					DD=0;
				}

				for (ii=ii0; ii!=ii1; ii+=iis){
				for (jj=jj0; jj!=jj1; jj+=jjs){

	//			for (ii=-1*track_dist; ii<=track_dist; ii++){
	//			for (jj=-1*track_dist; jj<=track_dist; jj++){

					if (ii==0 && jj==0) continue;
					inew = qi+ii; jnew = qj+jj;
					if (inew < 0 || inew >= ny || jnew < 0 || jnew >= nx) continue;
					pnew = nx*inew+jnew;

					if (FIRE_SEGM[pnew] != FIRE_SEGM[pold]) continue;
					if (sqrt(ii*ii+jj*jj) > track_dist) continue;

					// if new fire around dequeued pixel:
					if (NOW_BOOL[pnew]){
						// add pixel to segmentation
						OLD_SEGM[pnew] = OLD_SEGM[pold];
						OLD_BOOL[pnew] = true;
						// clear pixel from NOW
						NOW_BOOL[pnew] = false;

						id = OLD_SEGM[pnew]-1;

						d = get_direction(OBJ_SEED[0][id], OBJ_SEED[1][id], jnew, inew);

						OBJ_GAIN[t-1][0][id]++;
						OBJ_GAIN[t-1][d][id]++;

						// put the pixel in the queue
						if (!(ok = enqueue(fifo, inew, jnew))){
							printf("Failed to allocate enqueue memory\n");
							exit(1);
						}

					}

				}
				}


			}

			// free queue's memory
			destroy_queue(fifo);
			fifo = NULL;


			// *** STEP THREE: PUT NEW FIRES IN QUEUE ************************
			// ***************************************************************

			connectedcomponents(NOW_BOOL, NOW_SEGM, ny, nx);
			addfire = imax(NOW_SEGM, nc);

			reset_visited();

			// try to improve seeds
			for (i=0, p=0; i<ny; i++){
			for (j=0; j<nx; j++, p++){
				if (NOW_BOOL[p] && !VISITED[p]){
					sumi = sumj = sumk = 0;
					VISITED[p] = true;
					radial_search_2(i, j, p, NOW_SEGM[p], init__dist, &sumi, &sumj, &sumk);
					if (OBJ_SEED[0][NOW_SEGM[p]+nfire-1] > 0){
						printf("hmm\n"); } else OBJ_SEED[0][NOW_SEGM[p]+nfire-1] = (int) round(sumj/sumk);
					if (OBJ_SEED[1][NOW_SEGM[p]+nfire-1] > 0){ 
						printf("hii\n"); } else OBJ_SEED[1][NOW_SEGM[p]+nfire-1] = (int) round(sumi/sumk);
					OBJ_GAIN[t-1][0][NOW_SEGM[p]+nfire-1] = (int) sumk;
					FIRE_SEED[nx*((int)round(sumi/sumk))+((int)round(sumj/sumk))] = NOW_SEGM[p]+nfire;
				}
			}
			}

			if (addfire>0){
				// add nfire to Object IDs and add to OLD_SEGM
				for (p=0; p<nc; p++){
					if (NOW_BOOL[p]){
						OLD_SEGM[p] = NOW_SEGM[p]+nfire;
						OLD_BOOL[p] = true;
					}
				}
				if (nfire+addfire >= MAX_OBJECTS){
					printf("Critical failure: MAX_OBJECTS is set too low...\n"); exit(1);}
				for (id=nfire; id<(nfire+addfire); id++) OBJ_ID[id] = id+1;

				nfire+=addfire;
			}
			if (v) printf("number of new patches at DOY %03d: %d, number of patches: %d\n", t, addfire, nfire);

		}

		free((void*)NOW_BOOL);
		free((void*)NOW_SEGM);
		free((void*)FIRE_SEGM);


		// *** MOVE FIRE SEED CENTROIDS TO BURNT PIXELS ******************
		// ***************************************************************
		for (i=0, p=0; i<ny; i++){
		for (j=0; j<nx; j++, p++){

			seed = FIRE_SEED[p];
			if (seed > 0 && OLD_SEGM[p] != seed){

				maxp = nx*nx;
				px = py = pdx = 0;
				pdy = -1;

				for (pspiral=0; pspiral<maxp; pspiral++){

					if ((inkernel = spiral(&jj, &ii, nx/2, nx/2, pspiral, &px, &py, &pdx, &pdy))){

						if (!xyfocal_to_p(j, i, jj, ii, 
									&jnew, &inew, &pnew)) continue;

						if (OLD_BOOL[pnew] && OLD_SEGM[pnew] == seed){
							FIRE_SEED[p] = 0;
							FIRE_SEED[pnew] = seed;
							OBJ_SEED[0][seed-1] = jnew;
							OBJ_SEED[1][seed-1] = inew;
							break;
						}

					}
				}

			}

		}
		}


	/** PREPARING SECOND PHASE *** sub-segment the segmentation **************
	**************************************************************************
	**************************************************************************
	**************************************************************************
	*************************************************************************/

		alloc((void**)&SUB_SEGM, nc, sizeof(int));

		alloc((void**)&SUBOBJ_VALID, MAX_OBJECTS, sizeof(bool));
		alloc((void**)&SUBOBJ_MINTIME, MAX_OBJECTS, sizeof(int));

		// initialize new queue
		if ((fifo = create_queue()) == NULL){
			printf("failed to create new queue!\n"); exit(1);}

		reset_visited();
		nsub = 0;

		// sub-segment the segmentation
		for (i=0, p=0; i<ny; i++){
		for (j=0; j<nx; j++, p++){

			if (!VISITED[p] && OLD_BOOL[p]){

				nsub++;
				id = OLD_SEGM[p];

				// put the pixel in the queue
				if (!(ok = enqueue(fifo, i, j))){
					printf("Failed to allocate enqueue memory\n");
					exit(1);}

				// track the pixel
				while (dequeue(fifo, &qi, &qj)){

					pnew = xy_to_p(qj, qi);
					if (VISITED[pnew]) continue;

					// label this pixel
					SUB_SEGM[pnew] = nsub;
					VISITED[pnew]  = true;

					// if there is a seed in this patch, it is a valid patch
					if (FIRE_SEED[pnew] > 0) SUBOBJ_VALID[nsub-1] = true;

					// record the earliest pixel of this patch
					if (SUBOBJ_MINTIME[nsub-1] == 0 || 
						FIRE_TIME[pnew] < SUBOBJ_MINTIME[nsub-1]){
						SUBOBJ_MINTIME[nsub-1] = FIRE_TIME[pnew];}

					// record the earliest pixel overall
					if (SUBOBJ_MINTIME[nsub-1] < mintime) mintime = SUBOBJ_MINTIME[nsub-1];

					// put the neighboring pixels in queue if they belong to the same patch
					for (ii=-1; ii<=1; ii++){
					for (jj=-1; jj<=1; jj++){

						if (!xyfocal_to_p(qj, qi, jj, ii, 
										  &jnew, &inew, &pnew)) continue;

						if (!VISITED[pnew] && OLD_BOOL[pnew] && OLD_SEGM[pnew] == id){
							if (!(ok = enqueue(fifo, inew, jnew))){
								printf("Failed to allocate enqueue memory\n");
								exit(1);}
						}

					}
					}
				}

			} else VISITED[p] = true;
		}
		}

		// free queue's memory
		destroy_queue(fifo); fifo = NULL;

		alloc_2D((void***)&SUBOBJ_SEED, 2, nsub, sizeof(int));
		alloc_2D((void***)&SUBOBJ_SEEDCALC, 3, nsub, sizeof(double));

		// calculate the potential seed points for every sub-patch
		for (i=0, p=0; i<ny; i++){
		for (j=0; j<nx; j++, p++){
			if (!OLD_BOOL[p]) continue;
			subid = SUB_SEGM[p];
			if (SUBOBJ_MINTIME[subid-1] == FIRE_TIME[p]){
				SUBOBJ_SEEDCALC[0][subid-1] += j;
				SUBOBJ_SEEDCALC[1][subid-1] += i;
				SUBOBJ_SEEDCALC[2][subid-1] += 1;
			}
		}
		}

		for (i=0; i<nsub; i++){
			SUBOBJ_SEED[0][i] = (int)round(SUBOBJ_SEEDCALC[0][i]/SUBOBJ_SEEDCALC[2][i]);
			SUBOBJ_SEED[1][i] = (int)round(SUBOBJ_SEEDCALC[1][i]/SUBOBJ_SEEDCALC[2][i]);
		}

		free_2D((void**)SUBOBJ_SEEDCALC, 3);


	/** SECOND PHASE *** re-assign sub-patches with no own seed point ********
	**************************************************************************
	**************************************************************************
	**************************************************************************
	*************************************************************************/

		alloc((void**)&ADJ_ID, MAX_OBJECTS, sizeof(int));
		alloc((void**)&ADJ_SUBID, MAX_OBJECTS, sizeof(int));
		alloc((void**)&ADJ_TIME, MAX_OBJECTS, sizeof(int));
		alloc((void**)&ADJ_TODO, MAX_OBJECTS, sizeof(bool));

		// proceed until all sub-patches are valid!
		while ((nsub_invalid = nsub-bsum(SUBOBJ_VALID, nsub)) > 0 && mintime <=366){
			iter++;

			// if both are still equal after one iteration: add new seeds
			msub_invalid = nsub_invalid;

			if (v) printf("invalid patches: %d; total patches: %d; iteration %d\n", nsub_invalid, nsub, iter);


			// initialize new queue
			if ((fifo = create_queue()) == NULL){
				printf("failed to create new queue!\n"); exit(1);}

			reset_visited();

			// *** STEP ONE: RE-ASSIGN INVALID PATCHES ***********************
			// ***************************************************************

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
		//	for (i=0, p=0; i<ny; i++){
		//	for (j=0; j<nx; j++, p++){

				if (!VISITED[p] && OLD_BOOL[p]){


					subid = SUB_SEGM[p];

					// if patch is valid, i.e. if it has a seed: do nothing for now
					if (SUBOBJ_VALID[subid-1]){ VISITED[p] = true; continue;}

					// put the pixel in the queue
					if (!(ok = enqueue(fifo, i, j))){
						printf("Failed to allocate enqueue memory\n");
						exit(1);}

					nadj = 0;
					// track the pixel
					while (dequeue(fifo, &qi, &qj)){

						pnew = xy_to_p(qj, qi);
						if (VISITED[pnew]) continue;
						VISITED[pnew] = true;

						for (ii=-1; ii<=1; ii++){
						for (jj=-1; jj<=1; jj++){

							if (!xyfocal_to_p(qj, qi, jj, ii, 
										  &jnew, &inew, &pnew)) continue;

							// if the pixel belongs to a neighboring patch
							if (OLD_BOOL[pnew] && (neighborid = SUB_SEGM[pnew]) != subid){
								// if adjacent patch is valid: log it
								if (SUBOBJ_VALID[neighborid-1]){
									ADJ_SUBID[nadj] = neighborid;
									ADJ_ID[nadj]    = OLD_SEGM[pnew];
									ADJ_TIME[nadj]  = abs(FIRE_TIME[pnew]-FIRE_TIME[nx*qi+qj]);
									nadj++;
								}
							} else if (!VISITED[pnew] && OLD_BOOL[pnew] && SUB_SEGM[pnew] == subid){
								// put the pixel in the queue
								if (!(ok = enqueue(fifo, inew, jnew))){
									printf("Failed to allocate enqueue memory\n"); exit(1);}
							}

						}
						}
					}

					// if no adjacent valid patch was found: continue
					if (nadj == 0) continue;

					for (adj=0; adj<nadj; adj++) ADJ_TODO[adj] = true;

					// find the patch with the lowest time difference
					bestmean = 10000000;
					while (bsum(ADJ_TODO, nadj) > 0){
						for (adj=0, nowid=0, nowsubid=0; adj<nadj; adj++){
							if (!ADJ_TODO[adj]) continue;
							if (nowsubid == 0){
								nowsubid = ADJ_SUBID[adj];
								nowid    = ADJ_ID[adj];
								nowsum   = ADJ_TIME[adj];
								nownum   = 1;
								ADJ_TODO[adj] = false;
							} else if (ADJ_SUBID[adj] == nowsubid){
								nowsum += ADJ_TIME[adj];
								nownum++;
								ADJ_TODO[adj] = false;
							}
						}
						if ((nowmean = nowsum/nownum) < bestmean){
							bestmean = nowmean;
							bestid = nowid;
						}

					}

					// if temporally closest patch is not close enough: continue
					if (bestmean > temp__dist*2) continue;

					// put the pixel in the queue
					if (!(ok = enqueue(fifo, i, j))){
						printf("Failed to allocate enqueue memory\n"); exit(1);}

					// track the patch again and re-assign
					while (dequeue(fifo, &qi, &qj)){

						pnew = xy_to_p(qj, qi);

						// if we already re-assigned this pixel: continue
						if ((oldid = OLD_SEGM[pnew]) == bestid) continue;

						// re-assign
						OLD_SEGM[pnew] = bestid;

						// fire-time of this pixel
						t = FIRE_TIME[pnew];

						// adjust the spread rates #1: remove from old patch
						d = get_direction(OBJ_SEED[0][oldid-1], OBJ_SEED[1][oldid-1], qj, qi);
						OBJ_GAIN[t-1][0][oldid-1]--; // total spread rate
						OBJ_GAIN[t-1][d][oldid-1]--; // directional spread rate

						// adjust the spread rates #2: add to new patch
						d = get_direction(OBJ_SEED[0][bestid-1], OBJ_SEED[1][bestid-1], qj, qi);
						OBJ_GAIN[t-1][0][bestid-1]++; // total spread rate
						OBJ_GAIN[t-1][d][bestid-1]++; // directional spread rate


						for (ii=-1; ii<=1; ii++){
						for (jj=-1; jj<=1; jj++){

							if (!xyfocal_to_p(qj, qi, jj, ii, 
										  &jnew, &inew, &pnew)) continue;

							if (OLD_BOOL[pnew] && SUB_SEGM[pnew] == subid){
								// put the pixel in the queue
								if (!(ok = enqueue(fifo, inew, jnew))){
									printf("Failed to allocate enqueue memory\n"); exit(1);}
							}

						}
						}
					}

					// this patch is now valid
					SUBOBJ_VALID[subid-1] = true;
					msub_invalid--;

				} else VISITED[p] = true;

			}
			}

			// free queue's memory
			destroy_queue(fifo); fifo = NULL;


			// *** STEP TWO: ADD NEW PATCHES *********************************
			// ***************************************************************

			// if nothing could be re-assigned, add new seeds; start with oldest
			if (nsub_invalid == msub_invalid){

				// initialize new queue
				if ((fifo = create_queue()) == NULL){
					printf("failed to create new queue!\n"); exit(1);}

				mintime++; // increment the "oldest" pixels
				addfire = 0;

				reset_visited();

				for (i=0, p=0; i<ny; i++){
				for (j=0; j<nx; j++, p++){

					if (OLD_BOOL[p]){

						subid = SUB_SEGM[p];

						// if this patch is already valid: continue
						if (SUBOBJ_VALID[subid-1]) continue;

						// if old: add new seed
						if (SUBOBJ_MINTIME[subid-1] < mintime){

							if (!(ok = enqueue(fifo, i, j))){
								printf("Failed to allocate enqueue memory\n"); exit(1);}

							nfire++;  // increment total number of fire patches
							addfire++; // increment patches that were added in this iteration
							SUBOBJ_VALID[subid-1] = true; // this patch is now valid
							OBJ_SEED[0][nfire-1] = SUBOBJ_SEED[0][subid-1]; // create new seed
							OBJ_SEED[1][nfire-1] = SUBOBJ_SEED[1][subid-1]; // create new seed
							if (nfire >= MAX_OBJECTS){
								printf("Critical failure: MAX_OBJECTS is set too low...\n"); exit(1);}
							OBJ_ID[nfire-1] = nfire; // add newID
							FIRE_SEED[nx*OBJ_SEED[1][nfire-1]+OBJ_SEED[0][nfire-1]] = nfire; // draw new seed

							// track this patch
							while (dequeue(fifo, &qi, &qj)){

								pnew = xy_to_p(qj, qi);
								if (VISITED[pnew]) continue;
								VISITED[pnew] = true;

								t = FIRE_TIME[pnew];
								oldid = OLD_SEGM[pnew];
								OLD_SEGM[pnew] = nfire; // re-assign

								// adjust the spread rates #1: remove from old patch
								d = get_direction(OBJ_SEED[0][oldid-1], OBJ_SEED[1][oldid-1], qj, qi);
								OBJ_GAIN[t-1][0][oldid-1]--; // total spread rate
								OBJ_GAIN[t-1][d][oldid-1]--; // directional spread rate

								// if newer than oldest fire in subpatch: add to directional spread
								if (t >= mintime){
									d = get_direction(SUBOBJ_SEED[0][subid-1], SUBOBJ_SEED[1][subid-1], qj, qi);
									OBJ_GAIN[t-1][d][nfire-1]++; // directional spread rate
								}
								OBJ_GAIN[t-1][0][nfire-1]++; // total spread rate


								for (ii=-1; ii<=1; ii++){
								for (jj=-1; jj<=1; jj++){

									if (!xyfocal_to_p(qj, qi, jj, ii, 
												  &jnew, &inew, &pnew)) continue;

									if (!VISITED[pnew] && OLD_BOOL[pnew] && SUB_SEGM[pnew] == subid){
										// put the pixel in the queue
										if (!(ok = enqueue(fifo, inew, jnew))){
											printf("Failed to allocate enqueue memory\n"); exit(1);}
									}

								}
								}
							}
						
						}
					}
				}
				}

				if (v) printf("added %d new patches that were burnt before DOY %d\n", addfire, mintime);
				destroy_queue(fifo); fifo = NULL;

			}

		}

		free((void*)ADJ_ID);
		free((void*)ADJ_SUBID);
		free((void*)ADJ_TIME);
		free((void*)ADJ_TODO);
		free((void*)VISITED);
		free((void*)SUB_SEGM);
		free((void*)SUBOBJ_MINTIME);
		free((void*)SUBOBJ_VALID);
		free_2D((void**)SUBOBJ_SEED, 2);


		// *** MOVE FIRE SEED CENTROIDS TO BURNT PIXELS ******************
		// ***************************************************************
		for (i=0, p=0; i<ny; i++){
		for (j=0; j<nx; j++, p++){

			seed = FIRE_SEED[p];
			if (seed > 0 && OLD_SEGM[p] != seed){

				maxp = nx*nx;
				px = py = pdx = 0;
				pdy = -1;

				for (pspiral=0; pspiral<maxp; pspiral++){

					if ((inkernel = spiral(&jj, &ii, nx/2, nx/2, pspiral, &px, &py, &pdx, &pdy))){

						if (!xyfocal_to_p(j, i, jj, ii, 
									&jnew, &inew, &pnew)) continue;

						if (OLD_BOOL[pnew] && OLD_SEGM[pnew] == seed){
							FIRE_SEED[p] = 0;
							FIRE_SEED[pnew] = seed;
							OBJ_SEED[0][seed-1] = jnew;
							OBJ_SEED[1][seed-1] = inew;
							break;
						}

					}
				}




			}

		}
		}


		alloc((void**)&FIRE_DENSITY, nc, sizeof(int));

		// *** COMPUTE FIRE DENSITY **************************************
		// ***************************************************************
		for (i=0, p=0; i<ny; i++){
		for (j=0; j<nx; j++, p++){
			k = 0;
			for (ii=-1*densi_dist; ii<=densi_dist; ii++){
			for (jj=-1*densi_dist; jj<=densi_dist; jj++){
				inew = i+ii; jnew = j+jj;
				if (inew < 0 || inew >= ny || jnew < 0 || jnew >= nx) continue;
				if (FIRE_SEED[nx*inew+jnew] < 1 || sqrt(ii*ii+jj*jj) > densi_dist) continue;
				k++;
			}
			}
			FIRE_DENSITY[p] = k;
		}
		}

		// *** COMPUTE THE LIFETIME OF EACH FIRE *************************
		// ***************************************************************
		alloc((void**)&OBJ_LIFETIME, nfire, sizeof(int));
		alloc((void**)&OBJ_STARTTIME, nfire, sizeof(int));
		for (id=0; id<nfire; id++){
			for (t=0, t0=0, t1=0; t<366; t++){

				if (OBJ_GAIN[t][0][id] == 0){
					continue;
				} else if (OBJ_GAIN[t][0][id] > 0){
					if (t0 == 0){
						t0 = t;
					} else {
						t1 = t;
					}
				}

			}
			if (t1 == 0) t1 = t0;
			OBJ_LIFETIME[id]  = t1-t0+1;
			OBJ_STARTTIME[id] = t0+1;
		}

		// *** COMPUTE THE TOTAL SIZE OF EACH FIRE ***********************
		// ***************************************************************
		alloc((void**)&FIRE_HIST, nfire, sizeof(int));
		
		for (p=0; p<nc; p++){
			if (OLD_BOOL[p]) FIRE_HIST[OLD_SEGM[p]-1]++;
		}
		for (id=0; id<nfire; id++) if (FIRE_HIST[id] > 0) ntotal++;
		if (v) printf("total patches new: %d\n", ntotal);

		// *** WRITE THE OUTPUT TABLE ************************************
		// ***************************************************************
		sprintf(fname, "%s/fire-spread_%s_season-%02d.txt", argv[3], argv[4], S);
		ft = fopen(fname, "w");
		fprintf(ft, "ID SIZE IPX IPY IPT LIFE");
		for (t=0; t<366; t++) fprintf(ft, " SPREAD_T%03d", t+1);
		fprintf(ft, "\n");
		for (id=0; id<nfire; id++){
			if (FIRE_HIST[id] == 0) continue;
			fprintf(ft, "%07d %07d %06d %06d %03d %03d", 
				OBJ_ID[id], FIRE_HIST[id], 
				OBJ_SEED[0][id]+1, OBJ_SEED[1][id]+1, 
				OBJ_STARTTIME[id], OBJ_LIFETIME[id]);
			for (t=0; t<366; t++){
				fprintf(ft, " %d", OBJ_GAIN[t][0][id]);
				for (d=1; d<9; d++) fprintf(ft, "/%d", OBJ_GAIN[t][d][id]);
			}
			fprintf(ft, "\n");
		}
		fclose(ft);


		if ((output_driver = GDALGetDriverByName("GTiff")) == NULL){
			printf("%s driver not found\n", "GTiff"); exit(1);}

		sprintf(fname, "%s/fire-spread_%s_season-%02d.tif", dout, bout, S);
		if ((output_file = GDALCreate(output_driver, fname, nx, ny, 4, GDT_Int32, output_options)) == NULL){
			printf("Error creating memory file %s. ", fname); exit(1);}

		output_band = GDALGetRasterBand(output_file, 1);
		if (GDALRasterIO(output_band, GF_Write, 0, 0, 
					nx, ny, OLD_SEGM, 
					nx, ny, GDT_Int32, 0, 0) == CE_Failure){
					printf("Unable to write %s. ", fname); exit(1);}
		GDALSetDescription(output_band, "fire segmentation");
		GDALSetRasterNoDataValue(output_band, 0);

		output_band = GDALGetRasterBand(output_file, 2);
		if (GDALRasterIO(output_band, GF_Write, 0, 0, 
					nx, ny, FIRE_SEED, 
					nx, ny, GDT_Int32, 0, 0) == CE_Failure){
					printf("Unable to write %s. ", fname); exit(1);}
		GDALSetDescription(output_band, "fire seeds");
		GDALSetRasterNoDataValue(output_band, 0);

		output_band = GDALGetRasterBand(output_file, 3);
		if (GDALRasterIO(output_band, GF_Write, 0, 0, 
					nx, ny, FIRE_TIME, 
					nx, ny, GDT_Int32, 0, 0) == CE_Failure){
					printf("Unable to write %s. ", fname); exit(1);}
		GDALSetDescription(output_band, "fire timing");
		GDALSetRasterNoDataValue(output_band, 0);

		output_band = GDALGetRasterBand(output_file, 4);
		if (GDALRasterIO(output_band, GF_Write, 0, 0, 
					nx, ny, FIRE_DENSITY, 
					nx, ny, GDT_Int32, 0, 0) == CE_Failure){
					printf("Unable to write %s. ", fname); exit(1);}
		GDALSetDescription(output_band, "fire density");
		GDALSetRasterNoDataValue(output_band, 0);

		#pragma omp critical
		{
			GDALSetGeoTransform(output_file, geotran);
			GDALSetProjection(output_file,   proj);
		}
		
		GDALClose(output_file);


		free((void*)OLD_BOOL);
		free((void*)OLD_SEGM);

		free((void*)FIRE_TIME);
		free((void*)FIRE_SEED);
		free((void*)FIRE_HIST);
		free((void*)FIRE_DENSITY);

		free((void*)OBJ_ID);
		free((void*)OBJ_LIFETIME);
		free((void*)OBJ_STARTTIME);
		free_2D((void**)OBJ_SEED, 2);
		free_3D((void***)OBJ_GAIN, 366, 9);

	}

	}

	free_2D((void**)dirmask, 16);
	free_2D((void**)INP, nb);
	free((void*)yy);
	free((void*)mm);
	free((void*)bm);
	free((void*)ym);
	free((void*)season);

	CSLDestroy(output_options);

	return SUCCESS;

}

