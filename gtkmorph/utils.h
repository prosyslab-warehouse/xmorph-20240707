#ifndef __UTILS_H__
#define __UTILS_H__
#include "main.h"

gchar * compute_title (int j);

GtkWidget * create_menu_of_images(int avoid, GtkSignalFunc  callback ,
				  gboolean avoid_diffs);

//CHE CRETINO CHE SONO #define dstmesh  (&(sp->im_mesh[MAIN_WIN]))

void set_sensitive(GtkWidget * widget, gboolean flag);


/* void */
/* flash_point(GdkDrawable  *drawable, */
/* 	    int x,int y	    ); */

void
destroy_image_win_pixbufs(int lp, int what);
void
destroy_image_win_data(int lp);

void
alloc_image_win_data(int lp, int what);

void
create_and_show_image_win(int lp);

/* void */
/* init_image_win_data_and_set_all(int lp); /\* the slot where the data are put */
/* 					   in the arrays in sp-> *\/ */

void
editview_callback(int i);

void
set_editview(int lp, /* window number */
	     int status);



void
pixbuf_to_rrrgggbb(GdkPixbuf *pb, guint8 *r, guint8 *g, guint8 *b);


/* void */
/* rrrgggbbb_add_to_pixbuf(GdkPixbuf *pb, /\* destination *\/ */
/* 			const guint8 *r, const  guint8 *g, const guint8 *b, */
/* 			/\* source *\/ */
/* 			double factor /\* dissolving factor *\/ */
/* 			); */

void
do_warp_an_image_old(int lp); //, char * r, char *g, char * b);

void
do_warp_an_image_new(int lp);

#define do_warp_an_image do_warp_an_image_new

/**********   showerr()
 *  shows libc error 
 */

inline  void 
showerr(const char *file, const  char *msg);





/**********************************************************
 * accepts new backing pixmap of the appropriate size 
 *  save it in the data of the top window 
 * and in the sp-> structure
 * resizes the viewport
*/

gboolean
set_backing_pixmap        (GtkWidget       *widget,
		   GdkPixmap *newpixmap,
		   int width, int height);



/********************************************************************
		loads image from pixbuf to pixmap for image window "lp"
		you should unref the pixbuf   when it exists
		
		copies its data to r,g,b,
*/


void scale_loaded_pixbuf_to_pixmap_et_rrggbb(
					     // GdkPixbuf      *impixold,
		 int lp //image number
		 );



/****************************************************************************
 promote meshes to have same lines and columns 
***************************************************************************/

void promote_meshes();
/***********************************************************************
 which image are we displaying? 

   */
enum what_display{ PIXLOADED=0, PIXSUBIMAGE=1, 
		   PIXWARPED=2};

GdkPixbuf **
which_pixbuf_is_visible(int lp);

GdkPixmap **
which_pixmap_is_visible(int lp);

GdkPixbuf **
which_pixbuf(int lp, enum what_display what);

GdkPixmap **
which_pixmap(int lp, enum what_display what);


void
destroy_pixbuf(int lp, enum what_display what);

void
destroy_pixmap(int lp, enum what_display what);

#define PIXLOADEDBORDER 100
#define PIXEDITBORDER 30

static inline void
which_pixbuf_size(int lp, enum what_display what, int *w, int *h)
{
  *w=sp->resulting_width; *h=sp->resulting_height;
  if(what == PIXLOADED)
    { *w=sp->im_width[lp] ;  *h=sp->im_height[lp]; }
}


void
create__pixbuf(int lp, enum what_display what);

void
create__pixmap(int lp, enum what_display what);

static inline void
which_pixbuf_size_is_visible(int lp, int *w, int *h)
{
  which_pixbuf_size( lp,sp->which_pix[lp] , w, h);
}


void 
render_pixmap(int lp, enum what_display what);

/***************************************************************

 *		drawing area bookkeeping

 ***********************************************************
*/ 


/* creates new backing pixmap of the appropriate size 
   save it in the data of the top window

   it uses: the image filename, to load the image, or, if none
   it creates a black pixmap

   it sets what is necessary for properly displaying the
   drawing area*/


void
drawingarea_configure (int i);


void
setup_handlebox_factors();
void
setup_handlebox_factor(int lp, int force);



void   set_frame_label(int i);

void set_info_label(gchar *t);



/* #ifdef GDK_PIXBUF_MAJOR /\* interfaces accepting GdkPixbufs *\/ */
/* #include "libmorph/warp2.h" */

/* #endif */

#endif //__UTILS_H__
