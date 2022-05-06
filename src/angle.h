#ifndef ANGLE_H
#define ANGLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "math.h"

extern const double _R2D_CONV_;
extern const double _D2R_CONV_;
extern const double _M_PI2_;

float rad2deg(float x);
float deg2rad(float x);

#ifdef __cplusplus
}
#endif

#endif
