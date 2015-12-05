/********************************************************************
 function: Windowing Functions
 contains: hamming, hanning, blackman, bartlett, triangular and boxcar
           windowing functions
 last mod: $Id: lpc.h 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#ifndef _WINDOW_H_
#define _WINDOW_H_

/* access to window functions, where wind_type specifies type of window */
extern double setk_window(double n, int wlen, int wind_type);

/* returns name of used window function */
extern char * setk_wind_info(int wind_type);

#endif
