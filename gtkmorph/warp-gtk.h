
#ifndef __WARP_GTK_H__
#define __WARP_GTK_H__

#include <gdk-pixbuf/gdk-pixbuf.h>


inline
void warp_image_gdk_a
( GdkPixbuf *src, GdkPixbuf *dst,
  const double *xs, const double *ys, const double *xd, const double *yd,
  int mesh_width, int mesh_height)
{
  warp_image_versatile
    (gdk_pixbuf_get_pixels(src),
     gdk_pixbuf_get_width(src),gdk_pixbuf_get_height(src),
     gdk_pixbuf_get_n_channels(src),gdk_pixbuf_get_rowstride(src),gdk_pixbuf_get_n_channels(src),
     gdk_pixbuf_get_pixels(dst),
     gdk_pixbuf_get_width(dst),gdk_pixbuf_get_height(dst),
     gdk_pixbuf_get_n_channels(dst),gdk_pixbuf_get_rowstride(dst),gdk_pixbuf_get_n_channels(dst),
     xs,  ys, xd,  yd,
     mesh_width,  mesh_height);
}

inline
void warp_image_gdk_m
( GdkPixbuf *src, GdkPixbuf *dst,
  MeshT *srcmesh, MeshT *dstmesh)
{
  g_return_if_fail( srcmesh->nx == dstmesh->nx && srcmesh->ny == dstmesh->ny);
  warp_image_versatile
    (gdk_pixbuf_get_pixels(src),
     gdk_pixbuf_get_width(src),gdk_pixbuf_get_height(src),
     gdk_pixbuf_get_n_channels(src),gdk_pixbuf_get_rowstride(src),gdk_pixbuf_get_n_channels(src),

     gdk_pixbuf_get_pixels(dst),
     gdk_pixbuf_get_width(dst),gdk_pixbuf_get_height(dst),
     gdk_pixbuf_get_n_channels(dst),gdk_pixbuf_get_rowstride(dst),gdk_pixbuf_get_n_channels(dst),

     srcmesh->x, srcmesh->y, 
     dstmesh->x, dstmesh->y, 
     srcmesh->nx,srcmesh->ny);
}


#endif // __WARP_GTK_H__
