/* diw_map.c: Digital Image Warping X graphical user interface
//
// Written and Copyright (C) 1994-1999 by Michael J. Gourlay
//
// PROVIDED AS IS.  NO WARRANTEES, EXPRESS OR IMPLIED.
//
// These routines are used for the X Window System only and are not
// needed for tkmorph.
//
// A "diw_map" is treated in this module as a quasi-widget.  In lieu of
// making the diw_map a proper widget, I have created a structure which
// contains a widget which is used in reverse to look up the diw_map in a
// global list of diw_maps.  This way, callbacks and actions can use a
// diw_map almost as if it were a widget.
//
// diw_maps contain information about a pair of images which are to be
// warped and dissolved into each other.  This entails keeping track of
// two images, two meshed which define relative topologies, various
// variables about to what extent each image should be warped or
// dissolved, and which mesh liens and points should be drawn or selected
// for arrangement.  diw_maps in different windows can be controlled
// completely independantly, but for practical application it is far more
// useful to have diw_maps share images and meshes.  Everything else can
// be handled independantly (such as which meshes to draw or which image
// to display).
*/

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/Form.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Viewport.h>

#include <X11/bitmaps/gray>

#include "my_malloc.h"

#include "select.xbm"
#include "unselect.xbm"

#include "spline.h"
#include "spl-array.h"
#include "RgbaImage.h"
#include "image_cb.h"
#include "image_diw.h"
#include "mesh.h"
#include "mjg_dialog.h"
#include "main.h"

#include "diw_map.h"


#ifdef GIMP
# define NEED_GIMP 1
extern int plugin;
#endif



#define SGN(x)   (((x)>0) ? (1) : ( ((x)<0) ? (-1) : (0)  ))
#define MAX(x,y) ((x)>(y) ? (x) : (y))




/* Meshpoint picking and drawing parameters */
#define MP_SIZE 6
#define MP_ARC  (360*64)




/* --------------------------------------------------------------- */

/* NAME
//   draw_mesh: Draw mesh lines and points at interpolation parameter t
//
//
// GLOBAL VARIABLES
//   mpl_gc: graphics context for mesh lines
//
//   mp_gc: graphics context for (unselected) mesh points
//
//   mps_gc: graphics context for (selected) mesh points
//
//
//
// DESCRIPTION
//   Draw mesh lines and points.
//   Also draw the selected meshpoint.
//
//   The colors for the graphics contexts is to be set by the caller
//   just prior to calling draw_mesh.
//
//
// SEE ALSO
//   DrawMeshes
*/
static int
draw_mesh(Widget widget, diw_map_t *dmP)
{
  extern GC mpl_gc;  /* mesh line graphics context */
  extern GC mp_gc;   /* mesh point graphics context */
  extern GC mps_gc;  /* (selected) mesh point graphics context */

  Display *display = XtDisplay(widget);
  Window   window  = XtWindow(widget);
  XPoint  *xpoints;
  XArc    *xarcs;

  int xi, yi;
  const double *xsP = dmP->mesh_src.x;
  const double *ysP = dmP->mesh_src.y;
  const double *xdP = dmP->mesh_dst.x;
  const double *ydP = dmP->mesh_dst.y;
  const double  mpt = dmP->mesh_t;
  double *x_tmp;
  double *y_tmp;
  double *xrow;
  double *yrow;

  if(!(x_tmp=MY_CALLOC(MAX(dmP->mesh_src.nx,dmP->mesh_src.ny), double))) {
    return(1);
  }

  if(!(y_tmp=MY_CALLOC(MAX(dmP->mesh_src.nx,dmP->mesh_src.ny), double))) {
    return(1);
  }

  if(!(xrow=MY_CALLOC(MAX(dmP->width/4,dmP->height/4), double)))
  {
    return(1);
  }

  if(!(yrow=MY_CALLOC(MAX(dmP->width/4,dmP->height/4), double)))
  {
    return(1);
  }

  if(!(xpoints=MY_CALLOC(MAX(dmP->width/4,dmP->height/4), XPoint)))
  {
    return(1);
  }

  if(!(xarcs = MY_CALLOC(dmP->mesh_src.ny, XArc))) {
    return(1);
  }

  /* Draw meshpoints and vertical meshlines */
  for(xi=0; xi < dmP->mesh_src.nx; xi++) {

    /* Draw vertical mesh lines */
    if(dmP->meshlines_t_draw) {

      /* Create 1D array of mesh points knots */
      for(yi=0; yi<dmP->mesh_src.ny; yi++) {
        x_tmp[yi] = (1.0-mpt)* xsP[yi*dmP->mesh_src.nx + xi]
                       + mpt * xdP[yi*dmP->mesh_src.nx + xi];

        y_tmp[yi] = (1.0-mpt)* ysP[yi*dmP->mesh_src.nx + xi]
                       + mpt * ydP[yi*dmP->mesh_src.nx + xi];

      }

      /* Create abcissas of interpolant to make a smoothly drawn mesh line */
      for(yi=0; yi<(dmP->height/4); yi++)
        yrow[yi] = yi*4;

      /* Make sure last abcissa is set */
      yrow[yi-1] = dmP->height-1;

      /* Create the interpolated line segments */
      hermite3_array(y_tmp, x_tmp, dmP->mesh_src.ny, yrow, xrow, dmP->height/4);

      /* Create an array of XPoints to draw smoothly interpolated mesh line */
      for(yi=0; yi<(dmP->height/4); yi++) {
        xpoints[yi].x = (int) xrow[yi];
        xpoints[yi].y = (int) yrow[yi];
      }

      /* Draw the smoothly interpolated mesh line */
      XDrawLines(display, window, mpl_gc, xpoints, dmP->height/4,
                 CoordModeOrigin);
    }

    /* Draw mesh points */
    if(dmP->meshpoints_t_draw) {
      for(yi=0; yi<dmP->mesh_src.ny; yi++) {
        if(dmP->meshpoints_t_draw) {
          xarcs[yi].width = MP_SIZE;
          xarcs[yi].height = MP_SIZE;

          xarcs[yi].x = (int) ((1.0-mpt)*xsP[yi*dmP->mesh_src.nx + xi]
            + mpt*xdP[yi*dmP->mesh_src.nx + xi]) - xarcs[yi].width/2;
          xarcs[yi].y = (int) ((1.0-mpt)*ysP[yi*dmP->mesh_src.nx + xi]
            + mpt*ydP[yi*dmP->mesh_src.nx + xi]) - xarcs[yi].height/2;

          xarcs[yi].angle1=0;
          xarcs[yi].angle2=MP_ARC;
        }
      }
      XDrawArcs(display, window, mp_gc, xarcs, dmP->mesh_src.ny);
    }
  }

  FREE(xarcs);

  /* Draw horizontal meshlines */
  if(dmP->meshlines_t_draw) {
    for(yi=0; yi<dmP->mesh_src.ny; yi++) {
      for(xi=0; xi<dmP->mesh_src.nx; xi++) {
        x_tmp[xi] = (1.0-mpt)* xsP[yi*dmP->mesh_src.nx + xi]
                       + mpt * xdP[yi*dmP->mesh_src.nx + xi];

        y_tmp[xi] = (1.0-mpt)* ysP[yi*dmP->mesh_src.nx + xi]
                       + mpt * ydP[yi*dmP->mesh_src.nx + xi];
      }
      for(xi=0; xi<(dmP->width/4); xi++) xrow[xi] = xi*4;
      xrow[xi-1] = dmP->width-1;

      hermite3_array(x_tmp, y_tmp, dmP->mesh_src.nx, xrow, yrow, dmP->width/4);

      for(xi=0; xi<(dmP->width/4); xi++) {
        xpoints[xi].x = (int) xrow[xi];
        xpoints[xi].y = (int) yrow[xi];
      }
      XDrawLines(display, window, mpl_gc, xpoints, dmP->width/4,
                 CoordModeOrigin);

    }
  }

  FREE(x_tmp);
  FREE(y_tmp);
  FREE(xrow);
  FREE(yrow);
  FREE(xpoints);

  /* Draw the selected meshpoint filled in */
  if(dmP->meshpoint_selected) {
    short mp_x, mp_y;
    int   mp_i = dmP->meshpoint_selected_loc[0];
    int   mp_j = dmP->meshpoint_selected_loc[1];

    if(mpt<0.5) {
      mp_x = (short)xsP[mp_j*dmP->mesh_src.nx + mp_i] - MP_SIZE/2 - 1;
      mp_y = (short)ysP[mp_j*dmP->mesh_src.nx + mp_i] - MP_SIZE/2 - 1;
    } else {
      mp_x = (short)xdP[mp_j*dmP->mesh_src.nx + mp_i] - MP_SIZE/2 - 1;
      mp_y = (short)ydP[mp_j*dmP->mesh_src.nx + mp_i] - MP_SIZE/2 - 1;
    }

    /* Draw the selected MP disc */
    XFillArc(display, window, mps_gc, mp_x, mp_y, MP_SIZE+2, MP_SIZE+2,
             0, MP_ARC);
  }
  return(0);
}




/* --------------------------------------------------------------- */

/* diw_map_of_widget: map a widget to its diw_map and synchronize diw_maps
//
// Note that synchronization is a side effect.
// Synchronization is needed because there can be any number of DIW
// maps, but they should all share identical meshes.
//
// This routine is sometimes called for its side effect only, for
// example, when called with a NULL argument.  What a hack.
*/
diw_map_t *
diw_map_of_widget(Widget widget)
{
  register int indx;
  diw_map_t *dmP=NULL;

  for(indx=0; indx<NUM_DIW_MAPS; indx++) {
    if(indx>0 && global_diw_map[indx].width!=0) {
      /* Keep the diw_maps synchronized */
      global_diw_map[indx].width = global_diw_map[0].width;
      global_diw_map[indx].height = global_diw_map[0].height;

      global_diw_map[indx].mesh_src.x = global_diw_map[0].mesh_src.x;
      global_diw_map[indx].mesh_src.y = global_diw_map[0].mesh_src.y;
      global_diw_map[indx].mesh_dst.x = global_diw_map[0].mesh_dst.x;
      global_diw_map[indx].mesh_dst.y = global_diw_map[0].mesh_dst.y;
      global_diw_map[indx].mesh_src.nx = global_diw_map[0].mesh_src.nx;
      global_diw_map[indx].mesh_src.ny = global_diw_map[0].mesh_src.ny;
      global_diw_map[indx].mesh_dst.nx = global_diw_map[0].mesh_dst.nx;
      global_diw_map[indx].mesh_dst.ny = global_diw_map[0].mesh_dst.ny;

      global_diw_map[indx].meshpoint_selected_loc[0] =
        global_diw_map[0].meshpoint_selected_loc[0];
      global_diw_map[indx].meshpoint_selected_loc[1] =
        global_diw_map[0].meshpoint_selected_loc[1];
      global_diw_map[indx].meshpoint_selected =
        global_diw_map[0].meshpoint_selected;
    }
    /* Test for match of widget */
    if(global_diw_map[indx].widget == widget) {
      dmP = &global_diw_map[indx];
    }
  }
  return(dmP);
}




/* =============================================================== */
/*                           Actions                               */
/* --------------------------------------------------------------- */

/* NAME
//   DrawMeshes: action to draw src, dst, and tween meshes of a widget
//
//
// SEE ALSO
//   draw_mesh
*/
/*ARGSUSED*/
void
DrawMeshes(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms)
{
  extern GC mpl_gc;  /* mesh line graphics context */
  extern GC mp_gc;   /* mesh point graphics context */

  Display   *display = XtDisplay(widget);
  diw_map_t *diw_mapP;

  if((diw_mapP = diw_map_of_widget(widget)) == NULL) {
    fprintf(stderr, "DrawMeshes: Bad Widget for diw_map\n");
    return;
  }

  /* Draw source mesh */
  if(diw_mapP->meshlines_src_draw || diw_mapP->meshpoints_src_draw) {
    diw_map_t tmp_diw_map;

    XSetForeground(display, mpl_gc, diw_xcolors[XPI_GREEN].pixel);
    XSetForeground(display, mp_gc,  diw_xcolors[XPI_GREEN].pixel);

    tmp_diw_map                    = *diw_mapP;
    tmp_diw_map.meshpoint_selected = False;
    tmp_diw_map.meshlines_t_draw   = diw_mapP->meshlines_src_draw;
    tmp_diw_map.meshpoints_t_draw  = diw_mapP->meshpoints_src_draw;
    tmp_diw_map.mesh_t             = 0.0;
    draw_mesh(widget, &tmp_diw_map);
  }

  /* Draw destination mesh */
  if(diw_mapP->meshlines_dst_draw || diw_mapP->meshpoints_dst_draw) {
    diw_map_t tmp_diw_map;

#ifdef RED_GREEN_COLOR_BLIND
    XSetForeground(display, mpl_gc, diw_xcolors[XPI_BLUE].pixel);
    XSetForeground(display, mp_gc,  diw_xcolors[XPI_BLUE].pixel);
#else
    XSetForeground(display, mpl_gc, diw_xcolors[XPI_RED].pixel);
    XSetForeground(display, mp_gc,  diw_xcolors[XPI_RED].pixel);
#endif

    tmp_diw_map                    = *diw_mapP;
    tmp_diw_map.meshpoint_selected = False;
    tmp_diw_map.meshlines_t_draw   = diw_mapP->meshlines_dst_draw;
    tmp_diw_map.meshpoints_t_draw  = diw_mapP->meshpoints_dst_draw;
    tmp_diw_map.mesh_t             = 1.0;
    draw_mesh(widget, &tmp_diw_map);
  }

  /* Draw tween mesh */
  if(diw_mapP->meshlines_t_draw || diw_mapP->meshpoints_t_draw) {

    XSetForeground(display, mpl_gc, diw_xcolors[XPI_YELLOW].pixel);
    XSetForeground(display, mp_gc,  diw_xcolors[XPI_YELLOW].pixel);

    draw_mesh(widget, diw_mapP);
  }
}




/* --------------------------------------------------------------- */

/* DrawAllMeshes: action to draw all meshes of all diw_maps
*/
/*ARGSUSED*/
void
DrawAllMeshes(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms)
{
  int indx;

  for(indx=0; indx<NUM_DIW_MAPS; indx++) {
    if(global_diw_map[indx].width!=0) {
      DrawMeshes(global_diw_map[indx].widget, evt, prms, n_prms);
    }
  }
}




/* --------------------------------------------------------------- */

/* RefreshImage: action to draw image in diw_map image panel
// Does not redither.
// Sets cursor to "select" cursor.
*/
/*ARGSUSED*/
void
RefreshImage(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms)
{
  extern Cursor diw_wait_cursor;
  extern Cursor diw_select_cursor;
  extern GC     x_gc_clear;

  Window     window  = XtWindow(widget);
  Display   *display = XtDisplay(widget);
  diw_map_t *diw_mapP;

  if((diw_mapP = diw_map_of_widget(widget)) == NULL) {
    fprintf(stderr, "RefreshImage: Bad Widget for diw_map\n");
    return;
  }

  /* Use the "wait" cursor */
  XDefineCursor(display, window, diw_wait_cursor);
  /* Color the cursor white to indicate dithering */
  XRecolorCursor(display, diw_wait_cursor, DXC_WHITE, DXC_BLACK);

  /* Tell the X server to display the image */
  XCopyArea(/* display */display,
            /* src drawable */diw_mapP->pixmap,
            /* dest drawable */window,
            /* GC */ x_gc_clear,
            /* src x,y */ 0, 0,
            /* size */ diw_mapP->width, diw_mapP->height,
            /* dest x,y */ 0, 0);

  /* Use the "select" cursor */
  XDefineCursor(display, window, diw_select_cursor);
}




/* --------------------------------------------------------------- */

/* FakeExpose: action to fake an expose event to a diw_map widget
*/
/*ARGSUSED*/
void
FakeExpose(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms)
{
  Boolean propagate;
  long event_mask;
  XEvent xevent;
  diw_map_t *dmP;

  if((dmP = diw_map_of_widget(widget)) == NULL) {
    fprintf(stderr, "FakeExpose: Bad Widget for diw_map\n");
    abort();  /* dump core */
  }

  propagate = False;
  event_mask = ExposureMask;
  xevent.type=Expose;
  xevent.xexpose.type=Expose;
  xevent.xexpose.display=XtDisplay(widget);
  xevent.xexpose.window=XtWindow(widget);
  xevent.xexpose.x=0;
  xevent.xexpose.y=0;
  xevent.xexpose.width=dmP->width;
  xevent.xexpose.height=dmP->height;
  xevent.xexpose.count=0;
  if(!XSendEvent(xevent.xexpose.display, xevent.xexpose.window,
    propagate, event_mask, &xevent))
  {
    fprintf(stderr, "FakeExpose: XSendEvent could not convert\n");
  }
}




/* --------------------------------------------------------------- */

/* FakeAllExpose: action to fake an expose event to all diw_maps
*/
/*ARGSUSED*/
void
FakeAllExpose(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms)
{
  register int indx;

  for(indx=0; indx<NUM_DIW_MAPS; indx++) {
    if(global_diw_map[indx].width!=0) {
      FakeExpose(global_diw_map[indx].widget, evt, prms, n_prms);
    }
  }
}




/* --------------------------------------------------------------- */

/* ReditherImage: action to redither diw_map images into an image panel
*/
void
ReditherImage(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms)
{
  extern Cursor  diw_wait_cursor;
  extern Cursor  diw_select_cursor;
  extern Visual *x_visual;
  extern GC      x_gc_clear;

  Display   *display = XtDisplay(widget);
  Window     window  = XtWindow(widget);
  diw_map_t *diw_mapP;

  if((diw_mapP = diw_map_of_widget(widget)) == NULL) {
    fprintf(stderr, "ReditherImage: Bad Widget for diw_map\n");
    abort();
  }

  /* Use the "wait" cursor */
  XDefineCursor(display, window, diw_wait_cursor);

  /* Color the cursor -- white -- to indicate dithering */
  XRecolorCursor(display, diw_wait_cursor, DXC_WHITE, DXC_BLACK);

  /* Dither the 24-bit image */
  /* x_visual MJG 14sep95 */

  dither_image(x_visual, &diw_mapP->src_img, &diw_mapP->dst_img,
    diw_mapP->img_t, diw_mapP->img_brightness, diw_mapP->ximage);

  /* Send the image to the X server */
  XPutImage(/* display */display,
            /* drawable */diw_mapP->pixmap,
            /* GC */x_gc_clear,
            /* ximage */diw_mapP->ximage,
            /* src x,y */ 0, 0,
            /* dest x,y */ 0, 0,
            /* size */ diw_mapP->width, diw_mapP->height);

  /* Use the "select" cursor */
  XDefineCursor(display, window, diw_select_cursor);

  /* Refresh the window */
  RefreshImage(widget, evt, prms, n_prms);
  DrawMeshes(widget, evt, prms, n_prms);
}




/* --------------------------------------------------------------- */

/* ReditherAllImages: action to redither all diw_map images into image panel
*/
/*ARGSUSED*/
void
ReditherAllImages(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms)
{
  register int indx;

  for(indx=0; indx<NUM_DIW_MAPS; indx++) {
    if(global_diw_map[indx].width != 0) {
      ReditherImage(global_diw_map[indx].widget, NULL, NULL, NULL);
    }
  }
}




/* WarpImage: action to apply digital image warp algorithm to images
//
// Warps "original" images.
// Displays warped images.
//
// While images are being warped, cursor changes to indicate which
// image is being warped.
*/
void
WarpImage(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms)
{
  extern Cursor diw_wait_cursor;

  Window    win   = XtWindow(widget);
  Display   *disp = XtDisplay(widget);
  diw_map_t *dmP;

  if((dmP = diw_map_of_widget(widget)) == NULL) {
    fprintf(stderr, "WarpImage: Bad Widget for diw_map\n");
    return;
  }

  /* Use the "wait" cursor */
  XDefineCursor(disp, win, diw_wait_cursor);
  XRecolorCursor(disp, diw_wait_cursor, DXC_BLACK, DXC_WHITE);

  /* Warp the src image forward */
  rgbaImageWarp(&orig_image[0], &dmP->src_img, &dmP->mesh_src,
                &dmP->mesh_dst, dmP->mesh_t);

  XRecolorCursor(disp, diw_wait_cursor, DXC_WHITE, DXC_BLACK);

  /* Warp the dst image backward */
  rgbaImageWarp(&orig_image[1], &dmP->dst_img, &dmP->mesh_dst,
                &dmP->mesh_src, 1.0 - dmP->mesh_t);

  /* --- Display the warped images, dissolved */
  ReditherImage(widget, evt, prms, n_prms);
}





/* NAME
//   ChangeMeshLine: action to add or delete a mesh line
//
// PRMS
//   prms[0] must be either "add" or "delete"
//   prms[1] must be either "vertical" or "horizontal"
//
// Sets the number of rows or columns in the global_diw_map[0].mesh_src
// and global_diw_map[0].mesh_dst mesh.  Frees old mesh arrays.  Sets the
// mesh array pointers to newly allocated array memory.
*/
/*ARGSUSED*/
void
ChangeMeshLine(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms)
{
  diw_map_t *diw_mapP;
  char      action = *prms[0];
  char      line_type = *prms[1];

  /* This action requires two arguments */
  if(*n_prms < 2) return;

  if((diw_mapP = diw_map_of_widget(widget)) == NULL) {
    fprintf(stderr, "ChangeMeshLine: Bad Widget for diw_map\n");
    return;
  }

  {
    Display *display = XtDisplay(widget);
    Window window = XtWindow(widget);
    Window root, child;
    int root_x, root_y;
    int win_x, win_y;
    unsigned int mask;

    /* Find the mouse pointer location */
    XQueryPointer(display, window, &root, &child, &root_x, &root_y,
      &win_x, &win_y, &mask);

    if(diw_mapP->mesh_t < 0.5) {
      meshLineMouseModify(&global_diw_map[0].mesh_src,
        &global_diw_map[0].mesh_dst, win_x, win_y, line_type, action);
    } else {
      meshLineMouseModify(&global_diw_map[0].mesh_src,
        &global_diw_map[0].mesh_dst, win_x, win_y, line_type, action);
    }
  }

  /* Resynchronize the diw_map meshes */
  diw_map_of_widget(NULL);

  /* Redraw all the images */
  FakeAllExpose(0, NULL, NULL, NULL);
}




/* --------------------------------------------------------------- */

/* PickMeshpoint: action to grab a meshpoint
// Sets global_diw_map[0].meshpoint_selected_loc[01]
*/
/*ARGSUSED*/
void
PickMeshpoint(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms)
{
  extern GC mps_gc;  /* (selected) mesh point graphics context */

  Window window = XtWindow(widget);
  Display *display = XtDisplay(widget);

  int    win_x, win_y;

  int    mi, mj;
  int    md;
  MeshT  mesh;

  diw_map_t *diw_mapP;

  /* Find the pointer location */
  {
    Window root, child;
    int root_x, root_y;
    unsigned int mask;
    XQueryPointer(display, window, &root, &child, &root_x, &root_y,
      &win_x, &win_y, &mask);
  }

  if((diw_mapP = diw_map_of_widget(widget)) == NULL) {
    fprintf(stderr, "PickMeshpoint: Bad Widget for diw_map\n");
    return;
  }

  mesh.nx = diw_mapP->mesh_src.nx;
  mesh.ny = diw_mapP->mesh_src.ny;

  /* Use either src or dst, depending on mesh_t */
  if(diw_mapP->mesh_t < 0.5) {
    mesh.x = diw_mapP->mesh_src.x;
    mesh.y = diw_mapP->mesh_src.y;
  } else {
    mesh.x = diw_mapP->mesh_dst.x;
    mesh.y = diw_mapP->mesh_dst.y;
  }

  /* Search the meshpoints for the nearest to the pointer */
  md = meshPointNearest(&mesh, win_x, win_y, &mi, &mj, NULL, NULL);

  /* Check to see if the pointer is close enough to meshpoint */
  if(md < MP_PICK_DIST) {
    short x, y;

    /* Select the meshpoint */
    global_diw_map[0].meshpoint_selected_loc[0] = mi;
    global_diw_map[0].meshpoint_selected_loc[1] = mj;
    global_diw_map[0].meshpoint_selected = True;

    /* Propagate the changes */
    diw_map_of_widget(NULL);

    /* Draw the selected meshpoint filled in */
    /* Location of the disc marking the selected meshpoint */
    x = mesh.x[mj * mesh.nx + mi] - MP_SIZE/2;
    y = mesh.y[mj * mesh.nx + mi] - MP_SIZE/2;

    /* Draw the selected MP disc */
    XFillArc(display, window, mps_gc, x-1, y-1, MP_SIZE+2, MP_SIZE+2, 0, MP_ARC);
  }
}




/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */

/* Starting place for dragging the selected mesh point */
static int diw_drag_start_x, diw_drag_start_y;

/* Current place for dragging the selected mesh point */
static int diw_drag_current_x, diw_drag_current_y;




/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */

/* UnpickMeshpoint: action to release meshpoint after drag
//
// SETS THE NEW MESHPOINT POSITION to be the changed by as much as the
// vector difference of the end dragpoint minus the beginning dragpoint.
//
// Change the cursor to the "select" cursor.
//
// Sets the global_diw_map[0].meshpoint_selected to False
*/
/*ARGSUSED*/
void
UnpickMeshpoint(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms)
{
  extern Cursor diw_select_cursor;

  diw_map_t *diw_mapP;

  if((diw_mapP = diw_map_of_widget(widget)) == NULL) {
    fprintf(stderr, "UnpickMeshpoint: Bad Widget for diw_map\n");
    return;
  }

  if(diw_mapP->meshpoint_selected) {
    Window window = XtWindow(widget);
    Display *display = XtDisplay(widget);

    const int mi = diw_mapP->meshpoint_selected_loc[0];
    const int mj = diw_mapP->meshpoint_selected_loc[1];
    const int nx = diw_mapP->mesh_src.nx;
    const int ny = diw_mapP->mesh_src.ny;
    double mps_dx;
    double mps_dy;
    MeshT *mesh_selected;
    double mesh_point_new_x = 0.0;
    double mesh_point_new_y = 0.0;

    /* Decide which mesh to modify (src or dst) */
    if(diw_mapP->mesh_t < 0.5) {
      mesh_selected = &diw_mapP->mesh_src;
    } else {
      mesh_selected = &diw_mapP->mesh_dst;
    }

    /* Save the new position */
    mesh_point_new_x = mesh_selected->x[mj*nx + mi] ;
    if(mi != 0 && mi != (nx-1)) {
      /* Find the x component of the total motion vector */
      mps_dx = diw_drag_current_x - diw_drag_start_x;
      /* Compute the new x-location of the mesh point */
      mesh_point_new_x = mesh_selected->x[mj*nx + mi] + mps_dx;
    }
    mesh_point_new_y = mesh_selected->y[mj*nx + mi] ;
    if(mj != 0 && mj != (ny-1)) {
      /* Find the y component of the total motion vector */
      mps_dy = diw_drag_current_y - diw_drag_start_y;
      /* Compute the new y-location of the mesh point */
      mesh_point_new_y = mesh_selected->y[mj*nx + mi] + mps_dy;
    }

    meshSet(mesh_selected, mi, mj, mesh_point_new_x, mesh_point_new_y);

    /* Set the mesh drag status */
    global_diw_map[0].meshpoint_selected = False;

    /* Propagate the mesh changes */
    diw_map_of_widget(NULL);

    /* Use the "select" cursor */
    XDefineCursor(display, window, diw_select_cursor);

    /* Redraw all the images */
    FakeAllExpose(0, NULL, NULL, NULL);
  }
}




/* --------------------------------------------------------------- */

/* DragMeshpoint: action to set meshpoint drag "to" location
// mesh_t determines whether src or dst mesh is used
// if (mesh_t < 0.5) use src, otherwise, dst
*/
/*ARGSUSED*/
void
DragMeshpoint(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms)
{
  extern Cursor diw_drag_ud_cursor;
  extern Cursor diw_drag_lr_cursor;
  extern Cursor diw_drag_cursor;

  diw_map_t *diw_mapP;

  if((diw_mapP = diw_map_of_widget(widget)) == NULL) {
    fprintf(stderr, "DragMeshpoint: Bad Widget for diw_map\n");
    return;
  }

  if(diw_mapP->meshpoint_selected) {
    Window win = XtWindow(widget);
    Display *disp = XtDisplay(widget);

    Window root, child;
    int root_x, root_y;
    unsigned int mask;

    int mi = diw_mapP->meshpoint_selected_loc[0];
    int mj = diw_mapP->meshpoint_selected_loc[1];

    /* Find the current drag location */
    XQueryPointer(disp, win, &root, &child, &root_x, &root_y,
      &diw_drag_current_x, &diw_drag_current_y, &mask);

    /* Use the "drag" cursor */
    if(mi==0 || mi==(diw_mapP->mesh_src.nx-1)) {
      XDefineCursor(disp, win, diw_drag_ud_cursor);
    } else if(mj==0 || mj==(diw_mapP->mesh_src.ny-1)) {
      XDefineCursor(disp, win, diw_drag_lr_cursor);
    } else {
      XDefineCursor(disp, win, diw_drag_cursor);
    }
  }
}




/* --------------------------------------------------------------- */

/* StartDragMeshpoint: action to set meshpoint drag "from" location
// Set the cursor shape and color to indicate dragging.
// mesh_t determines whether src or dst mesh is used;
// if (mesh_t < 0.5) use src, otherwise, dst
*/
/*ARGSUSED*/
void
StartDragMeshpoint(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms)
{
  extern Cursor diw_drag_ud_cursor;
  extern Cursor diw_drag_lr_cursor;
  extern Cursor diw_drag_cursor;

  diw_map_t *diw_mapP;

  if((diw_mapP = diw_map_of_widget(widget)) == NULL) {
    fprintf(stderr, "StartDragMeshpoint: Bad Widget for diw_map\n");
    return;
  }

  if(diw_mapP->meshpoint_selected) {
    Window win = XtWindow(widget);
    Display *disp = XtDisplay(widget);
    Cursor xcursor;

    int mi = diw_mapP->meshpoint_selected_loc[0];
    int mj = diw_mapP->meshpoint_selected_loc[1];

    Window root, child;
    int root_x, root_y;
    unsigned int mask;

    /* Initialize the drag vector */
    XQueryPointer(disp, win, &root, &child, &root_x, &root_y,
      &diw_drag_start_x, &diw_drag_start_y, &mask);
    diw_drag_current_x = diw_drag_start_x;
    diw_drag_current_y = diw_drag_start_y;

    /* Draw the grids to show which point is highlighted */

    /* Use the "drag" cursor */
    if(mi==0 || mi==(diw_mapP->mesh_src.nx-1)) {
      XDefineCursor(disp, win, xcursor=diw_drag_ud_cursor);
    } else if(mj==0 || mj==(diw_mapP->mesh_src.ny-1)) {
      XDefineCursor(disp, win, xcursor=diw_drag_lr_cursor);
    } else {
      XDefineCursor(disp, win, xcursor=diw_drag_cursor);
    }

    /* Color the "drag" cursor */
    if(diw_mapP->mesh_t < 0.5) {
      /* Color the cursor green to indicate src mesh */
      XRecolorCursor(disp, xcursor, DXC_GREEN, DXC_BLACK);
    } else {
      /* Color the cursor red to indicate dst mesh */
      XRecolorCursor(disp, xcursor, DXC_RED, DXC_BLACK);
    }
  }
}




/* =============================================================== */
/*                           Callbacks                             */
/* --------------------------------------------------------------- */

/* warp_image_cb: callback to warp images
*/
/*ARGSUSED*/
static void
warp_image_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  diw_map_t *dmP = (diw_map_t *) client_data;
  WarpImage(dmP->widget, NULL, NULL, NULL);
}


/* --------------------------------------------------------------- */

/* redither_image_cb: callback to redither images
*/
/*ARGSUSED*/
static void
redither_image_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  diw_map_t *dmP = (diw_map_t *) client_data;
  ReditherImage(dmP->widget, NULL, NULL, NULL);
}


/* --------------------------------------------------------------- */

/* mpt_scroll_cb: callback for diw_map mesh scrollbar extremum jumps
*/
static void
mpt_scroll_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  diw_map_t *dmP = (diw_map_t *)client_data;
  int pixels = (int)call_data;

  if(pixels>0) {
    /* Left */
    XawScrollbarSetThumb(widget, 0.0, -1.0);
    dmP->mesh_t = 0.0;
  } else {
    /* Right */
    XawScrollbarSetThumb(widget, 1.0, -1.0);
    dmP->mesh_t = 1.0;
  }
  FakeExpose(dmP->widget, NULL, NULL, NULL);
}


/* --------------------------------------------------------------- */

/* mpt_jump_cb: callback for diw_map mesh scrollbar thumb drag
*/
/*ARGSUSED*/
static void
mpt_jump_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  diw_map_t *dmP = (diw_map_t *)client_data;
  float percent = *(float *)call_data;

  dmP->mesh_t = percent;
  FakeExpose(dmP->widget, NULL, NULL, NULL);
}


/* --------------------------------------------------------------- */

/* image_scroll_cb: callback for diw_map image disolve scrollbar extremum
*/
static void
image_scroll_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  diw_map_t *dmP = (diw_map_t *)client_data;
  int pixels = (int)call_data;

  if(pixels>0) {
    /* Left */
    XawScrollbarSetThumb(widget, 0.0, -1.0);
    dmP->img_t = 0.0;
  } else {
    /* Right */
    XawScrollbarSetThumb(widget, 1.0, -1.0);
    dmP->img_t = 1.0;
  }
  ReditherImage(dmP->widget, NULL, NULL, NULL);
}


/* --------------------------------------------------------------- */

/* image_jump_cb: callback for diw_map image disolve scrollbar drag
*/
/*ARGSUSED*/
static void
image_jump_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  diw_map_t *dmP = (diw_map_t *)client_data;
  float percent = *(float *)call_data;

  dmP->img_t = percent;
  ReditherImage(dmP->widget, NULL, NULL, NULL);
}




/* --------------------------------------------------------------- */

/* dp_menu_cb: callback for diw_map properties menu entries
*/
/*ARGSUSED*/
void
dp_menu_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  extern Pixmap xbm_select;
  extern Pixmap xbm_unselect;

  diw_map_t *dmP = (diw_map_t *)client_data;
  int bool;

  /* Toggle the values */
  if(widget == NULL) {
    fprintf(stderr, "dp_menu_cb: Bad Widget %p\n", widget);
    return;
  } else if(widget == dmP->smp) {
    bool = dmP->meshpoints_src_draw = !dmP->meshpoints_src_draw;
  } else if(widget == dmP->sml) {
    bool = dmP->meshlines_src_draw = !dmP->meshlines_src_draw;
  } else if(widget == dmP->dmp) {
    bool = dmP->meshpoints_dst_draw = !dmP->meshpoints_dst_draw;
  } else if(widget == dmP->dml) {
    bool = dmP->meshlines_dst_draw = !dmP->meshlines_dst_draw;
  } else if(widget == dmP->tmp) {
    bool = dmP->meshpoints_t_draw = !dmP->meshpoints_t_draw;
  } else if(widget == dmP->tml) {
    bool = dmP->meshlines_t_draw = !dmP->meshlines_t_draw;
  } else {
    fprintf(stderr, "dp_menu_cb: Bad Widget %p\n", widget);
    return;
  }

  /* Set the button bitmap */
  if(bool) {
    XtVaSetValues(widget, XtNleftBitmap, xbm_select, NULL);
  } else {
    XtVaSetValues(widget, XtNleftBitmap, xbm_unselect, NULL);
  }
}




/* --------------------------------------------------------------- */

/* dp_dim_cb: callback for diw_map properties menu entry "dim"
*/
/*ARGSUSED*/
static void
dp_dim_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  extern Pixmap xbm_select;
  extern Pixmap xbm_unselect;

  diw_map_t *dmP = (diw_map_t *)client_data;

  /* Toggle the value */
  if(dmP->img_brightness != 1.0) {
    dmP->img_brightness = 1.0;
    XtVaSetValues(widget, XtNleftBitmap, xbm_unselect, NULL);
  } else {
    dmP->img_brightness = 0.5;
    XtVaSetValues(widget, XtNleftBitmap, xbm_select, NULL);
  }

  ReditherImage(dmP->widget, NULL, NULL, NULL);
}


/* =============================================================== */


/* --------------------------------------------------------------- */


/* allocate_x_images: allocate pixmap and ximage
// (intended for the pixmap and ximage members of a diw_map)
*/
int
allocate_x_images(Widget widget, Pixmap *pxP, XImage **xiP, int nx, int ny)
{
  extern int x_depth;
  extern GC  x_gc_clear;

  Display *display = XtDisplay(widget);

#define FACTOR_OF_4
#ifdef FACTOR_OF_4
  /* Force the pixmap and ximage width to be a factor of 4, because
  // wraparound happens in some versions of X otherwise.
  // I think this has to do with ximage.bitmap_pad.
  // For now, I will live with this hack.
  */
  nx=((int)(nx/4.0 + 0.9999))*4;
#endif

  *pxP = XCreatePixmap(display, RootWindowOfScreen(XtScreen(widget)),
           /* width */ nx, /* height */ ny, /* depth */ x_depth);
  if(*pxP == 0) {
    fprintf(stderr, "allocate_x_images: XCreatePixmap failed\n");
    return(1);
  }

  /* Create a blank XImage to draw in */
#ifdef XCI
  /* I do not have enough info from any of the X docs to
  // understand XCreateImage.  I do not know whether bytes_per_line is
  // changed by the X server.  The docs are not clear about that.  Hence,
  // I do not know how much memory to allocate for the XImage data.
  // Also, the bitmap_pad seems to be different from what I specify
  // here, which is the reason for the FACTOR_OF_4 hack above.
  // -- MJG 18sep95
  */
  {
    char *xi_data;
  xi_data = MY_CALLOC(nx*ny*x_depth, char);
  *xiP = XCreateImage(display, x_visual, x_depth, ZPixmap,
         /*offset */0, /* data */xi_data,
         /* width */nx, /* height */ny,
         /* bitmap_pad */8, /*bytes_per_line*/0);
  }

#else

  /* I do not know whether the image data is allocated by the call
  ** to XGetImage or not.  The docs do not say.  I assume that the data
  ** is allocated there.
  */
  *xiP = XGetImage(display, /* drawable */ *pxP,
               /* x, y offset */ 0, 0, /* width, height */ nx, ny,
               /* plane mask */ AllPlanes, /* format */ ZPixmap);
#endif

  if(*xiP == NULL) {
    fprintf(stderr, "allocate_x_images: XGetImage failed\n");
    return(1);
  }

  /* Create a graphics context with the correct visual and depth info
  ** for drawing into the image space for the above structures
  */
  {
    XGCValues        gc_vals;

    /* Erase new window */
    gc_vals.foreground = BlackPixel(display, XDefaultScreen(display));
    x_gc_clear = XCreateGC(display, *pxP, GCForeground, &gc_vals);
    XFillRectangle(display, *pxP, x_gc_clear, 0, 0, nx, ny);
  }

  return(0);
}




/* --------------------------------------------------------------- */

/* initialize_diw_map: set sane values for most diw_map members
// Allocates memory for src and dst meshes
// Resets src and dst meshes.
*/
static void
initialize_diw_map(diw_map_t *dmP)
{
  int nx = dmP->width / 40 + 1;
  int ny = dmP->height / 40 + 1;

  if(meshAlloc(&dmP->mesh_src, nx, ny))
    exit(1);

  if(meshAlloc(&dmP->mesh_dst, nx, ny))
    exit(1);

  meshReset(&dmP->mesh_src, dmP->width, dmP->height);
  meshReset(&dmP->mesh_dst, dmP->width, dmP->height);

  dmP->meshpoint_selected = False;

  dmP->img_brightness = 1.0;

  /* Turn off all meshes and make the user turn them on */
  dmP->meshpoints_t_draw = False;
  dmP->meshlines_t_draw = False;

  dmP->meshpoints_src_draw = False;
  dmP->meshlines_src_draw = False;

  dmP->meshpoints_dst_draw = False;
  dmP->meshlines_dst_draw = False;
}




/* --------------------------------------------------------------- */

/* image_panel_translations: event-to-action mappings
*/
static char image_panel_translations[] = "\
  <Btn1Down>      : pick() start_drag() draw_all_meshes() \n\
  <Btn1Motion>    : drag() \n\
  <Btn1Up>        : unpick() \n\
  Shift<Btn2Down> : change_mesh(delete, vertical) \n\
  Shift<Btn3Down> : change_mesh(delete, horizontal) \n\
  <Btn2Down>(2)   : change_mesh(delete, vertical) \n\
  <Btn3Down>(2)   : change_mesh(delete, horizontal) \n\
  <Btn2Down>      : change_mesh(add, vertical) \n\
  <Btn3Down>      : change_mesh(add, horizontal) \n\
  <Expose>        : refresh() draw_meshes()\
";




/* --------------------------------------------------------------- */

/* NAME
//   set_x_visual:  This is supposed to find the best visual class and
// visual depth the hardware supports.  However, later, when a XPutImage
// is done, it fails.  This is because the Xt toolkit does not easily
// support using anything other than the default visual class.  Perhaps
// this will change in future versions of X.
*/
static void set_x_visual(Display *display)
{
  extern Visual *x_visual;
  extern int     x_depth;

  int screen_num = XDefaultScreen(display);

#ifdef VISUAL_WORKING
  XVisualInfo vTemplate;
  XVisualInfo *vlP;
  int visualsMatched;
  int vi;

  /* If x_visual is non-NULL, then it has already been found */
  if(x_visual != NULL) return;

  vTemplate.screen = screen_num;

  /* --- Look for 24 bit TrueColor visuals */
  vTemplate.class = TrueColor;
  vlP = XGetVisualInfo(display, VisualScreenMask | VisualClassMask,
          &vTemplate, &visualsMatched);

  if(visualsMatched) {
    Visual *beVis; /* visual corresponding to largest depth */
    int beDep=0; /* largest depth */

    for(vi = 0; vi < visualsMatched; vi++) {
      if(vlP[vi].depth > beDep) {
        beDep = vlP[vi].depth;
        beVis = vlP[vi].visual;
      }
    }
    XFree(vlP);
    if(beDep >=8) {
      printf("beVis: TrueColor %i bit\n", beDep);
      x_depth = beDep;
      x_visual = beVis;
      return;
    }
  }

  /* --- Look for 8 bit PseudoColor visuals */
  vTemplate.class = PseudoColor;
  vlP = XGetVisualInfo(display, VisualScreenMask | VisualClassMask,
    &vTemplate, &visualsMatched);

  if(visualsMatched) {
    Visual *beVis; /* visual corresponding to largest depth */
    int beDep=0; /* largest depth */

    for(vi = 0; vi < visualsMatched; vi++) {
      if(vlP[vi].depth > beDep) {
        beDep = vlP[vi].depth;
        beVis = vlP[vi].visual;
      }
    }
    XFree(vlP);
    if(beDep >=8) {
      printf("beVis: PseudoColor %i bit\n", beDep);
      x_depth = beDep;
      x_visual = beVis;
      return;
    }
  }

  /* Could not find a visual with at least 8 bits per pixel */
  /* Use the default and warn the user */
  fprintf(stderr, "set_x_visual: could not find sufficient video display\n");
#endif

  x_visual = DefaultVisual(display, screen_num);
  x_depth = DefaultDepth(display, screen_num);

  if(verbose) {
    fprintf(stderr, "set_x_visual: visual %i depth %i\n",
            x_visual->class, x_depth);
  }

  return;
}




/* --------------------------------------------------------------- */

/* create_diw_widget:  Create the widget heirarchy for the diw_map and image
// parent is the parent widget
//
// returns the widget of the image panel, not the form widget
//
// NOTE:  This is not a real widget yet.  -- 24may94 MJG
//
// About setting the "widget" member of the diw_map:
// This is a hack to make this diw structure behave like a widget.
// There is a global array of diw_map, each with its own Core
// widget.  When a callback or action is performed, the widget is
// passed to the routine, and the routine then looks up which
// diw_map is associated with that widget.  This could all be fixed up
// by making the diw_map a full-fledged widget, which I might
// eventually do, but for now, this works.
//
// side_effects:
//   global_variables x_visual, x_depth set.
//   menu string is allocated to create unique name for
// popup menu.  Athena uses strings, not widget IDs, to find popup
// shells for menus.  How bizarre.  How stupid.
*/
Widget
create_diw_widget(Widget parent, diw_map_t *dmP, int width, int height)
{
  extern Pixmap xbm_gray;
  extern Pixmap xbm_unselect;

  Widget   form;                   /* constraint geometry widget for layout */
  Widget     diw_prop_menu_button; /* menu button: properties */
  String      diw_prop_menu_name;  /* "name" is the way Athena finds the menu */
  Widget       diw_prop_menu;      /* menu: properties */
  Widget         smp_sme, sml_sme; /* toggle: src mesh lines and points */
  Widget         dmp_sme, dml_sme; /* toggle: dst mesh lines and points */
  Widget         tmp_sme, tml_sme; /* toggle: interp mesh lines and points */
  Widget         dim_sme;          /* toggle: dim image */
  String      diw_cmd_menu_name;   /* "name" is the way Athena finds the menu */
  Widget       diw_cmd_menu;       /* menu: commands */
  dialog_apdx_t *si_daP;           /* dialog: save image */
  Widget         si_sme;           /* button: popup save image dialog */
  Widget         warp_sme;         /* button: warp images */
  Widget         redither_sme;     /* button: redither images */
  Widget     viewport;             /* viewport: for image_panel */
  Widget       image_panel;        /* core: image panel */
  Widget     label_mesh, label_src_mesh, label_dst_mesh;
  Widget     scrollbar_mesh;
  Widget     label_image, label_src_image;
  Widget     scrollbar_image;
  Arg args[25];
  int argn;

  Display *display = XtDisplay(parent);

  static int serial=0;

  /* Find an appropriate bits-per-pixel depth (for the pixmap) */
  set_x_visual(display); /* WA: (MJG 13sep95) */

  /* Create the Form widget for the buttons and image region */
  form = XtVaCreateManagedWidget("dm_form", formWidgetClass, parent, NULL);
  dmP->form = form;

    /* Create a simple menu: properties */
    /* create a unique menu name */
    if((diw_prop_menu_name=MY_CALLOC(20, char))==NULL) {
      return(0);
    }
    sprintf(diw_prop_menu_name, "dpm_%i", serial++);
    diw_prop_menu = XtVaCreatePopupShell(diw_prop_menu_name,
      simpleMenuWidgetClass, parent, XtNbackgroundPixmap, xbm_gray, NULL);

      /* Create a menu entry: show source mesh points */
      dmP->smp = smp_sme = XtVaCreateManagedWidget("smp_sme", smeBSBObjectClass,
        diw_prop_menu, XtNleftBitmap, xbm_unselect, NULL);
      XtAddCallback(smp_sme, XtNcallback, dp_menu_cb, dmP);

      /* Create a menu entry: show source mesh lines */
      dmP->sml = sml_sme = XtVaCreateManagedWidget("sml_sme", smeBSBObjectClass,
        diw_prop_menu, XtNleftBitmap, xbm_unselect, NULL);
      XtAddCallback(sml_sme, XtNcallback, dp_menu_cb, dmP);

      /* Create a menu entry: show destination mesh points */
      dmP->dmp = dmp_sme = XtVaCreateManagedWidget("dmp_sme", smeBSBObjectClass,
        diw_prop_menu, XtNleftBitmap, xbm_unselect, NULL);
      XtAddCallback(dmp_sme, XtNcallback, dp_menu_cb, dmP);

      /* Create a menu entry: show destination mesh lines */
      dmP->dml = dml_sme = XtVaCreateManagedWidget("dml_sme", smeBSBObjectClass,
        diw_prop_menu, XtNleftBitmap, xbm_unselect, NULL);
      XtAddCallback(dml_sme, XtNcallback, dp_menu_cb, dmP);

      /* Create a menu entry: show tween mesh points */
      dmP->tmp = tmp_sme = XtVaCreateManagedWidget("tmp_sme", smeBSBObjectClass,
        diw_prop_menu, XtNleftBitmap, xbm_unselect, NULL);
      XtAddCallback(tmp_sme, XtNcallback, dp_menu_cb, dmP);

      /* Create a menu entry: show tween mesh lines */
      dmP->tml = tml_sme = XtVaCreateManagedWidget("tml_sme", smeBSBObjectClass,
        diw_prop_menu, XtNleftBitmap, xbm_unselect, NULL);
      XtAddCallback(tml_sme, XtNcallback, dp_menu_cb, dmP);

      /* menu item separator */
      XtVaCreateManagedWidget("sep_sme", smeLineObjectClass,
        diw_prop_menu, NULL);

      /* Create a menu entry: dim image */
      dim_sme = XtVaCreateManagedWidget("dim_sme", smeBSBObjectClass,
        diw_prop_menu, XtNleftBitmap, xbm_unselect, NULL);
      XtAddCallback(dim_sme, XtNcallback, dp_dim_cb, dmP);

    /* Create a menu button: properties */
    diw_prop_menu_button = XtVaCreateManagedWidget("diw_prop_menu_button",
      menuButtonWidgetClass, form, XtNmenuName, diw_prop_menu_name, NULL);


    /* Create a simple menu: commands */
    /* create a unique menu name */
    if((diw_cmd_menu_name=MY_CALLOC(20, char))==NULL) {
      return(0);
    }
    sprintf(diw_cmd_menu_name, "dcm_%i", serial++);
    diw_cmd_menu = XtVaCreatePopupShell(diw_cmd_menu_name,
      simpleMenuWidgetClass, parent, XtNbackgroundPixmap, xbm_gray, NULL);

#ifdef NEED_GIMP
    if (!plugin)
#endif
    {
      /* Create the dialog widget: save image */
      si_daP = create_mjg_dialog(parent, "si");
      si_daP->callback = save_img_cb;
      si_daP->client_data = (XtPointer)dmP;

      /* Create a menu entry: save image */
      si_sme = XtVaCreateManagedWidget("si_sme", smeBSBObjectClass,
        diw_cmd_menu, NULL);
      XtAddCallback(si_sme, XtNcallback, popup_dialog_cb, si_daP);
      si_daP->button = si_sme; /* button that pops up dialog */
    }

      /* Create a menu entry: warp image */
      warp_sme = XtVaCreateManagedWidget("warp_sme", smeBSBObjectClass,
        diw_cmd_menu, NULL);
      XtAddCallback(warp_sme, XtNcallback, warp_image_cb, dmP);

      /* Create a menu entry: redither image */
      redither_sme = XtVaCreateManagedWidget("redither_sme", smeBSBObjectClass,
        diw_cmd_menu, NULL);
      XtAddCallback(redither_sme, XtNcallback, redither_image_cb, dmP);

    /* Create a menu button: commands */
    XtVaCreateManagedWidget("diw_cmd_menu_button",
      menuButtonWidgetClass, form, XtNmenuName, diw_cmd_menu_name,
      XtNfromHoriz, diw_prop_menu_button, NULL);


    /* Create the viewport for the image panel */
    viewport = XtVaCreateManagedWidget("viewport", viewportWidgetClass, form,
      XtNfromVert, diw_prop_menu_button,
      NULL);


    /* Create the image region window */
    argn=0;
    XtSetArg(args[argn], XtNwidth, width); argn++;
    XtSetArg(args[argn], XtNheight, height); argn++;

    /* I am guessing about the depth and visual because I do not have my
    ** Xt docs with me right now.  -- MJG 18sep95
    */
    /*
    XtSetArg(args[argn], XtNdepth, x_depth); argn++;
    */

    XtSetArg(args[argn], XtNtranslations,
      XtParseTranslationTable(image_panel_translations)); argn++;
    image_panel = XtCreateManagedWidget("image_panel", widgetClass,
      viewport, args, argn);



    /* -- Warp scrollbar -- */
    /* Create a label */
    label_mesh = XtVaCreateManagedWidget("mesh_label", labelWidgetClass,
      form, XtNfromVert, viewport, NULL);

    /* Create a label */
    label_src_mesh = XtVaCreateManagedWidget("source_mesh", labelWidgetClass,
      form, XtNfromVert, viewport, NULL);

    /* Create a scrollbar */
    argn=0;
    XtSetArg(args[argn], XtNfromVert, viewport); argn++;
    /* length should be no more than width-(label_src)-(label_dst) */
    XtSetArg(args[argn], XtNlength, 100); argn++;
    /* thickness should be the same as the label height */
    XtSetArg(args[argn], XtNthickness, 21); argn++;
    scrollbar_mesh = XtCreateManagedWidget("scrollbar_mesh",
      scrollbarWidgetClass, form, args, argn);
    dmP->warp_sb = scrollbar_mesh;

    XtAddCallback(scrollbar_mesh, XtNscrollProc, mpt_scroll_cb, dmP);
    XtAddCallback(scrollbar_mesh, XtNjumpProc, mpt_jump_cb, dmP);

    XawScrollbarSetThumb(scrollbar_mesh, dmP->mesh_t, -1.0);

    /* Create a label */
    argn=0;
    XtSetArg(args[argn], XtNfromVert, viewport); argn++;
    label_dst_mesh = XtCreateManagedWidget("destination_mesh",
      labelWidgetClass, form, args, argn);


    /* -- Dissolve scrollbar -- */
    /* Create a label */
    label_image = XtVaCreateManagedWidget("image_label", labelWidgetClass,
      form, XtNfromVert, label_mesh, NULL);

    /* Create a label */
    label_src_image=XtVaCreateManagedWidget("source_image", labelWidgetClass,
      form, XtNfromVert, label_mesh, XtNfromHoriz, label_image, NULL);

    /* Create a scrollbar */
    argn=0;
    XtSetArg(args[argn], XtNfromVert, scrollbar_mesh); argn++;
    XtSetArg(args[argn], XtNfromHoriz, label_src_image); argn++;
    /* length should be no more than width-(label_src)-(label_dst) */
    XtSetArg(args[argn], XtNlength, 100); argn++;
    /* thickness should be the same as the label height */
    XtSetArg(args[argn], XtNthickness, 21); argn++;
    scrollbar_image = XtCreateManagedWidget("scrollbar_image",
      scrollbarWidgetClass, form, args, argn);
    dmP->dissolve_sb = scrollbar_image;

    XtAddCallback(scrollbar_image, XtNscrollProc, image_scroll_cb, dmP);
    XtAddCallback(scrollbar_image, XtNjumpProc, image_jump_cb, dmP);

    XawScrollbarSetThumb(scrollbar_image, dmP->img_t, -1.0);

    /* Create a label */
    argn=0;
    XtSetArg(args[argn], XtNfromVert, label_dst_mesh); argn++;
    XtSetArg(args[argn], XtNfromHoriz, scrollbar_image); argn++;
    XtCreateManagedWidget("destination_image",
      labelWidgetClass, form, args, argn);

    /* -- Scrollbar Geometry constraints -- */
    XtVaSetValues(label_src_mesh, XtNfromHoriz, label_image, NULL);
    XtVaSetValues(scrollbar_mesh, XtNfromHoriz, label_src_image, NULL);
    XtVaSetValues(label_dst_mesh, XtNfromHoriz, scrollbar_image, NULL);

  /* ----- ----- */

  /* Set the image panel geometry members in the diw_map */
  dmP->width = width;
  dmP->height = height;

  /* Set all the X Window System stuff for this DIW structure */
  /* Set the widget member */
  dmP->widget = image_panel;

  /* Create the pixmap and stuff */
  if(allocate_x_images(parent, &dmP->pixmap, &dmP->ximage, width, height)) {
    fprintf(stderr, "create_diw_widget :failed to allocate X image space\n");
  }

  /* Find best visual class available */
  /* redundant call.  --MJG:12jul96 */
  /* set_x_visual(display); */

  /* Here is the only part that is not window system specific */
  initialize_diw_map(dmP);

#if 0
  FREE(diw_prop_menu_name);
  FREE(diw_cmd_menu_name);
#endif

  return(image_panel);
}




/* --------------------------------------------------------------- */

/* NUM_QUERY_COLORS: number of colors to query in colormap search */
#define NUM_QUERY_COLORS 256

/* X_MAXVAL: maximum value for red,green, or blue value in X */
#define X_MAXVAL 65535

void
init_diw_stuff(Widget toplevel)
{
  extern GC     mpl_gc;            /* mesh line graphics context */
  extern GC     mp_gc;             /* mesh point graphics context */
  extern GC     mps_gc;            /* (selected) mesh point graphics context */
  extern Pixmap xbm_gray;
  extern int    x_depth;
  extern Pixmap xbm_select;
  extern Pixmap xbm_unselect;
  extern Cursor diw_drag_cursor;
  extern Cursor diw_drag_ud_cursor;
  extern Cursor diw_drag_lr_cursor;
  extern Cursor diw_select_cursor;
  /* extern Cursor diw_unselect_cursor; */
  extern Cursor diw_wait_cursor;

  XGCValues gcvals;
  XColor light_gray_s, light_gray_e;

  Display *display;
  int     screen_num;

  display = XtDisplay(toplevel);
  screen_num = XDefaultScreen(display);

  /* Make bitmaps for my toggle buttons */
  if(!XAllocNamedColor(display, DefaultColormap(display, screen_num),
    "light gray", &light_gray_s, &light_gray_e))
  {
    fprintf(stderr, "failed to allocate color 'light gray'\n");
    light_gray_s.pixel = WhitePixel(display, screen_num);
    light_gray_e.pixel = BlackPixel(display, screen_num);
  }

  set_x_visual(display); /* MJG 14sep95 */

  /* changed 8 to x_depth -- WA (MJG 13sep95) */
  xbm_gray = XCreatePixmapFromBitmapData(display,
    RootWindowOfScreen(XtScreen(toplevel)),
    gray_bits, gray_width, gray_height, light_gray_s.pixel,
    light_gray_e.pixel, x_depth);

  xbm_select = XCreateBitmapFromData(display,
    RootWindowOfScreen(XtScreen(toplevel)),
    select_bits, select_width, select_height);
  xbm_unselect = XCreateBitmapFromData(display,
    RootWindowOfScreen(XtScreen(toplevel)),
    unselect_bits, unselect_width, unselect_height);


  {
    /* ============================================================ */
    /* Allocate the read-only color cells for my Dithering Colormap */

    int dci;

    if(verbose) {
      fprintf(stderr, "i_d_s: NUM_DIW_XCOLORS=%i\n", NUM_DIW_XCOLORS);
      fprintf(stderr, "i_d_s: RED_NUM_BITS=%i\n", RED_NUM_BITS);
      fprintf(stderr, "i_d_s: GRN_NUM_BITS=%i\n", GRN_NUM_BITS);
      fprintf(stderr, "i_d_s: BLU_NUM_BITS=%i\n", BLU_NUM_BITS);
      fprintf(stderr, "i_d_s: RED_MAXVAL=%i\n", RED_MAXVAL);
      fprintf(stderr, "i_d_s: GRN_MAXVAL=%i\n", GRN_MAXVAL);
      fprintf(stderr, "i_d_s: BLU_MAXVAL=%i\n", BLU_MAXVAL);
      fprintf(stderr, "i_d_s: RED_SHIFT=%i\n", RED_SHIFT);
      fprintf(stderr, "i_d_s: GRN_SHIFT=%i\n", GRN_SHIFT);
      fprintf(stderr, "i_d_s: BLU_SHIFT=%i\n", BLU_SHIFT);
    }

    for(dci=0; dci<NUM_DIW_XCOLORS; dci++) {
      if(verbose>=2) {
        fprintf(stderr, "i_d_s: dci=%i\n", dci);
      }

      /* Construct the desired RGB color */
      /* ------------------------------- */
      /*
       * Here is the formula for finding a channel value from dci:
       *
       * What we start with is a "dithering colormap index" (DCI), which
       * is the index into the diw_xcolormap mapping array.  The general
       * idea of how to change this into a channel value that the
       * display will understand is simple, but since I want to do all
       * of it in integer arithmetic, it looks complicated.  I will
       * describe the procedure in floating point arithmetic first, to
       * make it more clear:
       *
       * (1) Mask the DCI to get the value of each channel, i.e. the
       *     red, green, and blue color values.
       *
       *
       * (2) Scale this to give a number between 0.0 and 1.0 where 0.0
       *     means no color and 1.0 means fully on for that channel.
       *
       * (3) Multiply by the display maximum value for that channel,
       *     which gives a number between 0 and the maximum value the
       *     display uses to indicate fully on.
       *
       * Here is how to do all of this in integer arithmetic.
       *
       * (1) Mask the DCI index with the XPI_ of the pure channel color,
       *     to get only the bits from the channel desired.
       *
       * (2) Shift right to get a value where the minimum is 0.  I.e.,
       *     now that we have the bits that we want, we need to put them
       *     in the correct place by shifting them right.  Now we will
       *     have a value between 0 and my scheme maximum value for
       *     that channel.
       *
       * (3) Multiply by the display maximum value the channel will
       *     allow.  This scales the color value up to the display
       *     scheme.  Now we have a value between zero and
       *     (display max val) * (my max val), which is too large by a
       *     factor of (my max val).
       *
       * (4) Add the maximum value of the dithered color for that
       *     channel, then subtract one.  This is to correct for
       *     roundoff introduced in the next step.
       *
       * (5) Divide by the maximum value of the dithered color for that
       *     channel.  This gives us a value between 0 and (display max val),
       *     which is what the display needs, so we are done.
       *
       */

      diw_xcolors[dci].red   = (RED_DCI(dci) * X_MAXVAL + RED_MAXVAL-1) / RED_MAXVAL;
      diw_xcolors[dci].green = (GRN_DCI(dci) * X_MAXVAL + GRN_MAXVAL-1) / GRN_MAXVAL;
      diw_xcolors[dci].blue  = (BLU_DCI(dci) * X_MAXVAL + BLU_MAXVAL-1) / BLU_MAXVAL;

      if(verbose>=2) {
        fprintf(stderr, "i_d_s: requesting allocation of color (%4x,%4x,%4x)\n",
          diw_xcolors[dci].red, diw_xcolors[dci].green, diw_xcolors[dci].blue);
      }

      /* Ask for the desired color to be allocated in the colormap */
      /* --------------------------------------------------------- */
      if(!XAllocColor(display, DefaultColormap(display, screen_num),
        &diw_xcolors[dci]))
      {
        /* Could not allocate color, so find a close one */
        /* --------------------------------------------- */
        int           pi, pim=0;        /* index into colormap */
        int           rd, gd, bd;       /* color channel distances */
        unsigned long pd;               /* distance between colors */
        unsigned long pdmin=0xffffffff; /* minimum distance so far */
        XColor        xcol[NUM_QUERY_COLORS]; /* array of queried colors */

        fprintf(stderr, "could not allocate color (%4x,%4x,%4x),",
          diw_xcolors[dci].red, diw_xcolors[dci].green, diw_xcolors[dci].blue);

        /* Find out what colors are available. */
        /* ----------------------------------- */
        /* -- Ask for colors indexed 0 through NUM_QUERY_COLORS */
        for(pi=0; pi<NUM_QUERY_COLORS; pi++) xcol[pi].pixel=pi;
        XQueryColors(display, DefaultColormap(display,screen_num),
                     xcol, NUM_QUERY_COLORS);

        /* Search available colors for closest match with the one requested */
        /* ---------------------------------------------------------------- */
        for(pi=0; pi<NUM_QUERY_COLORS; pi++) {
          /* Compute distance between color channel values */
          rd = (diw_xcolors[dci].red >> TRUNC_SHIFT)
               - (xcol[pi].red >> TRUNC_SHIFT);
          gd = (diw_xcolors[dci].green >> TRUNC_SHIFT)
               - (xcol[pi].green >> TRUNC_SHIFT);
          bd = (diw_xcolors[dci].blue >> TRUNC_SHIFT)
               - (xcol[pi].blue >> TRUNC_SHIFT);

          pd = rd*rd + gd*gd + bd*bd;  /* scalar color "distance" */

          /* If pixel distance is smallest found so far ... */
          if(pd<pdmin) {
            if(XAllocColor(display,DefaultColormap(display,screen_num),
                           &xcol[pi]) )
            {
              pdmin=pd;  /* store the smallest color distance so far */
              pim=pi;    /* store the index of the closest available color */
            }
          }
        }
        /* Store the mapping between the requested and found colors */
        diw_xcolors[dci] = xcol[pim];
        fprintf(stderr, " using (%4x,%4x,%4x)\n",
          diw_xcolors[dci].red, diw_xcolors[dci].green, diw_xcolors[dci].blue);
      }
    }
  }


  /* ------------------------------------------ */
  /* Create the Graphics context for mesh lines */
  {
    Window  window;
    unsigned long gcmask;

    window = RootWindowOfScreen(XtScreen(toplevel));

    gcmask = 0x0;
    mpl_gc=XCreateGC(display, window, gcmask, NULL);

    gcmask = 0x7fffff;
    XCopyGC(display, DefaultGC(display, screen_num), gcmask, mpl_gc);

    gcmask = GCFunction | GCForeground | GCLineStyle | GCLineWidth;
    gcvals.function = GXcopy;
    gcvals.foreground = WhitePixel(display, screen_num);
    gcvals.line_style = LineOnOffDash;
    gcvals.line_width = 2;
    XChangeGC(display, mpl_gc, gcmask, &gcvals);


    /* Create the Graphics context for unselected mesh points */
    gcmask = 0x0;
    mp_gc=XCreateGC(display, window, gcmask, NULL);

    gcmask = 0x7fffff;
    XCopyGC(display, mpl_gc, gcmask, mp_gc);
    gcmask= GCLineStyle;
    gcvals.line_style = LineSolid;
    XChangeGC(display, mp_gc, gcmask, &gcvals);

    /* Create the Graphics context for selected mesh points */
    gcmask = 0x0;
    mps_gc=XCreateGC(display, window, gcmask, NULL);

    gcmask = 0x7fffff;
    XCopyGC(display, DefaultGC(display, screen_num), gcmask, mps_gc);

    gcmask = GCFunction | GCForeground;
    gcvals.function = GXcopy;
    gcvals.foreground = WhitePixel(display, screen_num);
    XChangeGC(display, mps_gc, gcmask, &gcvals);
  }

  /* Create cursors */
  {
    diw_drag_cursor = XCreateFontCursor(display, 52); /* outward arrows */
    diw_drag_lr_cursor = XCreateFontCursor(display, 108); /* left/right */
    diw_drag_ud_cursor = XCreateFontCursor(display, 116); /* up/down */
    diw_select_cursor = XCreateFontCursor(display, 24); /* circle */
    diw_wait_cursor = XCreateFontCursor(display, 150); /* clock */
  }
}




/* --------------------------------------------------------------- */

/* Global structure for mesh map */
diw_map_t global_diw_map[NUM_DIW_MAPS];




/* Global X colormap for RGB colors */

/* diw_xcolors: mapping between a "dithering colormap index" and the X
// colormap index.
//
// The "dithering colormap index" (DCI) is a contrived scheme where the
// index value is the same as the value of a terse colormap which uses
// only a few bits per channel.  For example, the so-called RGB-222
// colormap uses 2 bits per each of the channels Red, Green, and Blue.
// Then, the "dithering colormap index" would be an integer whose value
// is ((red*16)+(green*4)+(blue)), where the values of red,green,blue
// can be from 0 to 3, inclusive (i.e., 2 bits each).  This yields
// values for the "dithering colormap index" ranging from 0
// (red=0,green=0,blue=0) which is black, to 63 (red=3,green=3,blue=3)
// which is white.
*/
XColor diw_xcolors[NUM_DIW_XCOLORS];




/* --------------------------------------------------------------- */

/* The scope of all of the following variables should be this file.
// They would normally be declared "static" but doing so conflicts with
// declaring them "extern" in each of the routines above which use
// them, but I prefer requiring an "extern" declaration because it
// clarifies which variables are being used in each routine.
*/




/* Cursors commonly used in this application */
Cursor diw_drag_cursor;
Cursor diw_drag_lr_cursor;
Cursor diw_drag_ud_cursor;
Cursor diw_select_cursor;
Cursor diw_wait_cursor;




/* bitmaps */
Pixmap xbm_select;
Pixmap xbm_unselect;
Pixmap xbm_gray;




/* depth of pixmap */
int x_depth=0;




/* Visual (has information about color schemes for display) */
Visual *x_visual=NULL;




/* Graphics context for drawing into image panels.
// (Has necessary visual and depth properties.)
*/
GC x_gc_clear;




/* meshlines graphic context */
GC mpl_gc;

/* unselected meshpoints graphic context */
GC mp_gc;

/* selected meshpoints graphic context */
GC mps_gc;
