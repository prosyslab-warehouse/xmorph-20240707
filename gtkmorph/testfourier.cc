#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

//#include <math.h>

#include <stdlib.h>
#include <cstring>

#define DEBUG

#include "fourier.hh"
#include "fourier.cc"

#include "stdio.h"

void help()
{ printf("Usage: testfourier -s|-c si sx sy di dx di [oi]\n\
 si sx sy = reference image, position  x , y\n\
 di dx dy = editing    image , position x,y\n\
 oi       = output of editing image with patch translated\n\
            from source to destination\n\n\
Will output the new suggested value for dx,dy\n"); }


void test_fft()
{
  for (int c=0;c<3;c++) {
    F_T  
      *srcf = prealloc_F_T + size*size*c,
      *dstf = prealloc_F_T + size*size*6,
      *cmpf = prealloc_F_T + size*size*( (c==0)?1:0);
    for (int i=0; i < size ; i++) {
      for (int j=0; j < size ; j++)  
	srcf[IND(i,j)]=drand48()*100+101;
    }
    fourier(srcf,dstf);    
    fourier_inv(dstf,cmpf);
    for (int i=0; i < size ; i++) {
      for (int j=0; j < size ; j++) { 
	g_assert( norm(cmpf[IND(i,j)] - srcf[IND(i,j)] ) <= 0.0001 ); 
      }}
  }
  {
    int n_channels=3;
    F_T  *src = prealloc_F_T;
    double e = correlation(src,src,n_channels,0,0) ;
    double f = ( energy(src,n_channels));
    g_assert( ABS(1-e/f) < 0.0001 );
    {
      double dx = correlation_dx(src,src,n_channels,0,0) ,
	dy = correlation_dy(src,src,n_channels,0,0) ;
      g_assert(ABS(dx/e)<0.01);
      g_assert(ABS(dy/e)<0.01);
    }
    {
      double dx = correlation_dx(src,src,n_channels,0,0) ,
	dy = correlation_dy(src,src,n_channels,0,0) ;
      printf(" test random dx %g dy %g \n",dx/e,dy/e);
    }
  }
}

int main(int argc,char* argv[])
{
  if(argc<7)
    { help() ; return -1 ; }

  int cheat=0;
  if(0==strcmp("-c",argv[1])) {
    cheat=1;
  }  else if(0!=strcmp("-s",argv[1]))
    { help() ; return -1 ; }

  argc--; argv++;

  double sx=atof(argv[2]), sy=atof(argv[3]),
    dx=atof(argv[5]),dy=atof(argv[6]) ,  tx,ty;

  gtk_set_locale ();
  gtk_init(&argc, &argv);
  gdk_rgb_init();

  fourier_init();

#if GTK_MAJOR_VERSION >= 2
#define ARG  ,NULL
#endif
  GdkPixbuf *src,*dst;
  src=gdk_pixbuf_new_from_file(argv[1] ARG);
  if(!src) { perror(argv[1]); exit(1);}
  dst=gdk_pixbuf_new_from_file(argv[4] ARG);
  if(!dst) { perror(argv[4]); exit(1);}

  g_return_val_if_fail( (gdk_pixbuf_get_n_channels (src)) ==
			(gdk_pixbuf_get_n_channels (dst)),1);

  //  testings of basic stuff
  test_fft();
#if USE_WINDOW == 0
  {
    GdkPixbuf *pixbuf;
    pixbuf=gdk_pixbuf_new_from_file(argv[1] ARG);
    if(!pixbuf) { perror(argv[1]); exit(1);}
    guint n_channels = gdk_pixbuf_get_n_channels (pixbuf);
    int width = gdk_pixbuf_get_width (pixbuf);
    int height = gdk_pixbuf_get_height (pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride (pixbuf);
    F_T * srcf = prealloc_F_T;
    fourier_image(src,srcf, (int)sx, (int)sy);
    inv_fourier_image(srcf, pixbuf, (int)sx, (int)sy) ;
    guchar *sp = gdk_pixbuf_get_pixels (src);
    guchar *cp = gdk_pixbuf_get_pixels (pixbuf);
    for (int c=0;c<3;c++) {
      for (int i=0; i < height; i++) {
	for (int j=0; j < width ; j++) {
	  int d=(i) * rowstride + (j) * n_channels; 
	  int v = sp[d]-cp[d];
	  g_assert( ABS(v)==0) ;
	}}}
  }
#endif

  //testing of translation detection
  if(cheat) {
    tx=dx; ty=dy;
  } else
    detect_translation(src,sx,sy, dst,dx,dy,  &tx, &ty);
  printf("tx %g ty %g\n",tx,ty);
  
  if(argc>7)
    {      dx=tx; dy=ty; double isx=floor(sx), isy=floor(sy),
	idx=floor(dx), idy=floor(dy);
      if (cheat)
	{ isx=idx; isy=idy;  }
      double fsx=sx-isx , fsy=sy-isy , fdx=dx-idx , fdy=dy-idy;
      double x = fdx-fsx,     y = fdy-fsy;
      F_T * srcf = prealloc_F_T;

      fourier_image(src,srcf, (int)isx , (int)isy) ; 
      int n_channels =gdk_pixbuf_get_n_channels (src);
      printf("delta x %g  delta y %g\n",x,y);
      for (int i=0; i < size ; i++) {
	for (int j=0; j < size ; j++) { 
	  F_T v=fourier_transl(i, j,x,y);
	  for (int c=0; c < n_channels;c++) {
	    F_T *s=srcf+c*size*size;
	    (s[IND(i,j)]) = s[IND(i,j)] *  v;
	  }}}      
      inv_fourier_image(srcf, dst, (int)idx, (int)idy) ;
      GError *error=NULL;gboolean result=TRUE;
      result = gdk_pixbuf_save(dst,argv[7],"png",&error,NULL);
      g_assert ((result  && !error ) || (!result && error ));
      if(error)
	{g_warning("%s",(error)->message);g_error_free (error);}   
  }
  return 0;
		     
}
