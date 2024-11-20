/* warp_menu.h : warp menu widgets and callbacks
//
// Written and Copyright (C) 1994-1999 by Michael J. Gourlay
//
// NO WARRANTEES, EXPRESS OR IMPLIED.
*/
#ifndef _WARP_MENU__INCLUDED_
#define _WARP_MENU__INCLUDED_

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

/* =============================================================== */
/* --------------------------------------------------------------- */

/* create_warp_menu: Create a warp menu and its button */
extern Widget create_warp_menu(Widget parent, Widget toplevel, Widget left_w);

#endif
