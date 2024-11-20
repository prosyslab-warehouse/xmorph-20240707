
#ifndef __GTK_TOP_DATA_H__
#define __GTK_TOP_DATA_H__

void gtk_widget_set_data_top (GtkWidget *widget, const char *key, 
			      gpointer data);
/*     Associate data with key in the data list of toplevel widget.  */

gpointer gtk_widget_get_data_top (GtkWidget *widget, const char *key);
/*     Retrieve the data associated with key in the data list of toplevel widget.  */

void gtk_widget_remove_data_top (GtkWidget *widget, const char *key);
/*     Remove the data associated with key in the data list of toplevel widget.  */
#endif // __GTK_TOP_DATA_H__
