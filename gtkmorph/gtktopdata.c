#include "stdio.h"

#include <gtk/gtk.h>


//#include <gdk/gdk.h>

//#include "callbacks.h"
//#include "interface.h"
//#include "support.h"
//#include "main.h"

#include "gtktopdata.h"



void gtk_widget_set_data_top(GtkWidget       *widget, const char *key,
			     gpointer data)
{
  GtkWidget * father_window= gtk_widget_get_toplevel    (widget);
  g_assert(father_window != NULL); 
  g_assert(key != NULL);
  gtk_object_set_data (GTK_OBJECT(father_window), 
		       key, data);
}

gpointer gtk_widget_get_data_top (GtkWidget *widget, const char *key)
{
  gpointer data;
  GtkWidget * father_window=  gtk_widget_get_toplevel (widget);     
  g_assert(father_window!= NULL); 
  g_assert(key != NULL);    
  data =      gtk_object_get_data (GTK_OBJECT(father_window),key);   
  return data;
}

void gtk_widget_remove_data_top (GtkWidget *widget, const char *key)
{

  GtkWidget       *  father_window=  gtk_widget_get_toplevel (widget);
     
  g_assert(father_window!= NULL);
  g_assert(key != NULL); 
  gtk_object_remove_data (GTK_OBJECT(father_window), key);
}
























