/* image_diw.c : image routines associated with DIW maps
//
// Written and Copyright (C) 1994-1999 by Michael J. Gourlay
//
// Provided as is.  No warrantees, express or implied.
*/

#ifndef _IMAGE_DIW_H__INCLUDED_
#define _IMAGE_DIW_H__INCLUDED_


#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include "RgbaImage.h"



/* X_BITS_PER_CHANNEL is the number of bit per channel X Window System
** uses to represent a color.
*/
#define X_BITS_PER_CHANNEL 16


/* STATIC_GRAY_MAXVAL is the maximum value I assume that a visual with
** StaticGray visual class uses to represent a pixel.  This is in general
** not a compile-time constant.  It should be a run-time value, set to
** something like ((1 << visual->depth) - 1).  Essentially, I assume a
** depth of 1 bit.
*/
#define STATIC_GRAY_MAXVAL 1


/* GRAY_THRESHOLD determines when I set a 1-bit monochrime pixel to
** "on" when the gray value exceeds this threshold.
*/
#define GRAY_THRESHOLD     RGBA_IMAGE_MAXVAL/2


/* TRUNC_SHIFT is the number of bits to shift right when converting
** between the X Window System color representation and the
** representation used by the RgbaImage type.
*/
#define TRUNC_SHIFT (X_BITS_PER_CHANNEL - BITS_PER_CHANNEL)


/* ???_MSB_MASK is used to pick out the appropriate bits from a
** channel of an RgbaImage in order to convert it into an image which is
** colormapped using my peculiar dithering colormap system (DCI).
*/
#define RED_MSB_MASK (RED_MAXVAL << (BITS_PER_CHANNEL - RED_NUM_BITS))
#define GRN_MSB_MASK (GRN_MAXVAL << (BITS_PER_CHANNEL - GRN_NUM_BITS))
#define BLU_MSB_MASK (BLU_MAXVAL << (BITS_PER_CHANNEL - BLU_NUM_BITS))


/* Rightward bit shifts for converting channel bits into a pixel index
*/
#define BLU_R_SHIFT (BITS_PER_CHANNEL - BLU_NUM_BITS)
#define GRN_R_SHIFT (BITS_PER_CHANNEL - BLU_NUM_BITS - GRN_NUM_BITS)
#define RED_R_SHIFT (BITS_PER_CHANNEL - BLU_NUM_BITS - GRN_NUM_BITS - RED_NUM_BITS)




extern void dither_image(Visual *visual, RgbaImageT *srcP, RgbaImageT *dstP, double t, double brite, XImage *ximage);


extern void reset_images(int type);




#endif
