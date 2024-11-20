#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>

#include <gtk/gtk.h>

#include "gtk-meta.h"

#ifdef USE_IMLIB
#include <gdk_imlib.h>
#else
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

#include "gdk-pixbuf-extra.h"

#include "gtktopdata.h"

#include "libmorph/warp.h"
#include "libmorph/warp2.h"
#include "libmorph/warp-gtk.h"
#include "libmorph/resample.h"

#include "callbacks.h"
#include "interface.h"
#include "support.h"
//#include "pixmaps.h"
#include "mesh-gtk.h"
#include "utils.h"
#include "main.h"
#include "settings.h"
#include "dialogs.h"

#include "libmorph/mesh.h"

#if HAVE_LIBPLY_H
#include "../plyview/bind.h"
#endif





/***************************** colors ***************/


/* color flashing */
GdkGC *flash_gc;


GdkColor    colors[] =   {
  //selected points, 
  {    0L,     63 * 1024,     63 * 1024,     40 * 1024   },
  //unselected points. This color is used also for lines
  {    0L,     40 * 1024,     50 * 1024,     40 * 1024   },

  //all these are for features
  //so, to change from feature_n to feature_gc you have to add 2
  //so, to change from feature_n to mesh label you have to add 1
  //{    0L,     63 * 1024,     63 * 1024,     63 * 1024   },
  //{    0L,     63 * 1024,     63 * 1024,     30 * 1024   },
  {    0L,     63 * 1024,     30 * 1024,     30 * 1024   },
  {    0L,     63 * 1024,     30 * 1024,     63 * 1024   },
  {    0L,     30 * 1024,     63 * 1024,     63 * 1024   },
  {    0L,     30 * 1024,     63 * 1024,     30 * 1024   },
  {    0L,     30 * 1024,     30 * 1024,     63 * 1024   },
  {    0L,     30 * 1024,     30 * 1024,     30 * 1024   },

  //{    0L,     63 * 1024,     63 * 1024,     63 * 1024   },
  {    0L,     63 * 1024,     63 * 1024,     50 * 1024   },
  {    0L,     63 * 1024,     50 * 1024,     63 * 1024   },
  {    0L,     63 * 1024,     50 * 1024,     50 * 1024   },
  {    0L,     50 * 1024,     63 * 1024,     63 * 1024   },
  {    0L,     50 * 1024,     63 * 1024,     50 * 1024   },
  {    0L,     50 * 1024,     50 * 1024,     63 * 1024   },
  {    0L,     50 * 1024,     50 * 1024,     50 * 1024   },

  //{    0L,     63 * 1024,     63 * 1024,     63 * 1024   },
  {    0L,     63 * 1024,     63 * 1024,     10 * 1024   },
  {    0L,     63 * 1024,     10 * 1024,     63 * 1024   },
  {    0L,     63 * 1024,     10 * 1024,     10 * 1024   },
  {    0L,     10 * 1024,     63 * 1024,     63 * 1024   },
  {    0L,     10 * 1024,     63 * 1024,     10 * 1024   },
  {    0L,     10 * 1024,     10 * 1024,     63 * 1024   },
  //{    0L,     10 * 1024,     10 * 1024,     10 * 1024   },

  //{    0L,     40 * 1024,     40 * 1024,     40 * 1024   },
  {    0L,     40 * 1024,     40 * 1024,     30 * 1024   },
  {    0L,     40 * 1024,     30 * 1024,     40 * 1024   },
  {    0L,     40 * 1024,     30 * 1024,     30 * 1024   },
  {    0L,     30 * 1024,     40 * 1024,     40 * 1024   },
  {    0L,     30 * 1024,     40 * 1024,     30 * 1024   },
  {    0L,     30 * 1024,     30 * 1024,     40 * 1024   },
  {    0L,     30 * 1024,     30 * 1024,     30 * 1024   }

};
GdkGC  *features_gc[100];
const int features_max_n=sizeof(colors)/sizeof(GdkColor)-2;


/* color for selected points and subimage selection*/
GdkGC   *mps_gc;
/* color for resulting points */
//GdkGC   *mpr_gc;
/* color for mesh lines */
GdkGC *mpl_gc;


void allocate_colors(GtkWidget * widget)
{
  /* allocate colors */
  int lp=sizeof(colors)/sizeof(GdkColor)-1;

  g_debug("allocating colors");

  //features_max_n=lp-2;

  g_assert(widget->window);
  g_assert(lp<100);
  for(;lp>=0;lp--) {
    features_gc[lp] = gdk_gc_new(widget->window);
    gdk_gc_copy(features_gc[lp], widget->style->white_gc );
    if(gdk_colormap_alloc_color (gdk_colormap_get_system (),
				 & (colors[lp]),
				 FALSE , //gboolean writeable,
				 TRUE //gboolean best_match
				 )
       == FALSE)
      g_warning("%s %d : can't allocate color\n",__FILE__,__LINE__);
    else
      gdk_gc_set_foreground ( features_gc[lp], &(colors[lp])); 
    g_assert(features_gc[lp]);
  }

  //FIXME CRUDE PROGRAMMING
  
  mpl_gc=gdk_gc_new(widget->window);
  g_assert(mpl_gc);
  gdk_gc_copy(mpl_gc, features_gc[1] );
  gdk_gc_set_line_attributes (mpl_gc,
			      1, //gint line_width,
			      GDK_LINE_DOUBLE_DASH,//GdkLineStyle line_style,
			      GDK_CAP_BUTT, //GdkCapStyle cap_style,
			      GDK_JOIN_MITER //GdkJoinStyle join_style
			      );

  mps_gc=gdk_gc_new(widget->window);
  g_assert(mps_gc);
  gdk_gc_copy(mps_gc, features_gc[2] );
  gdk_gc_set_line_attributes (mps_gc,
			      1, //gint line_width,
			      GDK_LINE_DOUBLE_DASH,//GdkLineStyle line_style,
			      GDK_CAP_BUTT, //GdkCapStyle cap_style,
			      GDK_JOIN_MITER //GdkJoinStyle join_style
			      );

#ifdef FIXME
  /* the xor method  flashes on/off 
       we should use the same clipping rectangle as above, but we dont */
  gdk_gc_set_function(mpl_gc,GDK_XOR);
#endif

  flash_gc = gdk_gc_new(widget->window);
  gdk_gc_copy(flash_gc,mpl_gc );
  gdk_gc_set_function(flash_gc,GDK_XOR);
    
  //  mps_gc= features_gc[0];
  //mpr_gc= features_gc[1];

}





























