#include "date.h"


void compact_date(int y, int m, int d, char formatted[], size_t size){
int nchar;


  nchar = snprintf(formatted, size, "%04d%02d%02d", y, m, d);
  if (nchar < 0 || nchar >= size){ 
    printf("Buffer Overflow in assembling compact date %04d%02d%02d\n", 
      y, m, d); exit(1);}

  return;
}


int doy2m(int doy){

  if (doy > 334){
    return 12;
  } else if (doy>304){
    return 11;
  } else if (doy>273){
    return 10;
  } else if (doy>243){
    return 9;
  } else if (doy>212){
    return 8;
  } else if (doy>181){
    return 7;
  } else if (doy>151){
    return 6;
  } else if (doy>120){
    return 5;
  } else if (doy>90){
    return 4;
  } else if (doy>59){
    return 3;
  } else if (doy>31){
    return 2;
  } else {
    return 1;
  }

}


int doy2d(int doy){

  if (doy>334){
    return doy-334;
  } else if (doy>304){
    return doy-304;
  } else if (doy>273){
    return doy-273;
  } else if (doy>243){
    return doy-243;
  } else if (doy>212){
    return doy-212;
  } else if (doy>181){
    return doy-181;
  } else if (doy>151){
    return doy-151;
  } else if (doy>120){
    return doy-120;
  } else if (doy>90){
    return doy-90;
  } else if (doy>59){
    return doy-59;
  } else if (doy>31){
    return doy-31;
  } else {
    return doy;
  }

}


int md2doy(int m, int d)
{
  int doy;

  switch(m)
  {
  case 1:
    doy = d;
    break;
  case 2:
    doy = 31+d;
    break;
  case 3:
    doy = 59+d;
    break;
  case 4:
    doy = 90+d;
    break;
  case 5:
    doy = 120+d;
    break;
  case 6:
    doy = 151+d;
    break;
  case 7:
    doy = 181+d;
    break;
  case 8:
    doy = 212+d;
    break;
  case 9:
    doy = 243+d;
    break;
  case 10:
    doy = 273+d;
    break;
  case 11:
    doy = 304+d;
    break;
  case 12:
    doy = 334+d;
    break;
  }
  return doy;
}
