/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2002             *
 * by the XIPHOPHORUS Company http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

  function: LPC low level routines
  last mod: $Id: lpc.h 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#ifndef HAVE_LPC_H
#define HAVE_LPC_H

/* simple linear scale LPC code */
extern double lpc_from_data(double *data, double *lpc, int n, int m);

extern void lpc_predict(double *coeff, double *prime, int m,
                        double *data, long n);

#endif
