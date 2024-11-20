#include <stdlib.h>
#include <stdio.h>

//#include "waili/gtk.h"

#include <math.h>
#include <string.h> //memcpy

#include <waili/Image.h>
#include <waili/Storage.h>



/*************/   



//#include "application.hh"






#include <gtk/gtk.h>
#ifdef USE_IMLIB
#include <gdk_imlib.h>
#else
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

#include "gtk-meta.h"









#ifdef G_HAVE_ISO_VARARGS
#define message(...)  printf(__VA_ARGS__)
#elif defined(G_HAVE_GNUC_VARARGS)
#define message(format...)      printf(format)
#else  /* no varargs macros */
static void
message (const char *format,      ...)
{
  va_list args;
  va_start (args, format);
  printf (format, args);
  va_end (args);
}
#endif


#define EXTRA_PRECISION 16


#define DITHER 1

PixType round_and_dither( double v)
{
  static double delta=0;
  v=v+delta;
  double nv=floor(v+0.5);
  delta=v-nv;
  return (PixType)nv;
}
PixType round_and_dither( PixType v)
{ return v; }


#if  EXTRA_PRECISION == 1
#define REDUCE_PRECISION(A) (round_and_dither(A))
#else
static inline gint REDUCE_PRECISION(double A) {
  return round_and_dither(A/(double)EXTRA_PRECISION);
}
#endif

static inline guchar pix_2_char(PixType A) {
  gint v= REDUCE_PRECISION(A)+128;
  return CLAMP( v ,0,255);
}

/*************/

   
#if DITHER
PixType dithers[511];
static void dither_init()
{
  int lp;
  for(lp=0;lp<512;lp++) {
    int x = rand();
    x = ABS(x);
    x= x % EXTRA_PRECISION;
    x = x - EXTRA_PRECISION /2 ;
    dithers[lp]=x;
  }
}
static PixType add_dither()
{
  static int d=0;
  d++;
  if(d>511) d=0;
  return dithers[d];
}
#endif

Image *gdk_to_waili(GdkPixbuf *src)
{
  guchar * data = gdk_pixbuf_get_pixels(src);
  int ch,c,r,
    channels=gdk_pixbuf_get_n_channels(src),
    stride=gdk_pixbuf_get_rowstride(src),
    w= gdk_pixbuf_get_width(src), h=gdk_pixbuf_get_height(src);

  Image * dst = new Image(w,h,channels); 
  g_assert(data);
  for (r = 0; r < h; r++)
    for (c = 0; c < w; c++)
      for (ch = 0; ch < channels; ch++) {
	  (*dst)(c, r, ch) = ((PixType)
	    ( data[ch+ c*channels + stride*r]-128))
#if EXTRA_PRECISION != 1
	    *EXTRA_PRECISION
#endif
#if DITHER

	    + add_dither()
#endif
	    ;
      }
  return dst;
}

/*************/   

void waili_to_gdk(GdkPixbuf  *dst,Image *src)
{
  int channels=src->GetChannels(), 
    w=src->GetCols(0),  h=src->GetRows(0),   c ,r ;
  guchar * data = gdk_pixbuf_get_pixels(dst);
  int    stride=gdk_pixbuf_get_rowstride(dst);
  assert(channels==gdk_pixbuf_get_n_channels(dst));
  assert(w==gdk_pixbuf_get_width(dst));
  assert(h==gdk_pixbuf_get_height(dst));
  switch(channels) {
  case 1:
    for (r = 0; r < h; r++)
      for (c = 0; c < w; c++)      {
	data[ c + stride*r]= 
	  data[ c + stride*r+1]= 
	  data[ c + stride*r+2]= pix_2_char((*src)(c, r, 0));
      }
    break;
  case 3:
  case 4:    
    for (r = 0; r < h; r++)
      for (c = 0; c < w; c++)
	for (int ch = 0; ch < channels; ch++) {
	  data[ch+ c*channels + stride*r]=pix_2_char((*src)(c, r, ch));	  
	}
    break;
  default: abort(); 
  }
}

/*************/   

GdkPixbuf  *waili_to_gdk(Image *src)
{
  int channels=src->GetChannels(), 
    w=src->GetCols(0),  h=src->GetRows(0) ;
  GdkPixbuf  *dst=gdk_pixbuf_new
    (GDK_COLORSPACE_RGB,//GdkColorspace colorspace,
     (channels==4?1:0),//gboolean has_alpha,
     8,//int bits_per_sample,
     w,//int width,
     h);//int height);
  waili_to_gdk(dst,src);
  return dst;
}





/********************************************************************/   
/*************/   



class statdata {
public:
  Channel *C;
#ifdef __APPLICATION_HH__  
  Application *H;
#endif
  double a; // average 
  double v; // variance
  double k; // 4th order centered moment
  PixType max, min;

  int depth, subband;

  statdata() {max=min=0; a=v=k=0; C=NULL;
#ifdef __APPLICATION_HH__  
  H=NULL;
#endif
  };

  void print() const {
  //http://mathworld.wolfram.com/Kurtosis.html excess kurtosis
    printf(\
"avg %+6.2f sigma %6.2f exc.kurtosis %6.2f min %+5.1f max %+5.2f",
	   a/EXTRA_PRECISION,	 sqrt(v)/EXTRA_PRECISION,
	   k/v/v-3,
	   ((double)min)/EXTRA_PRECISION,((double)max)/EXTRA_PRECISION );
  }

  ~statdata ()
  {
#ifdef __APPLICATION_HH__  
    if(H) delete H;
    H=NULL;
#endif
  }

  statdata operator+(const statdata O)
  {
    abort(); //should learn more on C++
    statdata st;
    st.v = O.v + this->v;
    st.a = O.a + this->a;
    st.k = O.k + this->k;
    st.min=MIN(O.min,this->min);
    st.max=MAX(O.max,this->max);
#ifdef __APPLICATION_HH__  
    st.H = *this->H + O.H  ;
#endif    
    return st;
  }

  statdata operator*(const double factor)
  {
    abort(); //should learn more on C++
    statdata st;
    st.v = factor * this->v;
    st.a = factor * this->a;
    st.k = factor * this->k;
    st.min=this->min;
    st.max=this->max;
#ifdef __APPLICATION_HH__  
    st.H =  *this->H * factor;
#endif
    return st;
  }
} ;


void sum(GNode *thi, GNode *other,double factor)
{
  statdata *st, *so;
    
  GNode *stc = g_node_first_child(thi),
    *soc = g_node_first_child(other);
  if( stc ) {
    while(stc) {
      sum(stc,soc,factor);
      stc=g_node_next_sibling(stc);
      soc=g_node_next_sibling(soc);
    }
  }  else {
    if(other) {
      st=(statdata *)(thi->data);
      so=(statdata *)(other->data);
      st->v += factor * so->v;
      st->a += factor * so->a;
      st->k += factor * so->k;
      st->min=MIN(st->min,so->min);
      st->max=MAX(st->max,so->max);
#ifdef __APPLICATION_HH__  
      Application *J=  (*(so->H)) * factor;
      if (st->H) {
	Application *S= (*(st->H)) + J;
	delete st->H, J; 
	st->H = S;
      } else st->H=J;
#endif
    } else g_critical(" wrong depth");
  }
}

void L2(statdata *s ) 
{
  Channel *C = s->C;
  u_int Cols = C->GetCols() ,  Rows = C->GetRows();
  PixType max=  (*C)(0, 0);
  PixType min=max;
  long long ss = 0; 
  long long mm = 0;
  for (u_int r = 0; r < Rows; r++)
    for (u_int c = 0; c < Cols; c++) {
      PixType p=(*C)(c, r);
      if(p<min) min=p;
      else if(p> max) max=p;
      long long d = p;
      mm += d;
      ss += d*d;
    }
  s->a = ((double)mm/(double)(Cols*Rows));
  s->v = ((double)ss/(double)(Cols*Rows)) - s->a * s->a;
  s->min=min;
  s->max=max;
  //http://mathworld.wolfram.com/Kurtosis.html
  double a=s->a, k=0;
  for (u_int r = 0; r < Rows; r++)
    for (u_int c = 0; c < Cols; c++) {
      double p=(double)(*C)(c, r);
      p=p-a;      p *= p;      p *= p;
      k +=  p;
    }
  s->k = k / (double)(Cols*Rows) ;
}



#if GLIB_CHECK_VERSION(2,4,0)
static gpointer    statscopy (gconstpointer  src ,gpointer data)
{
  if(data == NULL)
    return NULL;
  statdata *s=(statdata *)data;
  return new statdata(*s);
}
static   gpointer  statsclone (gconstpointer src,gpointer data)
{
  gpointer s=new statdata();
  return s;
}
#endif

void statsmalloc(GNode *node,gpointer data)
{
  g_node_children_foreach(node,  G_TRAVERSE_ALL,
			  statsmalloc, NULL);
  node->data=new statdata();
}


void statsfree(GNode *node,gpointer pippo)
{
  g_node_children_foreach(node,  G_TRAVERSE_ALL,
			  statsfree, NULL);
  statdata *s=(statdata *)node->data;
  delete s;
  node->data=NULL;
}

/*********************************/

int wavelet_depth=2;
const char * wavelet_depths[9] = 
{"1","2","3","4","5","6","7","8",NULL};

char * wavelet_equalization[5] = 
{"no","50%","yes","150%",NULL};

double wavelet_equalization_factor=1;

Wavelet *MyWavelet = NULL;

void Do_FStep_CR(Image *I)
{
    for (u_int ch = 0; ch < I->GetChannels(); ch++) {
	Channel *ch2 = (*I)[ch]->PushFwtStepCR(*MyWavelet);
	if (ch2 && ch2 != (*I)[ch]) {
	    delete (*I)[ch];
	    (*I)[ch] = ch2;
	}
    }
}


void transform(Image *I)
{
  if (! MyWavelet)
    MyWavelet = new Wavelet_SWE_13_7; //Wavelet_CRF_13_7
 (*I).Convert(IT_RGB, IT_YUVr);
  for(int lp=wavelet_depth;lp>0;lp--) {
    Do_FStep_CR(I); 
  }
}
void antitransform(Image *I)
{
  (*I).IFwt();
  (*I).Convert(IT_YUVr,IT_RGB);
}

/*****************************************************************************/

GNode * stats_recurse(Channel *C, int depth=-1)
{  
  statdata *s= new statdata; 
  s->C=C; s->depth=depth;
  GNode *  stat= g_node_new(s);  
  if (C->IsLifted()) { 
    unsigned int subbands=((LChannel *)C)->GetSubbands();
    for (u_int z = 0; z < subbands; z++) {
      SubBand c=(SubBand)z; 
      Channel *CS=(*(LChannel *)C)[c]; 
      g_assert(CS);
      g_debug("  subband=%d ", z);      
      GNode *cstat = stats_recurse(CS,depth+1);
      statdata *cs=(statdata *)cstat->data;
      cs->subband=z;
      g_node_append(stat,cstat);
    }
  } else {
#ifdef __APPLICATION_HH__  
    Application * HI = HistoGram(C); s->H =HI;
#endif
    L2(s); 
    assert( G_NODE_IS_LEAF(stat) );
    g_debug(" depth %d avg %2.4g sigma %2.4g min %2.4g  max %2.4g ",depth,
	    ((double)s->a)/EXTRA_PRECISION,sqrt(s->v)/EXTRA_PRECISION,
	    ((double)s->min)/EXTRA_PRECISION,((double)s->max)/EXTRA_PRECISION);
  }
  return stat;
}


static int remember_color;

GNode * stats(Image *I)
{
  transform(I);

  u_int channels = I->GetChannels();
  
  GNode *stat=g_node_new(NULL);

  for (unsigned int ch = 0; ch < channels; ch++) {
    g_debug("color %d ",ch);
    remember_color=ch;
    g_node_append(stat, stats_recurse((*I)[ch]) );  
  }
  return stat;
}

GNode * stats(GdkPixbuf *src)
{
  Image * I =gdk_to_waili(src);
  GNode * n = stats(I);
  delete I;
  return n;
}


/*********************************/

void equalize_channel (Channel *C, const  statdata *sw,
		       int depth, int subband)
{
  Channel *channel = C;
  u_int Cols = channel->GetCols() ,  Rows = channel->GetRows();
  
  statdata SA; //actual statdata
  SA.C = C;
  L2(&SA);

  printf("col %d dep %d sub %d ", remember_color,depth,subband);

#ifdef __APPLICATION_HH__
  if(depth<=3 && remember_color == 0) //other histograms are not significant
    {
      printf("(hist eq)");
      Application * HI = HistoGram(C);
      Application * EQ= histogram_equalizer(HI, sw->H );
      for (u_int r = 0; r < Rows; r++)
	for (u_int c = 0; c < Cols; c++) {
	  (*channel)(c, r) = round_and_dither( (*EQ)[  (*channel)(c, r)  ] );
	}
      if(depth<=0 && remember_color == 0)
	{
	  {
	    char str[300];
	    sprintf(str,"equalizer color %d depth %d subband %d",
		    remember_color,depth,subband);
	    //char *A[]={" with line",NULL};
	    EQ->plot(str," with line");
	  }
	  {
	    char title[300];
	    char templat[] = "/tmp/histodataXXXXXXXX";
	    char *plotdata = mytempfile(templat); 
	    sprintf(title,"histogram color %d depth %d subband %d",
		    remember_color,depth,subband);     
	    std::ofstream plotfile(plotdata, std::ios::out );
	    plotfile << "#" << title << "\n";
	    HI->write(&plotfile);	 plotfile << "\n\n";
	    sw->H->write(&plotfile);	 plotfile << "\n\n";	 
	    Application * GOT = HistoGram(C); GOT->write(&plotfile);delete GOT;
	    plotfile.close();
	    sprintf(title,"histogram color %d depth %d subband %d",
		    remember_color,depth,subband);
	    char *A[]={"index 0 title \"was\"  w l 1",
		       "index 1 title \"req\"  w l 2",
		       "index 2 title \"got\"  w p 3",
		       NULL};
	    gnuplot(plotdata,title,A);
	  }
	}
      delete EQ, HI;
    }
  else
#endif
    //  if(depth>2 || remember_color == 0)
    {
      double e=sqrt(sw->v/(SA.v + 0.000001)) 
	*  wavelet_equalization_factor + 1-
	 wavelet_equalization_factor;
      printf("(lin eq)");
      for (u_int r = 0; r < Rows; r++)
	for (u_int c = 0; c < Cols; c++) {
	  double d = (*channel)(c, r);
	  d=d - SA.a;
	  d=d * e;
	  d=d + sw->a;
	  PixType p= round_and_dither(d);
	  (*channel)(c, r)= p; //CLAMP(p,s->min,s->max);
	}
    }


  statdata SN; // after equaliz
  SN.C = C;
  L2(&SN);
  
  printf("\n WAS ");
  SA.print();
  printf("\n REQ ");
  sw->print();
  printf("\n GOT ");
  SN.print();
  double EDB= (SN.v / (SA.v+0.00001));   
  printf("\n energy %+.2f db\n",10*log(EDB)/log(10.));
}

void equalize_recurse(Channel *C, GNode *statwanted,
		      int depth=-1, int subband=0)
{
  if (C->IsLifted()) { 
    unsigned int subbands=((LChannel *)C)->GetSubbands();
    for (u_int z = 0; z < subbands; z++) {
      SubBand c=(SubBand)z; 
      Channel *CS=(*(LChannel *)C)[c];
      assert(CS);
      g_assert(! G_NODE_IS_LEAF(statwanted));
      //if( CS->IsLifted())
      equalize_recurse(CS,      g_node_nth_child(statwanted,z),
		       depth+1,z);
    }
  } else {
    g_assert(G_NODE_IS_LEAF(statwanted));
    statdata  *sw=(statdata *)(statwanted->data);
    //printf("depth %d: ",depth);
    equalize_channel(C,sw,depth,subband);
  }
}


/*****************************************************************
 fake an image by using gaussian noise
*/

// #define FAKE_TEST

#ifdef FAKE_TEST
    //  Uniform random generator between 0 and 1

static double ranu(void)
{
  double x;
#ifdef __WIN32__
  x = (double)rand()/(double)RAND_MAX;
#else
  x=drand48();
#endif
  return x;
}

//  Gaussian random generator
static double rang()
{
    static double x1, x2;
    double s, num1, num2, v1, v2, tvar;
    static int igauss = 0 ;
    if (igauss != 2) {
      do {
	num1 = ranu();
	num2 = ranu();
	v1 = 2.0*num1-1.0;
	v2 = 2.0*num2-1.0;
	s = v1*v1+v2*v2;
      } while (s >= 1.0 || s == 0.0);
      tvar = sqrt(-2.0*log(s)/s);
      x1 = v1*tvar;
      x2 = v2*tvar;
      igauss = 2;
      return x1;
    } else {
      igauss = 1;
      return x2;
    }
}
void Do_Noise(statdata *s )
{
  Channel *C=s->C;
  double stdev=sqrt(s->v);
  unsigned int w= C->GetCols(),h=C->GetRows();
  for (u_int r = 0; r < h; r++)
    for (u_int c = 0; c < w; c++)
      (*C)(c, r) = round_and_dither(stdev*rang()+s->a);
}

 void Do_View(Image * MyImage)
{
  char command[512];
  char template[] =  "/tmp/wailiXXXXXXXX";
  char *filename = mytempfile(template);
  MyImage->Export(filename);
  sprintf(command, "if which xv >/dev/null ; then xv %s & else display %s & fi", filename,filename);
  system(command);
}

void fake_channel(Channel *C, GNode *statfinal,int depth=-1)
{
  statdata  *sf;

  if (C->IsLifted()) { 
    unsigned int subbands=((LChannel *)C)->GetSubbands();
    for (u_int z = 0; z < subbands; z++) {
      SubBand c=(SubBand)z; 
      Channel *CS=(*(LChannel *)C)[c];
      assert(CS);
      g_assert(! G_NODE_IS_LEAF(statfinal));
      if(z != 0 || CS->IsLifted())
	fake_channel(CS,  g_node_nth_child(statfinal,z),depth+1);
    }
  } else {
    sf=(statdata *)(statfinal->data);
    statdata sfake; sfake.C=C;
    sf->C = C;
    Do_Noise(sf  );
    L2(&sfake); 
    printf("depth %d  faked to avg %g = %g sigma %g = %g\n",depth,
	   (sf->a)/EXTRA_PRECISION, (sfake.a)/EXTRA_PRECISION,
	   sqrt(sf->v)/EXTRA_PRECISION, sqrt(sfake.v)/EXTRA_PRECISION);
  }
}
#endif //FAKE_TEST
/******************************************************************/

void wavelet_equalize_(GdkPixbuf *src,GNode *l2_wanted_stat)
{
  Image * I =gdk_to_waili(src);

  transform(I);

  u_int channels = I->GetChannels();
#ifdef FAKE_TEST
  {

    Image J =*I;
    for (unsigned int ch = 0; ch < channels; ch++) {
      message(" - faking color %d",ch);
      fake_channel((J)[ch],g_node_nth_child (l2_wanted_stat,ch));  
    }
    (J).IFwt();
    (J).Convert(IT_YUVr,IT_RGB);
    for (unsigned int ch = 0; ch < channels; ch++) 
      (J)[ch]->Enhance(1.0/EXTRA_PRECISION);
    Do_View(&J);
  }
#endif

  for (unsigned int ch = 0; ch < channels; ch++) {
    message(" - equalize color %d\n",ch);
    remember_color=ch;
    equalize_recurse((*I)[ch], 
		     g_node_nth_child (l2_wanted_stat,ch));
  }
  antitransform(I);
  waili_to_gdk(src,I);
}


extern "C"
{
  GNode *  wavelet_stats(GdkPixbuf *src)
  {
    return stats(src);
  }

  void wavelet_stats_sum(GNode *ths, GNode *other,double factor)
  {
    sum(ths,other,factor);
  }

#if GLIB_CHECK_VERSION(2,4,0)  
  GNode * wavelet_stats_copy(GNode *ths)
  {
    return     g_node_copy_deep (ths, statscopy,NULL);
  }
  GNode * wavelet_stats_clone(GNode *ths)
  {
    return     g_node_copy_deep (ths, statsclone,NULL);
  }
#else
  GNode * wavelet_stats_clone(GNode *ths)
  {
    GNode *copy=     g_node_copy (ths);
    statsmalloc(copy,NULL);
    return copy;
  }
#endif

  void wavelet_stats_free(GNode *ths)
  {

    statsfree(ths,NULL);
    g_node_destroy (ths);
  }
  
  void wavelet_init()
  {
#if DITHER
    dither_init();
#endif
  }
  
  void wavelet_equalize(GdkPixbuf *dpb,GNode *l2_stats)
  {
     wavelet_equalize_(dpb,l2_stats);
  }
}
  
