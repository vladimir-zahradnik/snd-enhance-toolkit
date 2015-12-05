/* --------------------------------------------------------------------
! Program to calculate the first kind modified Bessel function
! of integer order N, for any REAL X, using the function BESSI(N,X).
! ---------------------------------------------------------------------*/

#ifndef HAVE_TBESSI_H
#define HAVE_TBESSI_H

#include "common.h"

/* N is order of Bessel function, X is real number */
extern double BESSI(int N, double X);

#endif