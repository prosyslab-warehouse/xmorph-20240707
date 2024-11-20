


void 
gdk_pixbuf_clear(GdkPixbuf *pb);


/******** measures square distance between images ***
 * useful for matching subimages
*/


double long gdk_pixbuf_sqrdist(GdkPixbuf *p1,GdkPixbuf *p2);


/*************** gdk_pixbuf_is_subimage ()

 simple test: is one image contained in the other ? 
*/

gboolean
gdk_pixbuf_is_subimage (int src_x,
			int src_y,
			int width,
			int height,
			int dst_width,
			int dst_height
			);

/********************************************   gdk_pixbuf_subimage ()


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
		      int height);

