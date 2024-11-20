#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "guide.h"
#include "guide_text.h"

#include "support.h"
#include "callbacks.h"


void
on_generic_help_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  show_info( _("\
Here are a few tips at using this program\n\
0) at startup the program is set for ``warping'': read the ``warp help''\n\
1) to morph, you need to have 2 or more `input images' :\n\
 use `add an image' (it is in the `file' menu); read the ``morph help''\n\
2) if you keep the mouse still on a menu voice or on a button\n\
  for a moment, you may read the help tips.\n\
3) when the mouse pointer is in on the mesh grid, by hitting\n\
  the right button, you get an useful menu.\n\
\n\n\
If you need more help, activate the guide.")  );
}




void
on_morph_help_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  show_info( _("\
To morph:\n\
  1) load an image in each `input image',\n\
  2) edit the meshes by dragging the points (and use the menu that you get\n\
       by the right mouse button)\n\
  3) set the `blending factors' and `mesh factors' as desired\n\
  4) and hit `do morph'\n\
")  );
}


/* this is only in the GTK2 version... unfortunately glade1 has disappeared!*/
void
on_feature_help_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  show_info( _(FEATURE_HELP));
}


void
on_mesh_tips_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  show_info( _(TIPS_HELP));
}




void
on_warp_help_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  show_info( _("\
It is not difficult to use this program for warping: just\n\
  1) load an image in the `input image 1',\n\
  2) edit the meshes by dragging the points (and use the menu that you get\n\
       by the right mouse button)\n\
  3) and hit `do warp'\n\
")  );
}
