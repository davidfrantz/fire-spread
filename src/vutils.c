#include "vutils.h"

//-------------------------------------------------------------------
//  return maximum of two integers
int imaxoftwo(int x, int y){
  if (x >= y) return(x); else return(y);
}

//-------------------------------------------------------------------
//  return maximum of two integers, INT2U method
int2u imaxoftwo2u(int2u x, int2u y){
  if (x >= y) return(x); else return(y);
}

//-------------------------------------------------------------------
//  return maximum of two integers, INT2S method
int2s imaxoftwo2s(int2s x, int2s y){
  if (x >= y) return(x); else return(y);
}

//-------------------------------------------------------------------
//  return maximum of two integers, INT1U method
int1u imaxoftwo1u(int1u x, int1u y){
  if (x >= y) return(x); else return(y);
}

//-------------------------------------------------------------------
//  return maximum of two integers, INT1S method
int1s imaxoftwo1s(int1s x, int1s y){
  if (x >= y) return(x); else return(y);
}

//-------------------------------------------------------------------
//  return maximum of two doubles
float fmaxoftwo(float x, float y){
  if (x >= y) return(x); else return(y);
}

//-------------------------------------------------------------------
//  return minimum of two integers
int iminoftwo(int x, int y){
  if (x <= y) return(x); else return(y);
}

//-------------------------------------------------------------------
//  return minimum of two integers, INT2U method
int2u iminoftwo2u(int2u x, int2u y){
  if (x <= y) return(x); else return(y);
}

//-------------------------------------------------------------------
//  return minimum of two integers, INT2S method
int2s iminoftwo2s(int2s x, int2s y){
  if (x <= y) return(x); else return(y);
}

//-------------------------------------------------------------------
//  return minimum of two integers, INT1U method
int1u iminoftwo1u(int1u x, int1u y){
  if (x <= y) return(x); else return(y);
}

//-------------------------------------------------------------------
//  return minimum of two integers, INT1S method
int1s iminoftwo1s(int1s x, int1s y){
  if (x <= y) return(x); else return(y);
}

//-------------------------------------------------------------------
//  return minimum of two doubles
float fminoftwo(float x, float y){
  if (x <= y) return(x); else return(y);
}

//-------------------------------------------------------------------
//  return maximum of integer array
int imax(int *mat, int n){
  int i;
  int maxim, val;

  maxim = mat[0];
  for (i=1; i<n; i++) {
    val = mat[i];
    if (val > maxim) maxim = val;
  }

  return(maxim);
} 

//-------------------------------------------------------------------
//  return maximum of integer array, INT2U method
int2u imax2u(int2u *mat, int n){
  int i;
  int2u maxim, val;

  maxim = mat[0];
  for (i=1; i<n; i++) {
    val = mat[i];
    if (val > maxim) maxim = val;
  }

  return(maxim);
} 

//-------------------------------------------------------------------
//  return maximum of integer array, INT2S method
int2s imax2s(int2s *mat, int n){
  int i;
  int2s maxim, val;

  maxim = mat[0];
  for (i=1; i<n; i++) {
    val = mat[i];
    if (val > maxim) maxim = val;
  }

  return(maxim);
} 

//-------------------------------------------------------------------
//  return maximum of integer array, INT1U method
int1u imax1u(int1u *mat, int n){
  int i;
  int1u maxim, val;

  maxim = mat[0];
  for (i=1; i<n; i++) {
    val = mat[i];
    if (val > maxim) maxim = val;
  }

  return(maxim);
} 

//-------------------------------------------------------------------
//  return maximum of integer array, INT1S method
int1s imax1s(int1s *mat, int n){
  int i;
  int1s maxim, val;

  maxim = mat[0];
  for (i=1; i<n; i++) {
    val = mat[i];
    if (val > maxim) maxim = val;
  }

  return(maxim);
} 

//-------------------------------------------------------------------
//  return maximum of float array
float floatmax(float *mat, int n){
  int i;
  float maxim, val;

  maxim = mat[0];
  for (i=1; i<n; i++) {
    val = mat[i];
    if (val > maxim) maxim = val;
  }

  return(maxim);
} 

//-------------------------------------------------------------------
//  return minimum of integer array
int imin(int *mat, int n){
  int i;
  int minim, val;

  minim = mat[0];
  for (i=1; i<n; i++) {
    val = mat[i];
    if (val < minim) minim = val;
  }

  return(minim);
} 

//-------------------------------------------------------------------
//  return minimum of integer array, INT2U method
int2u imin2u(int2u *mat, int n){
  int i;
  int2u minim, val;

  minim = mat[0];
  for (i=1; i<n; i++) {
    val = mat[i];
    if (val < minim) minim = val;
  }

  return(minim);
} 

//-------------------------------------------------------------------
//  return minimum of integer array, INT2S method
int2s imin2s(int2s *mat, int n){
  int i;
  int2s minim, val;

  minim = mat[0];
  for (i=1; i<n; i++) {
    val = mat[i];
    if (val < minim) minim = val;
  }

  return(minim);
} 

//-------------------------------------------------------------------
//  return minimum of integer array, INT1U method
int1u imin1u(int1u *mat, int n){
  int i;
  int1u minim, val;

  minim = mat[0];
  for (i=1; i<n; i++) {
    val = mat[i];
    if (val < minim) minim = val;
  }

  return(minim);
} 

//-------------------------------------------------------------------
//  return minimum of integer array, INT1S method
int1s imin1s(int1s *mat, int n){
  int i;
  int1s minim, val;

  minim = mat[0];
  for (i=1; i<n; i++) {
    val = mat[i];
    if (val < minim) minim = val;
  }

  return(minim);
} 

//-------------------------------------------------------------------
//  return minimum of float array
float floatmin(float *mat, int n){
  int i;
  float minim, val;

  minim = mat[0];
  for (i=1; i<n; i++) {
    val = mat[i];
    if (val < minim) minim = val;
  }

  return(minim);
} 

//-------------------------------------------------------------------
//  sums arrays, INT method
int isum(int *x, int n){
int y=0;
int i;
  for (i=0; i<n; i++) y += x[i];
  return y;
}

//-------------------------------------------------------------------
//  sums arrays, INT1U method
int isum1u(int1u *x, int n){
int y=0;
int i;
  for (i=0; i<n; i++) y += x[i];
  return y;
}

//-------------------------------------------------------------------
//  sums arrays, INT1S method
int isum1s(int1s *x, int n){
int y=0;
int i;
  for (i=0; i<n; i++) y += x[i];
  return y;
}

//-------------------------------------------------------------------
//  sums arrays, INT2U method
int isum2u(int2u *x, int n){
int y=0;
int i;
  for (i=0; i<n; i++) y += x[i];
  return y;
}

//-------------------------------------------------------------------
//  sums arrays, INT2S method
int isum2s(int2s *x, int n){
int y=0;
int i;
  for (i=0; i<n; i++) y += x[i];
  return y;
}

//-------------------------------------------------------------------
//  sums arrays, FLOAT method
float fsum(float *x, int n){
float y=0;
int i;
  for (i=0; i<n; i++) y += x[i];
  return y;
}

//-------------------------------------------------------------------
//  sums arrays, BOOL method
int bsum(bool *x, int n){
int y=0;
int i;
  for (i=0; i<n; i++) y += x[i];
  return y;
}


