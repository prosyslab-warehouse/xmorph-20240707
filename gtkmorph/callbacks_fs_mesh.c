#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#ifdef USE_IMLIB
#include <gdk_imlib.h>
#else
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

#include "gtk-meta.h"

#include "string.h" //strlen

#include "gtktopdata.h"
#include "gtk_subimagesel.h"

#include "main.h"
#include "utils.h"
#include "support.h"
#include "interface.h"
#include "dialogs.h"
#include "loadsave.h"

#include "callbacks_fs.h"
extern fileselection_hook_t fileselection_hook;

#ifndef IS_PLYMORPH
#include "loadsave_mesh.h"
#else
#include "loadsave_ply.h"
#endif


/***** 
       returns TRUE if it was already known
***/
static gboolean
set_fs_mesh_filename(int i)
{
  if ( sp->im_mesh_filename[i] != NULL) {   
      gtk_file_selection_set_filename(fileselection_g,
				      sp->im_mesh_filename[i]);
      return TRUE;
  } else {
      char *file= sp-> im_filename_in[i];
      
      if(file != NULL) { 
	char *s = g_strdup_printf("%s.mesh",file);
	gtk_file_selection_set_filename(fileselection_g,  s);
	g_free(s);
      }       else
	gtk_file_selection_complete(fileselection_g, "*.mesh");
      //gtk_file_selection_set_filename(fileselection_g,  "");
      return FALSE;
  }
}

void
on_loadmesh_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  int i =
    GPOINTER_TO_UINT(gtk_widget_get_data_top(GTK_WIDGET(button),"imagenum")); 
  fileselection_hook=load_mesh_from_file;
  fileselection1_for_image_num_g=i;
  show_fs(//GTK_WIDGET(button),
	  _( "load mesh for %s")
#if GTK_MAJOR_VERSION >= 2
	     ,	    GTK_FILE_CHOOSER_ACTION_OPEN
#endif	     
	  ); 
  set_fs_mesh_filename(i);
}













void
on_savemesh_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{

  int i = GPOINTER_TO_UINT(gtk_widget_get_data_top(GTK_WIDGET(button),"imagenum")); 

  fileselection_hook=save_mesh_to_file;
  


  //FIXME auto save doesnt work
  //if( set_fs_mesh_filename(i) == FALSE|| button->state & GDK_SHIFT_MASK)
  fileselection1_for_image_num_g=i;
  show_fs(//GTK_WIDGET(button), 
	  _( "save mesh for %s")
#if GTK_MAJOR_VERSION >= 2
	     ,	    GTK_FILE_CHOOSER_ACTION_SAVE
#endif	     
	  ); 

  set_fs_mesh_filename(i);
  //  else
  /* directly simulate as if OK was hit */
  //on_ok_button1_clicked   (NULL,NULL); 
}





static void
diff_save_it    (GtkMenuItem     *menuitem,
			 gpointer         data)
{
  unsigned int d=GPOINTER_TO_INT(data);
  unsigned int from=d/256, to=d&255;
  if(sp->im_mesh_diff[from]) meshUnref(sp->im_mesh_diff[from]);
  sp->im_mesh_diff[from] = &( sp->im_mesh[to]  ); 
  meshRef(sp->im_mesh_diff[from]);
  g_assert(!sp->im_widget_is_difference_mesh[to]);
  sp->im_mesh_diff_is_difference_mesh[from] =
    sp->im_widget_is_difference_mesh[to];
  fileselection_hook=save_diff_mesh_to_file;
  fileselection1_for_image_num_g=from;
  show_fs(//GTK_WIDGET(button), 
	  _( "save difference to mesh")
#if GTK_MAJOR_VERSION >= 2
	  ,	    GTK_FILE_CHOOSER_ACTION_SAVE
#endif	     
	  ); 
}
void
on_save_diff_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
  int i = GPOINTER_TO_UINT(gtk_widget_get_data_top(GTK_WIDGET(button),"imagenum")); 

  if(sp->im_mesh_diff[i]==NULL)    {
    GtkWidget *m=create_menu_of_images(i,diff_save_it,TRUE);
      gtk_menu_popup(m,//GtkMenu *menu,
		 NULL,//GtkWidget *parent_menu_shell,
		 NULL,//GtkWidget *parent_menu_item,
		 NULL,//GtkMenuPositionFunc func,
		 NULL,//gpointer data,
		 0,//guint button,
		 gtk_get_current_event_time());//guint32 activate_time);
  } else {
    fileselection_hook=save_diff_mesh_to_file;
    fileselection1_for_image_num_g=i;
    //FIXME auto save doesnt work
    //if( set_fs_mesh_filename(i) == FALSE|| button->state & GDK_SHIFT_MASK)
    show_fs(//GTK_WIDGET(button), 
	    _( "save difference to mesh")
#if GTK_MAJOR_VERSION >= 2
	    ,	    GTK_FILE_CHOOSER_ACTION_SAVE
#endif	     
	    ); 
  }
}




void
on_add_a_difference_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  int lp;
    for(lp=1 ; lp <= MAX_WINS;  lp ++)
      if(sp->im_widget[lp] == NULL)
	{	  
	  fileselection_hook=load_diff_from_file;
	  fileselection1_for_image_num_g= lp;

	  show_fs(//GTK_WIDGET(menuitem), 
		  _( "load difference mesh")
#if GTK_MAJOR_VERSION >= 2
		  ,	    GTK_FILE_CHOOSER_ACTION_OPEN
#endif	     
		  ); 
	  return; 
	}
}












