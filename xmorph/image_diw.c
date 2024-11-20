/* image_diw.c : image routines associated with DIW maps
//
// Written and Copyright (C) 1994-1999 by Michael J. Gourlay
//
// PROVIDED AS IS.  NO WARRANTEES, EXPRESS OR IMPLIED.
//
// These routines are used for the X Window System only and are not
// needed for tkmorph.
*/

#include <stdio.h>
#include <stdlib.h>

#include "my_malloc.h"

#include "diw_map.h"
#include "RgbaImage.h"
#include "main.h"

#include "image_diw.h"




/* reset_images: copy "original" image into image spaces of diw_maps
//
// The name "reset" here is a misnomer.  It really just synchronizes
// the "display" image with the "original" image.
//
// type: determines whether images are the "src" (1) or "dst" (2)
//
// Uses global variables:  orig_image, global_diw_map
*/
void
reset_images(int type)
{
  int indx;
  RgbaImageT *imgP;  /* image stored in "original" space */
  RgbaImageT *dmiP;  /* image stored in DIW map */

  /* Copy the "original" image into the diw_maps */
  for(indx=0; indx<NUM_DIW_MAPS; indx++) {
    if(global_diw_map[indx].width != 0) {
      switch(type) {
        case 1:
          imgP = &orig_image[0];
          dmiP = &global_diw_map[indx].src_img;
          break;

        case 2:
          imgP = &orig_image[1];
          dmiP = &global_diw_map[indx].dst_img;
          break;

        default:
          fprintf(stderr, "reset_images: Bad Value: type %i\n", type);
          return;
          /*NOTREACHED*/
          break;
      }

      rgbaImageFree(dmiP);

      /* Shallow copy the image info from the original image:
      // This is done to copy the image geometry, compression,
      // pixel_size, color_mapped, and type, swiftly.
      */
      *dmiP = *imgP;

      /* The shallow copy also copied pointer info, which isn't correct
      // for the DIW map copy, so erase that.
      */
      dmiP->ri = dmiP->gi = dmiP->bi = dmiP->ai = NULL;

      /* Allocate memory for the DIW map images */
      if(rgbaImageAlloc(dmiP, dmiP->ncols, dmiP->nrows))
        return;

      /* Copy the "original" image into the DIW map image space */
      memcpy(dmiP->ri, imgP->ri, dmiP->ncols * dmiP->nrows);
      memcpy(dmiP->gi, imgP->gi, dmiP->ncols * dmiP->nrows);
      memcpy(dmiP->bi, imgP->bi, dmiP->ncols * dmiP->nrows);
      memcpy(dmiP->ai, imgP->ai, dmiP->ncols * dmiP->nrows);
    }
  }
}




/* --------------------------------------------------------------- */

#define WHITE 0
#define BLACK 1

/* --------------------------------------------------------------- */

/* dither_image: dissolve 2 images and store into ximage
//
// t: determines the percent of dissolve between images.
// -- 0.0 means source image.
// -- 1.0 means dest image.
// -- 0.5 means halfway between, etc.
//
// brite: brightness factor applied to dissolved image, for dimming.
*/
void
dither_image(Visual *visual, RgbaImageT *srcP, RgbaImageT *dstP, double t, double brite, XImage *ximage)
{
  int           xi, yi;         /* image coordinate indices */
  int           re=0;           /* dithering error in red channel */
  int           ge=0;           /* dithering errir in green channel or gray */
  int           be=0;           /* dithering error in blue channel */
  unsigned char byte = 0;       /* 8 bits */
  int          *this_err=NULL;  /* error diffusion array */
  int          *next_err=NULL;  /* error diffusion array */


  /* 24bit variables added WA (MJG 13sep95) */
  int    rShift = 0;
  int    gShift = 0;
  int    bShift = 0;
  int    rRange = 0;
  int    gRange = 0;
  int    bRange = 0;

  /* 24bit color code added WA (MJG 13sep95) */
  /* Based on xpaint sources: palette.c */
  if (visual->class == TrueColor) {
    unsigned long v;
    rShift = gShift = bShift = 0;
    for (v = visual->red_mask; (v & 1) == 0; v >>= 1) rShift++;
    for (rRange=0; v; v>>=1) rRange++;
    for (v = visual->green_mask; (v & 1) == 0; v >>= 1) gShift++;
    for (gRange=0; v; v>>=1) gRange++;
    for (v = visual->blue_mask; (v & 1) == 0; v >>= 1) bShift++;
    for (bRange=0; v; v>>=1) bRange++;
  } else if (visual->class == StaticGray) {
    /* Allocate error diffusion array */
    this_err=MY_CALLOC(srcP->ncols, int);
    next_err=MY_CALLOC(srcP->ncols, int);
  }

  /* Dissolve and dither the image */
  /* ----------------------------- */
  for(yi=0; yi<srcP->nrows; yi++) {
    for(xi=0; xi<srcP->ncols; xi++) {
      int   rsi, gsi, bsi;  /* source image pixel channels */
      int   rdi, gdi, bdi;  /* dest image pixel channels */
      int   ri, gi, bi;     /* dissolved image pixel channels */
      Pixel pixel;          /* ximage pixel value */
                            /* changed from int to Pixel: WA (MJG 13sep95) */

      /* Assign source image pixel channel values */
      /* ---------------------------------------- */
      rsi = (1.0-t) * srcP->ri[yi * srcP->ncols + xi];
      gsi = (1.0-t) * srcP->gi[yi * srcP->ncols + xi];
      bsi = (1.0-t) * srcP->bi[yi * srcP->ncols + xi];

      /* Assign dest image pixel channel values */
      /* -------------------------------------- */
      if((dstP!=NULL) && (xi<dstP->ncols) && (yi < dstP->nrows)) {
        rdi = t * dstP->ri[yi * dstP->ncols + xi];
        gdi = t * dstP->gi[yi * dstP->ncols + xi];
        bdi = t * dstP->bi[yi * dstP->ncols + xi];
      } else {
        rdi = 0;
        gdi = 0;
        bdi = 0;
      }

      /* Dissolve and dim source and dest images */
      /* --------------------------------------- */
      ri = (int)(brite * (rsi + rdi) + 0.5);
      gi = (int)(brite * (gsi + gdi) + 0.5);
      bi = (int)(brite * (bsi + bdi) + 0.5);


      /* ======================================= */
      /* Dither and store RgbaImage into ximage */
      /* ======================================= */

      /* TrueColor code due to WA (MJG 13sep95) */
      if (visual->class == TrueColor) {
        /* True color requires no dithering */
        pixel = (((ri<<rRange)>>8) << rShift)
               |(((gi<<gRange)>>8) << gShift)
               |(((bi<<bRange)>>8) << bShift);
        /* Store TrueColor pixel in image */
        XPutPixel(ximage, xi, yi, pixel);

      } else if (visual->class == PseudoColor) {
        unsigned char rb, gb, bb;    /* dithered image pixel channels */

        /* assume 8-bit PseudoColor */
        if(ximage->bits_per_pixel != 8) {
          fprintf(stderr,
          "dither_image: assuming 8 bit per pixel but ximage has %i\n",
          ximage->bits_per_pixel);
        }

        /* Add dithering error from previous iteration */
        /* ------------------------------------------- */
        ri += re;  re = 0;
        gi += ge;  ge = 0;
        bi += be;  be = 0;


        /* Truncate pixel channel values to fit inside limits */
        /* -------------------------------------------------- */
        if(ri>RGBA_IMAGE_MAXVAL) {
          re += ri - RGBA_IMAGE_MAXVAL;/**/
          ri = RGBA_IMAGE_MAXVAL;
        } else if(ri<0) {
          re -= ri;
          ri = 0;
        }

        if(gi>RGBA_IMAGE_MAXVAL) {
          ge += gi - RGBA_IMAGE_MAXVAL;/**/
          gi = RGBA_IMAGE_MAXVAL;
        } else if(gi<0) {
          ge -= gi;
          gi = 0;
        }

        if(bi>RGBA_IMAGE_MAXVAL) {
          be += bi - RGBA_IMAGE_MAXVAL;/**/
          bi = RGBA_IMAGE_MAXVAL;
        } else if(bi<0) {
          be -= bi;
          bi = 0;
        }

        /* Approximate true color */
        /* ---------------------- */
        /* Pick out the bits that will make up the pixel index */
        /* Use only most significant bits per channel */
        rb = ri & RED_MSB_MASK;
        gb = gi & GRN_MSB_MASK;
        bb = bi & BLU_MSB_MASK;

        /* Construct a pixel index from the pixel color */
        pixel = (rb>>RED_R_SHIFT)|(gb>>GRN_R_SHIFT)|(bb>>BLU_R_SHIFT);

        /* Store dithering error for next iteration */
        re += ri - (diw_xcolors[pixel].red   >> TRUNC_SHIFT);
        ge += gi - (diw_xcolors[pixel].green >> TRUNC_SHIFT);
        be += bi - (diw_xcolors[pixel].blue  >> TRUNC_SHIFT);

        /* Store pixel index in colormapped image */
        /* MJG 19jul94: fixed indexing */
        /* MJG 12jul95: use bytes_per_line instead of width to index */
        ximage->data[yi * ximage->bytes_per_line + xi] =
          diw_xcolors[pixel].pixel;

      } else if (visual->class == GrayScale) {
        fprintf(stderr, "dither_image: GrayScale not implemented\n");

      } else if (visual->class == StaticGray) {
        int           gray;  /* gray value of pixel */
        unsigned char gb;    /* dithered image pixel gray value */
        int           xi_xi; /* ximage x index */
        int           bit;   /* bit index for byte fraction depths */

        /* Assume 1-bit depth */
        if(ximage->bits_per_pixel != 1) {
          fprintf(stderr,
          "dither_image: assuming 1 bit per pixel but ximage has %i\n",
          ximage->bits_per_pixel);
        }

        /* Convert color into gray */
        /* ----------------------- */
        gray = 0.299 * ri + 0.587 * gi + 0.114 * bi;


        /* Add dithering error from previous iteration */
        /* ------------------------------------------- */
        gray += this_err[xi];  this_err[xi] = 0;

        /* Truncate pixel value to fit inside limits */
        /* ----------------------------------------- */
        ge = 0;
        if(gray > RGBA_IMAGE_MAXVAL) {
          ge += gray - RGBA_IMAGE_MAXVAL;
          gray = RGBA_IMAGE_MAXVAL;
        } else if(gray < 0) {
          ge -= gray;
          gray = 0;
        }

        /* Approximate a gray pixel value and compute dithering error */
        /* ---------------------------------------------------------- */
        if(gray > GRAY_THRESHOLD) {
          gb = WHITE;
          ge += gray - RGBA_IMAGE_MAXVAL;
        } else {
          gb = BLACK;
          ge += gray;
        }

        /* Diffuse and store dithering error */
        /* --------------------------------- */
        next_err[xi]     += ( ge * 5 ) / 16;

        if(xi+1 < srcP->ncols) {
          this_err[xi+1] += ( ge * 5 ) / 16;
          next_err[xi+1] += ( ge * 3 ) / 16;
        } else {
          next_err[xi]   += ( ge * 8 ) / 16;
        }

        if(xi-1 >= 0) {
          next_err[xi-1] += ( ge * 3 ) / 16;
        } else {
          next_err[xi]   += ( ge * 3 ) / 16;
        }

        /* Compute bit index and ximage index */
        /* ---------------------------------- */
        xi_xi = xi / 8;
        if(((bit = xi % 8) == 0) && (xi_xi > 0)) {
          /* Store the accumulated 8 bits into the previous ximage byte */
          ximage->data[yi * ximage->bytes_per_line + xi_xi-1] = byte;

          /* Restart the bit accumulator */
          byte = 0;
        }

        /* Store the bit in the right place in the byte */
        /* -------------------------------------------- */
        /* Note that bit 0 goes on the left of the byte
        ** and that bit 7 goes on the right of the byte.
        */
        byte |= (gb << (7-bit));

      } else {
        fprintf(stderr, "dither_image: visual class %i not supported\n",
          visual->class);
      }
    } /* for xi */

    /* Swap error diffusion arrays */
    if(this_err != NULL) {
      int *tmp = this_err;
      this_err = next_err;
      next_err = tmp;
    }
  } /* for yi */

  /* Free error diffusion arrays */
  if(this_err != NULL) FREE(this_err);
  if(next_err != NULL) FREE(next_err);
}
