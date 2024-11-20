#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#ifdef USE_IMLIB
#include <gdk_imlib.h>
#else
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

#include "string.h" //memcpy

#include "gtk-meta.h"

#include "gtktopdata.h"
#include "gtk_subimagesel.h"

#include "main.h"
#include "utils.h"
#include "colors.h"
#include "mesh-gtk.h"




/********************************************************************
 *
 * input image window, drawingarea
 *

 note: many callbacks are shared with the "resulting image window"

 ******************************************************************
**/




/***************************************************************

 *		drawing area callbacks

 ***********************************************************
*/ 



void
on_drawingarea_realize                 (GtkWidget       *widget,
				     gpointer         user_data)
{
  int i=
    GPOINTER_TO_UINT(gtk_widget_get_data_top(widget,"imagenum")); 
  /* the drawing area widget will be accessible tru this */

  g_return_if_fail(sp->im_drawingarea_widget[i] == widget);//also done in create_and_show_image_win()
  
  render_pixmap(i, PIXSUBIMAGE);
}





/* gboolean */
/* on_drawingarea_configure_event         (GtkWidget       *widget, */
/*                                         GdkEventConfigure *event, */
/*                                         gpointer         user_data) */
/* { */
/*   int i=    GPOINTER_TO_UINT(gtk_widget_get_data_top(widget,"imagenum"));  */
/*   g_debug(" configure drawingarea %d",i); */
/*   // this loops */
/*   //`drawingarea_configure(i); */
/*   return FALSE; */
/* } */



double transl[6]={1,0,PIXEDITBORDER,0,1,PIXEDITBORDER},
  ident[6]={1,0,0,0,1,0};
double loaded[6];
double * matrix_for_image(int i) 
{
  double *a=ident;
  if( sp->im_editview[i]==EDITVIEW_EYES )
    {
      a=loaded;
      memcpy(a,&sp->transforms[i].subimage2loaded,6*sizeof(double));
      a[2]+=PIXLOADEDBORDER;
      a[5]+=PIXLOADEDBORDER;
    }
  if( sp->im_editview[i]==EDITVIEW_EDIT)
    a=transl;
  return a;
}

gboolean
on_expose_event                        (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
  int i=
    GPOINTER_TO_UINT(gtk_widget_get_data_top(widget,"imagenum")); 
  GdkPixmap *pixmap ,
    **pixmap_p=which_pixmap_is_visible(i);

  g_assert(i > 0);
  g_assert(pixmap_p);
  pixmap=*pixmap_p;
  
  if(pixmap == NULL) 
    return FALSE;
  //g_critical("THERE IS NO PIXMAP of type %d  for image %d",
  //       sp->which_pix[i],i);
  //  if ( ! mpl_gc)     allocate_colors( widget );
  if ( ! mpl_gc)    {
    g_critical("colors unavaible: can't draw meshes!");
    return FALSE; 
  }
  /* the gtk+ manual (see "drawing area" section)
     says we have to:*/
  gdk_gc_set_clip_rectangle (widget->style->fg_gc[widget->state],
			     &event->area);

  // this one flashes too much
  //gdk_window_clear_area (widget->window,
  //                        event->area.x, event->area.y,
  //                        event->area.width, event->area.height);

  /* then we draw...*/ 
  
  gdk_draw_pixmap(widget->window,
		    widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		    pixmap,
		    event->area.x, event->area.y,
		    event->area.x, event->area.y,
		    event->area.width, event->area.height);
 {
   double *a=matrix_for_image(i);
   if(i!=MAIN_WIN)
     gdk_draw_mesh(widget->window,
		   image_settings_get_value("view original mesh",i),
		   mpl_gc ,
		   (image_settings_get_value("view original points",i))?
		   features_max_n:0,
		   features_gc,
		   0,//image_settings_get_value("view original features",i),
		   &(sp->im_mesh[i]),// MeshT *mesh, 
		   //sp->resulting_height,
		   //sp->resulting_width,
		   a
		   );
   gdk_draw_mesh(widget->window,
		   image_settings_get_value("view warped mesh",i),
		   widget->style->white_gc ,
		   (image_settings_get_value("view warped points",i))?
		   features_max_n:0,
		   features_gc,
		   0,//image_settings_get_value("view warped features",i),
		   &(sp->im_mesh[MAIN_WIN]),// MeshT *mesh,
		   //sp->resulting_height,
		   //sp->resulting_width,
		   a);       
   if( sp->im_mesh_diff[i]) 
     gdk_draw_mesh_as_arrows( widget->window, widget->style->white_gc ,
			      sp->im_mesh_diff[i],
			      &(sp->im_mesh[i]),
			      sp->im_mesh_diff_is_difference_mesh[i],
			      a);
   }
  if(image_settings_get_value("view eyes",i)) {
    gdk_subimagesel_draw( widget->window, &(sp->subimasel[i]), mps_gc);
  }
/*    if( sp->im_editview[i]==EDITVIEW_EYES || */
/*        sp->im_editview[i]==EDITVIEW_EDIT) */
/*      { */
/*        a[2]-=PIXLOADEDBORDER; */
/*        a[5]-=PIXLOADEDBORDER; */
/*      } */
/*    if( sp->im_editview[i]==EDITVIEW_EYES || */
/*        sp->im_editview[i]==EDITVIEW_EDIT) */
/*      { */
/*        a[2]-=PIXLOADEDBORDER; */
/*        a[5]-=PIXLOADEDBORDER; */
/*      } */
 /* after that, we  */
 gdk_gc_set_clip_rectangle (widget->style->fg_gc[widget->state],
			    NULL);
 return TRUE;
}







/*****************************************************************************/








void
on_settings_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{  
  int i=
    GPOINTER_TO_UINT(gtk_widget_get_data_top(GTK_WIDGET(button),"imagenum")); 
  GtkWidget* m=sp->im_menu_settings[i];
  
  gtk_menu_popup (GTK_MENU(m),//GtkMenu *menu,
		  NULL,//GtkWidget *parent_menu_shell,
		  NULL,//GtkWidget *parent_menu_item,
		  NULL,//GtkMenuPositionFunc func,
		  NULL,//gpointer data,
		  1,//guint button,
		  0);//guint32 activate_time);
}






void
on_do_warp_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  int i=
    GPOINTER_TO_UINT(gtk_widget_get_data_top(GTK_WIDGET(button),"imagenum")); 
 
  /* geometry of the resulting image */
  
  do_warp_an_image(i);
}

















