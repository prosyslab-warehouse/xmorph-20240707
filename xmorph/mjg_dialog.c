/* my_dialog.c : My version of the Athena Dialog
//
// This dialog automatically adds the "okay" and "cancel" command
// buttons, registers callbacks for them to popdown the dialog, and adds
// the action to the Text widget so that pressing "Return" calls the
// "okay" callback.  Also, I provide for a callback that the "okay"
// callback calls after popping down the dialog.
//
// This translation should be added to the application's resources:
// Dialog.Text.translations:      #override <Key>Return: mjg_okay()
//
// The user only has to create the mjg_dialog by calling
// create_mjg_dialog() and setting the dialog_apdx.callback and
// dialog_apdx.button members.  The rest is taken care of by these
// routines.  The user may also set the dialog_apdx.client_data member
// to provide for client_data for the dialog_apdx.callback.
// That callback is also given the address of the dialog_apdx as the
// call_data.
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

#include "mjg_dialog.h"
#include "my_malloc.h"

/* --------------------------------------------------------------- */
/* Global structure for dialog appendices */
#define NUM_DIALOG_APDX 80
static int           mjg_dialog_count=0;
static dialog_apdx_t mjg_dialog[NUM_DIALOG_APDX];

/* --------------------------------------------------------------- */

/* dialog_apdx_of_widget : map a widget to its dialog_apdx
// incoming widget could be the dialog or any child of the dialog
*/
dialog_apdx_t *
dialog_apdx_of_widget(Widget w)
{
  int indx;

  for(indx=0; indx<mjg_dialog_count; indx++) {
    if(mjg_dialog[indx].dialog==w || XtParent(w)==mjg_dialog[indx].dialog) {
      return(&mjg_dialog[indx]);
    }
  }
  return(NULL);
}

/* =============================================================== */
/*                           Callbacks                             */
/* --------------------------------------------------------------- */

/* popup_dialog_cb : callback to popup dialog
*/
/*ARGSUSED*/
void
popup_dialog_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  dialog_apdx_t *daP = (dialog_apdx_t *)client_data;
  Position x, y;

  if(daP->button != NULL) {
    /* Find where to put the dialog */
    XtTranslateCoords(daP->button, 0, -30, &x, &y);
    XtVaSetValues(daP->shell, XtNx, x, XtNy, y, NULL);

    /* Desensitize the activation button */
    XtSetSensitive(daP->button, FALSE);
  }

  XtPopup(daP->shell, XtGrabNonexclusive);
}

/* --------------------------------------------------------------- */

/* cancel_dialog_cb : callback to cancel a dialog
*/
/*ARGSUSED*/
static void
cancel_dialog_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  dialog_apdx_t *daP = (dialog_apdx_t *)client_data;

  XtPopdown(daP->shell);
  if(daP->button != NULL) {
    XtSetSensitive(daP->button, TRUE);
  }
}

/* --------------------------------------------------------------- */

/* okay_dialog_cb : callback to accept a dialog
*/
/*ARGSUSED*/
static void
okay_dialog_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  dialog_apdx_t *daP = (dialog_apdx_t *)client_data;

  XtPopdown(daP->shell);
  if(daP->button != NULL) {
    XtSetSensitive(daP->button, TRUE);
  }

  if(daP->callback != NULL) {
    daP->callback(w, daP->client_data, (XtPointer)daP);
  }
}

/* =============================================================== */
/*                           Actions                               */
/* --------------------------------------------------------------- */

/* mjg_dialog_okay : action to accept dialog's value
*/
/*ARGSUSED*/
void
mjg_dialog_okay(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  dialog_apdx_t *daP;

  daP = dialog_apdx_of_widget(w);

  okay_dialog_cb(w, (XtPointer)daP, NULL);
}

/* =============================================================== */

/* mjg_dialog_translations : event-to-action mappings
// This translation should be added to the application's resources:
// Dialog.Text.translations:      #override <Key>Return: mjg_okay()
*/
char mjg_dialog_translations[] = "<Key>Return: mjg_okay()";

static XtActionsRec actions[] = {
  {"mjg_okay", mjg_dialog_okay}
};

/* --------------------------------------------------------------- */

/* create_mjg_dialog: create an mjg_dialog widget
//
// Side effect : allocates names for widgets
//
// Changes global variable mjg_dialog_count
//
// User must subsequently set the daP->button member to the command
// widget that launches this dialog.
*/
dialog_apdx_t *
create_mjg_dialog(Widget parent, char *name)
{
  dialog_apdx_t *daP;
  char *shell_name;
  char *dialog_name;

  if(mjg_dialog_count >= NUM_DIALOG_APDX) {
    fprintf(stderr, "create_mjg_dialog: exceeded max num of mjg_dialog's\n");
    return(NULL);
  }

  XtAppAddActions(XtWidgetToApplicationContext(parent), actions,
    XtNumber(actions));

  if((shell_name = MY_CALLOC(strlen(name)+strlen("mds_")+20, char))==NULL)
  {
    return(NULL);
  }
  if((dialog_name = MY_CALLOC(strlen(name)+strlen("md_")+20, char))==NULL)
  {
    return(NULL);
  }

  sprintf(shell_name, "mds_%s", name);
  sprintf(dialog_name, "md_%s", name);

  daP = &mjg_dialog[mjg_dialog_count]; mjg_dialog_count++;
  daP->client_data = NULL;
  daP->callback = NULL;
  daP->button = NULL;

  /* Create the Dialog shell widget */
  daP->shell = XtVaCreatePopupShell(shell_name,
    transientShellWidgetClass, parent, NULL);

    daP->dialog = XtVaCreateManagedWidget(dialog_name,
      dialogWidgetClass, daP->shell,
      NULL);

      /* Finding this sort of violates the dialog's privacy.  shhhh... */
      daP->text = XtNameToWidget(daP->dialog, "value");

      daP->okay = XtVaCreateManagedWidget("okay",
        commandWidgetClass, daP->dialog, NULL);

      XtAddCallback(daP->okay, XtNcallback, okay_dialog_cb, daP);

      daP->cancel = XtVaCreateManagedWidget("cancel",
        commandWidgetClass, daP->dialog, NULL);

      XtAddCallback(daP->cancel,XtNcallback,cancel_dialog_cb,daP);

#if 0
  FREE(shell_name);
  FREE(dialog_name);
#endif

  return(daP);
}
