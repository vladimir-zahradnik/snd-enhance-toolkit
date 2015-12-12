/********************************************************************
 function: Windowing Functions
 contains: hamming, hanning, blackman, bartlett, triangular and boxcar
           windowing functions
 last mod: $Id: lpc.h 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#ifndef HAVE_WINDOW_H
#define HAVE_WINDOW_H

#include "common.h"

typedef double (*window_func_t)(double *data, int datalen);

/* parse window type */
extern window_func_t parse_window_type(const char *name, bool verbose);

/* get window name */
char *get_window_name(const char *name);

/* apply_window */
extern double apply_window(double *data, int datalen, window_func_t calc_window);

extern double calc_hamming_window(double *data, int datalen);

extern double calc_hann_window(double *data, int datalen);

extern double calc_blackman_window(double *data, int datalen);

extern double calc_bartlett_window(double *data, int datalen);

extern double calc_triangular_window(double *data, int datalen);

extern double calc_rectangular_window(double *data, int datalen);

extern double calc_nuttall_window(double *data, int datalen);

#endif
