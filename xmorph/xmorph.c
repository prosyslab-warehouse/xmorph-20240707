/* xmorph.c : Digital Image Warping GUI for X Window System
//
// A graphical user interface to a mesh warping algorithm
//
// Some code for GIMP plug-in is also in this file.
//


   Written and Copyright (C) 1994-1999 by Michael J. Gourlay

This file is part of Xmorph.

Xmorph is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

Xmorph is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/




#include <stdio.h>
#include <stdlib.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/Form.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Scrollbar.h>




#include "diw_map.h"
#include "RgbaImage.h"
#include "image_diw.h"
#include "sequence.h"
#include "file_menu.h"
#include "mesh_menu.h"
#include "warp_menu.h"
#include "help_menu.h"
#include "my_malloc.h"

#include "main.h"

#include "xmorph.h"




#ifdef NEED_GIMP
# define PLUG_IN_NAME "xmorph"
# include <libgimp/gimp.h>

static void query (void);
static void run (char *name, int nparams, GParam * param,
                 int *nreturn_vals, GParam ** return_vals);

GPlugInInfo PLUG_IN_INFO =
{
  NULL,                         /* init_proc */
  NULL,                         /* quit_proc */
  query,                        /* query_proc */
  run,                          /* run_proc */
};

int plugin = FALSE;

#include <setjmp.h>

static jmp_buf exit_env;

#endif /* NEED_GIMP */




/* actions: action-to-function mappings
// determines the behaviour of this application
*/
static XtActionsRec actions[] = {
  {"refresh", RefreshImage},
  {"draw_meshes", DrawMeshes},
  {"draw_all_meshes", DrawAllMeshes},
  {"redither", ReditherImage},
  {"redither_all", ReditherAllImages},
  {"warp", WarpImage},
  {"change_mesh", ChangeMeshLine},
  {"pick", PickMeshpoint},
  {"unpick", UnpickMeshpoint},
  {"drag", DragMeshpoint},
  {"start_drag", StartDragMeshpoint},
  {"warp_sequence", WarpSequence},
};




/* fallback_resources: default application resources
// determines the look of this application
*/
String fallback_resources[] = {
  /* Application looks */
  "XMorph*font:                  -*-helvetica-medium-r-normal-*-14-*-*-*-*-*-*",
  "XMorph*foreground:                    MidnightBlue",
  "XMorph*background:                    LightGray",
  "XMorph*borderColor:                   DimGray",

  /* Dialog look and feel */
  "XMorph*Dialog.label.font:       -*-helvetica-bold-r-normal-*-14-*-*-*-*-*-*",
  "XMorph*Dialog.value:                    ",
  "XMorph*Dialog.Command.foreground:       black",
  "XMorph*Dialog.cancel.label:             Cancel",
  "XMorph*Dialog.okay.label:               Okay",
  "XMorph*Dialog.Text.translations:        #override <Key>Return: mjg_okay()",

  /* Menu looks */
  "XMorph*MenuButton.font:       -*-helvetica-bold-o-normal-*-14-*-*-*-*-*-*",
  "XMorph*SimpleMenu*font:       -*-helvetica-bold-o-normal-*-14-*-*-*-*-*-*",
  "XMorph*SimpleMenu*leftMargin:         24",
  "XMorph*SimpleMenu*rightMargin:        24",

  /* File titlebar menu */
  "XMorph*file_menu_button.label:        File",
  "XMorph*file_menu.fm_osi_sme.label:    Open source image...",
  "XMorph*file_menu.fm_odi_sme.label:    Open destination image...",
  "XMorph*file_menu.fm_osm_sme.label:    Open source mesh...",
  "XMorph*file_menu.fm_odm_sme.label:    Open destination mesh...",
  "XMorph*file_menu.fm_ssi_sme.label:    Save source image...",
  "XMorph*file_menu.fm_sdi_sme.label:    Save destination image...",
  "XMorph*file_menu.fm_ssm_sme.label:    Save source mesh...",
  "XMorph*file_menu.fm_sdm_sme.label:    Save destination mesh...",
  "XMorph*file_menu.fm_quit_sme.label:   Quit",
  "XMorph*TransientShell.md_osi.label:   Load source image from filename:",
  "XMorph*mds_osi.title:                 Load source image from filename:",
  "XMorph*TransientShell.md_odi.label:   Load destination image from filename:",
  "XMorph*mds_odi.title:                 Load destination image from filename:",
  "XMorph*TransientShell.md_osm.label:   Load source mesh from filename:",
  "XMorph*mds_osm.title:                 Load source mesh from filename:",
  "XMorph*TransientShell.md_odm.label:   Load destination mesh from filename:",
  "XMorph*mds_odm.title:                 Load destination mesh from filename:",
  "XMorph*TransientShell.md_ssm.label:   Save source mesh to filename:",
  "XMorph*mds_ssm.title:                 Save source mesh to filename:",
  "XMorph*TransientShell.md_sdm.label:   Save destination mesh to filename:",
  "XMorph*mds_sdm.title:                 Save destination mesh to filename:",
  "XMorph*TransientShell.md_si.label:    Save image to filename:",
  "XMorph*mds_si.title:                  Save image to filename:",

#ifdef NEED_GIMP
  /* Destination titlebar menu */
  "XMorph*dest_menu_button.label:        Destination",
#endif

  /* Mesh titlebar menu */
  "XMorph*mesh_menu_button.label:        Mesh",
  "XMorph*mesh_menu.mm_rsm_sme.label:    Reset source mesh",
  "XMorph*mesh_menu.mm_rdm_sme.label:    Reset destination mesh",
  "XMorph*mesh_menu.mm_msm_sme.label:    Functionalize source mesh",
  "XMorph*mesh_menu.mm_mdm_sme.label:    Functionalize destination mesh",

  /* Warp titlebar menu */
  "XMorph*warp_menu_button.label:        Morph Sequence",
  "XMorph*warp_menu.wm_ssfn_sme.label:   Set sequence name...",
  "XMorph*warp_menu.wm_ssns_sme.label:   Set sequence number of steps...",
  "XMorph*warp_menu.wm_seqp_sme.label:   Preview warp sequence",
  "XMorph*warp_menu.wm_seq_sme.label:    Warp sequence",
  "XMorph*TransientShell.md_ssfn.label:  Warp Sequence filename:",
  "XMorph*mds_ssfn.title:                Warp Sequence filename:",
  "XMorph*TransientShell.md_ssns.label:  Number of warp frames:",
  "XMorph*mds_ssns.title:                Number of warp frames:",
  "XMorph*TransientShell.md_ssfn.value:  warp",
  "XMorph*TransientShell.md_ssns.value:  30",


  /* Help titlebar menu */
  "XMorph*help_menu_button.label:          Help",
  "XMorph*help_menu_button.horizDistance:  100",
  "XMorph*help_menu.hm_about_sme.label:    About...",
  "XMorph*help_menu.hm_mmp_sme.label:      Manipulating the mesh...",
  "XMorph*help_menu.hm_file_sme.label:     File menu help...",
  "XMorph*help_menu.hm_mesh_sme.label:     Mesh menu help...",
  "XMorph*help_menu.hm_warp_sme.label:     Morph Sequence menu help...",
  "XMorph*help_menu.hm_dpm_sme.label:      DIW Properties menu help...",
  "XMorph*help_menu.hm_dcm_sme.label:      DIW Commands menu help...",
  "XMorph*help_menu.hm_quit_sme.label:     Quit XMorph",

  "XMorph*TransientShell.md_h_about.Text*font: -*-helvetica-medium-r-normal-*-18-*-*-*-*-*-*",
  "XMorph*TransientShell.md_h_about.label:                About XMorph",
  "XMorph*mds_h_about.title:                              About XMorph",
  "XMorph*TransientShell.md_h_about.Text.width:           640",
  "XMorph*TransientShell.md_h_about.Text.height:          480",
  "XMorph*TransientShell.md_h_about.Text.scrollHoriontal: whenneeded",
  "XMorph*TransientShell.md_h_about.okay.label:           Dismiss",


  "XMorph*TransientShell.md_h_mmp.Text*font: -*-helvetica-medium-r-normal-*-18-*-*-*-*-*-*",
  "XMorph*TransientShell.md_h_mmp.label:                Manipulating the mesh",
  "XMorph*mds_h_mmp.title:                              Manipulating the mesh",
  "XMorph*TransientShell.md_h_mmp.Text.width:           640",
  "XMorph*TransientShell.md_h_mmp.Text.height:          480",
  "XMorph*TransientShell.md_h_mmp.Text.scrollHoriontal: whenneeded",
  "XMorph*TransientShell.md_h_mmp.okay.label:           Dismiss",


  "XMorph*TransientShell.md_h_file.Text*font: -*-helvetica-medium-r-normal-*-18-*-*-*-*-*-*",
  "XMorph*TransientShell.md_h_file.label:                File menu Help",
  "XMorph*mds_h_file.title:                              File menu Help",
  "XMorph*TransientShell.md_h_file.Text.width:           640",
  "XMorph*TransientShell.md_h_file.Text.height:          480",
  "XMorph*TransientShell.md_h_file.Text.scrollHoriontal: whenneeded",
  "XMorph*TransientShell.md_h_file.okay.label:           Dismiss",

  "XMorph*TransientShell.md_h_mesh.Text*font: -*-helvetica-medium-r-normal-*-18-*-*-*-*-*-*",
  "XMorph*TransientShell.md_h_mesh.label:                Mesh menu Help",
  "XMorph*mds_h_mesh.title:                              Mesh menu Help",
  "XMorph*TransientShell.md_h_mesh.Text.width:           640",
  "XMorph*TransientShell.md_h_mesh.Text.height:          480",
  "XMorph*TransientShell.md_h_mesh.Text.scrollHoriontal: whenneeded",
  "XMorph*TransientShell.md_h_mesh.okay.label:           Dismiss",

  "XMorph*TransientShell.md_h_morph.Text*font: -*-helvetica-medium-r-normal-*-18-*-*-*-*-*-*",
  "XMorph*TransientShell.md_h_morph.label:              Morph Sequence menu Help",
  "XMorph*mds_h_morph.title:                            Morph Sequence menu Help",
  "XMorph*TransientShell.md_h_morph.Text.width:         640",
  "XMorph*TransientShell.md_h_morph.Text.height:        480",
  "XMorph*TransientShell.md_h_morph.Text.scrollHoriontal: whenneeded",
  "XMorph*TransientShell.md_h_morph.okay.label:         Dismiss",

  "XMorph*TransientShell.md_h_dpm.Text*font: -*-helvetica-medium-r-normal-*-18-*-*-*-*-*-*",
  "XMorph*TransientShell.md_h_dpm.label:                DIW Properties menu Help",
  "XMorph*mds_h_dpm.title:                              DIW Properties menu Help",
  "XMorph*TransientShell.md_h_dpm.Text.width:           640",
  "XMorph*TransientShell.md_h_dpm.Text.height:          480",
  "XMorph*TransientShell.md_h_dpm.Text.scrollHoriontal: whenneeded",
  "XMorph*TransientShell.md_h_dpm.okay.label:           Dismiss",

  "XMorph*TransientShell.md_h_dcm.Text*font: -*-helvetica-medium-r-normal-*-18-*-*-*-*-*-*",
  "XMorph*TransientShell.md_h_dcm.label:                DIW Commands menu Help",
  "XMorph*mds_h_dcm.title:                              DIW Commands menu Help",
  "XMorph*TransientShell.md_h_dcm.Text.width:           640",
  "XMorph*TransientShell.md_h_dcm.Text.height:          480",
  "XMorph*TransientShell.md_h_dcm.Text.scrollHoriontal: whenneeded",
  "XMorph*TransientShell.md_h_dcm.okay.label:           Dismiss",

  /* ----------------- */
  /* DIW box resources */
  "XMorph*diw_box.SimpleMenu.background:         white",

  /* Viewport scrollbars */
  "XMorph*Viewport.allowHoriz:           True",
  "XMorph*Viewport.allowVert:            True",

  /* warp and dissolve scrollbars */
  "XMorph*mesh_label.label:              warp:",
  "XMorph*source_mesh.foreground:        dark green",
  "XMorph*source_mesh.label:             source mesh",
#ifdef RED_GREEN_COLOR_BLIND
  "XMorph*destination_mesh.foreground:   blue",
#else
  "XMorph*destination_mesh.foreground:   red",
#endif
  "XMorph*destination_mesh.label:        destination mesh",
  "XMorph*scrollbar_mesh.foreground:     yellow",
  "XMorph*scrollbar_mesh.orientation:    horizontal",
  "XMorph*scrollbar_image.orientation:   horizontal",
  "XMorph*Scrollbar.foreground:          LightGray",
  "XMorph*Scrollbar.background:          DimGray",
  "XMorph*image_label.label:             dissolve:",
  "XMorph*source_image.label:            source image",
  "XMorph*destination_image.label:       destination image",

  /* DIW Properties menu */
  "XMorph*diw_prop_menu_button.label:            Properties",
  "XMorph*diw_box.SimpleMenu.smp_sme.label:      Show source mesh points",
  "XMorph*diw_box.SimpleMenu.sml_sme.label:      Show source mesh lines",
  "XMorph*diw_box.SimpleMenu.dmp_sme.label:      Show destination mesh points",
  "XMorph*diw_box.SimpleMenu.dml_sme.label:      Show destination mesh lines",
  "XMorph*diw_box.SimpleMenu.tmp_sme.label:      Show tween mesh points",
  "XMorph*diw_box.SimpleMenu.tml_sme.label:      Show tween mesh lines",
  "XMorph*diw_box.SimpleMenu.dim_sme.label:      Dim Image",

  /* DIW Commands menu */
  "XMorph*diw_cmd_menu_button.label:             Commands",
  "XMorph*diw_box.SimpleMenu.si_sme.label:       Save Image...",
  "XMorph*diw_box.SimpleMenu.warp_sme.label:     Warp Image",
  "XMorph*diw_box.SimpleMenu.redither_sme.label: Redither Image",

  NULL
};




/* initialize_application: lots of once-only initialization
// Build the widget heirarchy and initializes all GUI variables
*/
XtAppContext
initialize_application(int width, int height,
                       char *srcmesh, char *dstmesh,
                       int *argc, char **argv, int show)
{
  XtAppContext app;
  Widget         toplevel;
  Widget           form;
  Widget             file_menu_button;
  Widget             dest_menu_button;
  Widget             mesh_menu_button;
  Widget             warp_menu_button;
  Widget             help_menu_button;
  Widget             diw_box;
  Widget               diw_widget1, diw_widget2;
  Arg args[25];
  int argn;

  Display *display;

  /* initialize the DIW maps */
  for(argn=0; argn<NUM_DIW_MAPS; argn++) {
    global_diw_map[argn].width = 0;
    global_diw_map[argn].height = 0;
  }

  /* Create the application context */
  toplevel = XtAppInitialize(&app, "XMorph",
        NULL, 0, argc, argv, fallback_resources, NULL, 0);

  XtAppAddActions(app, actions, XtNumber(actions));

  display = XtDisplay(toplevel);

  XSynchronize(display, 1); /* for debugging */

  /* ---------------------------------------------------- */
  /* Create the Form widget for the buttons and diw_map's */
  form = XtVaCreateManagedWidget("form", formWidgetClass, toplevel, NULL);

  /* ------------------------------------------------------------ */
  /* Create the file menu, its button, and register its callbacks */
  file_menu_button = create_file_menu(form, toplevel, NULL);

  /* If we have the GIMP, then create an `open destination image' menu. */
#ifdef NEED_GIMP
  if (plugin)
    dest_menu_button = create_dest_menu (form, toplevel, file_menu_button);
  else
#endif /* !NEED_GIMP */
    dest_menu_button = file_menu_button;

  /* ------------------------------------------------------------ */
  /* Create the mesh menu, its button, and register its callbacks */
  mesh_menu_button = create_mesh_menu(form, toplevel, dest_menu_button);

  /* ------------------------------------------------------------ */
  /* Create the warp menu, its button, and register its callbacks */
  warp_menu_button = create_warp_menu(form, toplevel, mesh_menu_button);

  /* ------------------------------------------------------------ */
  /* Create the help menu, its button, and register its callbacks */
  help_menu_button = create_help_menu(form, toplevel, warp_menu_button);
  XtVaSetValues(help_menu_button, XtNright, XtChainRight, NULL);

  /* ================================== */
  /* Create the box for the DIW widgets */
  argn=0;
  XtSetArg(args[argn], XtNorientation, XtorientHorizontal); argn++;
  XtSetArg(args[argn], XtNfromVert, file_menu_button); argn++;
  /* box changed to paned: WA (MJG 13sep95) */
  diw_box = XtCreateManagedWidget("diw_box", panedWidgetClass, form, args, argn);

  /* Initialize GC's, cursors, bitmaps, palette... */
  init_diw_stuff(toplevel);

    /* === Create the DIW panels === */
    /* --- Make the first diw_map --- */
    global_diw_map[0].mesh_t = 0.0;
    global_diw_map[0].img_t = 0.0; /* for the scrollbar Thumb */
    diw_widget1 = create_diw_widget(diw_box, &global_diw_map[0], width, height);

    /* Turn on the source and tween mesh on the first diw_map */
    dp_menu_cb(global_diw_map[0].sml, (XtPointer)&global_diw_map[0], NULL);
    dp_menu_cb(global_diw_map[0].smp, (XtPointer)&global_diw_map[0], NULL);
    dp_menu_cb(global_diw_map[0].tml, (XtPointer)&global_diw_map[0], NULL);

    /* --- Make the second diw_map --- */
    global_diw_map[1].mesh_t = 1.0;
    global_diw_map[1].img_t = 1.0; /* for the scrollbar Thumb */
    diw_widget2 = create_diw_widget(diw_box, &global_diw_map[1], width, height);

    /* These arrays are not used, so I'll free them */
    FREE(global_diw_map[1].mesh_src.x);
    FREE(global_diw_map[1].mesh_src.y);
    FREE(global_diw_map[1].mesh_dst.x);
    FREE(global_diw_map[1].mesh_dst.y);

    /* Make the 2 diw_maps share the same meshes */
    /*diw_map_of_widget(NULL);*/
    global_diw_map[1].mesh_src.x = global_diw_map[0].mesh_src.x;
    global_diw_map[1].mesh_src.y = global_diw_map[0].mesh_src.y;
    global_diw_map[1].mesh_dst.x = global_diw_map[0].mesh_dst.x;
    global_diw_map[1].mesh_dst.y = global_diw_map[0].mesh_dst.y;

    /* Turn on the destination and tween mesh on the second diw_map */
    dp_menu_cb(global_diw_map[1].dml, (XtPointer)&global_diw_map[1], NULL);
    dp_menu_cb(global_diw_map[1].dmp, (XtPointer)&global_diw_map[1], NULL);
    dp_menu_cb(global_diw_map[1].tml, (XtPointer)&global_diw_map[1], NULL);

    /* --- Read in any argument-specified meshes --- */
    if (srcmesh) {
      if (meshRead(&global_diw_map[0].mesh_src, srcmesh)) {
        fprintf(stderr, "%s: meshRead %s failed\n", argv[0], srcmesh);
        exit (1);
      }
#ifdef GIMP
      set_src_mesh_name (srcmesh);
#endif
      meshMatch(&global_diw_map[0].mesh_dst, &global_diw_map[0].mesh_src);
    }

    if (dstmesh) {
      if (meshRead(&global_diw_map[0].mesh_dst, dstmesh)) {
        fprintf(stderr, "%s: meshRead %s failed\n", argv[0], dstmesh);
        exit (1);
      }
#ifdef GIMP
      set_dst_mesh_name (dstmesh);
#endif
      meshMatch(&global_diw_map[0].mesh_src, &global_diw_map[0].mesh_dst);
    }

    /* Propagate the mesh changes to all of the diw_maps */
    if (srcmesh || dstmesh)
      diw_map_of_widget(NULL);

  /* --------------------- */
  if (show)
    XtRealizeWidget(toplevel);

  /* Copy "original" images into the diw_map image spaces */
  reset_images(1);
  reset_images(2);

  /* Send the diw_map images to the X server */
  if (show)
    {
      ReditherImage(diw_widget1, NULL, NULL, NULL);
      ReditherImage(diw_widget2, NULL, NULL, NULL);
    }

  /* X Toolkit Main Loop (should be in main()) */
  return(app);
}




/*ARGSUSED*/
void
exit_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
#ifdef DEBUG
  fprintf(stderr, "exit_callback: expect 48 blocks numbered 0 to 47\n");
#endif

#ifdef NEED_GIMP
  if (plugin)
    {
      longjmp (exit_env, 1);

      /* Only reached if longjmp fails. */
      gimp_quit ();
    }
#endif
  exit(0);
}




#ifdef NEED_GIMP

/* Source drawable. */
GDrawable *drawable = NULL;

typedef struct {
  gint32 dst;
  gint32 steps;
  guchar srcmesh[512];
  guchar dstmesh[512];
  guchar basename[512];
} xmorph_vals_t;

static xmorph_vals_t xmvals = {
  -1, /* dst */
  3, /* steps */
  "", /* srcmesh */
  "", /* dstmesh */
  "warp", /* basename */
};




/* Function so that we can keep track of our destination image ID. */
void
set_dest_image_id (gint32 id)
{
  xmvals.dst = id;
}




/* NAME
//   query: typical GIMP plugin query procedure
//
//
// DESCRIPTION
//   Every GIMP plugin needs a procedure like this to be referenced by
//   the PLUGIN_INFO structure above.  This function declares to the GIMP
//   which procedures this plugin implements, in our case, ``xmorph''.
//   This procedure is only run by the GIMP during startup if the plugin
//   binary changed since the last time the GIMP was run.
//
//
// SEE ALSO
//   plug-ins/bumpmap.c from the latest GIMP distribution
//   (http://www.gimp.org/).
*/
static void
query (void)
{
  static GParamDef args[] =
    {
      { PARAM_INT32, "run_mode", "Interactive, non-interactive"},
      { PARAM_IMAGE, "image", "Output image" },
      { PARAM_DRAWABLE, "src", "Source drawable"},
      { PARAM_DRAWABLE, "dst", "Destination drawable"},
      { PARAM_INT32, "steps", "Number of steps"},
      { PARAM_STRING, "srcmesh", "Source mesh filename"},
      { PARAM_STRING, "dstmesh", "Destination mesh filename"},
      { PARAM_STRING, "basename", "Basename of output layers"},
    };

  static GParamDef *return_vals = NULL;
  static int nargs = sizeof (args) / sizeof (args[0]);
  static int nreturn_vals = 0;

  gimp_install_procedure (
      PLUG_IN_NAME,
      "Morph from one image to another",
      "This is a plug-in version of the Xmorph program by Michael J. \
Gourlay.  The SRC drawable is morphed into the DST drawable in STEPS steps.  \
Output drawables are created as new layers in IMAGE.  Optional parameters \
are SRCMESH (the source mesh filename), DSTMESH (the destination mesh \
filename), and BASENAME (a string specifying a prefix for created layers).",
      "Gordon Matzigkeit",
      "Gordon Matzigkeit",
      "1997-08-31",
      "<Image>/Filters/Effects/Morph",
      "RGB*, GRAY*",
      PROC_PLUG_IN,
      nargs, nreturn_vals,
      args, return_vals);
}




static char *fakeargv[2] = {PLUG_IN_NAME, 0};
static int fakeargc = 0;




/* NAME
//   load_images: convenience function to retrieve images from the GIMP
//
//
// DESCRIPTION
//   This function just loads the specified images from the GIMP.
//
//
// RETURN VALUES
//   Returns zero on success.
*/
static int
load_images ()
{
  RgbaImageT *src, *dst;

  /* Load the source image. */
  src = &orig_image[0];
  src->ri = src->gi = src->bi = src->ai = NULL;
  if (rgbaImageUnGIMP (src, drawable))
    return 1;

  /* Load the destination image */
  dst = &orig_image[1];
  dst->ri = dst->gi = dst->bi = dst->ai = NULL;
  if (xmvals.dst != -1)
    {
      GDrawable *ddrawable;
      ddrawable = gimp_drawable_get (xmvals.dst);
      if (rgbaImageUnGIMP (dst, ddrawable))
        return 1;
    }
  else
    {
      dst->ncols = src->ncols;
      dst->nrows = src->nrows;
      rgbaImageTestCreate (dst, 1);
    }

  /* Set the source and destination meshes. */
  return 0;
}




/* NAME
//   run: typical GIMP plugin run procedure
//
// DESCRIPTION
//   Every GIMP plugin needs a procedure like this to be referenced by
//   the PLUGIN_INFO structure above.  This function is called by gimp_main
//   whenever the GIMP requests a procedure call.  We handle `xmorph' calls
//   and abort any others.
//
// SEE ALSO
//   plug-ins/bumpmap.c from the latest GIMP distribution
//   (http://www.gimp.org/).
*/
static void
run (char *name, int nparams, GParam * param,
     int *nreturn_vals, GParam ** return_vals)
{
  static GParam values[2];
  GRunModeType run_mode;
  XtAppContext app;

  GStatusType  status = STATUS_SUCCESS;

  run_mode = param[0].data.d_int32;
  plugin = TRUE;

  *nreturn_vals = 1;
  *return_vals = values;

  values[0].type = PARAM_STATUS;
  values[0].data.d_status = STATUS_SUCCESS;

  drawable = gimp_drawable_get(param[2].data.d_drawable);

  switch (run_mode)
    {
    case RUN_INTERACTIVE:
      /* Possibly retrieve data */
      gimp_get_data(PLUG_IN_NAME, &xmvals);
      break;

    case RUN_NONINTERACTIVE:
      /* Make sure all the arguments are there!  */
      if (nparams < 5 || nparams > 8)
        status = STATUS_CALLING_ERROR;

      if (status == STATUS_SUCCESS)
        {
          /* Retrieve the parameters. */
          xmvals.dst = param[3].data.d_drawable;
          xmvals.steps = param[4].data.d_int32;

          if (nparams > 5 && param[5].data.d_string)
            {
              strncpy (xmvals.srcmesh, param[5].data.d_string,
                       sizeof (xmvals.srcmesh) - 1);
              xmvals.srcmesh[sizeof (xmvals.srcmesh) - 1] = '\0';
            }
          else
            xmvals.srcmesh[0] = '\0';

          if (nparams > 6 && param[6].data.d_string)
            {
              strncpy (xmvals.dstmesh, param[6].data.d_string,
                       sizeof (xmvals.dstmesh) - 1);
              xmvals.dstmesh[sizeof (xmvals.dstmesh) - 1] = '\0';
            }
          else
            xmvals.dstmesh[0] = '\0';

          if (nparams > 7 && param[7].data.d_string)
            {
              strncpy (xmvals.basename, param[7].data.d_string,
                       sizeof (xmvals.basename) - 1);
              xmvals.basename[sizeof (xmvals.basename) - 1] = '\0';
            }
          else
            xmvals.basename[0] = '\0';
        }
      break;

    case RUN_WITH_LAST_VALS:
      /*  Possibly retrieve data  */
      gimp_get_data(PLUG_IN_NAME, &xmvals);
      break;

    default:
      status = STATUS_CALLING_ERROR;
      break;
    }

  /* Now actually run the morph process. */
  if (status == STATUS_SUCCESS && !load_images ())
    {
      char *srcmesh, *dstmesh;
      srcmesh = (*xmvals.srcmesh) ? xmvals.srcmesh : 0;
      dstmesh = (*xmvals.dstmesh) ? xmvals.dstmesh : 0;

      /* Set the basename and number of steps. */
      set_sequence_file_name (xmvals.basename);
      set_sequence_num_frames (xmvals.steps);

      app = initialize_application(orig_image[0].ncols,
                                   orig_image[0].nrows,
                                   srcmesh, dstmesh,
                                   &fakeargc, fakeargv,
                                   run_mode == RUN_INTERACTIVE);

      /* Only run the main loop if we are interactive. */
      if (run_mode == RUN_INTERACTIVE)
        {
          /* Return to this spot on exit. */
          if (!setjmp (exit_env))
            XtAppMainLoop (app);

          /* STATUS may be clobbered by longjmp, so reset it. */
          status = STATUS_SUCCESS;
        }
      else
        warp_sequence (&global_diw_map[0], xmvals.steps, xmvals.basename, FALSE, FALSE);

      /* If run mode is interactive, flush displays */
      if (run_mode != RUN_NONINTERACTIVE)
        gimp_displays_flush();

      /* Store data */
      if (run_mode == RUN_INTERACTIVE)
        {
          /* Save the values that were set by the user. */
          xmvals.steps = get_sequence_num_frames ();

          if (get_sequence_file_name ())
            {
              strncpy (xmvals.basename, get_sequence_file_name (),
                       sizeof (xmvals.basename) - 1);
              xmvals.basename[sizeof (xmvals.basename) - 1] = '\0';
            }
          else
            xmvals.basename[0] = '\0';

          /* Get the names of the source and dest meshes. */
          srcmesh = get_src_mesh_name ();
          dstmesh = get_dst_mesh_name ();

          if (srcmesh)
            {
              strncpy (xmvals.srcmesh, srcmesh, sizeof (xmvals.srcmesh) - 1);
              xmvals.srcmesh[sizeof (xmvals.srcmesh) - 1] = '\0';
            }
          else
            xmvals.srcmesh[0] = '\0';

          if (dstmesh)
            {
              strncpy (xmvals.dstmesh, dstmesh, sizeof (xmvals.dstmesh) - 1);
              xmvals.dstmesh[sizeof (xmvals.dstmesh) - 1] = '\0';
            }
          else
            xmvals.dstmesh[0] = '\0';

          gimp_set_data(PLUG_IN_NAME, &xmvals, sizeof(xmorph_vals_t));
        }
    }
  else
    status = STATUS_EXECUTION_ERROR;

  values[0].data.d_status = status;
  gimp_drawable_detach(drawable);
}




/* NAME
//   null_print_func: a GIMP printing function that doesn't do anything
//
// DESCRIPTION
//   The xmorph GIMP plugin uses a technique pioneered by SANE to determine
//   when it was executed by the GIMP, and when it was run standalone.  This
//   technique nullifies the GIMP error message function so that regular
//   users don't get an ugly error from gimp_main every time they call
//   xmorph in standalone mode.
//
// SEE ALSO
//   frontend/xscanimage.c from the latest SANE distribution
//   (http://www.azstarnet.com/~davidm/sane/).
*/
void
null_print_func (gchar *msg)
{
}

#endif /* NEED_GIMP */
