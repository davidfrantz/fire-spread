#include "focalfuns.h"


//-------------------------------------------------------------------
//  TRACER FUNCTION for CONNECTED COMPONENTS LABELING
void tracer(int *cy, int *cx, int *tracingdirection, bool *MAT, int *CCL, int nrow, int ncol){

  int i, y, x, tval;
  static int SearchDirection[8][2] = {{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1}};


  for (i=0; i<7; i++){

    y = *cy + SearchDirection[*tracingdirection][0];
    x = *cx + SearchDirection[*tracingdirection][1];
    
    if (y>=0 && y<nrow && x>=0 && x<ncol){

      tval = MAT[ncol*y+x];
      
    } else {

      tval = 0;

    }

    if (tval == 0) {

      if (y>=0 && y<nrow && x>=0 && x<ncol) CCL[ncol*y+x] = -1;
      *tracingdirection = (*tracingdirection + 1) % 8;

    } else {

      *cy = y;
      *cx = x;
      break;

    }
  }
}

//-------------------------------------------------------------------
//  CONTOUR TRACING FUNCTION for CONNECTED COMPONENTS LABELING
void contourtracing(int cy, int cx, int labelindex, int tracingdirection, 
                    bool *MAT, int *CCL, int nrow, int ncol){

  char tracingstopflag = 0, SearchAgain = 1;
  int fx, fy, sx = cx, sy = cy;

  tracer(&cy, &cx, &tracingdirection, MAT, CCL, nrow, ncol);

  if (cx != sx || cy != sy){

    fx = cx;
    fy = cy;

    while (SearchAgain){

      tracingdirection = (tracingdirection + 6) % 8;
      CCL[ncol*cy+cx] = labelindex;
      tracer(&cy, &cx, &tracingdirection, MAT, CCL, nrow, ncol);

      if (cx == sx && cy == sy){

        tracingstopflag = 1;

      } else if (tracingstopflag){

        if (cx == fx && cy == fy){
          SearchAgain = 0;
        } else {
          tracingstopflag = 0;
        }

      }
    }
  }
}

//-------------------------------------------------------------------
//  CONNECTED COMPONENTS LABELING
int connectedcomponents(bool *MAT, int *CCL, int nrow, int ncol){

/*
connected components labelling based on 
Chang, F., C.-J. Chen, and C.-J. Lu. 2004. A linear-time component-labeling algorithm using contour tracing technique. Comput. Vis. Image Underst. 93:206-220.
*/

  int i, j, ind;
  int tracingdirection, CCCount = 0, labelindex = 0;

  for (i=0; i<nrow; i++){
    for (j=0,labelindex=0; j<ncol; j++){

      ind = ncol*i+j;


      if (MAT[ind])  {// black pixel

        if (labelindex != 0) {// use pre-pixel label

          CCL[ind] = labelindex;

        } else {

          labelindex = CCL[ind];

          if (labelindex == 0)  {

            labelindex = ++CCCount;
            tracingdirection = 0;
            contourtracing(i, j, labelindex, tracingdirection, MAT, CCL, nrow, ncol);// external contour
            CCL[ind] = labelindex;

          }

        }

      } else if (labelindex != 0){// white pixel & pre-pixel has been labeled

        if (CCL[ind] == 0){

          tracingdirection = 1;
          contourtracing(i, j - 1, labelindex, tracingdirection, MAT, CCL, nrow, ncol);// internal contour

        }

        labelindex = 0;

      }

    }
  }


  //cycle through and replace -1 with 0
  for (i=0; i<nrow; i++){  
    for (j=0,labelindex=0; j<ncol; j++){

      ind = ncol*i+j;
      if (CCL[ind]==-1)  CCL[ind]=0;

    }
  }

  return(CCCount);

}

