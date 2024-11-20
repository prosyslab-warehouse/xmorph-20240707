/* diw_map.h : Digital Image Warping graphical user interface header
//
// Written and Copyright (C) 1994-1999 by Michael J. Gourlay
//
// Provided as is.  No warrantees, express or implied.
*/

#ifndef _DIW_MAP__INCLUDED_
#define _DIW_MAP__INCLUDED_


#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>




#include "mesh.h"
#include "image_diw.h"




typedef struct diw_map_ {
  /* in lieu of making the DIW its own widget... */
  Pixmap        pixmap;
  XImage        *ximage;
  int           width, height;          /* X pixmap/image geometry */
  Widget        widget;                 /* core widget, for drawing images in */
  Widget        form;    /* geometry widget, parent of all diw_map children */
  Widget        smp, sml;       /* Toggle: source mesh [points, lines] */
  Widget        dmp, dml;       /* Toggle: destination mesh [points, lines] */
  Widget        tmp, tml;       /* Toggle: tween mesh [points, lines] */
  Widget        dissolve_sb, warp_sb;   /* scrollbars */

  RgbaImageT    src_img;
  RgbaImageT    dst_img;
  double        img_t;                  /* RGBA crossfade parameter */
  double        img_brightness;         /* brightness factor of image */
  MeshT         mesh_src;
  MeshT         mesh_dst;
  double        mesh_t; /* parameter between 0.0 and 1.0 for s or d map */
  Boolean       meshpoint_selected;
  int           meshpoint_selected_loc[2];      /* i,j indices */
  Boolean       meshpoints_t_draw;      /* draw interp mesh points over image */
  Boolean       meshlines_t_draw;       /* draw interp mesh lines over image */
  Boolean       meshpoints_src_draw;    /* draw src mesh points over image */
  Boolean       meshlines_src_draw;     /* draw src mesh lines over image */
  Boolean       meshpoints_dst_draw;    /* draw dst mesh points over image */
  Boolean       meshlines_dst_draw;     /* draw dst mesh lines over image */
} diw_map_t;




extern int verbose; /* for debugging */




/* Dithering colormap macros */
/* ------------------------- */

/* ???_NUM_BITS: number of bits per channel for the dithering colormap. */
#define RED_NUM_BITS 2
#define GRN_NUM_BITS 3
#define BLU_NUM_BITS 2

#define RGB_TOTAL_BITS (RED_NUM_BITS + GRN_NUM_BITS + BLU_NUM_BITS)

/* ???_MAXVAL: maximum values for each channel in the dithering colormap */
#define RED_MAXVAL ((1 << RED_NUM_BITS) - 1)
#define GRN_MAXVAL ((1 << GRN_NUM_BITS) - 1)
#define BLU_MAXVAL ((1 << BLU_NUM_BITS) - 1)

#define RED_SHIFT (GRN_NUM_BITS + BLU_NUM_BITS)
#define GRN_SHIFT (BLU_NUM_BITS)
#define BLU_SHIFT 0

/* DCI: converts from r,g,b values to dithering colormap index */
#define DCI(r,g,b) (((r)<<RED_SHIFT) | ((g)<<GRN_SHIFT) | (b))


/* ???_DCI: converts from dithering colormap index to red, green, or
blue value */
#define RED_DCI(d) (((d) & XPI_RED) >> RED_SHIFT)
#define GRN_DCI(d) (((d) & XPI_GREEN) >> GRN_SHIFT)
#define BLU_DCI(d)  ((d) & XPI_BLUE)

#define NUM_DIW_XCOLORS (1<<RGB_TOTAL_BITS)




/* XPI_*: "dithering colormap indices" for commonly used colors */
#define XPI_RED    (RED_MAXVAL << RED_SHIFT)
#define XPI_GREEN  (GRN_MAXVAL << GRN_SHIFT)
#define XPI_BLUE   (BLU_MAXVAL)

#define XPI_YELLOW   (XPI_RED | XPI_GREEN)
#define XPI_MAGENTA  (XPI_RED | XPI_BLUE)
#define XPI_CYAN     (XPI_GREEN | XPI_BLUE)

#define XPI_BLACK  (0)
#define XPI_WHITE  (XPI_RED | XPI_GREEN | XPI_BLUE)

/* DXC_*: useful abbreviations for refering to common colors */
#define DXC_RED    &diw_xcolors[XPI_RED]
#define DXC_GREEN  &diw_xcolors[XPI_GREEN]
#define DXC_BLUE   &diw_xcolors[XPI_BLUE]
#define DXC_YELLOW &diw_xcolors[XPI_YELLOW]
#define DXC_BLACK  &diw_xcolors[XPI_BLACK]
#define DXC_WHITE  &diw_xcolors[XPI_WHITE]




#define NUM_DIW_MAPS 4
extern diw_map_t global_diw_map[];

extern XColor diw_xcolors[];


extern diw_map_t *diw_map_of_widget(Widget widget);

/* DrawMeshes: action to draw src, dst, and tween meshes of a widget */
extern void DrawMeshes(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms);

/* DrawAllMeshes: action to draw all meshes of all diw_maps */
extern void DrawAllMeshes(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms);

/* RefreshImage: action to draw image in diw_map image panel */
extern void RefreshImage(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms);

/* FakeExpose: action to fake an expose event to a diw_map widget */
extern void FakeExpose(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms);

/* FakeAllExpose: action to fake an expose event to all diw_map's */
extern void FakeAllExpose(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms);

/* ReditherImage: action to redither diw_map images into an image panel */
extern void
ReditherImage(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms);

/* ReditherAllImages: action to redither all diw_map images into image panel */
extern void ReditherAllImages(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms);

/* WarpImage: action to apply digital image warp algorithm to images */
extern void WarpImage(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms);

/* ChangeMeshLine: action to add or delete a mesh line */
extern void ChangeMeshLine(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms);

/* PickMeshpoint: action to grab a meshpoint */
extern void PickMeshpoint(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms);

/* UnpickMeshpoint: action to release meshpoint after drag */
extern void UnpickMeshpoint(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms);

/* DragMeshpoint: action to set meshpoint drag "to" location */
extern void DragMeshpoint(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms);

/* StartDragMeshpoint: action to set meshpoint drag "from" location */
extern void StartDragMeshpoint(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms);

/* dp_menu_cb: callback for diw_map properties menu entries */
extern void dp_menu_cb(Widget widget, XtPointer client_data, XtPointer call_data);

extern int allocate_x_images(Widget widget, Pixmap *pxP, XImage **xiP, int nx, int ny);

extern Widget create_diw_widget(Widget parent, diw_map_t *dmP, int width, int height);

extern void init_diw_stuff(Widget toplevel);




#endif /* _DIW_MAP__INCLUDED_ */
