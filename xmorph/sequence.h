/* sequence.h: routines to make a sequence of digital image warp frames
//
// Written and Copyright (C) 1994-1999 by Michael J. Gourlay
//
// NO WARRANTEES, EXPRESS OR IMPLIED.
*/
#ifndef _SEQUENCE__INCLUDED_
#define _SEQUENCE__INCLUDED_

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include "diw_map.h"
#include "RgbaImage.h"

extern void warp_rgba_image(RgbaImageT *inP, RgbaImageT *outP, double *sxP, double *syP, double *dxP, double *dyP, int nx, int ny);

extern void warp_sequence(diw_map_t *dmP, int steps, char *basename, int show, int preview);

extern void set_sequence_file_name_cb(Widget widget, XtPointer client_data, XtPointer call_data);
extern void set_sequence_file_name (char *fname);
extern char *get_sequence_file_name (void);

extern void set_sequence_num_frames_cb(Widget widget, XtPointer client_data, XtPointer call_data);
extern void set_sequence_num_frames (int frames);
extern int get_sequence_num_frames (void);

extern void warp_sequence_cb(Widget widget, XtPointer client_data, XtPointer call_data);

/* WarpSequence: action to generate a warp sequence */
extern void WarpSequence(Widget widget, XEvent *evt, String *prms, Cardinal *n_prms);

#endif
