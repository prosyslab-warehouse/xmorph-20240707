/* my_dialog.h : My version of the Athena Dialog
//
// Written and Copyright (C) 1994-1999 by Michael J. Gourlay
//
// NO WARRANTEES, EXPRESS OR IMPLIED.
*/


#ifndef _MJG_DIALOG_H__INCLUDED_
#define _MJG_DIALOG_H__INCLUDED_

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Shell.h>

#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Text.h>
#include <X11/Xaw/Command.h>

/* The Athena widgets are particularly lame about dialogs
** so I add this appendix of members to their dialog widget
** You'd think that people from MIT would do better.
*/
typedef struct dialog_apdx_ {
  Widget    toplevel;
  Widget    button;
  Widget    shell;
  Widget    dialog;
  Widget    text;
  Widget    okay;
  Widget    cancel;
  void      (*callback)(Widget w, XtPointer client_data, XtPointer call_data);
  XtPointer client_data;
  int       type;
} dialog_apdx_t;

extern dialog_apdx_t *dialog_apdx_of_widget(Widget w);

/* popup_dialog_cb : callback to popup dialog */
extern void popup_dialog_cb(Widget w, XtPointer client_data, XtPointer call_data);

extern void mjg_dialog_okay(Widget w, XEvent *event, String *params, Cardinal *num_params);

extern dialog_apdx_t *create_mjg_dialog(Widget parent, char *name);

#endif /* _MJG_DIALOG_H__INCLUDED_ */
