#include <gtk/gtk.h>
#include <glib.h>
//stubs for older version of gtk
// unfortunately glade uses unexistant code 
 
#if GTK_MAJOR_VERSION == 2
#if GTK_MINOR_VERSION <= 2

#warning please use gtk 2.4  !!!  some functions are missing !!

gpointer GTK_RADIO_TOOL_BUTTON(gpointer d){return d;}
gpointer GTK_TOOL_BUTTON(gpointer d){return d;}
gpointer GTK_TOOL_ITEM(gpointer d){return d;}
gpointer GTK_TOGGLE_TOOL_BUTTON(gpointer d){return d;}

GSList *the_group=NULL;
GSList  *the_active=NULL;

typedef GtkButton  GtkRadioToolButton;
typedef GtkButton  GtkToolButton;
typedef GtkBin  GtkToolItem;

GtkToolItem* gtk_tool_item_new(void){ return gtk_button_new(); }

void       gtk_toggle_tool_button_set_active(){};
void       gtk_toggle_button_set_active    (GtkToggleButton *toggle_button,
                                             gboolean is_active)
{ if(is_active) the_active = toggle_button; }

void gtk_tool_item_set_tooltip(void *a,void*b) {};



GSList *     gtk_radio_tool_button_get_group (GtkRadioToolButton *button)
{ return the_group;}

void        gtk_radio_tool_button_set_group (GtkRadioToolButton *button,
                                             GSList *group)
{  the_group=group;  g_slist_append(the_group,button); }


GtkWidget*  gtk_button_new                  (void);
GtkToolItem* gtk_tool_button_new            (GtkWidget *icon_widget,
                                             const gchar *label);
GtkToolItem* gtk_radio_tool_button_new      (GSList *group)
{
  GtkButton *b=    gtk_button_new();
  GtkButton *v=    gtk_vbox_new(TRUE,1);
  gtk_widget_show(v);
  gtk_widget_show(b);
  gtk_container_add(b,v);
  return b;
 }


void        gtk_button_set_label            (GtkButton *button,
                                             const gchar *label);
void        gtk_tool_button_set_label       (GtkToolButton *button,
                                             const gchar *label)
{
  GtkWidget *v=gtk_bin_get_child (button);
  if(v && GTK_IS_CONTAINER(v) ) {
    GtkWidget *w=gtk_label_new(label);
    gtk_widget_show(w);
    gtk_container_add(v,w); }
}

void        gtk_tool_button_set_icon_widget (GtkToolButton *button,
                                             GtkWidget *icon_widget)
{ GtkWidget *v=gtk_bin_get_child (button);
 if(v && GTK_IS_CONTAINER(v) )
   gtk_container_add(v,icon_widget); 
}
#endif
#endif
