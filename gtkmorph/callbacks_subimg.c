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

#include "main.h"
#include "support.h"

/***************************************************************************
**************************************************************************

                  handlebox of ranges 

              ****************************

  used to enter the numerical values for im_warp_factor
  and  im_dissolve_factor

**********************************************************************
 */



void 
on_handlebox_factors_show                     (GtkWidget       *widget,
					       gpointer         user_data)
{
/*   int i= */
/*     GPOINTER_TO_UINT(gtk_widget_get_data_top(widget,"imagenum"));  */
/*   g_assert(i > 0); */
/*   //FIXME this doesnt work since we use a different window in glade */
/*   // so it is actually unused now */
/*   if ( i == MAIN_WIN) */
/*     gtk_widget_hide(widget); */
}


unsigned int lock_factor_normalize= 0;

void redraw_spins(int skip)
{
  //extern unsigned int lock_factor_normalize;
  lock_factor_normalize++;
  int lp;
  GtkWidget *w;
  GtkSpinButton *s;
  for (lp=1 ; lp<=MAX_WINS ; lp++) {
    w=sp->im_widget[lp];
    if ( w != NULL) {
      if( lp!=skip ) {
	s=GTK_SPIN_BUTTON(lookup_widget(w,"spinbutton_mesh"));
	gtk_spin_button_set_value (s,sp->mf.im_warp_factor[lp] * 100);
        if( !sp->im_widget_is_difference_mesh[lp] )
	  {
	    s=GTK_SPIN_BUTTON(lookup_widget(w,"spinbutton_image"));
	    gtk_spin_button_set_value (s,sp->mf.im_dissolve_factor[lp] * 100);
	  }
      }
    }
  }
  lock_factor_normalize--;
}



void im_factor_normalize(double *p, int i)
{
  double t=0,tt=0;
  int lp;
  
  if((p==sp->mf.im_warp_factor && 
      0== settings_get_value("mesh factors sum to 1"))
     || 
     (p==sp->mf.im_dissolve_factor && 
      0== settings_get_value("image factors sum to 1")))
    return;

  for (lp=1 ; lp<=MAX_WINS ; lp++)
    if(sp->im_widget[lp] != NULL && !sp->im_widget_is_difference_mesh[lp] ) {
      tt += p[lp];
      if(i!=lp) t+= p[lp];
    }

  for (lp=1 ; lp<=MAX_WINS ; lp++)
    if(sp->im_widget[lp] != NULL && !sp->im_widget_is_difference_mesh[lp] ) {
      if(i!=lp) {
	if(sp->max_wins==2) 
	  p[lp] = 1-p[i];
	else {
	  if( ABS(t) > 0.01)
	    p[lp] = p[lp] * (1 - p[i]) / t;
	  else
	    p[lp] = (1 - p[i]) / ((double) sp->max_wins-1.0) ;
	}
      }
    }
  if(lock_factor_normalize>0)  {
    g_debug("factor normalize is locked %u",lock_factor_normalize);
    return;
  }	else  	g_debug("factor normalize is not locked");
  lock_factor_normalize++;
  redraw_spins(i);
  lock_factor_normalize--;
}


void
on_spinbutton_mesh_changed             (GtkEditable     *editable,
                                        gpointer         user_data)
{
  GtkSpinButton *spin = GTK_SPIN_BUTTON (editable);
  GtkRange *range=GTK_RANGE(lookup_widget(GTK_WIDGET(editable),
					  "hscale_mesh"));

  double val;
  int i=GPOINTER_TO_UINT(gtk_widget_get_data_top(GTK_WIDGET(editable)
						 ,"imagenum"));

  g_debug ( "spin mesh %d  = %0.*f",i, spin->digits,
  	      gtk_spin_button_get_value_as_float (spin));

  
  sp->mf.im_warp_factor[i]=val=gtk_spin_button_get_value_as_float (spin)/100.0;
  if(!sp->im_widget_is_difference_mesh[i]) 
    im_factor_normalize(sp->mf.im_warp_factor,i);
  g_debug(" spin mesh %d = %g",i,sp->mf.im_warp_factor[i]);
  if(range) {     
    range->adjustment->value=val * 100;
    gtk_signal_emit_by_name (GTK_OBJECT (range->adjustment), "changed");
#ifndef IS_PLYMORPH
    if( settings_get_value("automatic mesh interpolation")&& sp->max_wins>1)
      on_interpolate_meshes1_activate        (NULL,NULL);
#endif
  } else g_critical("no adjustment");    
}


void
on_spinbutton_image_changed            (GtkEditable     *editable,
                                        gpointer         user_data)
{
  GtkSpinButton *spin = GTK_SPIN_BUTTON (editable);
  //GtkRange *range=GTK_RANGE(user_data);
  GtkRange *range=GTK_RANGE(lookup_widget(GTK_WIDGET(editable),
					  "hscale_image"));
  double val;
  int i=GPOINTER_TO_UINT(gtk_widget_get_data_top(GTK_WIDGET(editable),
						 "imagenum"));
    
  g_debug ( "spin image %d = %0.*f",i, spin->digits,
  	      gtk_spin_button_get_value_as_float (spin));
  
  sp->mf.im_dissolve_factor[i]=val=gtk_spin_button_get_value_as_float (spin)/100.0; 
  im_factor_normalize(sp->mf.im_dissolve_factor, i);
  g_debug(" spin image %d = %g",i,sp->mf.im_warp_factor[i]);
 
  if(range) {
    range->adjustment->value=val * 100;
    gtk_signal_emit_by_name (GTK_OBJECT (range->adjustment), "changed");

#ifndef IS_PLYMORPH
    if( settings_get_value("automatic blending")&& sp->max_wins>1
	&& sp-> im_warped_pixbuf[MAIN_WIN] && sp-> im_warped_pixbuf[i]
	&& sp-> im_warped_pixmap[MAIN_WIN] && sp-> im_warped_pixmap[i]
	&& lock_factor_normalize == 0)
      on_do_mixing_clicked(NULL,NULL);
#endif
  } else g_critical("no v adjustment");
}

/*************/

#if GTK_MAJOR_VERSION == 1
gboolean
on_hscale_mesh_button_release_event    (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
#else
void on_hscale_mesh_changed    (GtkRange *range,
				gpointer         user_data)
#endif
{
#if GTK_MAJOR_VERSION == 1
  GtkRange *range=GTK_RANGE(widget);
#endif
  GtkSpinButton *spin=GTK_SPIN_BUTTON
    (lookup_widget(GTK_WIDGET(range),"spinbutton_mesh"));
  if(spin) {
    gtk_spin_button_set_value (spin, range->adjustment->value);
  } else g_critical("no adjustment");
  return
#if GTK_MAJOR_VERSION == 1
    FALSE
#endif
    ;
}

/**************/

#if GTK_MAJOR_VERSION == 1
gboolean
on_hscale_image_button_release_event    (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
#else
void on_hscale_image_changed    (GtkRange *range,
				 gpointer         user_data)
#endif
{
#if GTK_MAJOR_VERSION == 1
  GtkRange *range=GTK_RANGE(widget);
#endif
  GtkSpinButton *spin=GTK_SPIN_BUTTON
    (lookup_widget(GTK_WIDGET(range),"spinbutton_image"));
  if(spin) {  
    gtk_spin_button_set_value (spin, range->adjustment->value);
  } else g_critical("no adjustment");
  return
#if GTK_MAJOR_VERSION == 1
    FALSE
#endif
    ;
}














