/* file_menu.c : file menu widgets and callbacks for X
//
// Written and Copyright (C) 1994-1999 by Michael J. Gourlay
//
// PROVIDED AS IS.  NO WARRANTEES, EXPRESS OR IMPLIED.
//
// These routines are used for the X Window System only and are not
// needed for tkmorph.
*/

#ifdef GIMP
# define NEED_GIMP 1
# include <libgimp/gimp.h>
extern int plugin;
#endif

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>

#include <stdio.h>
#include <stdlib.h>

#include "xmorph.h"
#include "mjg_dialog.h"
#include "mesh_cb.h"
#include "image_cb.h"
#include "file_menu.h"

/* =============================================================== */
/* --------------------------------------------------------------- */

/* create_file_menu: Create a file menu and its button
** returns the widget of the menu button
*/
Widget
create_file_menu(Widget parent, Widget toplevel, Widget left_w)
{
  Widget file_menu_button;
  Widget file_menu;
  Widget   fm_osi_sme, fm_odi_sme, fm_osm_sme, fm_odm_sme;
  Widget   fm_ssm_sme, fm_sdm_sme;
  Widget   fm_quit_sme;
  dialog_apdx_t *fm_osi_daP, *fm_odi_daP;
  dialog_apdx_t *fm_osm_daP, *fm_odm_daP;
  dialog_apdx_t *fm_ssm_daP, *fm_sdm_daP;

  /* ========================= */
  /* Create the Dialog widgets */
  /*  - - - - - - - - - - - - - - - - -  */
#ifdef NEED_GIMP
  if (plugin)
    fm_osi_daP = 0;
  else
#endif
  {
    /* Create the Open Source Image dialog */
    fm_osi_daP = create_mjg_dialog(toplevel, "osi");
    fm_osi_daP->client_data = (XtPointer)1;
    fm_osi_daP->callback = load_img_cb;
  }

#ifdef NEED_GIMP
  if (plugin)
    fm_odi_daP = 0;
  else
#endif
  /* - - - - - - - - - - - - - - - - - - - -  */
  {
    /* Create the Open Destination Image dialog */
    fm_odi_daP = create_mjg_dialog(toplevel, "odi");
    fm_odi_daP->client_data = (XtPointer)2;
    fm_odi_daP->callback = load_img_cb;
  }

  /*  - - - - - - - - - - - - - - - - -  */
  /* Create the Open Source Mesh dialog */
  fm_osm_daP = create_mjg_dialog(toplevel, "osm");
  fm_osm_daP->client_data = (XtPointer)1;
  fm_osm_daP->callback = load_mesh_cb;

  /* - - - - - - - - - - - - - - - - - - - -  */
  /* Create the Open Destination Mesh dialog */
  fm_odm_daP = create_mjg_dialog(toplevel, "odm");
  fm_odm_daP->client_data = (XtPointer)2;
  fm_odm_daP->callback = load_mesh_cb;

  /*  - - - - - - - - - - - - - - - - -  */
  /* Create the Save Source Mesh dialog */
  fm_ssm_daP = create_mjg_dialog(toplevel, "ssm");
  fm_ssm_daP->client_data = (XtPointer)1;
  fm_ssm_daP->callback = save_mesh_cb;

  /*  - - - - - - - - - - - - - - - - -  */
  /* Create the Save Destination Mesh dialog */
  fm_sdm_daP = create_mjg_dialog(toplevel, "sdm");
  fm_sdm_daP->client_data = (XtPointer)2;
  fm_sdm_daP->callback = save_mesh_cb;


  /* ----------------------------- */
  /* Create a simple menu for File */
  file_menu = XtVaCreatePopupShell("file_menu", simpleMenuWidgetClass, parent,
    NULL);

    /* - - - - - - - - - - - - - - - - - - - -  */
#ifdef NEED_GIMP
  if (!plugin)
#endif
  {
    /* Create a menu entry for "open source image" */
    fm_osi_sme = XtVaCreateManagedWidget("fm_osi_sme", smeBSBObjectClass,
      file_menu, NULL);
    XtAddCallback(fm_osi_sme, XtNcallback, popup_dialog_cb, fm_osi_daP);

    fm_osi_daP->button = fm_osi_sme;
  }

    /* - - - - - - - - - - - - - - - - - - - -  */
#ifdef NEED_GIMP
  if (!plugin)
#endif /* NEED_GIMP */
  {
    /* Create a menu entry for "open destination image" */
    fm_odi_sme = XtVaCreateManagedWidget("fm_odi_sme", smeBSBObjectClass,
      file_menu, NULL);
    XtAddCallback(fm_odi_sme, XtNcallback, popup_dialog_cb, fm_odi_daP);

    fm_odi_daP->button = fm_odi_sme;
  }

    /* - - - - - - - - - - - - - -  */
    /* Create a menu item separator */
    XtVaCreateManagedWidget("fm_sep_sme", smeLineObjectClass,
      file_menu, NULL);

    /* - - - - - - - - - - - - - - - - - - - -  */
    /* Create a menu entry for "open source mesh" */
    fm_osm_sme = XtVaCreateManagedWidget("fm_osm_sme", smeBSBObjectClass,
      file_menu, NULL);
    XtAddCallback(fm_osm_sme, XtNcallback, popup_dialog_cb, fm_osm_daP);

    fm_osm_daP->button = fm_osm_sme;

    /* - - - - - - - - - - - - - - - - - - - -  */
    /* Create a menu entry for "open destination mesh" */
    fm_odm_sme = XtVaCreateManagedWidget("fm_odm_sme", smeBSBObjectClass,
      file_menu, NULL);
    XtAddCallback(fm_odm_sme, XtNcallback, popup_dialog_cb, fm_odm_daP);

    fm_odm_daP->button = fm_odm_sme;

    /* - - - - - - - - - - - - - -  */
    /* Create a menu item separator */
    XtVaCreateManagedWidget("fm_sep_sme", smeLineObjectClass, file_menu, NULL);

    /* - - - - - - - - - - - - - - - - - - - -  */
    /* Create a menu entry for "save source mesh" */
    fm_ssm_sme = XtVaCreateManagedWidget("fm_ssm_sme", smeBSBObjectClass,
      file_menu, NULL);
    XtAddCallback(fm_ssm_sme, XtNcallback, popup_dialog_cb, fm_ssm_daP);

    fm_ssm_daP->button = fm_ssm_sme;

    /* - - - - - - - - - - - - - - - - - - - -  */
    /* Create a menu entry for "save destination mesh" */
    fm_sdm_sme = XtVaCreateManagedWidget("fm_sdm_sme", smeBSBObjectClass,
      file_menu, NULL);
    XtAddCallback(fm_sdm_sme, XtNcallback, popup_dialog_cb, fm_sdm_daP);

    fm_sdm_daP->button = fm_sdm_sme;

    /* - - - - - - - - - - - - - -  */
    /* Create a menu item separator */
    XtVaCreateManagedWidget("fm_sep_sme", smeLineObjectClass, file_menu, NULL);

    /* - - - - - - - - - - - - - -  */
    /* Create a menu item separator */
    XtVaCreateManagedWidget("fm_sep_sme", smeLineObjectClass, file_menu, NULL);

    /* - - - - - - - - - - - - - - - - - - - -  */
    /* Create a menu entry for "quit" */
    fm_quit_sme = XtVaCreateManagedWidget("fm_quit_sme", smeBSBObjectClass,
      file_menu, NULL);
    XtAddCallback(fm_quit_sme, XtNcallback, exit_callback, NULL);

  /* ------------------------- */
  /* Create a file menu button */
  if(left_w != NULL) {
    file_menu_button = XtVaCreateManagedWidget("file_menu_button",
      menuButtonWidgetClass, parent, XtNmenuName, "file_menu",
      XtNfromHoriz, left_w, NULL);
  } else {
    file_menu_button = XtVaCreateManagedWidget("file_menu_button",
      menuButtonWidgetClass, parent, XtNmenuName, "file_menu", NULL);
  }

  return(file_menu_button);
}


#ifdef NEED_GIMP
static char*
my_base_name (char *str)
{
  char *t;

  t = strrchr (str, '/');
  if (!t)
    return str;
  return t+1;
}

/* create_dest_menu: create an open destination image menu */
Widget
create_dest_menu(Widget parent, Widget toplevel, Widget left_w)
{
  Widget dest_menu_button, dest_menu, menuitem;

  char *name;
  char *image_label;
  char *label;
  gint32 *images;
  gint32 *layers;
  gint32 *channels;
  int nimages;
  int nlayers;
  int nchannels;
  int i, j, k;

  /* FIXME: how can we dynamically update this menu? */

  /* Open destination image menu that asks for a layer. */
  dest_menu = XtVaCreatePopupShell ("dest_menu", simpleMenuWidgetClass,
                                      parent, NULL);

  /* Add an entry for each of the drawables we have.  Shamelessly
     stolen from libgimp/gimpmenu.c (gimp_drawable_menu_new) */

  images = gimp_query_images (&nimages);
  for (i = 0, k = 0; i < nimages; i++)
    /* FIXME: check any constraints. */
    if (1)
      {
        name = gimp_image_get_filename (images[i]);
        image_label = g_new (char, strlen (name) + 16);
        sprintf (image_label, "%s-%d", my_base_name (name), images[i]);
        g_free (name);

        layers = gimp_image_get_layers (images[i], &nlayers);
        for (j = 0; j < nlayers; j++)
          /* FIXME: check any constraints. */
          if (1)
            {
              name = gimp_layer_get_name (layers[j]);
              label = g_new (char, strlen (image_label) + strlen (name) + 2);
              sprintf (label, "%s/%s", image_label, name);
              g_free (name);

              menuitem = XtVaCreateManagedWidget (label, smeBSBObjectClass,
                                               dest_menu, NULL);
              XtAddCallback (menuitem, XtNcallback, open_gimp_dest_cb,
                             (XtPointer)layers[j]);

              g_free (label);
              k += 1;
            }

        channels = gimp_image_get_channels (images[i], &nchannels);
        for (j = 0; j < nchannels; j++)
          /* FIXME: check any constraints. */
          if (1)
            {
              name = gimp_channel_get_name (channels[j]);
              label = g_new (char, strlen (image_label) + strlen (name) + 2);
              sprintf (label, "%s/%s", image_label, name);
              g_free (name);

              menuitem = XtVaCreateManagedWidget (label, smeBSBObjectClass,
                                               dest_menu, NULL);
              XtAddCallback (menuitem, XtNcallback, open_gimp_dest_cb,
                             (XtPointer)channels[j]);

              g_free (label);
              k += 1;
            }

        g_free (image_label);
      }
  g_free (images);

  if (k == 0)
    {
      menuitem = XtVaCreateManagedWidget ("none", smeBSBObjectClass,
                                             dest_menu, NULL);
      /* No callback, since there is no drawable. */
    }

  if(left_w != NULL) {
    dest_menu_button = XtVaCreateManagedWidget("dest_menu_button",
      menuButtonWidgetClass, parent, XtNmenuName, "dest_menu",
      XtNfromHoriz, left_w, NULL);
  } else {
    dest_menu_button = XtVaCreateManagedWidget("dest_menu_button",
      menuButtonWidgetClass, parent, XtNmenuName, "dest_menu", NULL);
  }

  return(dest_menu_button);
}
#endif
