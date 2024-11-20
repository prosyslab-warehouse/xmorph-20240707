#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include "gtk-meta.h"

#ifdef USE_IMLIB
#include <gdk_imlib.h>
#else
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

#include "libmorph/mesh.h"

#include "main.h"
#include "utils.h"
#include "mesh-gtk.h"
#include "callbacks.h"
#include "mag.h"
#include "support.h"
#include "dialogs.h"
#include "gtktopdata.h"
#include "fourier.hh"

/***************************************************************

 *		drawing area mesh editing

 ***********************************************************
*/ 





/******************************* sometimes when we change the labels..*/
int all_images_need_redraw=0;


static void
flash_point(GdkDrawable  *drawable,
	    int x,int y	    )
{ 
  int MP_SIZE=10, MP_ARC=360 * 64;
  extern GdkGC *flash_gc;
  gdk_draw_arc  (drawable, flash_gc,
		 TRUE,
		 x - MP_SIZE/2,  y - MP_SIZE/2, MP_SIZE,MP_SIZE,0,MP_ARC);
}

guint flash_points_id;
int flash_mi=0,flash_mj=0;

static gboolean flash_points(gpointer ignored)
{
  int lp=MAIN_WIN; 
  all_images_need_redraw++;
  for(; lp>=0; lp--) 
    if(sp->im_drawingarea_widget[lp] != NULL) {
      flash_point(sp->im_drawingarea_widget[lp]->window,
		  PIXEDITBORDER+meshGetx(&sp->im_mesh[lp],flash_mi,flash_mj),
		  PIXEDITBORDER+meshGety(&sp->im_mesh[lp],flash_mi,flash_mj)    );
		  
    }
  return TRUE;
}

int last_clicked_image = -1;

gboolean
on_drawingarea_key_press_event         (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
  int i= GPOINTER_TO_UINT(gtk_widget_get_data_top(widget,"imagenum")); 
  g_assert(i > 0);
  last_clicked_image = i;
  g_debug("tasto %d",event->keyval);
  if(sp->im_editview[i]==EDITVIEW_EYES  )     
    return FALSE;//gdk_subimagesel_button_press_event (widget,event,user_data);

  if( ( i == MAIN_WIN) && sp->max_wins>1 &&
      settings_get_value("automatic mesh interpolation")
      //&& EDITVIEW_FEATURES != sp->im_editview[MAIN_WIN]
      ) 
    { 
      show_warning(_("you cant edit this mesh - it is automatically generated as an interpolation\nof the input images meshes .\n(but if you really want to edit, unset 'automatic mesh interpolation')"));
      return FALSE;     
    }

  /* if the menu says "view", do not edit */
  if( sp->im_editview[i] == EDITVIEW_SHOWMESHES ||
      sp->im_editview[i] == EDITVIEW_SHOW  )
    {
      show_warning(_("\
You are currently viewing the warped version of the image\n\
you may not edit this mesh (which refers to the loaded image).\n\
To edit the mesh, select `edit mesh' in the menu (at top center).\n\
I have done it now for you")); 
      set_editview(i, EDITVIEW_EDIT); 
      return FALSE; 
    }
  if(i!=MAIN_WIN)
    {
      /* hide bar-makes room for editing */
      GtkWidget* hb= lookup_widget  ( sp->im_widget[i]  ,
				      "handlebox_factors");
      GtkWidget* hs= lookup_widget  ( sp->im_widget[i]  ,
				      "handleboxsubimage");
      gtk_widget_hide(hb);gtk_widget_hide(hs);
    }

  gboolean readonly = image_settings_get_value("mesh is readonly",  i);
  
  if(!readonly) {
    int mi,mj,mlabel; enum tools action;
    gboolean b;
    b= gdk_mesh_key_press_event  (widget,event,
				  user_data, &(sp->im_mesh[i]),
				  &mi,&mj,&mlabel, &action
				  );
    
    mag_draw();
    return b;
  }

  return FALSE;
}



gboolean
on_button_press_event                  (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{

  int i= GPOINTER_TO_UINT(gtk_widget_get_data_top(widget,"imagenum")); 
  g_assert(i > 0);
  last_clicked_image = i;
  //mag_start_track(i,event);

  gtk_widget_grab_focus (widget);

  if(sp->im_editview[i]==EDITVIEW_EYES  )     
    return gdk_subimagesel_button_press_event (widget,event,user_data);  

  if( ( i == MAIN_WIN) && sp->max_wins>1 &&
      settings_get_value("automatic mesh interpolation")
      ) 
    { 
      show_warning(_("you cant edit this mesh - it is automatically generated as an interpolation\nof the input images meshes .\n(but if you really want to edit, unset 'automatic mesh interpolation')"));
      return FALSE;     
    }

  /* if the menu says "view", do not edit */
  if( sp->im_editview[i] == EDITVIEW_SHOWMESHES ||
      sp->im_editview[i] == EDITVIEW_SHOW  )
    {
      show_warning(_("\
You are currently viewing the warped version of the image\n\
you may not edit this mesh (which refers to the loaded image).\n\
To edit the mesh, select `edit mesh' in the menu (at top center).\n\
I have done it now for you")); 
      set_editview(i, EDITVIEW_EDIT); 
      return FALSE; 
    }
  if(i!=MAIN_WIN)
    {
      /* hide bar-makes room for editing */
      GtkWidget* hb= lookup_widget  ( sp->im_widget[i]  ,
				      "handlebox_factors");
      GtkWidget* hs= lookup_widget  ( sp->im_widget[i]  ,
				      "handleboxsubimage");
      gtk_widget_hide(hb);gtk_widget_hide(hs);
    }

  gboolean readonly = image_settings_get_value("mesh is readonly",  i);
  {
    int mi,mj,mlabel; enum tools action;
    gboolean b;
    b= gdk_mesh_button_press_event  (widget,event,
				     user_data, &(sp->im_mesh[i]),
				     &mi,&mj,&mlabel, &action,readonly);
    if(mlabel>0)
      feat_entry_set(mlabel-1);
    if(b && event->button==1) { flash_mi=mi; flash_mj=mj;
      flash_points_id=gtk_timeout_add(200,flash_points,NULL);
      gtk_widget_grab_focus (widget);
    }
    return b;
  }
}




gboolean
on_motion_notify_event                 (GtkWidget       *widget,
                                        GdkEventMotion  *event,
                                        gpointer         user_data)
{
  int i=
    GPOINTER_TO_UINT(gtk_widget_get_data_top(widget,"imagenum")); 
  g_assert(i > 0);

  if(sp->im_editview[i]==EDITVIEW_EYES )
    return gdk_subimagesel_motion_notify_event (widget,event,user_data);

  if( ( i == MAIN_WIN) && sp->max_wins>1 &&
      settings_get_value("automatic mesh interpolation")) {
    mag_track(i,event);
    return FALSE;
  }

  /* if the menu says "view", do not edit */
  if (sp->im_editview[i] == EDITVIEW_SHOWMESHES ||
      sp->im_editview[i] ==EDITVIEW_SHOW ) {
    mag_track(i,event);
    return FALSE;
  }

  {
    int mi,mj;
    //g_warning("mesh");
    gboolean b=gdk_mesh_motion_notify_event   (widget,event,
					       user_data, &(sp->im_mesh[i]),
					       &mi,&mj);  
    mag_track(i,event);
    
    return b;
      {
	//if(settings_get_value("mesh auto sync"))
	//  flash_points(NULL);

      }
  }

}

void redraw_images()
{
  int lp=MAX_WINS; for(; lp>=0; lp--) 
    if(sp->im_drawingarea_widget[lp] != NULL) { 
      MY_GTK_DRAW(sp->im_drawingarea_widget[lp]);
    }
  all_images_need_redraw=0;
}

gboolean
on_drawingarea_button_release_event    (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{

  int i=
    GPOINTER_TO_UINT(gtk_widget_get_data_top(widget,"imagenum")); 
  gboolean bool;
  if(flash_points_id)
    gtk_timeout_remove(flash_points_id);
 
  g_assert(i > 0);
  if(//event->state & GDK_BUTTON1_MASK && 
     sp->im_editview[i]==EDITVIEW_EYES )  {
      bool= gdk_subimagesel_button_release_event (widget,event,user_data);
      if (bool && i == MAIN_WIN )
	all_images_need_redraw++;
  } else {
    // FIXME perche'??????????
    //if( ( i == MAIN_WIN) && sp->max_wins>1) {
    //  if(settings_get_value("automatic mesh interpolation")) {
    //on_interpolate_meshes1_activate(NULL,NULL);
    //	gtk_widget_draw (sp->im_drawingarea_widget[MAIN_WIN] , NULL);
    // }
    //return FALSE;
    //}

    /* if the menu says "view", do not edit */
    if (sp->im_editview[i] == EDITVIEW_SHOWMESHES || sp->im_editview[i]==EDITVIEW_SHOW )
      return FALSE;
    
    { 
      int mi,mj; 
      bool=gdk_mesh_button_release_event    ( widget,event,
					      user_data, &(sp->im_mesh[i]),
					      &mi,&mj);
      if( settings_get_value("auto point adjust")) {
	int j=i-1;
	while (j>0 && (!sp->im_widget[j] || ! sp->im_warped_pixbuf[j]))
	  j--;
	if(j>0) {
	  MeshT *srcmesh =&(sp->im_mesh[j]);
	  GdkPixbuf *src = (sp->im_subimage_pixbuf[j]);
	  MeshT *mesh=&(sp->im_mesh[i]);
	  GdkPixbuf *dst= (sp->im_subimage_pixbuf[i]);	
	  double nx,ny;	
	  double ox=meshGetx(mesh,mi,mj),oy=meshGety(mesh,mi,mj);
	  gboolean e=detect_translation
	    (src,meshGetx(srcmesh,mi,mj),meshGety(srcmesh,mi,mj),
	     dst,ox,oy,
	     // new suggested destination 
	     &nx,&ny);
	  if ( (e) ) {
	    meshSetNoundo(mesh,mi,mj,nx,ny);
	    g_debug("auto point adjust from %g %g to %g %g",ox,oy,nx,ny);
	}
      }
      }

      if( settings_get_value("automatic mesh interpolation")&& sp->max_wins>1)
	{
	  on_interpolate_meshes1_activate(NULL,NULL);	 
	  MY_GTK_DRAW (sp->im_drawingarea_widget[MAIN_WIN]);
	}      
      set_frame_label(i);

 ///this needs a lot of work
      if(settings_get_value("cursor jump"))
      {
	GtkWidget *d=gtk_widget_get_data_top((sp->im_widget[i]),
					     "scrolledwindow_image");
	//if(d)
	// d=gtk_widget_get_parent(d);
	if(d) {
	  gint width, height;
#if GTK_MAJOR_VERSION == 1
	  //gtk_widget_get_usize(d,&width,&height);
    //  gdk_window_get_size(drawable,&width,&height);
#else
    // gdk_drawable_get_size(drawable,&width,&height);
#endif
	  GtkAdjustment* H=gtk_scrolled_window_get_hadjustment
	    (GTK_SCROLLED_WINDOW(d));
	  GtkAdjustment* V=gtk_scrolled_window_get_vadjustment
	    (GTK_SCROLLED_WINDOW(d));

	  width=d->allocation.width;
	  height=d->allocation.height;

	  //g_message("scroll frame window %d %d", width,height);
	  if(H && V) {
	    H->value=event->x-width/2;
	    V->value=event->y-height/2;
	    gtk_scrolled_window_set_hadjustment(GTK_SCROLLED_WINDOW(d),H);
	    gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(d),V);
	    gtk_adjustment_value_changed(H);
	    gtk_adjustment_value_changed(V);
	  }  else  g_warning("adjustments of scrolls %i unavailable",i);
	} else  g_warning("scrooolls %i unavailable",i);
      }


    }
  }


  if ( all_images_need_redraw) {
    redraw_images();
  }
  else { 
    MY_GTK_DRAW (widget);  
  }
  return bool;
}



