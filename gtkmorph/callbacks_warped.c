#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#ifdef USE_IMLIB
#include <gdk_imlib.h>
#else
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif


#include "gtktopdata.h"
#include "main.h"



gboolean
on_window_warped_delete_event          (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  /* there are two kinds of "window_warped"  */
  int i= GPOINTER_TO_UINT(gtk_widget_get_data_top(widget,"imagenum")); 
  if(i>0) { /* showing a warp of an image */
    g_return_val_if_fail(sp->im_warped_widget[i] == widget,FALSE);
    sp->im_warped_widget[i]=NULL;
  } else { /* showing an animation of a morph */
    GdkPixmap *pixmap =(gtk_widget_get_data_top(widget,"pixmap")); 
    if(pixmap)
      gdk_pixmap_unref(pixmap);
  }
  return FALSE;
}


gboolean
on_drawingarea_warped_expose_event     (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
  GdkPixmap *pixmap;
  //  int w=sp->resulting_width, h=sp->resulting_height;
  int i=    GPOINTER_TO_UINT(gtk_widget_get_data_top(widget,"imagenum")); 
  if(i>0)
    pixmap=sp->im_warped_pixmap[i];
  else
    pixmap=gtk_widget_get_data_top(widget,"pixmap"); 
  //extern GdkPixmap **movie_pixmaps;
  //extern movie_pixmaps_num ;

  //if(movie_pixmaps_num == 0) return FALSE ;
  g_return_val_if_fail(pixmap,FALSE);
  //g_return_if_fail(movie_pixmaps[0]);

  //g_(pixmap);
  gdk_gc_set_clip_rectangle (widget->style->fg_gc[widget->state],
			       &event->area);
  gdk_draw_pixmap(widget->window,
		  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		  pixmap,
		  event->area.x, event->area.y,
		  event->area.x, event->area.y,
		  event->area.width, event->area.height);

  gdk_gc_set_clip_rectangle (widget->style->fg_gc[widget->state],
			     NULL);    
  return TRUE;
}



gboolean
on_drawingarea_warped_configure_event  (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data)
{
  
  return FALSE;
}














