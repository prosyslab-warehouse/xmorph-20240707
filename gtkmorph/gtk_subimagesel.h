#ifndef __GTK_SUBIMAGE_SEL_H__
#define __GTK_SUBIMAGE_SEL_H__

#include <gdk/gdk.h>
#include <stdio.h>

typedef struct gtk_subimage_sel_t_ gtk_subimage_sel_t ;

struct  gtk_subimage_sel_t_ {
  /* this is the size of the original image */
  unsigned int orig_width;
  unsigned int orig_height;

  /** this is the position of the 'eyes&mouth' : 3 points that we use
      to put all images in the same positions
  **/  
  double eyes[3][2];

  /* the rectangle in the subimage that is zoomed to 
     resulting_width, resulting_height
  */
  GdkRectangle subimage;

  //int magic;
} ;


void gtk_subimasel_save(gtk_subimage_sel_t * sis, FILE *f);

int gtk_subimasel_load(gtk_subimage_sel_t * sis, FILE *f);



void subimage2affines(int lp);



void gtk_subimasel_reset(gtk_subimage_sel_t * sis,
			 unsigned int w,
			 unsigned int h);



gboolean
gdk_subimagesel_button_press_event (GtkWidget       *widget,
				 GdkEventButton  *event,
				    gpointer         user_data);



gboolean
gdk_subimagesel_motion_notify_event (GtkWidget       *widget,
				     GdkEventMotion  *event,
				     gpointer         user_data);

gboolean
gdk_subimagesel_button_release_event (GtkWidget       *widget,
				      GdkEventButton  *event,
				      gpointer         user_data);

gboolean
gdk_subimagesel_draw(GdkDrawable  *drawable,
		     gtk_subimage_sel_t * sis,
		     GdkGC *mpl_gc);


void
on_reset_subimage_clicked              (GtkButton       *button,
                                        gpointer         user_data);




gboolean
on_spinbuttonx_expose                  (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

gboolean
on_spinbuttony_expose                  (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

gboolean
on_spinbuttonw_expose                  (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

gboolean
on_spinbuttonh_expose                  (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

#endif // __GTK_SUBIMAGE_SEL_H__
