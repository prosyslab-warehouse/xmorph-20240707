#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>


#include "main.h"
#include "dialogs.h"
#include "gtk-meta.h"
#include "support.h"



/* gboolean mesh_need_saving(int lp){ */
/*   return sp->im_widget[lp] && sp->im_mesh[lp].changed>0; */
/* } */

/* did the user use it somehow?  */
/* gboolean image_used(int lp) */
/* { */
/*   return sp->im_widget[lp] && ( sp->im_mesh_filename[lp] || */
/* 				sp->im_mesh[lp].changed>0 || */
/* 				sp->im_filename_in[lp] ||  */
/* 				sp->im_filename_out[lp]); */
/* } */




void
on_interpolate_meshes1_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  
  int lp,
    nx=sp-> im_mesh[1].nx ,
    ny=sp-> im_mesh[1].ny ;
  double totwarp;

 if( sp->max_wins<=1)
    {
      show_warning( _("\
to interpolate meshes, you must have at least two input images"));
      return;
    }
 totwarp=0;
 for ( lp =1; lp <= MAX_WINS; lp++)
   if( sp->im_widget[lp] != NULL && !sp->im_widget_is_difference_mesh[lp])
     totwarp += sp->mf.im_warp_factor[lp];

 // FIXME this is not shown
 sp->mf.im_warp_factor[MAIN_WIN]=totwarp;
 
 if ( ABS(totwarp)< 0.0001) {
   show_warning( _("\
to interpolate the meshes, the sum of the all `mesh factors' should be nonzero\n\
I have set some default values for you") );
   do_mesh_factors_equal();
   totwarp =1;
   redraw_spins(-3);
 }

 /*check to be on the safe side */
 for ( lp =1; lp <= MAIN_WIN; lp++)
   if( sp->im_widget[lp] != NULL)
     g_return_if_fail( meshAllocated( &(sp->im_mesh[lp]) ));

 {
   int xi, yi;  
   double vx,vy;
     
     for(yi=0; yi < ny; yi++) {
       for(xi=0; xi < nx; xi++) {
	 vx=vy=0;
	 for ( lp =1; lp <= MAX_WINS; lp++)
	   if( sp->im_widget[lp] != NULL)
	     {
	       vx += meshGetx( &(sp->im_mesh[lp]), xi, yi)
		 * sp->mf.im_warp_factor[lp];
	       vy += meshGety( &(sp->im_mesh[lp]), xi, yi)
		 * sp->mf.im_warp_factor[lp];	      
	     }
	 //printf("%d %d %f %f\n",  xi, yi, vx, vy);
	 meshSetNoundo((&(sp->im_mesh[MAIN_WIN])),  xi, yi, vx /totwarp, vy /totwarp);
       }}
  }
  for ( lp =1; lp <= MAIN_WIN; lp++)
    if( sp->im_drawingarea_widget[lp] != NULL)
      MY_GTK_DRAW(sp->im_drawingarea_widget[lp]);
}

