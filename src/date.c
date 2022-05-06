#include "date.h"


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
