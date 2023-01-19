#ifndef DTYPE_H
#define DTYPE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int int4u;
typedef signed int int4s;
typedef unsigned short int int2u;
typedef signed short int int2s;
typedef unsigned char int1u;
typedef signed char int1s;

#define MAX_OBJECTS 1000000

typedef struct {
  int n;
  char finp[1024];
  char fdat[1024];
  char dout[1024];
  char bout[1024];
  int init__dist, track_dist, temp__dist, densi_dist;
  int min___size, smoothdist;
  int ncpu, queue_size;
  bool v;
} args_t;


typedef struct {
  int n; // number of bands
  int *years; 
  int *months;
  int *seasons;
  int min_month;
  int min_doy;
  int nseasons;
} bands_t;


typedef struct {

  bool *OLD_BOOL; 
  int  *OLD_SEGM;

  bool *VISITED;

  int  *FIRE_HIST;
  int  *FIRE_TIME;
  int  *FIRE_SEED;
  int  *FIRE_DENSITY;

  int nfire;
  int   *OBJ_ID;
  int   *OBJ_LIFETIME;
  int   *OBJ_STARTTIME;
  int  **OBJ_SEED;
  int ***OBJ_GAIN;

} fire_t;

typedef struct {
  bool *NOW_BOOL;
  int  *NOW_SEGM;
  int  *FIRE_SEGM;
} phase1_t;

typedef struct {
  int     *SEGM;
  bool    *VALID;
  int     *MINTIME;
  int    **SEED;
} phase2_subobjects_t;

typedef struct {
  bool *TODO;
  int  *ID;
  int  *SUBID;
  int  *TIME;
} phase2_adjacent_t;


#ifdef __cplusplus
}
#endif

#endif
