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


void editview_callback(int i);
void drawingarea_configure (int i);


#include "utils2_c"


GtkWidget * create_menu_of_images(int avoid_this, 
				  GtkSignalFunc  callback,
				  gboolean avoid_diffs)
{
  GtkWidget *m=gtk_menu_new (); gtk_widget_show(m);
  int to;for(to=1;to<=MAIN_WIN;to++){
    if(to != avoid_this && sp->im_widget[to] != NULL
       && ( !avoid_diffs || !sp->im_widget_is_difference_mesh[to] ) ) {
      GtkWidget *i;
      gchar * N = compute_title (to);
      i =gtk_menu_item_new_with_label ( N );
      g_free(N);
      gtk_widget_show(i);gtk_container_add (GTK_CONTAINER (m), i);
      gtk_signal_connect (GTK_OBJECT (i), "activate",
			  GTK_SIGNAL_FUNC (callback ),
			  GINT_TO_POINTER(to + 256 * avoid_this) );
    }

  }
  return m;
}

/***********************************************************************
allocates / deallocate  pixbufs and pixmaps

********************************************************************/


void
create__pixbuf(int lp, enum what_display what)
{
  GdkPixbuf **pb=which_pixbuf(lp,what);
  int w,h;
  which_pixbuf_size(lp, what, &w, &h);

  g_assert(*pb == NULL);

  *pb=gdk_pixbuf_new (GDK_COLORSPACE_RGB,//GdkColorspace colorspace,
		      FALSE, //gboolean has_alpha,
		      8,//int bits_per_sample,
		      w,h); //HACK FIXME sp->resulting_width, sp->resulting_height);
  g_assert( gdk_pixbuf_get_bits_per_sample (*pb) == 8);  
  gdk_pixbuf_clear(*pb);
}


void
destroy_pixmap(int lp, enum what_display what)
{
  GdkPixmap **pm=which_pixmap(lp,what);
  if(*pm)
    gdk_pixmap_unref(*pm);
  else   
    g_debug("%s:%d destroy pixmap [%d] type %d which is already NULL\n",	       __FILE__,__LINE__, lp, what);
  *pm=NULL;
}


void
destroy_pixbuf(int lp, enum what_display what)
{
  GdkPixbuf **pb=which_pixbuf(lp,what);
  if(*pb)
    gdk_pixbuf_unref(*pb);
  else 
    g_debug("%s:%d destroy pixbuf [%d] type %d which is already NULL\n",       __FILE__,__LINE__,lp, what);
  *pb=NULL;
}


/* which image are we displaying?    */

GdkPixbuf **
which_pixbuf(int lp, enum what_display what)
{
  GdkPixbuf **pb ;
  g_assert(lp>0 && lp < MAX_WINS+2);
  switch( what)
    {
    case PIXLOADED:
      pb=&sp->im_loaded_pixbuf[lp];      
      break;
    case PIXSUBIMAGE:
      pb=&sp->im_subimage_pixbuf[lp];
      break; 
    case PIXWARPED:
      pb=&sp->im_warped_pixbuf[lp];
      break;
    default: abort();
    }
  return pb;
}

GdkPixbuf **
which_pixbuf_is_visible(int lp)
{
  return which_pixbuf(lp,sp->which_pix[lp]);
}

GdkPixmap **
which_pixmap(int lp, enum what_display what)
{
  GdkPixmap **pb ;
  g_assert(lp>0 && lp < MAX_WINS+2);
  switch( what)
    {
    case PIXLOADED:
      pb=&sp->im_loaded_pixmap[lp];
      break;
    case PIXSUBIMAGE:
      pb=&sp->im_subimage_pixmap[lp];
      break; 
    case PIXWARPED:
      pb=&sp->im_warped_pixmap[lp];
      break;
    default: abort();
    }
  return pb;
}

GdkPixmap **
which_pixmap_is_visible(int lp)
{
  return which_pixmap(lp,sp->which_pix[lp]);
}



void
create__pixmap(int lp, enum what_display what)
{
  GdkPixmap **pm=which_pixmap(lp,what);
  int w,h;
  static GdkFont* back_font=NULL;
  char *xlfd="-*-helvetica-*-r-normal--*-120-*-*-*-*-iso8859-1";
  if(!back_font) {
    back_font=gdk_font_load(xlfd); //const gchar *font_name);
    if(!back_font)
      g_warning("can't load font %s",xlfd);
  }

  which_pixbuf_size(lp, what, &w, &h);
  /* some border around the image */
  if ( what == PIXLOADED)
    { w+=2*PIXLOADEDBORDER; h+=2*PIXLOADEDBORDER; }
  else if ( what == PIXSUBIMAGE)
    { w+=2*PIXEDITBORDER; h+=2*PIXEDITBORDER; }

  g_assert(*pm == NULL);
  
  /* creates backing pixmaps anyway */
  *pm = gdk_pixmap_new(sp->im_widget[lp]->window,
		       w,
		       h,
		       -1);
  
  gdk_draw_rectangle (*pm,
		      sp->im_widget[lp]->style->black_gc,
		      TRUE,
		      0, 0,
		      w,
		      h);
  
  if(back_font ) { const gchar *text="(no image loaded)";
    gdk_draw_text((*pm),//GdkDrawable *drawable,
  		  back_font,//GdkFont *font,
  		  sp->im_widget[lp]->style->white_gc,//GdkGC *gc,
  		  PIXEDITBORDER+30,PIXEDITBORDER+30,//gint x, gint y,
  		  text,strlen(text));//const gchar *text,  gint text_length);
  }
}

static void dim(GdkPixbuf* new)
{
  int i,j,
    width =  gdk_pixbuf_get_width(new),
    height = gdk_pixbuf_get_height(new);
  int rowstride= gdk_pixbuf_get_rowstride (new) ;
  guchar  *data = gdk_pixbuf_get_pixels(new);
  long  dp=0; 
  int ch= gdk_pixbuf_get_n_channels(new);
  //if(ch!=3)   FIXME how do I dim the alpha channel??
  //  g_warning("gtkmorph (%s:%d:%s) image has %d channels! dimming may fail",
  //	      __FILE__,__LINE__,__FUNCTION__,ch);
  for( j=0; j<  height ; j++)
    {
      dp= rowstride * j;
      for( i=0; i<  width*ch ; i++) {
	data[dp] = data[dp] * 3/4;
	dp++;
      }
    }
}

void
render_pixmap(int lp, enum what_display what)
{
  GdkPixbuf **pb=which_pixbuf(lp,what);
  GdkPixmap **pm=which_pixmap(lp,what);
  int w,h;

  which_pixbuf_size(lp, what, &w, &h);

  if( *pm==NULL) {
    g_debug("creating pixmap [%d] type %d",lp,what);
    create__pixmap(lp,what);
  }
  if( *pb==NULL) {
    g_debug("creating pixbuf [%d] type %d",lp,what);
    create__pixbuf(lp,what);
  }
#ifdef DIM
  //FIXME dim image a bit to better view mesh DONE BELOW
  gdk_pixbuf_saturate_and_pixelate
                                            (const GdkPixbuf *src,
                                             GdkPixbuf *dest,
                                             gfloat saturation,
                                             gboolean pixelate);
#endif
  
  {
    int dx=0, dy=0;
    GdkPixbuf* new;
    if ( what == PIXLOADED )
      { dy+=PIXLOADEDBORDER; dx+=PIXLOADEDBORDER; }
     if (  what == PIXSUBIMAGE)
      { dy+=PIXEDITBORDER; dx+=PIXEDITBORDER; }
    
    if(what != PIXWARPED && image_settings_get_value("dim image",lp)) 
      {
	new=gdk_pixbuf_copy (*pb);
	dim(new);
      }
    else
      {
	new=*pb;
	gdk_pixbuf_ref(new);
      }
    gdk_pixbuf_render_to_drawable  
      (new,
       *pm,
       sp->im_widget[lp]->style->black_gc, //GdkGC *gc,
       0, //int src_x,
       0, //int src_y,
       dx, //int dest_x,
       dy, //int dest_y,
       w,h, //width, height,
       GDK_RGB_DITHER_NORMAL,//GdkRgbDither dither,
       0, //int x_dither,
       0 ); //int y_dither);
    gdk_pixbuf_unref(new);
  }
}






void
showhide_subimage(int i)
{
  if(i!= MAIN_WIN)    {
    GtkWidget *m=gtk_widget_get_data_top(sp->im_widget[i],"handleboxsubimage");
    g_assert(m);
    set_sensitive(m,(sp->im_editview[i] == EDITVIEW_EYES));
  }
}





#if GTK_MAJOR_VERSION >= 2  
#define FILENAME_UTF8(A)			(g_filename_to_utf8((A),-1,NULL,NULL,NULL))
#else
#define FILENAME_UTF8(A)    (A)
#endif


void   set_frame_label(int i)
{
  GtkWidget *m=lookup_widget(sp->im_widget[i],"image_frame");
  gchar *t;
  double dist;
  g_assert(m);


  if(sp->im_editview[i] == EDITVIEW_EDIT  
     || sp->im_editview[i] == EDITVIEW_EYES ) {
    if(i== MAIN_WIN)
	t=  _("template image");
    else
      t=  _("input image");
    if(sp-> im_filename_in[i])
      t=g_strdup_printf("%s: %s",t,FILENAME_UTF8(sp-> im_filename_in[i]));
    
  } else {
    if(i== MAIN_WIN) //FIXME if only one input?
      t= _("morphed image");
    else
      t= _("warped image");
    if(sp-> im_filename_out[i])
      t=g_strdup_printf("%s: %s",t,FILENAME_UTF8(sp-> im_filename_out[i]));
    }
  if ( i != MAIN_WIN ) {
    dist=meshDistance(&sp->im_mesh[i],&sp->im_mesh[MAIN_WIN],0);
    //g_message("%i dist %g",i, dist);
    t=g_strdup_printf("%s, dist %.1f",t,dist);
  }
  gtk_frame_set_label(GTK_FRAME(m),t);
}

/*sets things according to the edit view menu value */
void
editview_callback(int i)
{
  if(sp->im_widget_is_difference_mesh[i]) return;
  set_frame_label(i);  
  showhide_subimage(i);
  if(i!= MAIN_WIN) 
    setup_handlebox_factor(i,FALSE);

  if(sp->im_editview[i] == EDITVIEW_EYES)
    sp->which_pix[i]=PIXLOADED;
  else
    if(sp->im_editview[i] == EDITVIEW_SHOW ||
       sp->im_editview[i] == EDITVIEW_SHOWMESHES )
      sp->which_pix[i]=PIXWARPED;
    else
      sp->which_pix[i]=PIXSUBIMAGE;

  switch (sp->im_editview[i]) {
  case EDITVIEW_EDIT: 
    {
    int b=(i==MAIN_WIN)?TRUE:FALSE; 
    image_settings_set_value("dim image",i,1);
    image_settings_set_value("view original mesh",  i, !b);
    image_settings_set_value("view original points",  i, !b);
    image_settings_set_value("view warped mesh",  i, b);
    image_settings_set_value("view warped points",  i, b);
    image_settings_set_value("view eyes",i,0);
    //image_settings_set_value("view .../loaded image/subimage/warped image/",  i, 1);
    image_settings_set_value("mesh is readonly",  i, 0); 
  }
    break;
  case EDITVIEW_SHOW:    
    image_settings_set_value("dim image",i,0);
    image_settings_set_value("view original mesh",  i, 0);
    image_settings_set_value("view original points",  i, 0);
    image_settings_set_value("view warped mesh",  i, 0);
    image_settings_set_value("view warped points",  i, 0);
    image_settings_set_value("view eyes",i,0);
    //image_settings_set_value("view .../loaded image/subimage/warped image/",  i, 2);
    image_settings_set_value("mesh is readonly",  i, 1);
    break;
  case EDITVIEW_SHOWMESHES:
    {
      int b=(i==MAIN_WIN)?TRUE:FALSE; 
    image_settings_set_value("dim image",i,1);
    image_settings_set_value("view original mesh",  i, !b);
    image_settings_set_value("view original points",  i, !b);
    image_settings_set_value("view warped mesh",  i, 1);
    image_settings_set_value("view warped points",  i, 1);
    image_settings_set_value("view eyes",i,0);
    //image_settings_set_value("view .../loaded image/subimage/warped image/", i, 2);
    image_settings_set_value("mesh is readonly",  i,1);
    }
    break;
  case EDITVIEW_EYES:
    {
      int f=0;
      if(i==MAIN_WIN) f=!0;
      //image_settings_set_value("dim image",i,1);
      image_settings_set_value("view original mesh",  i, !f);
      image_settings_set_value("view original points",  i, !f);
      image_settings_set_value("view warped mesh",  i, f);
      image_settings_set_value("view warped points",  i, f);
      image_settings_set_value("view eyes",i,1);
      //image_settings_set_value("view .../loaded image/subimage/warped image/",  i, 0);
      image_settings_set_value("mesh is readonly",  i, 1);
    }
    break;
    //case EDITVIEW_SHOWANIM:
    //g_critical("showanim unimplemented");
  }
}



/** sets the state of all  widgets: visible invisible etc etc**/
void set_state()
{
  int i,s;

  i=settings_get_value( "use antialiasing warping");
  mesh_resample_choose_aa(i);

  for(i=0;i<MAIN_WIN;i++) {
    if(sp->im_widget[i]) {
      editview_callback(i);
      drawingarea_configure(i);
    }
  }
  i= MAIN_WIN;
  s=settings_get_value( "automatic mesh interpolation");
  //if(sp->im_widget[i])
  {
    GtkWidget *m=
      lookup_widget(sp->im_widget[i],"button_interp_meshes");
    g_assert(m);
    set_sensitive(m,!s);
  }
  s=settings_get_value( "automatic blending");
  {
    GtkWidget *m=
      lookup_widget(sp->im_widget[i],"do_mixing");
    g_assert(m);
    set_sensitive(m,!s);
  }
  {
    GtkWidget *m=gtk_widget_get_data_top(sp->im_widget[i],"hbox_feature");
    int flag=//(sp->im_editview[i] == EDITVIEW_FEATURES) &&
      settings_get_value("edit features");
    int l=0;
    gchar *T[]={"tool_select","tool_unselect","tool_assign",NULL};
    g_assert(m);
    set_sensitive(m,flag);
    while(T[l]) {
      GtkWidget *m2=gtk_widget_get_data_top(sp->im_widget[i],T[l]);
      set_sensitive(m2,flag);
      l++;
    }
  }
}



/******** sets the "edit mesh/show warp" option menu value */

void
set_editview(int lp, /* window number */
	     int status)
{
  
  /* this trick comes from the FAQ of glade*/
  GtkWidget * option_menu = lookup_widget (sp->im_widget[lp],
					   "optionmenu_editview");
  sp->im_editview[lp]=status;

  gtk_option_menu_set_history(GTK_OPTION_MENU( option_menu),
			      status);
  
  editview_callback(lp);
  drawingarea_configure(lp);
  //FIXME HACK HACK lets see what happens if I skip this 
  //gtk_widget_draw(option_menu, NULL);
}  

/*****************************************************/

#if HAVE_WAILI
#include "wavelet.hh"
#endif



static void __after_warp(int lp)
{
  int val=1;

#if HAVE_WAILI
  {
    extern GNode *l2_warped_stats[MAX_WINS];
    if(   settings_get_value ("wavelet equalization")) {     
      g_debug(" ----- wavelet L2 energies for image %d",lp);
      l2_warped_stats[lp]=wavelet_stats( sp-> im_warped_pixbuf[lp]);
    }  else {
      if(l2_warped_stats[lp]) wavelet_stats_free(l2_warped_stats[lp]);
      l2_warped_stats[lp]=NULL;
    }
 }
#endif

  val =  settings_get_value("warped image in other win");      
  if( val)
    {
      if( sp->im_warped_widget[lp]==NULL) {
	sp->im_warped_widget[lp]=create_window_warped(); 
	//gtk_widget_show (sp->im_widget[lp]);  
	gtk_widget_set_data_top(sp->im_warped_widget[lp],"imagenum",
				GUINT_TO_POINTER(lp));
	//gtk_widget_set_data_top(sp->im_warped_widget[lp],"pixmap",
	//		sp->im_warped_pixmap[lp]);
      }
	{
	  GtkWidget *widget=lookup_widget(sp->im_warped_widget[lp],
						    "warped_frame"); 
	  gchar *label=g_strdup_printf("input '%s' image %4d%% mesh %4d%%",
				       sp->im_filename_in[lp],
				       (int)(100.*sp->mf.im_dissolve_factor[lp]),
				       (int)(100.*sp->mf.im_warp_factor[lp]));
	  gtk_frame_set_label   (GTK_FRAME(widget),label);
	  g_free(label);	
	}

      {
	/* resizes the viewport so that the scrolling bars will work ok */ 
	int w=sp->resulting_width, h=sp->resulting_height;
	  
	GtkWidget *g =lookup_widget
	  (sp->im_warped_widget[lp],"drawingarea_warped");
	g_assert(g);
#if GTK_MAJOR_VERSION < 2
	gtk_widget_set_usize(g,w,h);
#else
	gtk_widget_set_size_request(g,w,h);
#endif	
      }

      render_pixmap(lp,2);
      gtk_widget_show (sp->im_warped_widget[lp]); 
      MY_GTK_DRAW (sp->im_warped_widget[lp]); 
    }
  else
    {	
      if(settings_get_value( "show warp after warp"))
	set_editview(lp, EDITVIEW_SHOW);

      gtk_widget_show (sp->im_widget[lp]);  
      MY_GTK_DRAW(sp->im_widget[lp] );  
    }
  render_pixmap(lp,2);
}

#include "affine.h"


void
do_warp_an_image_new(int lp)
{
  GdkPixbuf *pb = * which_pixbuf(lp,PIXWARPED);
  if(pb==NULL) {
    create__pixbuf(lp,PIXWARPED);
  }
#ifdef WARP_FROM_SUBIMAGE
  warp_image_gdk_m
    (* which_pixbuf(lp,PIXSUBIMAGE),
     * which_pixbuf(lp,PIXWARPED),
     & sp-> im_mesh[lp],
     & sp-> im_mesh[MAIN_WIN]);
#else
 {
   MeshT *mesh=& sp-> im_mesh[lp], 
     *tmp=meshNew(mesh->nx,mesh->ny);
   double *affine = sp->transforms[lp].subimage2loaded;
   mesh_X_affine(tmp,mesh,affine);
   warp_image_gdk_m
    (* which_pixbuf(lp,PIXLOADED),
     * which_pixbuf(lp,PIXWARPED),
     tmp,
     & sp-> im_mesh[MAIN_WIN]);
  meshUnref(tmp);
 }
#endif
  __after_warp(lp);
}





/********************************************************************
		loads image from pixbuf to pixmap for image window "lp"
		you should unref the pixbuf   when it exists
		
		copies its data to r,g,b,
*/

void scale_loaded_pixbuf_et_rrggbb(
				   //GdkPixbuf      *impixfile,
		 int lp //image number
		 )
{
  GdkPixbuf 
    ** pbl = which_pixbuf(lp,PIXLOADED),
    **pbs = which_pixbuf(lp,PIXSUBIMAGE);

  double *a=sp->transforms[lp].loaded2subimage;

  if(a[0]==1. && a[1]==0. && a[2]==0. && 
     a[3]==0. && a[4]==1. && a[5]==0. ) {
    gdk_pixbuf_unref(*pbs);
    *pbs=gdk_pixbuf_copy(*pbl);
  } else
    gdk_pixbuf_scale(*pbl,//const GdkPixbuf *src,
		     *pbs,//GdkPixbuf *dest,
		     0,0,//int dest_x,  int dest_y,
		     sp->resulting_width,//int dest_width,
		     sp->resulting_height,//int dest_height,
		     a[2],//double offset_x,
		     a[5],//double offset_y,
		     a[0],//double scale_x,
		     a[4],//double scale_y,
		     GDK_INTERP_HYPER); //GdkInterpType interp_type);

  
  render_pixmap(lp,PIXSUBIMAGE);

#ifdef STORE_RRGGBB
  pixbuf_to_rrrgggbb(*pbs,
		     sp->red[lp],
		     sp->green[lp],
		     sp->blue[lp]  );
#endif
}










/***************************************************************

 *		drawing area bookkeeping

 ***********************************************************
*/ 




void
drawingarea_configure (int i)
{
  int w,h;
  g_return_if_fail(sp-> im_widget[i]);
  if(sp->im_widget_is_difference_mesh[i]) return;
  {
    GtkWidget *d=sp->im_drawingarea_widget[i];
    if(d) {
      which_pixbuf_size(i,sp->which_pix[i],  &w, &h);
    /* some border around the image */
      if ( sp->which_pix[i] == PIXLOADED)
	{ w+=2*PIXLOADEDBORDER; h+=2*PIXLOADEDBORDER; }
      /* some border around the image */
      if ( sp->which_pix[i] == PIXSUBIMAGE)
	{ w+=2*PIXEDITBORDER; h+=2*PIXEDITBORDER; }

      //FIXME I think this is already done render_pixmap(i, sp->which_pix[i]); 
      
      /* resizes the viewport so that the scrolling bars will work ok */
      g_debug("scrolled viewport %d set to w %d h %d",i,w,h);
#if GTK_MAJOR_VERSION < 2
      gtk_widget_set_usize(d,w,h);
#else
      gtk_widget_set_size_request(d,w,h);
#endif
    } else g_critical(" im_drawingarea_widget[%d] == NULL",i);
  }

  //HACK  why only here // if ( sp->which_pix[i] == PIXSUBIMAGE)
  {
    GtkScrolledWindow *d=gtk_widget_get_data_top((sp->im_widget[i]),
						 "scrolledwindow_image");
    if(d) {
      GtkAdjustment* H=gtk_scrolled_window_get_hadjustment(d);
      GtkAdjustment* V=gtk_scrolled_window_get_vadjustment(d);
      //g_message("scrolled %d",i);
      if( H && V ) {
	/* TRY DISABLING 	H->value=PIXEDITBORDER; */
	/* 	V->value=PIXEDITBORDER; */
	gtk_scrolled_window_set_hadjustment(d,H);
	gtk_scrolled_window_set_vadjustment(d,V);
	gtk_adjustment_value_changed(H);
	gtk_adjustment_value_changed(V);
	
	//if(sp->im_widget[i])
	//HACK I dont think is needed MY_GTK_DRAW(sp->im_widget[i] );
	//else g_warning("redraw %i unavailable",i);
      } else g_warning("scrolls %i unavailable",i);
    } else g_warning("scrolling area %i unavailable",i);  
  }

  //else
  /* this is not an error: this function gets called before the
     windows are realized :-( */
  //g_warning("drawing area %i unavailable\n",i);
}




/*     GtkWidget *d=NULL; */
/*     if(sp->im_drawingarea_widget[i]) { */
/*       d=gtk_widget_get_parent( sp->im_drawingarea_widget[i]); */
/*       //d=gtk_bin_get_child(d); */
/*       if(d && !GTK_IS_VIEWPORT (d)) */
/* 	d=gtk_widget_get_parent(d); */
/*       if(d && !GTK_IS_VIEWPORT (d)) */
/* 	d=gtk_widget_get_parent(d); */
/*       if(d && !GTK_IS_VIEWPORT (d)) */
/* 	d=gtk_widget_get_parent(d); */
/*       if(d && GTK_IS_VIEWPORT (d)) {  */
/* 	//DARNED GTK */
/* 	GtkAdjustment* HH=gtk_viewport_get_hadjustment(d); */
/* 	GtkAdjustment* VV=gtk_viewport_get_vadjustment(d); */
/* 	g_message("viewport %d",i); */
/* 	if(HH && VV ) { */
/* 	  HH->value=PIXEDITBORDER; */
/* 	  VV->value=PIXEDITBORDER; */
/* 	  gtk_adjustment_value_changed(HH); */
/* 	  gtk_adjustment_value_changed(VV); */
/* 	  if(sp->im_drawingarea_widget[i]) */
/* 	    gtk_widget_draw (sp->im_drawingarea_widget[i] , NULL); */
/* 	} */
/*       } */
/*     } */
/*     else  */
/*       { */
/*     } */










/****************************************************************************
 promote meshes to have same lines and columns 
***************************************************************************/



/* adds lines and columns to have this mesh of the given size */
void
promote_mesh(MeshT * mesh,int nx, int ny)
{
  while( nx > mesh->nx) {
    int i=0;
    while( i < mesh->nx-1 && mesh->nx < nx)
      {
	meshLineAdd(mesh, i, 0.5, 1);
	i+=2;
      }
  }
  while( ny > mesh->ny) {
    int i=0;
    while( i < mesh->ny-1 && mesh->ny < ny)
      {
	meshLineAdd(mesh, i, 0.5, 2);
	i+=2;
      }
  }
}

void
promote_meshes()
{
  int lp=MAIN_WIN;
  for(; lp>=0; lp--) 
    if( sp->im_widget[lp] != NULL) {
      int c=sp->im_mesh[lp].changed;
      promote_mesh(&sp->im_mesh[lp], sp->meshes_x,sp->meshes_y);
      if ( c != sp->im_mesh[lp].changed)
	MY_GTK_DRAW(sp->im_widget[lp]);
    }
}





































/*****************************************************************************

 *               edit/view option menu

 ****************************************************************************/





















/****************************************************************************

 *             create/ destroy image windows

 ***************************************************************************/


void
destroy_image_win_pixbufs(int lp, int what)
{ 
  g_assert(what==0 || what == 1);

#ifdef STORE_RRGGBB  
  g_free(sp->red[lp]);   g_free(sp->blue[lp]  ); g_free( sp->green[lp] );
#endif

  { int j ; for (j=2;j>=what;j--) {
    destroy_pixbuf(lp,j);    destroy_pixmap(lp,j);
  }}

  if(sp->im_warped_widget[lp])
    {
      gtk_widget_destroy(sp->im_warped_widget[lp]);
      sp->im_warped_widget[lp]=NULL;
    }
}


void
destroy_image_win_data(int lp)
{
  smooth_idle_stop_by_mesh(&(  sp->im_mesh[lp] ));
  
  int j;for(j=1;j<=MAIN_WIN;j++)
    if( sp->im_mesh_diff[j] == &( sp->im_mesh[lp]  ))      {
      meshUnref(sp->im_mesh_diff[j]);  sp->im_mesh_diff[j]=NULL;
      if(sp->im_drawingarea_widget[j])
	MY_GTK_DRAW(sp->im_drawingarea_widget[j]);
    }

  meshUnref( &(  sp->im_mesh[lp] ));
  meshInit(&(  sp->im_mesh[lp] ));
  destroy_image_win_pixbufs(lp,0);
#define myfree(A) {g_free(A); A=NULL;}
  if(sp->im_filename_in[lp]) myfree(sp->im_filename_in[lp]);
  if(sp->im_filename_out[lp]) myfree(sp->im_filename_out[lp]);
  if(sp->im_mesh_filename[lp]) myfree(sp->im_mesh_filename[lp]);
  sp->im_widget[lp]=0;
  sp->im_drawingarea_widget[lp]=0;
}




static void
init_image_win_data_and_set_all(int lp) /* the slot where the data are put
					   in the arrays in sp-> */
{
  
  GtkWidget * topw=sp->im_widget[lp]; /* the top window */
  /*sets size and subimage selection */
  //  set_image_win_initially(lp);
  /* to start with... until the user loads an image */
  sp->im_width[lp]=sp->resulting_width;
  sp->im_height[lp]=sp->resulting_height;
  gtk_subimasel_reset(&(sp->subimasel[lp]),
		      sp->im_width[lp],sp->im_height[lp] );

  { 
    double a[6]={1,0,0,0,1,0};
    memcpy(sp->transforms[lp].subimage2loaded,a,6*sizeof(double));
    memcpy(sp->transforms[lp].loaded2subimage,a,6*sizeof(double));
  }

  /* NO : this would mean that the image has been loaded
     sp->im_filename[lp] = g_strdup_printf("%s%d.png",_("image"),lp);
  */

  
  /*
    pixbufs and rr gg bb buffers are alloced here,
    whilst the im_pixmap_subimage and im_pixwarped are set elsewhere 
  */
  //alloc_image_win_data(lp,0);
  {
    int j ; 
    for (j=2;j>=0;j--) {
      create__pixbuf(lp,j);   
      // can't do this now-- need a window --create__pixmap(lp,j);
  }}

  create_image_menu_settings (lp);
  if(lp==MAIN_WIN) {
    image_settings_set_value("view warped mesh",lp,TRUE);
    image_settings_set_value("view warped points",lp,TRUE);
    image_settings_set_value("view original mesh",lp,TRUE);
    image_settings_set_value("view original points",lp,TRUE);
    gtk_settings_set_sensitive("view original mesh",sp->im_settings[lp],FALSE);
    gtk_settings_set_sensitive("view original points",sp->im_settings[lp],FALSE);    
  }

  {
    /* this trick comes from the FAQ of glade*/
    GtkWidget *
      option_menu = lookup_widget (topw, "optionmenu_editview");
    gtk_signal_connect(GTK_OBJECT (GTK_OPTION_MENU (option_menu)->menu),
		       "deactivate",
		       GTK_SIGNAL_FUNC (on_optionmenu_editview__selected ),
		       GUINT_TO_POINTER(lp));
  }
	
  /* NOTE THE ORDER: first the widget is created
   then we set the "imagenum"
   then we show it, so the pixmaps are created and stored in 
   the  *sp  structure 
  */
  gtk_widget_set_data_top(topw,"imagenum",
			  GUINT_TO_POINTER(lp));
  
  {
    GtkWidget *w=(lookup_widget(topw,"back_to_guide"));
    extern GtkWidget *guide_widget;
    if(w && ! guide_widget) gtk_widget_hide(w);
  }

  //FIXME this is risky ... and messy
  gtk_widget_set_data_top(topw,"gtk_subimagesel", &(sp->subimasel[lp]));
  
  //this is set in "set_backing_pixmap()"
  //sp->im_pixmap_subimage[lp] =  get_pixmap_addr (sp->im_widget[lp]);

  sp->im_mesh_diff[lp]=0;

  meshInit( &(  sp->im_mesh[lp] ));
  if( settings_get_value("mesh auto sync") && lp!=MAIN_WIN)
    meshCopy( &(sp->im_mesh[lp]), &(sp->im_mesh[MAIN_WIN]));
  else {
    meshAlloc( &(sp->im_mesh[lp]), sp->meshes_x, sp->meshes_y);
    meshReset( &(sp->im_mesh[lp]), sp->resulting_width, sp->resulting_height);
  }
#ifdef NO_BORDER_HACK
  {
    MeshT *m =&sp->im_mesh[lp];
    meshSetLabel(m,0,0,MESHPOINTSELECTED);
    meshSetLabel(m,0,m->ny-1,MESHPOINTSELECTED);
    meshSetLabel(m,m->nx-1,0,MESHPOINTSELECTED);
    meshSetLabel(m,m->nx-1,m->ny-1,MESHPOINTSELECTED);
  }
#endif
  
  sp->mf.im_warp_factor[lp]=0.1;
  sp->mf.im_dissolve_factor[lp]=0.1;
}

#include "callbacks_subimg.h" //redraw_spins

void
create_and_show_image_win(int lp)
{   


  sp->im_widget_is_difference_mesh[lp]=FALSE;

  if (lp== MAIN_WIN) {
    GtkWidget *window_main=sp->im_widget[lp]=create_window_main(); 
    {
      GtkMenuItem *f=GTK_MENU_ITEM(lookup_widget(window_main,"file"));
      gtk_widget_set_data_top(f->submenu,"imagenum",
			      GUINT_TO_POINTER(MAIN_WIN));
    }
    {
      GtkMenuItem *f=GTK_MENU_ITEM(lookup_widget(window_main,"settings_menu"));
      GtkWidget *menuSettings_g=NULL;
      menuSettings_g=create_gtkmorph_menuSettings();
      gtk_menu_item_set_submenu (f,menuSettings_g );
    }	
  } else {
    sp->im_widget[lp]=create_image_win_1();
  }
  {
    gchar * N=compute_title (lp);
    gtk_window_set_title (GTK_WINDOW(sp->im_widget[lp]),N);
    g_free(N);
  }
  gtk_window_set_default_size(GTK_WINDOW(sp->im_widget[lp]),-1,460);
  
  sp->im_drawingarea_widget[lp]=
    lookup_widget(sp->im_widget[lp],"drawingarea");
  g_assert(sp->im_drawingarea_widget[lp]);

  init_image_win_data_and_set_all(lp);
  
  gtk_widget_show (sp->im_widget[lp]);
  /* note: this MUST be after the widget_show */
  set_editview(lp,EDITVIEW_EDIT);
  
  redraw_spins(-3);
}



/******************************************************************/

#include "fourier.hh"


void adjust_1_point_wrt_morph(MeshT *dstmesh, GdkPixbuf   *dst , int mi,int mj)
{
  MeshT *srcmesh =&(sp->im_mesh[MAIN_WIN]);
  GdkPixbuf *src = (sp->im_warped_pixbuf[MAIN_WIN]);
  double nx,ny;
  gboolean e=detect_translation(src,
			      meshGetx(srcmesh,mi,mj),meshGety(srcmesh,mi,mj),
			      dst,
			      meshGetx(dstmesh,mi,mj),meshGety(dstmesh,mi,mj),
			      // new suggested destination 
			      &nx,&ny);
  if ( e)
    meshSetNoundo(dstmesh,mi,mj,nx,ny);
}



void
on_adjust_all_meshes_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  int lp=MAIN_WIN-1;
  for(; lp>=0; lp--) {
    if( sp->im_widget[lp] != NULL)      
      {
	int xi, yi;
	MeshT *mesh=&(sp->im_mesh[lp]);
	GdkPixbuf   *dst= (sp->im_warped_pixbuf[lp]);
	for(xi=0; xi < mesh->nx ; xi++) {
	  for(yi=0; yi<mesh->ny  ; yi++) {
	    if( 0 !=  meshGetLabel(mesh,xi,yi)) {
	      adjust_1_point_wrt_morph(mesh,dst ,  xi, yi);
	    }
	  }
	}
	MY_GTK_DRAW(sp->im_widget[lp]);
      }
  }
}


