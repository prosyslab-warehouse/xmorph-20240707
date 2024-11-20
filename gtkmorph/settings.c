#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "gtk-meta.h"

#ifdef USE_IMLIB
#include <gdk_imlib.h>
#else
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

#include "gtktopdata.h"
#include "gtksettings.h"

#include "settings.h"

//#include "callbacks.h"
//#include "interface.h"
#include "support.h"
#include "main.h"
#include "utils.h"



void
gtkmorph_settings_callback(GtkWidget* thismenu, gpointer userdata)
{
#if HAVE_WAILI
  extern int wavelet_depth;
  extern double wavelet_equalization_factor;
  wavelet_equalization_factor
    =0.5 * settings_get_value("wavelet equalization");
  wavelet_depth=settings_get_value("wavelet depth")+1;
#endif

/* if the meshes do not have the same sizes, something will crash */
  if (settings_get_value("mesh auto sync"))
    promote_meshes();
#ifndef IS_PLYMORPH
  smooth_idle_stop();
  if (settings_get_value("preserve border")){
    int lp=MAX_WINS; for(; lp>=0; lp--) 
      if(sp->im_widget[lp] != NULL && sp->im_mesh[lp].x != NULL) {
	meshFunctionalize(&sp->im_mesh[lp],
			  sp->resulting_width,sp->resulting_height);
/* 	meshSet(&sp->im_mesh[lp],0,0,0,0); */
/* 	meshSet(&sp->im_mesh[lp],sp->im_mesh[lp].nx-1,sp->im_mesh[lp].ny-1, */
/* 		sp->resulting_width,sp->resulting_height); */
      }}
#endif

  set_state();
}

/* trick, so these get in the po files */
#ifdef ENABLE_NLS
#undef _
#define _(S) (S)
#endif


extern char * resample_array_inv_names[] ;
extern char * smooth_mesh_energy_names[] ;
extern char * wavelet_depths[] ;
extern char * wavelet_equalization[] ;

//char * wavelet_stat_origin[]=;


gpointer *gtkmorph_settings=NULL,
  gtkmorph_template[] =
{ 
  //gtkmorph_settings_callback,

  /****** next*******/
  _("no warnings"), //name
  NULL,//integer value, stored in pointer
  NULL,//accels
  _("dont warn, just beep; warnings may be read with 'why the beep' in the 'help' menu"),
  NULL, 

  /****** next*******/
  _("edit features"), //name
  GUINT_TO_POINTER(1),//integer value, stored in pointer
  NULL,//accels
  _("permits to add and delete mesh lines, edit features"),
  NULL, //magic pointer

  /****** next *****/
  _("preserve border"), //name
  GUINT_TO_POINTER(0),//integer value, stored in pointer
  NULL,//accels NOW IGNORED
  _("border points of the mesh are forced to stay on the border of the image"), //tooltip (they are translated below)
  NULL, //magic pointer

  /****** next*******/
  _("hide unusable"), //name
  GUINT_TO_POINTER(1),//integer value, stored in pointer
  NULL,//accels
  _("hide unusable widgets, instead of deactivating them"),
  NULL, //magic pointer

  /****** next*******/
  _("cursor jump"), //name
  GUINT_TO_POINTER(0),//integer value, stored in pointer
  NULL,//accels
  _("moves the image to recenter the last edited point"),
  NULL, //magic pointer

  /****** next *****/
  _("mesh cant overlap"), //name
  GUINT_TO_POINTER(0),//integer value, stored in pointer
  NULL,//accels NOW IGNORED
  _("a mesh point cannot enter in a neighbouring cell (this is currently not very well enforced)"), //tooltip (they are translated below)
  NULL, //magic pointer

  /****** next *****/
  _("use antialiasing warping"), //name
  GUINT_TO_POINTER(2),//integer value, stored in pointer
  NULL,//accels NOW IGNORED
  _("choose antialiasing method that is used when warping: the lanczos kernels are slower, but are necessary if the images have fine or grained textures for animations"), //tooltip (they are translated below)
  resample_array_inv_names, //magic pointer

#if HAVE_WAILI
  /****** next *****/
  _("wavelet equalization"), //name
  GUINT_TO_POINTER(1),//integer value, stored in pointer
  NULL,//accels NOW IGNORED
  _("the averaging of images will lose details; you may enable an enhancing algorithm (based on wavelets)"), //tooltip (they are translated below)
  wavelet_equalization, //magic pointer

  /****** next *****/
  _("wavelet depth"), //name
  GUINT_TO_POINTER(2),//integer value, stored in pointer
  NULL,//accels NOW IGNORED
  _("this is the depth of the wavelets"), //tooltip (they are translated below)
  wavelet_depths, //magic pointer
#endif

  /****** next *****/
  _("energy for mesh smoothing"), //name
  GUINT_TO_POINTER(1),//integer value, stored in pointer
  NULL,//accels NOW IGNORED
  _("type of energy that is minimized to decide the position of the non-selected mesh points: thin plate energy is better but slower, to speed up use elastic energy when editing meshes"), //tooltip (they are translated below)
  smooth_mesh_energy_names, //magic pointer

  /****** next *****/
  _("auto point adjust"), //name
  GUINT_TO_POINTER(0),//integer value, stored in pointer
  NULL,//accels NOW IGNORED
  _("adjust the position of a mesh point by comparing this image with the previous one and doing a best match. Achieves sub-pixel precision but may move your point around erratically."),
  NULL, //magic pointer

 /****** next *****/
  _("mesh factors sum to 1"), //name
  GUINT_TO_POINTER(1),//integer value, stored in pointer
  NULL,//accels NOW IGNORED
  _("always force the mesh factors so that they sum to 1;\n(if not, they are internally renormalized before using)"), //tooltip 
  NULL, //magic pointer

 /****** next *****/
  _("image factors sum to 1"), //name
  GUINT_TO_POINTER(1),//integer value, stored in pointer
  NULL,//accels NOW IGNORED
  _("always force the image factors so that they sum to 1;\n(if not, they are internally renormalized before using)"), //tooltip 
  NULL, //magic pointer

 /****** next *****/
  _("automatic mesh interpolation"), //name
  GUINT_TO_POINTER(1),//integer value, stored in pointer
  NULL,//accels NOW IGNORED
  _("automatically interpolates the resulting mesh when morphing (note that in this case you cannot edit it)" ) , //tooltip 
  NULL, //magic pointer 

 /****** next *****/
  _("automatic blending"), //name
  GUINT_TO_POINTER(1),//integer value, stored in pointer
  NULL,//accels NOW IGNORED
  _("automatically blends the images when the image blending factors are changed" ) , //tooltip 
  NULL, //magic pointer 

 /****** next *****/ 
  _("mesh auto sync"), //name
  GUINT_TO_POINTER(1),//integer value, stored in pointer
  NULL,//accels NOW IGNORED
  _("any change (add/del, label/unlabel) to a mesh is replicated"), //tooltip 
  NULL, //magic pointer

  /****** next *****/
  _("warped image in other win"), //name
  GUINT_TO_POINTER(0),//integer value, stored in pointer
  NULL,//accels NOW IGNORED
  _("put warped images in another window"), //tooltip (they are translated below)
  NULL, //magic pointer

  /****** next*******/
  /*      what is shown after a "warp" or "mix" or "morph"
     button is hit ?*/
  _("show warp after warp"),
  GUINT_TO_POINTER(EDITVIEW_SHOW),
  NULL,
  _("switch the input window(s) to the warped image after a warp (overridden by the above)"), //name
  NULL,

  /****** next*******/
  NULL};


GtkWidget*
create_gtkmorph_menuSettings ()
{

  g_assert(gtkmorph_settings==NULL);
  gtkmorph_settings=gtk_settings_alloc(gtkmorph_template);
  g_assert(gtkmorph_settings);
  return   gtk_settings_create(gtkmorph_template, gtkmorph_settings,
			       gtkmorph_settings_callback,NULL);
}



int
settings_get_value(char *name)
{
  return gtk_settings_get_value(name, gtkmorph_settings);
}


int
settings_set_value(char *name, int val)
{
  return gtk_settings_set_value(name, gtkmorph_settings, val);
}






/***************************** image pane settings ******************/



void
gtkmorph_image_settings_callback(GtkWidget* thismenu, gpointer userdata)
{
  int i=
    GPOINTER_TO_UINT(userdata);
  //userdata==gtk_widget_get_data_top(thismenu,"userdata"));
  g_assert(i>0);
  if (sp->im_widget[i] && GTK_WIDGET_DRAWABLE(sp->im_widget[i]))
    {
      render_pixmap(i, PIXSUBIMAGE);
      render_pixmap(i, PIXLOADED);
    }
  MY_GTK_DRAW(sp->im_widget[i]);
}


gpointer image_settings_template[] =
{ 
  //gtkmorph_image_settings_callback,

  /****** next *****/
  _("preserve aspect ratio"), //name
  GUINT_TO_POINTER(1),//integer value, stored in pointer
  NULL,//accels NOW IGNORED
  _("when loading an image, preserve aspect ratio"), //tooltip (they are translated below)
  NULL, //magic pointer

  /****** next*******/
  _("dim image"), //name
  GUINT_TO_POINTER(1),//integer value, stored in pointer
  NULL,//accels
  "",
  NULL, //magic pointer

  /****** next*******/
  _("view original mesh"), //name
  GUINT_TO_POINTER(1),//integer value, stored in pointer
  NULL,//accels
  "",
  NULL, //magic pointer

  /****** next *****/
  _("view original points"), //name
  GUINT_TO_POINTER(1),//integer value, stored in pointer
  NULL,//accels NOW IGNORED
  "",
  NULL, //magic pointer

  /****** next *****/
/*   _("view original features"), //name */
/*   GUINT_TO_POINTER(1),//integer value, stored in pointer */
/*   NULL,//accels NOW IGNORED */
/*   "", */
/*   NULL, //magic pointer */

  /****** next*******/
  _("view warped mesh"), //name
  GUINT_TO_POINTER(1),//integer value, stored in pointer
  NULL,//accels
  "",
  NULL, //magic pointer

  /****** next *****/
  _("view warped points"), //name
  GUINT_TO_POINTER(1),//integer value, stored in pointer
  NULL,//accels NOW IGNORED
  "",
  NULL, //magic pointer

  /****** next *****/
/*   _("view warped features"), //name */
/*   GUINT_TO_POINTER(1),//integer value, stored in pointer */
/*   NULL,//accels NOW IGNORED */
/*   "", */
/*   NULL, //magic pointer */

  /****** next *****/
  _("view eyes"), //name
  GUINT_TO_POINTER(1),//integer value, stored in pointer
  NULL,//accels NOW IGNORED
  "",
  NULL, //magic pointer

  //  /****** next *****/ /* HACK FIXME : NOW IGNORED */
  //  "view .../loaded image/subimage/warped image/", //name
  //GUINT_TO_POINTER(1),//integer value, stored in pointer
  //NULL,//accels NOW IGNORED
  //"view warped image instead of original image",
  //NULL, //magic pointer

  /****** next *****/
  _("mesh is readonly"), //name
  GUINT_TO_POINTER(1),//integer value, stored in pointer
  NULL,//accels NOW IGNORED
  "",
  NULL, //magic pointer

  /****** next*******/
  NULL};

static void
show_diff_set_it    (GtkMenuItem     *menuitem,
			 gpointer         data)
{
  unsigned int d=GPOINTER_TO_INT(data);
  unsigned int from=d/256, to=d&255;
  if(sp->im_mesh_diff[from]) meshUnref(sp->im_mesh_diff[from]);
  //GtkWidget *b=lookup_widget(sp->im_widget[from],"save_diff");
  if(to>0) {
    g_return_if_fail(meshAllocated ( &( sp->im_mesh[to]  )));
    sp->im_mesh_diff[from] = &( sp->im_mesh[to]  ); 
    meshRef(sp->im_mesh_diff[from]);
    sp->im_mesh_diff_is_difference_mesh[from] =
      sp->im_widget_is_difference_mesh[to];
    //gtk_widget_show(b);
  } else {
    sp->im_mesh_diff[from] = NULL;
    //gtk_widget_hide(b);
  }
  MY_GTK_DRAW(sp->im_drawingarea_widget[from]);
} 

static void
show_diff_show (GtkMenuItem     *menuitem,
			gpointer         data)
{
  unsigned int to , from = GPOINTER_TO_INT(data);
  GtkWidget *m=create_menu_of_images(from,show_diff_set_it,FALSE);  
  {
    GtkWidget *i = gtk_separator_menu_item_new     ();
    gtk_widget_show(i);gtk_container_add (GTK_CONTAINER (m), i);
    i =gtk_menu_item_new_with_label ( _("disable"));
    gtk_widget_show(i);gtk_container_add (GTK_CONTAINER (m), i);
    gtk_signal_connect (GTK_OBJECT (i), "activate",
			GTK_SIGNAL_FUNC (show_diff_set_it ),
			GINT_TO_POINTER( 256 * from) );
  }
  gtk_menu_popup(m,//GtkMenu *menu,
		 NULL,//GtkWidget *parent_menu_shell,
		 NULL,//GtkWidget *parent_menu_item,
		 NULL,//GtkMenuPositionFunc func,
		 NULL,//gpointer data,
		 0,//guint button,
		 gtk_get_current_event_time());//guint32 activate_time);
}

GtkWidget*
create_image_menu_settings (int i)
{

  sp->im_settings[i] = gtk_settings_alloc(image_settings_template);

  g_assert(sp->im_settings[i]);
  sp->im_menu_settings[i]=
    gtk_settings_create(image_settings_template,
			sp->im_settings[i],
			gtkmorph_image_settings_callback,GINT_TO_POINTER(i));
  g_assert(sp->im_menu_settings[i]);

  {
    GtkWidget *menuSettings=sp->im_menu_settings[i];
    GtkWidget *item;
    GtkTooltips *tooltips;
    tooltips = gtk_tooltips_new ();
    item = gtk_menu_item_new_with_label ( _("show mesh difference to..."));
    //gtk_widget_set_name (item,name);
    gtk_tooltips_set_tip (tooltips, item, _("show difference between this mesh and another mesh, as white arrows") , NULL);
    //gtk_widget_ref (item );
    gtk_widget_show (item);
    gtk_container_add (GTK_CONTAINER (menuSettings), item);
    gtk_signal_connect (GTK_OBJECT (item), "activate",
			GTK_SIGNAL_FUNC(show_diff_show ),
			GINT_TO_POINTER(i));
      }

  return sp->im_menu_settings[i];
}



int
image_settings_get_value(char *name, int i)
{
  return gtk_settings_get_value(name, sp->im_settings[i]);
}

int
image_settings_set_value(char *name, int i, int value)
{
  return gtk_settings_set_value(name,sp->im_settings[i],value);
}
