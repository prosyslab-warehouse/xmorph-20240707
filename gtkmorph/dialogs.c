#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#ifdef USE_IMLIB
#include <gdk_imlib.h>
#else
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif


#include <string.h> //strncpy

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "dialogs.h"
//#include "settings.h"

/****************************************************************************/
/*********************************************************************
**********************************************************************

 * warning dialog hook
 FIXME: I think this is not the right way to do it

**********************************************************************
**********************************************************************
**/


/* the text of the warning */
char dialogwarning_text[1001]="";

GtkWidget *dialogwarning_g=NULL; 
 
void
on_labelwarning_show                   (GtkWidget       *widget,
                                        gpointer         user_data)
{
  gtk_label_set_text              (GTK_LABEL(widget),
				   dialogwarning_text);
}

void
on_labelwarning_realize                (GtkWidget       *widget,
                                        gpointer         user_data)
{
  gtk_label_set_text              (GTK_LABEL(widget),
				   dialogwarning_text);
}





//GtkWidget *menu_image_num_g=NULL;

void show_info(const char *str)
{
  strncpy(dialogwarning_text,str,1000);
  dialogwarning_g= create_dialogwarning();
  gtk_window_set_title(GTK_WINDOW(dialogwarning_g), _("info") );
  gtk_widget_show(dialogwarning_g);
}

void show_warning(const char *str)
{
  strncpy(dialogwarning_text,str,1000);
  if(settings_get_value("no warnings")==0) {
    dialogwarning_g= create_dialogwarning();
    gtk_window_set_title(GTK_WINDOW(dialogwarning_g), _("warning"));
    gtk_widget_show(dialogwarning_g);
  }
  else
    gdk_beep();
}

void show_error(const char *str)
{
  strncpy(dialogwarning_text,str,1000);
  dialogwarning_g= create_dialogwarning();
  gtk_window_set_title(GTK_WINDOW(dialogwarning_g), _("error"));
  gtk_widget_show(dialogwarning_g);
  gdk_beep();
}

void
on_why_the_beep_1_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
 if(*dialogwarning_text) {
   dialogwarning_g= create_dialogwarning();
   gtk_widget_show(dialogwarning_g);
 }
 *dialogwarning_text=0;
}







/******************* new glade callbacks *********************/

gboolean
on_dialogwarning_delete_event          (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{

  return FALSE;
}


gboolean
on_question_delete_event               (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{

  return FALSE;
}


void
on_yes_clicked                         (GtkButton       *button,
                                        gpointer         user_data)
{
  //GtkWidget *b=lookup_widget(GTK_WIDGET(button),"question");
  //gtk_widget_destroy(b);
}


void
on_no_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *b=lookup_widget(GTK_WIDGET(button),"question");
  gtk_widget_destroy(b);
}
