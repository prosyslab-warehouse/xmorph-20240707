/* image_cb.c : Callbacks for Digital Image Warp image related widgets
//
// Written and Copyright (C) 1994-1999 by Michael J. Gourlay
//
// PROVIDED AS IS.  NO WARRANTEES, EXPRESS OR IMPLIED.
//
// These routines are used for the X Window System only and are not
// needed for tkmorph.
*/

#ifdef GIMP
# define NEED_GIMP 1
# include <libgimp/gimp.h>
extern int plugin;
#endif

#include <stdio.h>

#ifndef apollo
/* Word has it that Apollo Domain/OS SR10.4.1, BSD 4.3 X11 includes
// these, and the redundancy causes compiler errors.  Thanks to PW.
*/
#include <string.h>
#include <memory.h>
#endif

#include "diw_map.h"
#include "RgbaImage.h"
#include "image_diw.h"
#include "mjg_dialog.h"
#include "main.h"

#include "image_cb.h"




/* clear_widget: clear a widget's window
*/
void
clear_widget(Widget widget, int width, int height)
{
  Display *display = XtDisplay(widget);
  GC               gc_clear;
  XGCValues        gc_vals;

  /* Erase new window */
  XtVaGetValues(widget, XtNbackground, &gc_vals.foreground, NULL);
  gc_clear = XCreateGC(display, XtWindow(widget), GCForeground, &gc_vals);
  XFillRectangle(display, XtWindow(widget), gc_clear, 0, 0, width, height);
  XFreeGC(display, gc_clear);
}



/* --------------------------------------------------------------- */

/* Handle some common work between load_image_cb and open_gimp_dest_cb. */
static void
check_image (int dest)
{
  RgbaImageT *imgP, *altP;

  if (dest)
    {
      imgP = &orig_image[1];
      altP = &orig_image[0];
    }
  else
    {
      imgP = &orig_image[0];
      altP = &orig_image[1];
    }

  /* Check to see if geometry matches */
  if(   (imgP->ncols != global_diw_map[0].width)
     || (imgP->nrows != global_diw_map[0].height))
  {
    /* Geometry of the new image does NOT match the previous */
    int indx;

    /* Try to resize the diw_map windows */
    for(indx=0; indx<NUM_DIW_MAPS; indx++) {
      if(global_diw_map[indx].width != 0) {
        {
          /* Resize the core widgets */
          Dimension        width, height;
          XtGeometryResult xmrr;

          width = imgP->ncols;
          height = imgP->nrows;
          xmrr=XtMakeResizeRequest(global_diw_map[indx].widget, width, height,
            &width, &height);
          if(xmrr==XtGeometryAlmost) {
            /* Resize window */
            xmrr=XtMakeResizeRequest(global_diw_map[indx].widget, width, height,
              &width, &height);
            clear_widget(global_diw_map[indx].widget, width, height);
          }
        }

        /* Reallocate the pixmap and ximages */
        allocate_x_images(global_diw_map[indx].widget,
          &global_diw_map[indx].pixmap, &global_diw_map[indx].ximage,
          imgP->ncols, imgP->nrows);

        /* Set new diw_map geometry */
        global_diw_map[indx].width = imgP->ncols;
        global_diw_map[indx].height = imgP->nrows;

        /* Scale the meshes to fit the new image */
        meshScale(&global_diw_map[indx].mesh_src,
          global_diw_map[indx].width, global_diw_map[indx].height);

        meshScale(&global_diw_map[indx].mesh_dst,
          global_diw_map[indx].width, global_diw_map[indx].height);
      }
    }

    /* Free the space of the previous alternate image */
    /* Fill the alternate image with a test image */
    rgbaImageFree(altP);
    altP->ncols = imgP->ncols;
    altP->nrows = imgP->nrows;
    rgbaImageTestCreate (altP, dest ? 2 : 1);
    reset_images(dest ? 1 : 2);
  }

  /* Force the diw_maps to use the new image */
  reset_images(dest ? 2 : 1);

  /* Display the new image */
  ReditherAllImages(NULL, NULL, NULL, NULL);
}



/* load_img_cb: Callback to load image into memory
//
// 1. Load the image into the "original" image space
// 2. Copy the "original" image space into each of the diw_map image spaces
// 3. Redither each diw_map
//
// The term "alternate image" refers to which ever image is not being
// loaded by this call of the routine.
//
// If the image geometry of this new image differs from the previous:
// Fill the remaining image with a test image of appropriate size,
// Reset both meshes.
*/
/*ARGSUSED*/
void
load_img_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  dialog_apdx_t *daP = (dialog_apdx_t *)call_data;
  int           type = (int) client_data;
  String        fn;
  RgbaImageT *imgP;

  /* Fetch the filename from the dialog box text widget */
  fn = XawDialogGetValueString(daP->dialog);

  /* Decide where in memory to put the image: source or destination */
  switch(type) {
    case 1:
      imgP = &orig_image[0];
      break;

    case 2:
      imgP = &orig_image[1];
      break;

    default:
      fprintf(stderr, "load_img_cb: Bad Value: dialog.type %i\n", type);
      return;
      break;
  }

  /* Allocate and read the image */
  if(rgbaImageRead(imgP, fn)) {
    fprintf(stderr, "load_img_cb: Failed to load '%s'\n", fn);
    return;
  }

  check_image ((type == 2) ? 1 : 0);
}


#ifdef NEED_GIMP
/* open_gimp_dest_cb: open a destination layer */
void
open_gimp_dest_cb (Widget w, XtPointer client_data, XtPointer call_data)
{
  extern void set_dest_image_id (gint32 id);
  gint32 drawable_ID = (gint32) client_data;

  if(rgbaImageUnGIMP (&orig_image[1], gimp_drawable_get (drawable_ID))) {
    fprintf(stderr, "open_gimp_dest_cb: Failed to import drawable `%d'\n",
            drawable_ID);
    return;
  }

  set_dest_image_id (drawable_ID);
  check_image (1);
}
#endif /* !NEED_GIMP */


/* --------------------------------------------------------------- */

/* save_img_cb: Callback to save image to file
** Save the image from the diw_map space to a file
*/
/*ARGSUSED*/
void
save_img_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  dialog_apdx_t *daP = (dialog_apdx_t*)call_data;
  diw_map_t     *dmP = (diw_map_t*)client_data;
  String         fn;

  /* Fetch the filename from the dialog box text widget */
  fn = XawDialogGetValueString(daP->dialog);

  if(rgbaImageWrite(fn, &dmP->src_img, &dmP->dst_img, dmP->img_t)) {
    fprintf(stderr, "save_img_cb: Failed to save to '%s'\n", fn);
  }
}
