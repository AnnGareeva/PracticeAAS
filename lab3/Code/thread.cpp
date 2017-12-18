#include "thread.h"
#include <iostream>
#include <math.h>

double LenPoints(double aX1, double aX2, double aY1, double aY2) {
  return sqrt((aX1 - aX2) * (aX1 - aX2) + (aY1 - aY2) * (aY1 - aY2));
}

double LenThread(int aCount, double aRad, double (*pCoord)[2]) {
  if(aCount <= 0) {
    return -2;
  }
  const double PI = 3.14159;
  double sum = 0;
  //
  for(int i = 0; i < (aCount - 1); ++i)
    sum += LenPoints(pCoord[i][0], pCoord[i + 1][0], pCoord[i][1], pCoord[i + 1][1]);
  //
  sum += LenPoints(pCoord[0][0], pCoord[aCount - 1][0], pCoord[0][1], pCoord[aCount - 1][1]) + 2 * PI * aRad;
  sum = trunc(sum * 100.0) / 100.0;
  return sum;
}
