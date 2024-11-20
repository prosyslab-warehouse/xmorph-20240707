
/***********************
 *
this file may be engineered to become a piece of a new library called
called gtk-extra

but I am too lazy to
 *
 ****************/

#include "stdio.h"
#include "string.h"

#include "gtk/gtk.h"

#ifdef USE_IMLIB
#include <gdk_imlib.h>
#else
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

#include "gtktopdata.h"

#include "gtk_subimagesel.h"

#include "gtk-meta.h"

#include "callbacks.h"
#include "guide.h"
#include "support.h"
#include "utils.h"
#include "main.h"
#include "dialogs.h"



#define SQR(A) ( ((unsigned long)(A)) * ((unsigned long)(A)) )

/*****************************************************************************/
/*****************************************************************************/

void gtk_subimasel_save(gtk_subimage_sel_t * sis, FILE *f)
{
  int lp;
  fprintf(f,"<SIS>\n<orig>\n%u %u\n</orig>\n",
	  sis->orig_width, sis->orig_height);
  fprintf(f,"<rect>\n%d %d %d %d\n</rect>\n",
	  (int)sis->subimage.x,	  (int)sis->subimage.y,
	  (int)sis->subimage.width,	  (int)sis->subimage.height );
  for(lp=0;lp<3;lp++)
    fprintf(f,"<eye>\n%f %f\n</eye>\n",
	    (double)sis->eyes[lp][0],	   (double) sis->eyes[lp][1]);
  fprintf(f,"</SIS>\n");
}

gboolean gtk_subimasel_load(gtk_subimage_sel_t * sis, FILE *f)
{
  char s[201]; int lp=0;
  fgets(s,200,f);
  if(strcmp(s,"<SIS>\n")) {
    g_message("parsing of subimage info failed at first header!");
    return 1;
  }
  fgets(s,200,f);/*orig*/
  fgets(s,200,f);
  sscanf(s,"%u %u",&sis->orig_width, &sis->orig_height);
  fgets(s,200,f);/* /orig*/
  fgets(s,200,f);/* rect */
  fgets(s,200,f);
  { 
    int a,b,c,d;
    sscanf(s,"%d %d %d %d", &a, &b, &c , &d);  
    sis->subimage.x=a;	  sis->subimage.y=b;
    sis->subimage.width=c;	  sis->subimage.height=d;
  }
  fgets(s,200,f);/* /rect */
  fgets(s,200,f);/* ? <eye> ? */
  while( 0==strcmp(s,"<eye>\n" )) {
    fgets(s,200,f);
    if(lp<3) 
      sscanf(s,"%lf %lf", &sis->eyes[lp][0], &sis->eyes[lp][1] );
    fgets(s,200,f); /* </eye */
    fgets(s,200,f);/* <eye> ? */
    lp++;
  }
  if(lp!=3)
    g_warning("there where %d eyes in this subimage?",lp);
  if(strcmp(s,"</SIS>\n")) {
    g_warning("parsing of subimage info failed at end of subimage.");
    return 2;
  }
  else return 0;
}
/*****************************************************************************/

void gtk_subimasel_reset(gtk_subimage_sel_t * sis,
			 unsigned int width ,
			 unsigned int height)
{
  sis->subimage.height= sis->orig_height=height;
  sis->subimage.width = sis->orig_width =width;
  sis->subimage.x=0;
  sis->subimage.y=0;

  sis->eyes[0][0]=0.33*width;
  sis->eyes[0][1]=0.45*height;
  sis->eyes[1][0]=0.67*width;
  sis->eyes[1][1]=0.45*height;
  sis->eyes[2][0]=0.5*width;
  sis->eyes[2][1]=0.8*height;

}


/*****************************************************************************/
#include "affine.h"

/*****************************************************************************/


void eyes2affine(int lp)
{
  gtk_subimage_sel_t *sis= &(sp->subimasel[lp]);
  //unsigned int w=sis->orig_width,  h=sis->orig_height;
  gtk_subimage_sel_t *destsis= &(sp->subimasel[MAIN_WIN]);
  //unsigned int w=sis->orig_width,  h=sis->orig_height;
  double *f=sp->transforms[lp].loaded2subimage;
  double *b=sp->transforms[lp].subimage2loaded;
  double *df=sp->transforms[MAIN_WIN].loaded2subimage;
  //double *db=sp->transforms[MAIN_WIN].subimage2loaded;

  int l,i;
  double //e[3][2],
    de[3][2];  
  double avg[2], davg[2], var[2], dvar[2], covar[2];
  for(l=0; l<3; l++) {
    //eyes in subimage coordinates
    affine_X_vector(&(de[l][0]),&(destsis->eyes[l][0]),df);
    //affine_X_vector(&e[l][0], &sis->eyes[l][0],    f);
  }
  /* good old least squares */  
  for(i=0; i<2; i++) {
    avg[i]=davg[i]=0;
    for(l=0; l<3; l++) {
      avg[i]+=sis->eyes[l][i];
      davg[i]+=      de[l][i];
    }
    avg[i]/=3.0; davg[i]/=3.0;
  }
  for(i=0; i<2; i++) {
    dvar[i]=var[i]=covar[i]=0;
    for(l=0; l<3; l++) {
      var[i] += SQR( sis->eyes[l][i] - avg[i] );
      dvar[i]+= SQR( de[l][i]        -davg[i] );
      covar[i]+=( sis->eyes[l][i] - avg[i] ) *( de[l][i]       -davg[i] ); 
    }
    var[i]/=3.0; dvar[i]/=3.0; covar[i]/=3.0;
  }
  f[0]=covar[0]/var[0];    f[4]=covar[1] / var[1];
  f[2]=davg[0]-avg[0]* f[0]; f[5]=davg[1]-avg[1]* f[4];
  invert(b,f);
}
void affine2subimage(int lp)
{
  gtk_subimage_sel_t *sis= &(sp->subimasel[lp]);
  //unsigned int w=sis->orig_width,  h=sis->orig_height;
  //gtk_subimage_sel_t *destsis= &(sp->subimasel[MAIN_WIN]);
  //double *f=sp->transforms[lp].loaded2subimage;
  double *b=sp->transforms[lp].subimage2loaded;
  //double *df=sp->transforms[MAIN_WIN].loaded2subimage;
  //double *db=sp->transforms[MAIN_WIN].subimage2loaded;

  sis->subimage.width  = sp->resulting_width  * b[0];
  sis->subimage.height = sp->resulting_height * b[4];
  sis->subimage.x=b[2]; //-f[2]/f[0] ;//    -sis->subimage.width/2;
  sis->subimage.y=b[5]; //-f[5]/f[4] ;//    -sis->subimage.height/2;
}
//void eyes2subimage(int lp)
//{
// eyes2affine(lp); affine2subimage(lp); 
//}
/* computes affines from subimage */
void subimage2affines(int lp)
{
  double *f=sp->transforms[lp].loaded2subimage;
  double *b=sp->transforms[lp].subimage2loaded;
  gtk_subimage_sel_t *sis= &(sp->subimasel[lp]);

  double srcxcen=sis->subimage.x+ (double)sis->subimage.width/2.0,
    dstxcen=(double)sp->resulting_width/2.0;

  if(image_settings_get_value("preserve aspect ratio",lp))
    { 

      f[0]=f[4] =(double)sp->resulting_height
	/ (double)sis->subimage.height  ;
      f[2]=dstxcen -f[0]*srcxcen;
      f[5]=-f[4]*(double)sis->subimage.y;
    }
  else
    {
      f[0]=(double)sp->resulting_width
	/ (double)sis->subimage.width;
      f[2]=-f[0]*(double)sis->subimage.x;
      f[4] =(double)sp->resulting_height
	/ (double)sis->subimage.height  ;
      f[5]=-f[4]*(double)sis->subimage.y;
    }
  invert(b,f);
}




/* reset eyes position so that it reflects the position of
 the eyes in the main window */

void affine2eyes(int lp)
{ 
  gtk_subimage_sel_t *sis= &(sp->subimasel[lp]),
    *destsis= &(sp->subimasel[MAIN_WIN]);
  //unsigned int w=sis->orig_width,  h=sis->orig_height;
  //double *f=sp->transforms[lp].loaded2subimage;
  double *b=sp->transforms[lp].subimage2loaded;
  double *df=sp->transforms[MAIN_WIN].loaded2subimage;
  //double *db=sp->transforms[MAIN_WIN].subimage2loaded;

  int l;
  double de[2];  
  for(l=0; l<3; l++) {
    affine_X_vector(de,&destsis->eyes[l][0],df);
    affine_X_vector(&sis->eyes[l][0],de,b);
  }
}


/************************************************************************/

void
update_eyes( gtk_subimage_sel_t * sis,
	     //coordinates in the loaded image
	     int x,int y)
{
  int lp,which=-1;
  unsigned long dist= 1000L,d;
  //unsigned int w=sis->orig_width,  h=sis->orig_height;

  for(lp=0;lp<=2;lp++)
    {
      d= SQR(x - sis->eyes[lp][0])+ SQR(y - sis->eyes[lp][1]);
      if (d<dist) {
	dist=d;
	which=lp;
      }
    }
  if(which<0  ) {  
    //g_critical("can't find nearest eye"); 
    return ;
  } 
  sis->eyes[which][0]=(double)x;///(double)w;
  sis->eyes[which][1]=(double)y;///(double)h;
}

/*****************************************************************************/


gboolean
gdk_subimagesel_button_press_event (GtkWidget       *widget,
				 GdkEventButton  *event,
				 gpointer         user_data)
{
  gtk_subimage_sel_t * sis=
    (gtk_widget_get_data_top(widget,"gtk_subimagesel")); 
  g_assert(sis);
  if(event->button == 1) {
    update_eyes(sis,event->x - PIXLOADEDBORDER, event->y - PIXLOADEDBORDER);
    //FIXME should probably be expose in gtk2.0
    //http://developer.gnome.org/dotplan/porting/ar01s18.html
    MY_GTK_DRAW(widget);
    return TRUE;
  } else 
    return FALSE;
}

/*****************************************************************************/
void subimages_spinbuttons_set(int i);

gboolean
gdk_subimagesel_motion_notify_event (GtkWidget       *widget,
				     GdkEventMotion  *event,
				     gpointer         user_data)
{
  gtk_subimage_sel_t * sis=
    (gtk_widget_get_data_top(widget,"gtk_subimagesel")); 
  int i=
    GPOINTER_TO_UINT(gtk_widget_get_data_top(widget,"imagenum"));
  int x,y;
  GdkModifierType state;
  static int howmany=0;
  howmany++;

  g_assert(sis == &(sp->subimasel[i]));
  g_assert(i && sp->im_widget[i]);
  g_assert(sis);


  if (event->is_hint)
    gdk_window_get_pointer (event->window, &x, &y, &state);
  else
    {
      x = event->x;
      y = event->y;
      state = event->state;
    }

  if(state & GDK_BUTTON1_MASK) {
    update_eyes(sis,x - PIXLOADEDBORDER,y  -PIXLOADEDBORDER);
    /* show how the whole is being dragged */
    if (howmany > 9 && i != MAIN_WIN) {
      eyes2affine(i); affine2subimage(i);
      howmany=0;
      MY_GTK_DRAW(lookup_widget(widget,"handleboxsubimage"));
    }
    MY_GTK_DRAW(widget);
    return TRUE;
  } else 
    return FALSE;
}
gboolean
gdk_subimagesel_button_release_event (GtkWidget       *widget,
				      GdkEventButton  *event,
				      gpointer         user_data)
{
  int i=
      GPOINTER_TO_UINT(gtk_widget_get_data_top(widget,"imagenum"));
  g_assert(i && sp->im_widget[i]);
  if (i != MAIN_WIN) {
    subimages_spinbuttons_set(i);
    MY_GTK_DRAW(lookup_widget(widget,"handleboxsubimage"));
  }
  return TRUE;
}



/*****************************************************************************/


gboolean
gdk_subimagesel_draw(GdkDrawable  *drawable,
		     gtk_subimage_sel_t * sis,
		     GdkGC *gc)
{
  int l,x,y, MP_SIZE=10,MP_ARC=360 * 64; 
  for(l=0;l<3;l++) {
     x=sis->eyes[l][0] +PIXLOADEDBORDER;
    y=sis->eyes[l][1] +PIXLOADEDBORDER;
    gdk_draw_arc  (drawable, gc,
		   TRUE,
		   x - MP_SIZE/2,  y - MP_SIZE/2, MP_SIZE,MP_SIZE,0,MP_ARC);
  } 
  { 
    GdkRectangle *s=&(sis->subimage);
    GdkPoint xpoints[4];
    xpoints[0].x = s->x +PIXLOADEDBORDER;
    xpoints[0].y = s->y +PIXLOADEDBORDER;
    xpoints[1].x = s->x+s->width +PIXLOADEDBORDER;
    xpoints[1].y = s->y+s->height +PIXLOADEDBORDER;
    xpoints[2].x = s->x +PIXLOADEDBORDER;
    xpoints[2].y = s->y+s->height +PIXLOADEDBORDER;
    xpoints[3].x = s->x+s->width +PIXLOADEDBORDER;
    xpoints[3].y = s->y +PIXLOADEDBORDER;
    gdk_draw_lines(drawable, gc , xpoints, 4);
    xpoints[0].x = s->x +PIXLOADEDBORDER;
    xpoints[0].y = s->y+s->height +PIXLOADEDBORDER;
    xpoints[1].x = s->x +PIXLOADEDBORDER;
    xpoints[1].y = s->y +PIXLOADEDBORDER;
    xpoints[2].x = s->x+s->width +PIXLOADEDBORDER;
    xpoints[2].y = s->y +PIXLOADEDBORDER;
    xpoints[3].x = s->x+s->width +PIXLOADEDBORDER;
    xpoints[3].y = s->y+s->height +PIXLOADEDBORDER;
    gdk_draw_lines(drawable, gc , xpoints, 4);
  }
  return TRUE;
}





/********************************************************************
 *
 * input image window, subimage handlebox
 *
 ******************************************************************
**/

void
on_subimage_apply_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
  int i=
    GPOINTER_TO_UINT(gtk_widget_get_data_top(GTK_WIDGET(button),"imagenum"));
  if ( sp-> im_filename_in[i] == NULL
       || strlen( sp-> im_filename_in[i]) == 0) {

    show_warning( _("\
you must load an image before you may choose a subimage") );    
  } else {
    subimage2affines(i);
    reload_and_scale_image(i);
    drawingarea_configure  ( i);
    set_editview( i, EDITVIEW_EDIT);
    guide_callback("select subimage",i);
  }
}


/*****************************************************************************/


void
on_handleboxsubimage_show              (GtkWidget       *widget,
                                        gpointer         user_data)
{
  int i=
    GPOINTER_TO_UINT(gtk_widget_get_data_top(widget,"imagenum")); 

  g_assert(i > 0);  

  //FIXME this doesnt work so we use a different window in glade
  if ( i == MAIN_WIN)
    gtk_widget_hide(widget);
  else
    subimages_spinbuttons_set(i);
}

/*****************************************************************************/

static gboolean lok=FALSE;
void subimages_spinbuttons_set(int i)
{
  gtk_subimage_sel_t *sis= &(sp->subimasel[i]); 
  GtkWidget *w,*W=GTK_WIDGET(sp->im_widget[i]);
  if(lok || i == MAIN_WIN) return;
  lok=TRUE;
  w=lookup_widget(W,"spinbuttonh");
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(w),
			     sis->subimage.height); 
  w=lookup_widget(W,"spinbuttonw");
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(w),
			     sis->subimage.width);
  w=lookup_widget(W,"spinbuttony");
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(w),
			     sis->subimage.y+sis->subimage.height/2); 
  w=lookup_widget(W,"spinbuttonx");
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(w),
			     sis->subimage.x+sis->subimage.width/2);
  lok=FALSE;
}



#undef  CLAMP_SUBIMAGE

void
on_spinbuttonx_changed                 (my_GtkSpinButton   *editable,//GtkEditable     *editable,
                                        gpointer         user_data)
{

  int i=
    GPOINTER_TO_UINT(gtk_widget_get_data_top(GTK_WIDGET(editable),
					     "imagenum")); 
  gtk_subimage_sel_t *sis= &(sp->subimasel[i]);

  g_assert(i>0);
  sis->subimage.x=
    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON(editable))
    -sis->subimage.width/2;

#ifdef CLAMP_SUBIMAGE
  sis->subimage.x = CLAMP( sis->subimage.x,
			     0, sp-> im_width[i] -sis->subimage.width );
  //  gtk_spin_button_set_value (GTK_SPIN_BUTTON(widget),
  //		     sis->subimage.x+sis->subimage.width/2);

#endif
  subimages_spinbuttons_set(i);
  subimage2affines(i);
  affine2eyes(i);
  MY_GTK_DRAW(sp->im_drawingarea_widget[i]);
}


void
on_spinbuttony_changed                 (my_GtkSpinButton     *editable,
                                        gpointer         user_data)
{
   int i=
    GPOINTER_TO_UINT(gtk_widget_get_data_top(GTK_WIDGET(editable),
					     "imagenum")); 
   gtk_subimage_sel_t *sis= &(sp->subimasel[i]);

  g_assert(i>0);
  sis->subimage.y=
    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON(editable))
    -  sis->subimage.height/2; 
#ifdef CLAMP_SUBIMAGE
  sis->subimage.y = CLAMP( sis->subimage.y,
			     0, sp-> im_height[i] -sis->subimage.height );
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(editable),
			     sis->subimage.y+sis->subimage.height/2);
#endif
  subimages_spinbuttons_set(i);
  subimage2affines(i);
  affine2eyes(i);
  MY_GTK_DRAW(sp->im_drawingarea_widget[i]);
}



void
on_spinbuttonw_changed                 (my_GtkSpinButton     *editable,
                                        gpointer         user_data)
{
  int i=
    GPOINTER_TO_UINT(gtk_widget_get_data_top(GTK_WIDGET(editable),
					     "imagenum")); 
  gtk_subimage_sel_t *sis= &(sp->subimasel[i]);
  int cx=sis->subimage.x+sis->subimage.width/2;

  g_assert(i>0);

  sis->subimage.width=
    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON(editable));

  sis->subimage.x=
    cx //gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON(editable))
    -sis->subimage.width/2;


#ifdef CLAMP_SUBIMAGE
  sis->subimage.width = CLAMP( sis->subimage.width,
				 8, sp-> im_width[i] -sis->subimage.x );
#endif
  if(image_settings_get_value("preserve aspect ratio",i)) {
    int cy=sis->subimage.y+sis->subimage.height/2;
    sis->subimage.height= sis->subimage.width *
      sp->resulting_height  /  sp->resulting_width ;
      sis->subimage.y=
	cy //gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON(editable))
	-  sis->subimage.height/2; 
  }
  subimages_spinbuttons_set(i);
  subimage2affines(i);
  affine2eyes(i);
  MY_GTK_DRAW(sp->im_drawingarea_widget[i]);
}










void
on_spinbuttonh_changed                 (my_GtkSpinButton     *editable,
                                        gpointer         user_data)
{
  int i=
    GPOINTER_TO_UINT(gtk_widget_get_data_top(GTK_WIDGET(editable),
					     "imagenum")); 
  gtk_subimage_sel_t *sis= &(sp->subimasel[i]);
  int cy=sis->subimage.y+sis->subimage.height/2;

   
  g_assert(i>0);
  sis->subimage.height=
    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON(editable));
  sis->subimage.y=
    cy //gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON(editable))
    -  sis->subimage.height/2; 

#ifdef CLAMP_SUBIMAGE
  sis->subimage.height = CLAMP( sis->subimage.height,
				  8, sp-> im_height[i] -sis->subimage.y );
#endif
  if(image_settings_get_value("preserve aspect ratio",i)) {
    int cx=sis->subimage.x+sis->subimage.width/2;    
    sis->subimage.width= sis->subimage.height *
      sp->resulting_width / sp->resulting_height;
    sis->subimage.x=
      cx //gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON(editable))
      -sis->subimage.width/2;
  }
  subimages_spinbuttons_set(i);
  subimage2affines(i);
  affine2eyes(i);
  MY_GTK_DRAW(sp->im_drawingarea_widget[i]);
}


/*****************************************************************************/


void
on_reset_subimage_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
 int i=
    GPOINTER_TO_UINT(gtk_widget_get_data_top(GTK_WIDGET(button),
					     "imagenum")); 
   gtk_subimage_sel_t *sis= &(sp->subimasel[i]);

  g_assert(i>0);
  gtk_subimasel_reset(sis,sis->orig_width,sis->orig_height);
  subimages_spinbuttons_set(i);
  MY_GTK_DRAW(sp->im_drawingarea_widget[i]);
}

