/*************************************************************
 * This file implements basic windowing functions            * 
 *************************************************************/

#include <stdio.h>
#include "window.h"
#include <math.h>
#include "i18n.h"

/* Declarations of Window Functions used in this module */

/* hamming window */
static double hamming(double * data, int datalen);

/* hanning window */
static double hanning(double * data, int datalen);

/* blackman window */
static double blackman(double * data, int datalen);

/* bartlett window */
static double bartlett(double * data, int datalen);

/* triangular window */
static double triang(double * data, int datalen);

/* boxcar window */
static double boxcar(double * data, int datalen);

/* --------------------------------------- */

/* calculate window */
double calc_window (double * data, int datalen, const char * window_name)
{
  if (window_name == NULL) {
    puts(_("\nNo window type was specified. Using default.\n"));
    return (hamming (data, datalen));
  }
  if (strcmp (window_name, "hamming") == 0)
    return ( hamming (data, datalen) );
  if (strcmp (window_name, "hanning") == 0)
    return ( hanning (data, datalen) );
  if (strcmp (window_name, "blackman") == 0)
    return ( blackman (data, datalen) );
  if (strcmp (window_name, "bartlett") == 0)
    return ( bartlett (data, datalen) );
  if (strcmp (window_name, "triangular") == 0)
    return ( triang (data, datalen) );
  if (strcmp (window_name, "boxcar") == 0)
    return ( boxcar (data, datalen) );
  
  puts(_("\nError: Bad window type. Using default.\n\n"));
  return (hamming (data, datalen));
}

/* hamming window */
static double hamming(double * data, int datalen)
{
  int n;
  double winGain = 0.0;
  
  for (n = 0; n < datalen; n++) {
    data [n] = ((n >= 0) && (n <= datalen-1)) ? 0.54-0.46 * cos (2 * M_PI * n / (datalen-1)) : 0;
    winGain += data [n];
  }
  return winGain;  
}

/* hanning window */
static double hanning(double * data, int datalen)
{
  int n;
  double winGain = 0.0;
  
  for (n = 0; n < datalen; n++) {
    data [n] = ((n >= 0) && (n <= datalen-1)) ? 0.5 * (1 - cos (2 * M_PI * (n+1) / (datalen+1))) : 0;
    winGain += data [n];
  }
  return winGain;
}

/* blackman window */
static double blackman(double * data, int datalen)
{
  int n;
  double winGain = 0.0;
  
  for (n = 0; n < datalen; n++) {
     data [n] = ((n >= 0) && (n <= datalen-1)) ? 
          0.42-0.5 * cos (2 * M_PI * n / (datalen-1)) + 0.08 * cos (4 * M_PI * n / (datalen-1)) : 0;
     winGain += data [n];
  }
  return winGain;
}

/* bartlett window */
static double bartlett(double * data, int datalen)
{
  int n;
  double winGain = 0.0;
  
  for (n = 0; n < datalen; n++) {
   if ((datalen % 2) == 0)
    { /* n is even */
      if ((n >= 0) && (n <= datalen/2-1))  
        data [n] =  2.0*n/(datalen-1);
      else if ((n >= datalen/2) && (n <= datalen-1))
        data [n] =  2.0*(datalen-n-1)/(datalen-1);
      else 
        data [n] = 0.0;
    }
    else 
    { /* n is odd */
      if ((n >= 0) && (n <= (datalen-1)/2))  
        data [n] = 2.0*n/(datalen-1);
      else if ((n > (datalen-1)/2) && (n <= datalen-1))
        data [n] = 2-2.0*n/(datalen-1);
      else 
        data [n] = 0.0;
    }
    winGain += data [n];
  }
  return winGain;
}

/* triangular window */
static double triang(double * data, int datalen)
{
  int n;
  double winGain = 0.0;
  
  for (n = 0; n < datalen; n++) {
    if ((datalen % 2) == 0)
    { /* n is even */
      if ((n >= 0) && (n <= datalen/2-1))  
        data [n] = (2.0*n+1)/datalen;
      else if ((n >= datalen/2) && (n <= datalen-1))
        data [n] = (2.0*(datalen-n)-1)/datalen;
      else 
        data [n] = 0.0;
    }
    else 
    { /* n is odd */
      if ((n >= 0) && (n <= (datalen-1)/2))  
        data [n] = 2.0*(n+1)/(datalen+1);
      else if ((n > (datalen-1)/2) && (n <= datalen-1))
        data [n] = 2.0*(datalen-n)/(datalen+1);
      else 
        data [n] = 0.0;
    }
    winGain += data [n];
  }
  return winGain;
}

/* boxcar window */
static double boxcar(double * data, int datalen)
{
  int n;
  double winGain = 0.0;
  
  for (n = 0; n < datalen; n++) {
    data [n] = ((n >= 0) && (n < datalen)) ? 1 : 0;
    winGain += data [n];
  }
  return winGain;
}
