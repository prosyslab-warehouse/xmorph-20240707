/* mesh_menu.h : mesh menu widgets and callbacks
//
// Written and Copyright (C) 1994-1999 by Michael J. Gourlay
//
// Provided as is.  No warrantees, express or implied.
*/

#ifndef _MESH_MENU__INCLUDED_
#define _MESH_MENU__INCLUDED_

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

/* create_mesh_menu: Create a mesh menu and its button */
extern Widget create_mesh_menu(Widget parent, Widget toplevel, Widget left_w);

#endif
