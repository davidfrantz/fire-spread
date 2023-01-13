#include "angle.h"

const double _R2D_CONV_ = 180.0/M_PI;
const double _D2R_CONV_ = M_PI/180.0;
const double _M_PI2_ = 2.0*M_PI;

//-------------------------------------------------------------------
// Convert radian angle to degrees
float rad2deg(float x){
  return (x*_R2D_CONV_);
}

//-------------------------------------------------------------------
// Convert degree angle to radians
float deg2rad(float x){
  return (x*_D2R_CONV_);
}
