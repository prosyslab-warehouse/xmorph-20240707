

#include <gdk/gdk.h>

#include <gdk-pixbuf/gdk-pixbuf.h>


#include "gdk-pixbuf-extra.h"

#include "string.h"

/*************** gdk_pixbuf_is_subimage ()

 simple test: is one image contained in the other ? 
*/


void 
gdk_pixbuf_clear(GdkPixbuf *pb)     
{
  int     width =  gdk_pixbuf_get_width(pb),
    height = gdk_pixbuf_get_height(pb);

  long size = width *  height;

  guchar  *data = gdk_pixbuf_get_pixels(pb);	

  memset(data, 0, size * gdk_pixbuf_get_n_channels(pb) );
}











/******** measures square distance between images ***
 * useful for matching subimages
*/


double long gdk_pixbuf_sqrdist(GdkPixbuf *p1,GdkPixbuf *p2)
{
  int i,j, r1 , r2, w,w1,w2, h,h1,h2, c;
  int r;
  double res=0;
  guchar * d1, * d2;

  c=gdk_pixbuf_get_n_channels(p1);
  g_assert (!gdk_pixbuf_get_has_alpha(p1) 
	    || !gdk_pixbuf_get_has_alpha(p2)
	    || 8!=gdk_pixbuf_get_bits_per_sample(p2)
	    || 8!=gdk_pixbuf_get_bits_per_sample(p1)
	    || c != gdk_pixbuf_get_n_channels(p2)
	    );

  d1=gdk_pixbuf_get_pixels(p1) ;
  d2=gdk_pixbuf_get_pixels(p2) ;
  r1= gdk_pixbuf_get_rowstride(p1); 
  r2= gdk_pixbuf_get_rowstride(p2);
  w1=gdk_pixbuf_get_width(p1);
  w2=gdk_pixbuf_get_width(p2);
  h1=gdk_pixbuf_get_width(p1);
  h2=gdk_pixbuf_get_width(p2);
  w=MIN(w1,w2);
  h=MIN(h1,h2);
  for(j=h; j>=0; j--) 
    for(i=0; i < w*c ; i++){
      r=(d1[ i + r1 * j] - d2[ i + r2 * j]);
      res += r*r;
      }
  return res / (double)w /  (double)h ; 
}












static void 
gdk_pixbuf_subimage_GdkPixbufDestroyNotify(guchar *pixels,
					   gpointer data)
{
  gdk_pixbuf_unref( (GdkPixbuf*) data );
}

gboolean
gdk_pixbuf_is_subimage (
			int src_x,
			int src_y,
			int width,
			int height,
			int dst_width,
			int dst_height
			)
{
  if ( src_x < 0 || src_y < 0 
       || src_x + width > dst_width
       || src_y + height > dst_height
       )  
    return FALSE;
  else
    return TRUE;
}
/* 


     src_pixbuf :
                Source pixbuf.
          src_x :
                Source X coordinate within src_pixbuf.
          src_y :
                Source Y coordinate within src_pixbuf.
          width :
                Width of the area to copy.
         height :
                Height of the area to copy.
   

	returns : subimage pixbuf, or NULL if there is an error

   This function is similar to    gdk_pixbuf_copy_area()
   but it uses the same buffer as the buffer in    src_pixbuf

   for this reason, the subimage must be contained in the given image 
 */

GdkPixbuf*
gdk_pixbuf_subimage  (GdkPixbuf *src_pixbuf,
		      int src_x,
		      int src_y,
		      int width,
		      int height)
{
  if( src_pixbuf == NULL)
    {
      g_warning(  "%s (in %s:%d): %s\n",	
		 __FUNCTION__ , __FILE__ , __LINE__     ,
		 ( "src_pixbuf is null" ));
      return NULL;
    }
  if (  width < 1 || height < 1      )    {
    g_warning(  "%s (in %s:%d): %s\n",	
		__FUNCTION__ , __FILE__ , __LINE__     ,       
		( "subimage has zero size" ));
    return NULL;
  }
  if ( ! gdk_pixbuf_is_subimage ( src_x,
				  src_y,
				  width,
				  height,
				  gdk_pixbuf_get_width(src_pixbuf),
				  gdk_pixbuf_get_height(src_pixbuf)) )   {
    g_warning(  "%s (in %s:%d): %s\n",	
		__FUNCTION__ , __FILE__ , __LINE__     ,       
		( "subimage is not contained in image" ));
    return NULL;
  }
  {
    /* bytes per sample */
    int b= gdk_pixbuf_get_n_channels(src_pixbuf)  *
      gdk_pixbuf_get_bits_per_sample(src_pixbuf) / 8;

    /* increment the reference count.
       this way, we are sure that the buffer will not disappear
       beneath this pixbuf 
    */      
    gdk_pixbuf_ref(   src_pixbuf);
    /* the ref count will be decremented when the
       subimage pixbuf will be destroyed 
    */      

    return  gdk_pixbuf_new_from_data       
      (gdk_pixbuf_get_pixels(src_pixbuf) //+
       + gdk_pixbuf_get_rowstride(src_pixbuf) * src_y +    src_x * b,

       gdk_pixbuf_get_colorspace(src_pixbuf),
       gdk_pixbuf_get_has_alpha(src_pixbuf),
       gdk_pixbuf_get_bits_per_sample(src_pixbuf),
       width, 
       height, 
       gdk_pixbuf_get_rowstride(src_pixbuf), //rowstride
      gdk_pixbuf_subimage_GdkPixbufDestroyNotify,
       (gpointer)src_pixbuf//gpointer destroy_fn_data
       );
  }
}
