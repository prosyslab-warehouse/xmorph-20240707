/*****************************************************************
calibration of points position thru Fourier transform
*/
#include "config.h"

#include <gtk/gtk.h>
#ifdef USE_IMLIB
#include <gdk_imlib.h>
#else
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif
#include "gtk-meta.h"

#include <math.h>

#include <complex>
#include <stdlib.h>

#include "fourier.hh"

#define GOOD_TRANSLATION_MATCH(A)  ((A) >= 0.95)
#define BEST_TRANSLATION_MATCH(A)  ((A) >= 0.9997)

//complex exponential
static inline std::complex<double> expI(double a)
{
  std::complex<double>  e( cos(a), sin(a));
  return e;
}

const int size = 32;
const double S2= (double)size*(double)size;
const double  M2PI (2*M_PI);
const double  M2PIS = (2.0 * M_PI/(double)size);

//row major indexing, that is, C style
#define IND(I,J) ((I)+(size)*(J))


//type of data for fourier transform
typedef  std::complex<double>  F_T;

#if HAVE_FFTW
#include <fftw3.h>
#define F_T_C(x)  reinterpret_cast<fftw_complex*>(x)
// according to 
// file:///usr/share/doc/fftw3-doc/html/Complex-numbers.html#Complex%20numbers
// this should work with fftw as well , by using the cast F_T_C
// but I double check
#else
#define F_T_C(x)  (x)
//#define newF()  (new F_T[size*size*4])
//#define deleteF(p) delete (p)
#endif

// precomputed fourier matrix and vector
#if HAVE_FFTW == 0
F_T *F_MAT     =new  F_T[size*size];
#endif
F_T *F_VEC     =new  F_T[size];

//use windowing
#define USE_WINDOW 0

//use full formula to compute translation
// currently it is buggy
#define FULL 0

#if USE_WINDOW
double *window=new  double[size*size];
#endif

#if HAVE_FFTW
fftw_plan fftw_prestored_plan_forw=NULL,  fftw_prestored_plan_back=NULL;
#endif
F_T * prealloc_F_T=NULL; F_T * prealloc_F_T_tmp=NULL;
void fourier_init_()
{
#if USE_WINDOW
  double s = ((double)(size-1))/2.0;
  for (int i=0; i < size ; i++) {
    for (int j=0; j < size ; j++)
      {
	double r = ((i-s)*(i-s)+(j-s)*(j-s))/s/s;
	if ( r<=1)
	  window[IND(i,j)]= cos(r * M_PI/2);
	else
	  window[IND(i,j)]= 0;
      }}
#endif   
#if HAVE_FFTW
  g_assert ((sizeof(F_T)) == 2*sizeof(double));
  {
    void *p= fftw_malloc(size*size*9*sizeof(F_T));
    prealloc_F_T=  reinterpret_cast<F_T *>(p);
    g_assert(prealloc_F_T);
  }
  { F_T *D =  prealloc_F_T ,  *S = prealloc_F_T +6*size*size;
    fftw_prestored_plan_forw=
      fftw_plan_dft_2d(size, size, F_T_C(S), F_T_C(D),
		       -1, 0);//FFTW_UNALIGNED | FFTW_ESTIMATE);
    fftw_prestored_plan_back=
      fftw_plan_dft_2d(size, size, F_T_C(D), F_T_C(S),
		       1, 0);//FFTW_UNALIGNED | FFTW_ESTIMATE);
 }
#else
  prealloc_F_T=new F_T [size*size*9];
  //precompute fourier matrix
  for (int i=0; i < size ; i++)
    for (int j=0; j < size ; j++)
      F_MAT[IND(i,j)] = expI( - M2PIS*(double)(i*j)) ;
#endif  
  prealloc_F_T_tmp = prealloc_F_T +8*size*size;
  for (int i=0; i < size ; i++)
    F_VEC[i] = expI( M2PIS*i );
}

//complex exponential precomputed
static inline std::complex<double> expI_S(int i)
{
  //is a precomputed version of   return expI( M2PIS*(double)(i));
  if(i<0) {
    i=((-i)%size);
    if(i>0) i=size-i;
  }  else
    i=i%size;
  g_assert(i>=0 && i<size);
  return F_VEC[i];
}

void transf_divide(F_T *D)
{
  const double ss=((double)size);
    for (int j=0; j < size ; j++)
      for (int i=0; i < size ; i++)
	D[IND(i,j)] /= ss;
}



#if HAVE_FFTW
void  fourier(F_T * S, F_T *D)
{
  //fftw_prestored_plan_forw=
  //  fftw_plan_dft_2d(size, size, F_T_C(S), F_T_C(D),-1,FFTW_ESTIMATE);
  g_assert(fftw_prestored_plan_forw);
  fftw_execute_dft(fftw_prestored_plan_forw,F_T_C(S),F_T_C(D));
  transf_divide(D);
}
void  fourier_inv(F_T * S, F_T *D)
{
  //fftw_prestored_plan_back=
  //  fftw_plan_dft_2d(size, size, F_T_C(S), F_T_C(D),1,FFTW_ESTIMATE);
  g_assert(fftw_prestored_plan_back);
  fftw_execute_dft(fftw_prestored_plan_back,F_T_C(S),F_T_C(D));
  transf_divide(D);
}
//#define  fourier(S,D) fourier_fastest_s(S,D,-1)
//#define  fourier_inv(S,D) fourier_fastest_s(S,D,1)
#else
template<class C>   void fourier_orig(const C * S,F_T *D,const int sign)
{
  //original formula
  for (int i=0; i < size ; i++) 
    for (int j=0; j < size ; j++)
      {   D[IND(i,j)]=0;}
  for (int i=0; i < size ; i++)
    for (int j=0; j < size ; j++)
	for (int l=0; l < size;  l++)
	  for (int k=0; k < size ; k++)
	  D[IND(i,j)]  +=   expI_S ( sign* (i*l +j*k) ) * S[IND(l,k)]; 
  transf_divide(D);
}
template <class C> void  fourier_faster(const C * S, F_T *D)
{
  F_T *fouriertmp = new F_T[size*size];
  for (int i=0; i < size ; i++) 
    for (int j=0; j < size ; j++)
      { fouriertmp[IND(i,j)]=0;  D[IND(i,j)]=0;}
  g_assert(norm(F_MAT[1]));//Will trigger if fourier_init was not called
  for (int j=0; j < size ; j++) 
    for (int l=0; l < size ; l++)
      for (int k=0; k < size ; k++) 
	fouriertmp[IND(l,j)] += F_MAT[IND(j,k)] * S[IND(l,k)];
  for (int i=0; i < size ; i++)
    for (int j=0; j < size ; j++) 
      for (int l=0; l < size;  l++)
	D[IND(i,j)]  += F_MAT[IND( i,l)] * fouriertmp[IND(l,j)];
  delete fouriertmp;
  transf_divide(D);
}
#define  fourier(S,D) fourier_faster(S,D)
#define  fourier_inv(S,D) fourier_orig(S,D,1)
#endif


void fourier_image(GdkPixbuf *pixbuf,F_T *transf,
		   int x, int y   )
{
  int width = gdk_pixbuf_get_width (pixbuf);
  int height = gdk_pixbuf_get_height (pixbuf);
  int rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  guchar *pixels = gdk_pixbuf_get_pixels (pixbuf);
  guint n_channels = gdk_pixbuf_get_n_channels (pixbuf);

  g_assert (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB);
  g_assert (gdk_pixbuf_get_bits_per_sample (pixbuf) == 8);
  g_assert (n_channels<=4);
  g_assert ( (size & 1 ) == 0);

  x-=size/2;  y-=size/2;

  int  leftl=0, rightl=size, leftk=0, rightk=size;  
  if(x<0) { leftl=-x;}
  if(y<0) { leftk=-y;}
  if( x+size >=  width  ) { rightl=width -x; }
  if( y+size >=  height ) { rightk=height-y; }
 
  F_T * imagetmp=prealloc_F_T_tmp;
  if (n_channels >= 3) {
    for (int c=0;c<3;c++) {
      for (int j=leftl; j < rightl ; j++) {
	for (int i=leftk; i < rightk ; i++) {
	  guchar * p =pixels + (i+y) * rowstride + (j+x) * n_channels + c;  
	  double v= (double) (*p)
#if USE_WINDOW
	    *  window[IND(i,j)]
#endif
	    ;
	  imagetmp[IND(i,j)]= F_T(v,0);	  
	}}
      fourier(imagetmp ,transf+ size*size*c );
    }
  //       for (int c=0;c<3;c++) {
  // 	for (int i=0; i < size; i++) {
  // 	  for (int j=0; j < size ; j++) {
  // 	    F_T *v;
  // 	    v=(imagef+ (size*size*c + IND(i,j)));
  // 	    *v= F_T(v->real(),0);
  // 	}}}
  } else {
    if (n_channels == 1)
      for (int i=leftk; i < rightk ; i++) {	  
	guchar *  p=pixels + (i+y) * rowstride + (leftl+x);
	for (int j=leftl; j < rightl ; j++) {
	  double v= (double)(*p) 
#if USE_WINDOW
	    *  window[IND(i,j)]
#endif
	    ;
	  p ++;
	  imagetmp[IND(i,j)]=F_T(v);
	}
	fourier(imagetmp,transf);
      }
    else throw "wrong number of channels in pixbuf";
  }
}

void inv_fourier_image(F_T * transf,
		       GdkPixbuf *pixbuf,
		       int x, int y)
{
  guint n_channels = gdk_pixbuf_get_n_channels (pixbuf);
  int width = gdk_pixbuf_get_width (pixbuf);
  int height = gdk_pixbuf_get_height (pixbuf);
  int rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  guchar *pixels = gdk_pixbuf_get_pixels (pixbuf);
  x-=size/2;  y-=size/2;
  g_return_if_fail (x >= 0 && x < width-size &&y >= 0 && y < height-size
		    && n_channels >= 3 );
  F_T * imagetmp=prealloc_F_T_tmp;
  for (int c=0;c<3;c++) {
    fourier_inv(transf+ size*size*c ,imagetmp );
    for (int j=0; j < size ; j++) {
      for (int i=0; i < size; i++) {
	guchar *  p=pixels + (i+y) * rowstride + (j+x) * n_channels + c; 
	double v=(imagetmp[IND(i,j)]).real();
	v=CLAMP( v , 0,255);
	*p =  (guchar)round(v);
      }}}
}


static double energy(//const int size, 
		     F_T *src,
		     int n_channels)
{
  int i,j;
  double e=0; 
  F_T  v;
  for (int c=0; c<n_channels ; c++ ) {
    for (i=0; i < size ; i++)
      for (j=0; j < size ; j++) {
	v=src[IND(i,j)];       
	e += v.real()*v.real() + v.imag()*v.imag() ;
      }
    src+=size*size; 
  }
  return e/S2;
}


static inline F_T fourier_transl(const int i,const int j,
				 const  double x,const  double y)
{
  //fourier coefficient i,j that will translate image of x,y
  //it was
#if FULL
  F_T I(0,1);
  return expI( M2PIS * (x * j + y * i ));
#else
  //instead we use linear interpolation
  int ix=(int)floor(x), iy=(int)floor(y); double dx=x-(double)ix, dy=y-(double)iy;
  return  
    (1-dy) *(1-dx) *  expI_S( - (j*(ix  )+i*(iy  ))  ) +
    (1-dy) *(  dx) *  expI_S( - (j*(ix+1)+i*(iy  ))  ) +
    (  dy) *(1-dx) *  expI_S( - (j*(ix  )+i*(iy+1))  ) +
    (  dy) *(  dx) *  expI_S( - (j*(ix+1)+i*(iy+1))  ) ;
#endif
}

static inline F_T fourier_transl_dx(const int i,const int j,
				    const  double x,const  double y)
{
  // WAS
#if FULL 
  F_T I(0,1);
  return expI( M2PIS * ( x * j + y * i) ) *(double)i * I * M2PIS;
#else
  int ix=(int)floor(x), iy=(int)floor(y); double  dy=y-(double)iy;
  return  
   -(1-dy) *          expI_S( - (j*(ix  )+i*(iy  ))  ) +
    (1-dy) *          expI_S( - (j*(ix+1)+i*(iy  ))  ) 
   -(  dy) *          expI_S( - (j*(ix  )+i*(iy+1))  ) +
    (  dy) *          expI_S( - (j*(ix+1)+i*(iy+1))  ) ;
#endif
}

static inline F_T fourier_transl_dy(const int i,const int j,
				    const  double x,const  double y)
{
  // WAS
#if FULL
  F_T I(0,1);
  return expI( M2PIS * ( x * j + y * i) ) *(double)j * I * M2PIS;
#else
  int ix=(int)floor(x), iy=(int)floor(y); double dx=x-(double)ix;
  return  
          - (1-dx) *  expI_S( - (j*(ix  )+i*(iy  ))  ) 
          - (  dx) *  expI_S( - (j*(ix+1)+i*(iy  ))  ) +
            (1-dx) *  expI_S( - (j*(ix  )+i*(iy+1))  ) +
            (  dx) *  expI_S( - (j*(ix+1)+i*(iy+1))  ) ;
#endif
}


static double distance(F_T *src, F_T *dst,
		       int n_channels,	      double x,double y)
{
  int i,j;
  double e=0; 
  F_T  a,b,v; 
  for (int c=0; c<n_channels;c++) {
    for (i=0; i < size ; i++)
      for (j=0; j < size ; j++) { 
	a=(src[IND(i,j)]) ;
	b=(dst[IND(i,j)]) *fourier_transl(i,j,x,y);
	v=a-b;
	e += v.real()*v.real() + v.imag()*v.imag() ;
      }
    src+=size*size;  dst+=size*size;
  }
  return e/S2;
}

static double correlation(F_T *src,	      F_T *dst,
			  const int n_channels,	      double x,double y)
{
  int i,j;
  double e=0; 
  F_T I(0,1) , a,b,v;
  for (int c=0; c<n_channels;c++) {
    for (i=0; i < size ; i++) 
      for (j=0; j < size ; j++) {
	a=(src[IND(i,j)]) ;
	b=(dst[IND(i,j)]) *fourier_transl(i,j,x,y);
	v= a * conj(b);         e += v.real();
	//e += a.real() * b.real() +a.imag()*b.imag();
      }
    src+=size*size;  dst+=size*size;
  }
  return e/S2;
}

static double correlation_dx(F_T *src,F_T *dst,
			     const int n_channels,double x,double y)
{
  int i,j;
  double e=0; 
  F_T I(0,1) , a,b,v; 
  for (int c=0; c<n_channels;c++) {
    for (j=0; j < size ; j++)
      for (i=0; i < size ; i++) {
	a=(src[IND(i,j)]) ;
	b=(dst[IND(i,j)]) * fourier_transl_dx(i,j,x,y);
	v= a * conj(b*I);         e += v.real();
	//e += a.real() * b.imag() - a.imag()*b.real();
      }
    src+=size*size;  dst+=size*size;
  }
  return e/S2;
}

static double correlation_dy(F_T *src,F_T *dst,
			     const int n_channels,double x,double y)
{
  int i,j;
  double e=0; 
  F_T I(0,1) , a,b,v;
  for (int c=0; c<n_channels;c++) {
    for (j=0; j < size ; j++)  
      for (i=0; i < size ; i++) {
	a=(src[IND(i,j)]) ;
	b=(dst[IND(i,j)]) * fourier_transl_dy(i,j,x,y);
	v= a * conj(b*I);         e += v.real();
	//e += a.real() * b.imag() +a.imag()*b.real();
      }
    src+=size*size;  dst+=size*size;
  }
  return e/S2;
}

#ifdef __WIN32__
double drand48()
{
  return (double)rand() / (double)RAND_MAX;
}
#endif

//returns correlation
bool search_fourier_translation(F_T src[], double fs, F_T dst[],
				  int n_channels,
				  double &x,double &y,
				  double &e,int &lp)
{
  const double span=2.0;
  static int success_random=0, success_gradient=0;
  F_T I(0,1);
  double fd= sqrt( energy(dst,n_channels)),     f=fs*fd;

#ifdef DEBUG
  printf("energy src = %.5g , dst = %.5g  corr %+1.5f \n",fs,fd,e/f);
#define PRINT(R,A)    fprintf(stderr,\
    "%c%5d : grad %3d rand %3d corr %+1.5f nx %+1.4f ny %+1.4f   %c",\
            	    R,lp,success_gradient,success_random,ne,nx,ny,A);    
#else
#define PRINT(R,A)
#endif

  if(f< size*size) {
#ifdef DEBUG
    g_message("(not enough energy %g for auto adjust)",f);
#endif
    return 0;
  }

  double  ne = correlation(src,dst,n_channels,x,y) / f , 
    nx=x, ny=y ;
  if(ne>e) e=ne;

  int lq;

  PRINT('L','\n');

  while (lp>0 && x>=-2 && y>=-2 && x<=2 && y<=2 && 
	 ! BEST_TRANSLATION_MATCH(e/f)) {    
    lq=4;
    while(lp>0 && lq>0) {
      lp--;lq--;
      nx +=  (drand48()-0.5)/2;
      ny +=  (drand48()-0.5)/2;
      nx=CLAMP(nx,-span,span);
      ny=CLAMP(ny,-span,span);     
      ne = correlation(src,dst,n_channels,nx,ny)/f;      
      if (e<ne)   {
	x=nx; y=ny; e=ne;
	success_random++; PRINT('R','\n');
      } else PRINT('r','\r');
    }
    lq=8;
    while(lp>0 && lq>0) {
      lp--;lq--;
      double cdx=correlation_dx(src,dst,n_channels,x,y),
	cdy=correlation_dy(src,dst,n_channels,x,y);
      nx = x + cdx/10;
      ny = y + cdy/10;
      nx=CLAMP(nx,-span,span);
      ny=CLAMP(ny,-span,span);
      ne = correlation(src,dst,n_channels,nx,ny)/f;
      if (e<ne)   {
	x=nx; y=ny; e=ne;
	success_gradient++; PRINT('G','\n');
      } else {    
	PRINT('g',' ');
#ifdef DEBUG
	fprintf(stderr," dx %.4g dy %.4g   \r",cdx,cdy);  break;
#endif
      }
    }
  }
  PRINT('-','\n')

  return 1 ;
}

gboolean detect_translation_
//source, or reference, image : the one we want to translate to
(
 GdkPixbuf *src, double sx, double sy,
 //destination, or editing, image : the one we are dealing with
 GdkPixbuf *dst, double odx, double ody,
 // new suggested destination point position in editing image
 double *nx,double *ny)
{
  double  corr=0;
  g_return_val_if_fail( gdk_pixbuf_get_n_channels (src)
			==gdk_pixbuf_get_n_channels (dst), 0);
  guint n_channels = gdk_pixbuf_get_n_channels (src);
  F_T * srcf = prealloc_F_T;
  double isx=round(sx), isy=round(sy);
  double fsx=sx-isx , fsy=sy-isy ;
  fourier_image(src, srcf, (int)isx , (int)isy) ;
  double src_ene= sqrt( energy(srcf,n_channels)) ;
  for(int ex=-2; ex<=2; ex++) {
    for(int ey=-2; ey<=2; ey++) {
#ifdef DEBUG
      fprintf(stderr," ex %d ey %d \n",ex,ey);
#endif
      double dx=odx+ex, dy=ody+ey;
      int loop=30;
      while(loop>0 ) {
	double idx=round(dx), idy=round(dy);
	double fdx=dx-idx , fdy=dy-idy;
	double tx = fdx-fsx,     ty = fdy-fsy;
	F_T  * dstf = prealloc_F_T +size*size*4;
	fourier_image(dst, dstf, (int)idx , (int)idy) ;
	{
	  double newcorr=corr;
	  search_fourier_translation(srcf, src_ene, dstf, n_channels,
				     tx, ty, newcorr, loop);
	  g_assert(newcorr<=1);
	  if(newcorr>corr) {
	    corr=newcorr;
	    *nx =   tx-(fdx-fsx) + dx;
	    *ny =   ty-(fdy-fsy) + dy;
	  }
	}
	if( BEST_TRANSLATION_MATCH(corr)) return TRUE;
	loop--;
      }}}
#ifdef DEBUG
  printf("\n");
#endif
  return GOOD_TRANSLATION_MATCH(corr); 
}

extern "C" { 
  gboolean detect_translation(GdkPixbuf *src, double sx, double sy,
			      GdkPixbuf *dst, double dx, double dy,
			      // new suggested destination 
			      double *nx,double *ny)
  {
    bool res= detect_translation_(src,sx,sy,    dst,dx,dy,  	nx,ny);
    if(res ) 
      g_message ("autoadjust %3g %3g to       %3g %3g ",dx,dy,*nx,*ny);
    else 
      g_message ("autoadjust %3g %3g stays at %3g %3g ",dx,dy,*nx,*ny);
    return res;
  }

  void fourier_init() {fourier_init_();}
}

