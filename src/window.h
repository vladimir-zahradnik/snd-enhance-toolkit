/********************************************************************
 function: Windowing Functions
 contains: hamming, hanning, blackman, bartlett, triangular and boxcar
           windowing functions
 last mod: $Id: lpc.h 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#ifndef HAVE_WINDOW_H
#define HAVE_WINDOW_H

/* calculate window */
/* window types: hamming, hanning, blackman, bartlett, triangular, boxcar */
extern double calc_window (double * data, int datalen, const char * window_name);

#endif
