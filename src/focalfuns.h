#ifndef FOCALFUNS_H
#define FOCALFUNS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>


void tracer(int *cy, int *cx, int *tracingdirection, bool *MAT, int *CCL, int nrow, int ncol);
void contourtracing(int cy, int cx, int labelindex, int tracingdirection, 
                    bool *MAT, int *CCL, int nrow, int ncol);
int connectedcomponents(bool *MAT, int *CCL, int nrow, int ncol);

#ifdef __cplusplus
}
#endif

#endif
