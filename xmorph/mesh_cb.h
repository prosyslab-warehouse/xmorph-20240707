/* mesh_cb.h : Callbacks for Digital Image Warp mesh related widgets
//
// Written and Copyright (C) 1994-1999 by Michael J. Gourlay
//
// NO WARRANTEES, EXPRESS OR IMPLIED.
*/

#ifndef _MESH_CB_H__INCLUDED_
#define _MESH_CB_H__INCLUDED_

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

extern void reset_mesh_cb(Widget w, XtPointer client_data, XtPointer call_data);

extern void functionalize_mesh_cb(Widget w, XtPointer client_data, XtPointer call_data);

extern void load_mesh_cb(Widget w, XtPointer client_data, XtPointer call_data);

extern void save_mesh_cb(Widget w, XtPointer client_data, XtPointer call_data);

#endif
