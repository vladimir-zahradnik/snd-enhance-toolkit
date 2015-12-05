/*************************************************************
 * This file implements basic windowing functions            * 
 *************************************************************/

/* n : indicates n-th element of vector,
   wlen : size of a vector */

#include <math.h>
#include "window.h"

/* Declarations of Window Functions used in this module */

/* hamming window */
static double hamming(double n, int wlen);

/* hanning window */
static double hanning(double n, int wlen);

/* blackman window */
static double blackman(double n, int wlen);

/* bartlett window */
static double bartlett(double n, int wlen);

/* triangular window */
static double triang(double n, int wlen);

/* boxcar window */
static double boxcar(double n, int wlen);

/* --------------------------------------- */

/* access to window functions, where wind_type specifies type of window */
double setk_window(double n, int wlen, int wind_type)
{
  double wind_ret;
  
  switch (wind_type) {
    case 0: /* hamming window */
      wind_ret = hamming(n, wlen);
      break;
    case 1: /* hanning window */
      wind_ret = hanning(n, wlen);
      break;
    case 2: /* blackman window */
      wind_ret = blackman(n, wlen);
      break;
    case 3: /* bartlett window */
      wind_ret = bartlett(n, wlen);
      break;
    case 4: /* triangular window */
      wind_ret = triang(n, wlen);
      break;
    case 5: /* boxcar window */
      wind_ret = boxcar(n, wlen);
      break;
    default: /* hamming window is default */
      wind_ret = hamming(n, wlen);
      break;
  }
  return wind_ret;
}

/* returns name of used window function */
char * setk_wind_info(int wind_type)
{
  char * wind_name[] = { "Hamming Window", "Hanning Window", "Blackman Window",
                         "Bartlett Window", "Triangular Window", "Boxcar Window" };

    return wind_name[wind_type];
}

/* hamming window */
static double hamming(double n, int wlen)
{
  return ((n >= 0) && (n <= wlen-1)) ? 0.54-0.46*cos(2*M_PI*n/(wlen-1)) : 0;
}

/* hanning window */
static double hanning(double n, int wlen)
{
  return ((n >= 0) && (n <= wlen-1)) ? 0.5*(1-cos(2*M_PI*(n+1)/(wlen+1))) : 0;
}

/* blackman window */
static double blackman(double n, int wlen)
{
  return ((n >= 0) && (n <= wlen-1)) ? 
    0.42-0.5*cos(2*M_PI*n/(wlen-1))+0.08*cos(4*M_PI*n/(wlen-1)) : 0;
}

/* bartlett window */
static double bartlett(double n, int wlen)
{
  double pom;
  pom = (double) wlen/2;
  if (pom == wlen/2) 
  { /* n is even */
    if ((n >= 0) && (n <= wlen/2-1))  
      return  2*n/(wlen-1);
    else if ((n >= wlen/2) && (n <= wlen-1))
      return  2*(wlen-n-1)/(wlen-1);
    else 
      return 0;
  }
  else 
  { /* n is odd */
    if ((n >= 0) && (n <= (wlen-1)/2))  
      return 2*n/(wlen-1);
    else if ((n > (wlen-1)/2) && (n <= wlen-1))
      return 2-2*n/(wlen-1);
    else 
      return 0;
  }
}

/* triangular window */
static double triang(double n, int wlen)
{
  double pom;
  pom = (double) wlen/2;
  if (pom == wlen/2) 
  { /* n is even */
    if ((n >= 0) && (n <= wlen/2-1))  
      return  (2*n+1)/wlen;
    else if ((n >= wlen/2) && (n <= wlen-1))
      return  (2*(wlen-n)-1)/wlen;
    else 
      return 0;
  }
  else 
  { /* n is odd */
    if ((n >= 0) && (n <= (wlen-1)/2))  
      return 2*(n+1)/(wlen+1);
    else if ((n > (wlen-1)/2) && (n <= wlen-1))
      return 2*(wlen-n)/(wlen+1);
    else 
      return 0;
  }
}

/* boxcar window */
static double boxcar(double n, int wlen)
{
  return ((n >= 0) && (n < wlen)) ? 1 : 0;
}