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
#include "support.h"
#include "interface.h"
#include "dialogs.h"

#include "loadsave.h"

#ifdef IS_PLYMORPH
#include "utils__.h"
#include "loadsave_ply.h"
#else
#include "utils.h"
#include "loadsave_mesh.h"
#endif

#include "callbacks_fs.h"
fileselection_hook_t fileselection_hook=NULL;



/********************************************************************
 *
 * input image window, top bar buttons
 *

 note: many callbacks are shared with the "resulting image window"

 ******************************************************************
**/



/***********************************************************************
  load/save interface
*/




void create_fs()
{
  /*gtk_widget_destroy(imageselection1_g);*/
  // I changed idea - the widget is destroyed each and every time

  if(fileselection_g &&      GTK_IS_WIDGET (fileselection_g))
    gtk_widget_destroy(GTK_WIDGET(fileselection_g));

#ifdef USE_FILE_CHOOSER      // this is unfinished work
  fileselection_g =      gtk_file_chooser_dialog_new 
    ("Open File",
     sp->im_widget[MAIN_WIN],
     GTK_FILE_CHOOSER_ACTION_OPEN,
     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
     GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
     NULL);
#else
  fileselection_g = GTK_FILE_SELECTION(create_imageselection1() );
#if GTK_MAJOR_VERSION >= 2
  gtk_file_selection_set_select_multiple( fileselection_g,FALSE);
#endif
#endif    
  gtk_signal_connect (GTK_OBJECT (fileselection_g), 
		      "destroy",
		      GTK_SIGNAL_FUNC (gtk_widget_destroyed),
		      &fileselection_g);  
}

gboolean
on_imageselection1_destroy_event       (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  fileselection_g = NULL;
  g_message("bye bye"); // this is never called !
  return FALSE;
}


/****************  show file selector 
      if  title  contains %s ,then %s is substituded by the window name 
*/
void show_fs(//GtkWidget *widget,
	     char *title 
#if GTK_MAJOR_VERSION >= 2
	     ,	     GtkFileChooserAction action
#endif
	     )
{
  create_fs();

 /*  fileselection1_for_image_num_g=  */
/*     GPOINTER_TO_UINT(gtk_widget_get_data_top(widget,"imagenum"));  */

  g_assert(fileselection1_for_image_num_g > 0 && title );

  if( strstr(title,"%s") ) {
    int j=fileselection1_for_image_num_g;
    int l=strlen(title);
    gchar * N=compute_title (j) ;
    char s[l+50];
    sprintf(s,title,N);
    gtk_window_set_title(GTK_WINDOW (fileselection_g), s);  
    g_free(N);
  } else   gtk_window_set_title(GTK_WINDOW (fileselection_g), title); 
 
#if GTK_MAJOR_VERSION >= 2
#ifdef USE_FILE_CHOOSER
  // unfinished code
  gtk_file_chooser_set_action( imageselection1_g,action);
#else
  if(action== GTK_FILE_CHOOSER_ACTION_OPEN)
    gtk_file_selection_hide_fileop_buttons (fileselection_g);
  else
    gtk_file_selection_show_fileop_buttons (fileselection_g);
#endif
#endif

  gtk_widget_show( GTK_WIDGET(fileselection_g));
}

/* currently unused */
static void fileselection_title(char *name)
{
#ifdef USE_FILE_CHOOSER
  gtk_file_chooser_set_filename (fileselection_g, name);
#else
  gtk_file_selection_set_filename (fileselection_g, name);
#endif
}

void
#ifdef IS_PLYMORPH
on_loadply_clicked
#else
on_loadimage_clicked
#endif              
(GtkButton       *button, gpointer         user_data)
{
  
  int i = GPOINTER_TO_UINT(gtk_widget_get_data_top(GTK_WIDGET(button),"imagenum")); 

#ifdef IS_PLYMORPH
  fileselection_hook=load_ply_from_file;
#else
  fileselection_hook=load_image_from_file;
#endif
  fileselection1_for_image_num_g=i;
  show_fs(//GTK_WIDGET(button), 
	  _("load %s")
#if GTK_MAJOR_VERSION >= 2
	     ,	    GTK_FILE_CHOOSER_ACTION_OPEN
#endif	     
	     );  
  
  if (sp->im_filename_in[i] != NULL)
    gtk_file_selection_set_filename ((fileselection_g),
				     sp->im_filename_in[i] );
  else {
    if(i==MAIN_WIN) i=0;
    gtk_file_selection_set_filename ((fileselection_g), 
#ifdef IS_PLYMORPH
				     g_strdup_printf("image%d.ply",i)
#else
				     g_strdup_printf("image%d.png",i)
#endif
				     );
  }
}


/*****************************************************************************/
#ifdef HAVE_GDK_FORMATS
static void add_if_writable (gpointer d, GSList **list)
{
  GdkPixbufFormat *data=d;
  if (gdk_pixbuf_format_is_writable (data))
    *list = g_slist_prepend (*list, data);
}

GSList *readable_formats = NULL;
GSList *writable_formats = NULL;
void create_list_of_formats()
{
  if(readable_formats==NULL) {
    readable_formats =gdk_pixbuf_get_formats ();
    g_slist_foreach (readable_formats, add_if_writable, &writable_formats);
  }
}
 // find image type according to extension (if any)
int      type_by_extension    (const char *file)
{
  char *ext=extension(file);
  gint     lp=0;
  
  if(writable_formats == NULL)
    create_list_of_formats();

  GSList *F=writable_formats;
  
  if(ext) {
    while(F) {
      gpointer data=F->data;
      GdkPixbufFormat *f=data;
      gchar * type= gdk_pixbuf_format_get_name (f);
      if (0== strcmp(ext,type)) {
	return lp;
      } 
      F=g_slist_next(F);
      lp++;
    }
    {
      if (0== strcmp(ext,"ppm"))
	return lp;
      else
	return -1;
    }
  }
  return -1;
}
#endif
/*****************************************************************/
#ifdef HAVE_GTK_COMBO
void     on_image_type_changed      (GtkComboBox *C,
				     gpointer user_data)
{
  const char *file=
    gtk_file_selection_get_filename    ( fileselection_g);
  gint   sel =    gtk_combo_box_get_active        (C);
  GSList *F=g_slist_nth(writable_formats,sel);
  GdkPixbufFormat *f;
  char *type;
  if (F) {
    gpointer data=F->data;
    f=data;
    type=gdk_pixbuf_format_get_name (f);
  } else type="ppm";
  {
    int len=strlen(file);
    len--;
    while(len>0 && file[len] != '.') len--;
    if(len>0) {
      char *f=g_strdup(file),*z;
      f[len]=0;
      z=g_strdup_printf("%s.%s",f,type);
      gtk_file_selection_set_filename    ( fileselection_g,z);
      g_free(z); g_free(f);
    }
  }
}
GtkWidget * create_image_type_combo()
{
  GtkWidget *C=gtk_combo_box_new_text(); 
  GSList *F=writable_formats;
  while(F) {
    gpointer data=F->data;
    GdkPixbufFormat *f=data;
    gtk_combo_box_append_text(GTK_COMBO_BOX(C),gdk_pixbuf_format_get_name (f));
    F=g_slist_next(F);
  } 
  gtk_combo_box_append_text(GTK_COMBO_BOX(C),"ppm");  
  gtk_widget_set_name (C,"file_type_combo");
  g_signal_connect ((gpointer) C,"changed",on_image_type_changed,NULL);
  return C;
}
// set image type according to extension (if any)
void     combo_set_image_type    (GtkComboBox *C,const char *file)
{
  int lp=type_by_extension (file);
  if(lp>=0)
    gtk_combo_box_set_active(C,lp);
}
#endif // HAVE_GTK_COMBO
/*****************************************************************/



#ifdef USE_FILE_CHOOSER //unfinished code
    //GtkDialog *D=GTK_DIALOG(fileselection_g->fileop_dialog);
    GtkWidget *toggle;
    toggle = gtk_check_button_new_with_label ("Open file read-only");
    gtk_widget_show (toggle);
    gtk_file_chooser_set_extra_widget (fileselection_g, toggle);
#endif





void
on_save_image_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
  int i = GPOINTER_TO_UINT(gtk_widget_get_data_top(GTK_WIDGET(button),"imagenum")); 

#ifndef IS_PLYMORPH
  GdkPixbuf * pb=*(which_pixbuf_is_visible(i));
  if( pb ==NULL)    {
    show_error(_("internal error: the image doesnt exist!"));
    return ;
  }
#endif
  fileselection1_for_image_num_g=i;
  show_fs(//GTK_WIDGET(button), 
#if GTK_MAJOR_VERSION < 2
	  _( "save %s (only PPM format)") 
#else
	  _( "save %s")    ,    GTK_FILE_CHOOSER_ACTION_SAVE
#endif
	  ); 

  fileselection_hook=save_image_to_file;

  if(sp-> im_filename_out[i] != NULL)  { 
    gtk_file_selection_set_filename(fileselection_g,sp-> im_filename_out[i]);
  } else  {
    if(sp-> im_filename_in[i] != NULL) {
      gchar *s = g_strdup_printf("%s_out.ppm",sp-> im_filename_in[i]);
      gtk_file_selection_set_filename(fileselection_g, s);
      g_free(s);
    } else 
      gtk_file_selection_set_filename(fileselection_g, "");
  }
#ifdef HAVE_GDK_FORMATS
  if(!writable_formats ) create_list_of_formats();
 {
  GtkWidget  *label; 
  GtkWidget  *V=gtk_hbox_new(TRUE,4);  
  label = gtk_label_new ("image format: ");
#ifdef HAVE_GTK_COMBO
  GtkWidget *C=create_image_type_combo();
  {
    char *file=gtk_file_selection_get_filename(fileselection_g);
    combo_set_image_type(GTK_COMBO_BOX(C),file);
  }
  gtk_widget_set_data_top (GTK_WIDGET(fileselection_g),"file_type_combo",C);
#else //HAVE_GTK_COMBO
  GtkWidget *C= gtk_label_new ("jpeg,png,ico,ppm");
#endif //HAVE_GTK_COMBO
  gtk_container_add (GTK_CONTAINER (V),label);
  gtk_container_add (GTK_CONTAINER (V),C);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(fileselection_g)->vbox),V);
  gtk_widget_show_all(V);
 }
#endif //HAVE_GTK_FORMATS
}

