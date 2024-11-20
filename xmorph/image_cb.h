/* image_cb.h : Callbacks for Digital Image Warp image related widgets
//
// Written and Copyright (C) 1994-1999 by Michael J. Gourlay
//
// NO WARRANTEES, EXPRESS OR IMPLIED.
*/

#ifndef _IMAGE_CB_H__INCLUDED_
#define _IMAGE_CB_H__INCLUDED_

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

extern void load_img_cb(Widget w, XtPointer client_data, XtPointer call_data);
extern void save_img_cb(Widget w, XtPointer client_data, XtPointer call_data);

#ifdef NEED_GIMP
extern void open_gimp_dest_cb (Widget w, XtPointer client_data, XtPointer call_data);
#endif

#endif
