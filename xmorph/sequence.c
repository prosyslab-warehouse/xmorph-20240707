/* sequence.c: routines to make a sequence of digital image warp frames
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
#include <string.h>
#include <math.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/Scrollbar.h>

#ifdef GIMP
# define NEED_GIMP 1
# include <libgimp/gimp.h>
/* Number of steps in the progress meter. */
int prog_step = 0;
int prog_nsteps = 0;
#endif

#include "diw_map.h"
#include "mjg_dialog.h"
#include "warp.h"
#include "mesh.h"
#include "RgbaImage.h"
#include "image_diw.h"
#include "my_malloc.h"
#include "main.h"

#include "sequence.h"




static char *sequence_file_name = NULL;
static int  sequence_num_frames = -1;



#define SHARPNESS 2.0

static double
sigmoid_func(double x)
{
  register double as;
  as=atan(SHARPNESS);
  return((atan((x-0.5)*SHARPNESS*2.0)+as)/(2.0*as));
}




static double
linear_func(double x)
{
  return(x);
}




void
warp_rgba_image(RgbaImageT *inP, RgbaImageT *outP, double *sxP, double *syP,
double *dxP, double *dyP, int nx, int ny)
{
  warp_image(inP->ri,outP->ri, inP->ncols,inP->nrows, sxP,syP, dxP,dyP, nx,ny);
  warp_image(inP->gi,outP->gi, inP->ncols,inP->nrows, sxP,syP, dxP,dyP, nx,ny);
  warp_image(inP->bi,outP->bi, inP->ncols,inP->nrows, sxP,syP, dxP,dyP, nx,ny);
  warp_image(inP->ai,outP->ai, inP->ncols,inP->nrows, sxP,syP, dxP,dyP, nx,ny);
}




void
warp_sequence(diw_map_t *dmP, int steps, char *basename, int show, int preview)
{
  float x;          /* frame "time" parameter: between 0 and 1 */
  float dissolve_t; /* adjuested "time" parameter for dissolve */
  float warp_t;     /* adjusted "time" parameter for warp */
  MeshT tween_mesh;
  int   frame;
  char  iname[255];
#ifdef NEED_GIMP
  extern int plugin;
#endif

#if 0
  fprintf (stderr, "FIXME: debug %d\n", getpid ());
  kill (getpid (), 19);
#endif

  meshInit(&tween_mesh);
  if(meshAlloc(&tween_mesh, dmP->mesh_src.nx, dmP->mesh_src.ny))
    return;

  if(steps<0)
    steps=0;

#ifdef NEED_GIMP
  if (plugin)
    {
      if (!preview)
        {
          /* Do two warp steps and one save step. */
          gimp_progress_init ("Morphing...");
          prog_nsteps = (steps - 1) * 3;
        }
      else
        /* Just do the warp steps. */
        prog_nsteps = 0;
      prog_step = 0;
    }

  /* For the GIMP, we don't need to output a first frame. */
  for(frame= plugin ? 1 : 0; frame<steps; frame++)
#else
  for(frame=0; frame<steps; frame++)
#endif
  {

    x          = (double)frame / (steps-1);

    dissolve_t = sigmoid_func(x);
    warp_t     = linear_func(x);

    if(show) {
      /* Update the GUI controls for dissolve and warp */

      /* Remember the user-set value */
      float dt = dmP->img_t;
      float wt = dmP->mesh_t;

      dmP->img_t  = dissolve_t;
      dmP->mesh_t = warp_t;
      XawScrollbarSetThumb(dmP->dissolve_sb, dissolve_t, -1.0);
      XawScrollbarSetThumb(dmP->warp_sb, warp_t, -1.0);
      WarpImage(dmP->widget, NULL, NULL, NULL);

      /* Restore the user-set values */
      dmP->img_t  = dt;
      dmP->mesh_t = wt;

    } else {

      meshInterpolate(&tween_mesh, &dmP->mesh_src, &dmP->mesh_dst, warp_t);

      warp_rgba_image(&orig_image[0], &dmP->src_img, dmP->mesh_src.x, dmP->mesh_src.y, tween_mesh.x, tween_mesh.y, tween_mesh.nx, tween_mesh.ny);
      warp_rgba_image(&orig_image[1], &dmP->dst_img, dmP->mesh_dst.x, dmP->mesh_dst.y, tween_mesh.x, tween_mesh.y, tween_mesh.nx, tween_mesh.ny);

    }

    /* Don't save if in preview mode. */
    if (preview)
      continue;

#ifdef NEED_GIMP
    if (plugin)
      {
        extern GDrawable *drawable;
        GDrawable *d;
        gint32 image_ID, layer_ID;

        image_ID = gimp_drawable_image_id (drawable->id);

        /* Create a new layer in the source image. */
        sprintf(iname, "%s%04i.tga", basename, frame);
        layer_ID = gimp_layer_new (image_ID, iname,
                                   dmP->src_img.ncols, dmP->src_img.nrows,
                                   gimp_drawable_type (drawable->id),
                                   100, NORMAL_MODE);
        gimp_image_add_layer (image_ID, layer_ID, 0);

        d = gimp_drawable_get (layer_ID);
        if (rgbaImageGIMP (d, &dmP->src_img, &dmP->dst_img, dissolve_t))
          break;
        gimp_drawable_flush (d);
        gimp_drawable_detach (d);
        if (prog_nsteps > 0)
          {
            /* Make sure the progress meter has been fully updated. */
            prog_step = frame * 3;
            gimp_progress_update ((float) prog_step / (float) prog_nsteps);
          }
      }
    else
#endif /* NEED_GIMP */
    {
      /* GWM - Add the .tga suffix to Targa files. */
      sprintf(iname, "%s%04i.tga", basename, frame);

      rgbaImageWrite(iname, &dmP->src_img, &dmP->dst_img, dissolve_t);
    }
  }

  /* meshFree added -- WA (MJG 13sep95) */
  meshFree(&tween_mesh);

#ifdef NEED_GIMP
  if (plugin)
    {
      if (prog_nsteps > 0)
        {
          gimp_progress_update (1.0);
          prog_step = 0;
          prog_nsteps = 0;
        }

      /* We exit after we save the image data in plugin mode. */
      if (!preview)
        gimp_quit ();
    }
#endif
}




/* =============================================================== */
/*                           Callbacks                             */
/* --------------------------------------------------------------- */
/*ARGSUSED*/
void
set_sequence_file_name_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  dialog_apdx_t *daP = (dialog_apdx_t *)call_data;
  char *fn;

  fn = XawDialogGetValueString(daP->dialog);
  if(fn==NULL)
    return;

  set_sequence_file_name (fn);
}

void
set_sequence_file_name (char *fname)
{
  int len;
  len = strlen (fname);

  if(sequence_file_name)
    FREE(sequence_file_name);

  sequence_file_name = MY_CALLOC (len + 1, char);
  if (sequence_file_name)
    strcpy (sequence_file_name, fname);
}

char *
get_sequence_file_name (void)
{
  return sequence_file_name;
}

/* --------------------------------------------------------------- */

/*ARGSUSED*/
void
set_sequence_num_frames_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  dialog_apdx_t *daP = (dialog_apdx_t *)call_data;
  char *nf;

  nf = XawDialogGetValueString(daP->dialog);
  if(nf==NULL)
    return;

  set_sequence_num_frames (atoi (nf));
}

void
set_sequence_num_frames (int frames)
{
  sequence_num_frames = frames;
}

int
get_sequence_num_frames (void)
{
  return sequence_num_frames;
}

/* --------------------------------------------------------------- */

/*ARGSUSED*/
void
warp_sequence_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  int preview = (int)client_data;
  warp_sequence(&global_diw_map[0], sequence_num_frames, sequence_file_name, True, preview);
}




/* =============================================================== */
/*                           Actions                               */
/* --------------------------------------------------------------- */


/* WarpSequence: action to generate a warp sequence
*/
/*ARGSUSED*/
void
WarpSequence(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms)
{
  diw_map_t *diw_mapP;

  if((diw_mapP = diw_map_of_widget(widget)) == NULL) {
    fprintf(stderr, "WarpSequence: Bad Widget for diw_map\n");
    return;
  }
  warp_sequence(diw_mapP, sequence_num_frames, sequence_file_name, True, FALSE);
}
