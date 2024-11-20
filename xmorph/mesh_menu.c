/* mesh_menu.c : mesh menu widgets and callbacks
//
// Written and Copyright (C) 1994-1999 by Michael J. Gourlay
//
// PROVIDED AS IS.  NO WARRANTEES, EXPRESS OR IMPLIED.
//
// These routines are used for the X Window System only and are not
// needed for tkmorph.
*/

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/Form.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>

#include <stdio.h>
#include <stdlib.h>

#include "mesh_cb.h"
#include "mesh_menu.h"

/* =============================================================== */
/* --------------------------------------------------------------- */

/* create_mesh_menu: Create a mesh menu and its button
// returns the widget of the menu button
*/
/*ARGSUSED*/
Widget
create_mesh_menu(Widget parent, Widget toplevel, Widget left_w)
{
  Widget mesh_menu_button;
  Widget mesh_menu;
  Widget   mm_rsm_sme, mm_rdm_sme; /* reset src mesh, reset dst mesh */
  Widget   mm_msm_sme, mm_mdm_sme; /* functionalize src, dst mesh */

  /* ----------------------------- */
  /* Create a simple menu for Command */
  mesh_menu = XtVaCreatePopupShell("mesh_menu", simpleMenuWidgetClass,
    parent, NULL);

    /* - - - - - - - - - - - - - - - - - - - -  */
    /* Create a menu entry for "reset source mesh" */
    mm_rsm_sme = XtVaCreateManagedWidget("mm_rsm_sme", smeBSBObjectClass,
      mesh_menu, NULL);
    XtAddCallback(mm_rsm_sme, XtNcallback, reset_mesh_cb, (XtPointer)1);

    /* - - - - - - - - - - - - - - - - - - - -  */
    /* Create a menu entry for "reset destination mesh" */
    mm_rdm_sme = XtVaCreateManagedWidget("mm_rdm_sme", smeBSBObjectClass,
      mesh_menu, NULL);
    XtAddCallback(mm_rdm_sme, XtNcallback, reset_mesh_cb, (XtPointer)2);

    /* - - - - - - - - - - - - - -  */
    /* Create a menu item separator */
    XtVaCreateManagedWidget("mm_sep_sme", smeLineObjectClass,
      mesh_menu, NULL);

    /* - - - - - - - - - - - - - - - - - - - -  */
    /* Create a menu entry for "functionalize source mesh" */
    mm_msm_sme = XtVaCreateManagedWidget("mm_msm_sme", smeBSBObjectClass,
      mesh_menu, NULL);
    XtAddCallback(mm_msm_sme, XtNcallback, functionalize_mesh_cb, (XtPointer)1);

    /* - - - - - - - - - - - - - - - - - - - -  */
    /* Create a menu entry for "functionalize destination mesh" */
    mm_mdm_sme = XtVaCreateManagedWidget("mm_mdm_sme", smeBSBObjectClass,
      mesh_menu, NULL);
    XtAddCallback(mm_mdm_sme, XtNcallback, functionalize_mesh_cb, (XtPointer)2);

  /* ------------------------- */
  /* Create a mesh menu button */
  mesh_menu_button = XtVaCreateManagedWidget("mesh_menu_button",
    menuButtonWidgetClass, parent, XtNmenuName, "mesh_menu",
    XtNfromHoriz, left_w, NULL);

  return(mesh_menu_button);
}
