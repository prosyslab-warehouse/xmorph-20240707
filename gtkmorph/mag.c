
#include <gtk/gtk.h>

#include "gtk-meta.h"

#ifdef USE_IMLIB
#include <gdk_imlib.h>
#else
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

#include "mag.h"
#include "main.h"
#include "utils.h"
#include "colors.h"
#include "mesh-gtk.h"
#include "interface.h"
#include "settings.h"
#include "support.h"
#include "callbacks.h"

/*********************** mag window *****************************/

gint mag_width=0, mag_height=0;
double mag_zoom=3;
GtkWidget *mag_window=NULL;;
gboolean mag_tracking=TRUE;
GdkPixmap *mag_pixmap=0;
GdkPixbuf *mag_pixbuf=0;
GtkWidget *mag_area=NULL;
GtkLabel *mag_label=NULL;

void mag_draw()
{ 
  if(!mag_area) return ;
  gtk_widget_draw(mag_area,NULL);
}

gint  mag_x=0, mag_y=0,  mag_source=MAIN_WIN;

gint mag_ox=0, mag_oy=0;
void mag_pix()
{
  GdkPixbuf  **src_p;
  src_p=which_pixbuf_is_visible(mag_source);

  mag_ox=(-(mag_x)*mag_zoom+mag_width/2);
  mag_oy=(-(mag_y)*mag_zoom+mag_height/2);

  if(!*src_p) return;
  gdk_pixbuf_scale(*src_p,//const GdkPixbuf *src,
		   mag_pixbuf,//GdkPixbuf *dest,
		   0,0,//int dest_x,  int dest_y,
		   mag_width,//int dest_width,
		   mag_height,//int dest_height,
		   mag_ox,//double offset_x,
		   mag_oy,//double offset_y,
		   mag_zoom,//double scale_x,
		   mag_zoom,//double scale_y,
		   GDK_INTERP_BILINEAR); //GdkInterpType interp_type);
  mag_draw();
}

void mag_xy_track(int i, int x, int y)
{
  //g_debug("mag at %d %d",x,y);
  if(!mag_area) return ;
  if(  mag_source!=i)
    {char s[30];sprintf(s,"%d",i);
    gtk_label_set_text (mag_label,s);
    }
  mag_source=i;
  mag_x=x;
  mag_y=y;
  if(sp->im_editview[i]==EDITVIEW_EDIT ) {
    mag_x-= PIXEDITBORDER;
    mag_y-= PIXEDITBORDER;
  }
  mag_pix();
  gtk_widget_draw(mag_area,NULL);
}

gboolean mag_start_track(int i, GdkEventButton  *event)
{
  if(!mag_area) return FALSE;
  mag_xy_track(i,event->x,event->y);
  return TRUE;
}

void mag_track(gint i,GdkEventMotion  *event)
{
  GdkModifierType state=event->state;;
  if(!mag_area) return;
  if(mag_tracking) { 
    if (event->is_hint)
      gdk_window_get_pointer (event->window, &mag_x, &mag_y, &state);
    else {
      mag_x=event->x;
      mag_y=event->y;      
    }
    mag_xy_track(i,mag_x,mag_y);
  }
}

void
on_mag_spinbutton_activate             (
#if GTK_MAJOR_VERSION >= 2
					GtkEntry     *entry,
#else
					GtkEditable     *entry,
#endif
                                        gpointer         user_data)
{
  mag_zoom=gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(entry))/100.;
  mag_pix();
}

void
on_mag_spinbutton_changed              (GtkEditable     *editable,
                                        gpointer         user_data)
{
  mag_zoom=gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(editable))/100.;
  mag_pix();
}


void
on_mag_track_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  mag_tracking=gtk_toggle_button_get_active(togglebutton);
}


gboolean
on_mag_drawingarea_configure_event     (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data)
{
  mag_width=event->width;
  mag_height=event->height;
  mag_area=widget;
  if(mag_pixbuf) gdk_pixbuf_unref(mag_pixbuf);
  mag_pixbuf=gdk_pixbuf_new(GDK_COLORSPACE_RGB,TRUE,8,mag_width,mag_height);
  if(mag_source>0 && mag_area)
    mag_pix();
  return TRUE;
}


gboolean
on_mag_drawingarea_expose_event        (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
  int i=mag_source;
  g_return_val_if_fail(i > 0,FALSE);

  //if ( ! mpl_gc)     allocate_colors( widget );   

  gdk_gc_set_clip_rectangle (widget->style->fg_gc[widget->state],
			     &event->area);
  gdk_pixbuf_render_to_drawable  
    (       mag_pixbuf,widget->window,
	    sp->im_widget[MAIN_WIN]->style->black_gc, //GdkGC *gc,
	    0, //int src_x,
	    0, //int src_y,
	    0, //int dest_x,
	    0, //int dest_y,
	    mag_width,mag_height, //width, height,
	    GDK_RGB_DITHER_NORMAL,//GdkRgbDither dither,
	    0, //int x_dither,
	    0 ); //int y_dither);
  {
    //FIXME the subimage selection is wrong!
    //double *c=matrix_for_image(i),b[6]; 
    //FIXME the following should be a multiplication
    double a[6]={mag_zoom,0,mag_ox,
		 0,mag_zoom,mag_oy   };

    if(i!=MAIN_WIN)
      gdk_draw_mesh(widget->window,
		    image_settings_get_value("view original mesh",i),
		    mpl_gc ,
		    (image_settings_get_value("view original points",i))?
		    features_max_n:0,
		    features_gc,
		    0,//image_settings_get_value("view original features",i),
		    &(sp->im_mesh[i]),a);// MeshT *mesh, 
    //mag_height,mag_width,		 a );
   gdk_draw_mesh(widget->window,
		 image_settings_get_value("view warped mesh",i),
		 widget->style->white_gc ,
		 (image_settings_get_value("view warped points",i))?
		 features_max_n:0,
		 features_gc,
		 0,//image_settings_get_value("view warped features",i),
		 &(sp->im_mesh[MAIN_WIN]),a);// MeshT *mesh,
   //		 mag_height,mag_width,	 a );  
   if( sp->im_mesh_diff[i]) 
     gdk_draw_mesh_as_arrows( widget->window, widget->style->white_gc ,
			      sp->im_mesh_diff[i],
			      &(sp->im_mesh[i]),
			      sp->im_mesh_diff_is_difference_mesh[i],
			      a);
  }
  if(mag_tracking) {
    gdk_draw_line    (widget->window,
		      widget->style->white_gc ,
		      mag_width/2,    mag_height/2,
		      mag_width/2+10,
		      mag_height/2+10);
    gdk_draw_line    (widget->window,
		      widget->style->white_gc ,
		      mag_width/2,    mag_height/2,
		      mag_width/2+5,
		      mag_height/2+2);
    gdk_draw_line    (widget->window,
		      widget->style->white_gc ,
		      mag_width/2,    mag_height/2,
		      mag_width/2+2,
		      mag_height/2+5);
  }
   return FALSE;
}

void
on_show_mag_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if(!mag_area)
    create_mag_();
}


void
on_mag_unrealize                       (GtkWidget       *widget,
                                        gpointer         user_data)
{
  mag_label=0; mag_area=mag_window=0;
}





void create_mag_()
{
  mag_window= create_mag();
  mag_label=GTK_LABEL(lookup_widget(mag_window,"mag_label"));
  gtk_widget_show(mag_window);
}
