#include "stdio.h"
#include <string.h> //strlen

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#ifdef USE_IMLIB
#include <gdk_imlib.h>
#else
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif



#include "gtk-meta.h"

#include <math.h>

#include "gtktopdata.h"
#include "gtk_subimagesel.h"
#include "main.h"

#include "callbacks.h"
#include "interface.h"
#include "guide.h"
#include "support.h"
//#include "pixmaps.h"

#ifndef IS_PLYMORPH
#include "mesh-gtk.h"
#include "utils.h"
#include "feature.h"
#include "../libmorph/relax.h"
#endif

#include "mag.h"
#include "fourier.hh"
#include "dialogs.h"
#include "callbacks_fs.h"

#include "loadsave.h"

#ifndef IS_PLYMORPH
#include "loadsave_mesh.h"
#else
#include "loadsave_ply.h"
#endif

#include "callbacks_subimg.h" //redraw_spins

/* some code was taken from the gtk-tutorial , the "scribble" example */


/*******************************************************************
******************************************************************

                    main window 

**********************************************************************
**********************************************************************

*/





gboolean
on_window_main_delete                  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  if(guide_callback("delete_event",MAIN_WIN)) {
    gtk_widget_hide(widget);  
    return TRUE;
  }
  else
    {
      on_quit1_activate(NULL,NULL);
      return TRUE;
    }
}


void 
on_quit1_activate                      (GtkMenuItem     *menuitem,
					gpointer         user_data) 
{ 
  int lp; char s[MAX_WINS*5+10];
  s[0]=0;
  /* FIXME < or <= ?? */
  for(lp=1;lp<=MAX_WINS; lp++) {
    if(   sp->im_widget[lp] && 
#ifdef IS_PLYMORPH
	  sp->im_ply_labels_unsaved[lp]
#else
	  sp->im_mesh[lp].changed>0
#endif
	  ) {
      int l=strlen(s); sprintf(s+l," %d",lp);
    }
  }
  if ( s[0]) {
    GtkWidget *q=create_question(), *b;
    char *z=g_strdup_printf
      (_("the mesh(es) %s were not saved! do you want to exit anyway?"),s);
    //gtk_window_set_title(GTK_WINDOW(q),z);
    
    gtk_widget_show(q);
    b=lookup_widget(q,"yes");
    gtk_signal_connect (GTK_OBJECT (b), "clicked",
			GTK_SIGNAL_FUNC (gtk_main_quit),
			NULL);
    b=lookup_widget(q,"no");
    gtk_signal_connect (GTK_OBJECT (b), "clicked",
			GTK_SIGNAL_FUNC (gtk_widget_destroy),
			q);
    b=lookup_widget(q,"questionlabel"); 
    gtk_label_set_text(GTK_LABEL(b),z);

    g_free(z);
  }
  else
    gtk_main_quit();
}




void
on_resulting_image_size_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget* hb= lookup_widget  ( sp->im_widget[MAIN_WIN]  ,
				  "handlebox_res_size");
  g_assert(hb);
  gtk_widget_show(hb);
}


void
on_show_morph_factors_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    int lp;
  for(lp=1 ; lp <= MAX_WINS;  lp ++)
    if(sp->im_widget[lp] != NULL) {      
      //set_editview(lp,EDITVIEW_SHOWMESHES );
      setup_handlebox_factor(lp,TRUE);
    }
}



//void
//on_logtext_realize                     (GtkWidget       *widget,
//                                        gpointer         user_data)
//{
//
//}



/*
  main menus callbacks 

 */
#include <time.h>

gchar *session_name=NULL;
void
on_load_session_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  fileselection_hook=load_session;
  fileselection1_for_image_num_g=MAIN_WIN;
  show_fs(//GTK_WIDGET(menuitem), 
	  _("load session") 
#if GTK_MAJOR_VERSION >= 2
	  , GTK_FILE_CHOOSER_ACTION_OPEN
#endif
	  );
  if(session_name)
    gtk_file_selection_set_filename ((fileselection_g),session_name);
}


void
on_save_session_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{  
  fileselection_hook=save_session;
  fileselection1_for_image_num_g=MAIN_WIN;
  show_fs(//GTK_WIDGET(menuitem),
	  _("save session") 
#if GTK_MAJOR_VERSION >= 2
	  , GTK_FILE_CHOOSER_ACTION_SAVE
#endif
	  );
  if(!session_name){
#if GLIB_MAJOR_VERSION == 1 
    session_name=g_strdup_printf("gtkmorph.session");
#else
    GDate D;
    g_date_set_time (&D, time (NULL));
    session_name=g_strdup_printf("%d-%02d-%02d.session",
				 g_date_get_year (&D),g_date_get_month (&D),
				 g_date_get_day (&D));
#endif
  }
  gtk_file_selection_set_filename ((fileselection_g),session_name);
}



void
on_add_an_image_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    int lp;
    for(lp=1 ; lp <= MAX_WINS;  lp ++)
      if(sp->im_widget[lp] == NULL)
	{
	  /* we set this before, 
	     otherwise the "factor_normalize" goes berseker */
	  sp->max_wins++;

	  create_and_show_image_win(lp);	
	  setup_handlebox_factor(lp,FALSE);
	  return; 
	}
}










void
on_view_images1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  int lp;
  for(lp=1 ; lp <= MAIN_WIN ; lp ++)
    {
      if(sp->im_widget[lp] != NULL)
	gtk_widget_show (sp->im_widget[lp]);
    }
}



double
tot_mixing_factors()
{
  double  totdiss;
  int lp;
  totdiss=0;
  for ( lp =1; lp <= MAX_WINS; lp++)
    if(sp->im_widget[lp] != NULL)
      totdiss += sp->mf.im_dissolve_factor[lp];
  // FIXME this is not shown
  sp->mf.im_dissolve_factor[MAIN_WIN]=totdiss; 
  
  return totdiss;
}

double
tot_mixing_factors_nice()
{
  double totdiss=tot_mixing_factors();
  int lp;
  if ( ABS(totdiss) < 0.001)
    {
      show_warning( _("\
to blend the images, the sum of the all `image mixing factors' must be nonzero\
\nI have put default values for you"));
      
      for ( lp =1; lp <= MAX_WINS; lp++)
	if(sp->im_widget[lp] != NULL) {
	  sp->mf.im_dissolve_factor[lp]=1.0/(double)sp->max_wins;
	  //gtk_widget_get_data_top(sp->im_widget[lp],"imagenum")
	}
      totdiss =1;
      redraw_spins(-3);
    }
  return totdiss;
}


/********************************
does the actual morphing 
********************************/

#define PRINT_MORPH_TIME

void
on_morph_images1_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  int lp;
  //double  totdiss=tot_mixing_factors_nice();

#ifdef PRINT_MORPH_TIME
  GTimeVal bef,aft;
  g_get_current_time(&bef);
#endif

  if( sp->max_wins<=1)
    {
      show_error( _("\
to morph, you must have at least two input images"));
      return;
    }

  on_interpolate_meshes1_activate (NULL,NULL);


  for ( lp =1; lp <= MAX_WINS; lp++)
    if(sp->im_widget[lp] != NULL
#ifndef IS_PLYMORPH  
       && !sp->im_widget_is_difference_mesh[lp]
#endif
       )
      {
#ifdef IS_PLYMORPH   
	extern int reference_ply_surface_n;
	g_return_if_fail(reference_ply_surface_n>0);
	if( lp !=  reference_ply_surface_n && ABS(sp->mf.im_dissolve_factor[lp]) < 0.001 )      
#else
	if(ABS(sp->mf.im_dissolve_factor[lp]) < 0.001 )      
#endif
	  g_message("NOT morphing image %d to resultin mesh", lp);
	else
	  {
	    g_message("warping image %d to resulting mesh", lp);
	    //	    do_warp_an_image_old(lp,r,g,b);
	    do_warp_an_image_new(lp);
	  }
      }

  on_do_mixing_clicked(NULL,NULL);
#ifdef PRINT_MORPH_TIME
  g_get_current_time(&aft);
  g_message("morph time %.2f",(double)(aft.tv_sec-bef.tv_sec) +
	    (double)(aft.tv_usec -bef.tv_usec) /1e6 );
#endif

#ifdef IS_PLYMORPH
  __after_morph_ply_callback();
#endif
  
  if(settings_get_value( "show warp after warp"))
    set_editview(MAIN_WIN, EDITVIEW_SHOW);

  MY_GTK_DRAW (sp->im_widget[MAIN_WIN]);
}






/***********************************************************
 *
 *************************** file selection  ************
 *
 **********************************************************

 the following hook is set 
 so that the same fileselection dialog will be used for many
 different purposes

*/


void
on_ok_button1_realize                  (GtkWidget       *widget,
                                        gpointer         user_data)
{
//  GtkWidget       *widget_f=  gtk_widget_get_toplevel (widget);
// doesnt work
//  gtk_container_add (GTK_CONTAINER (widget_f), 
//		     menu_image_num_g);

//  gtk_menu_item_set_submenu(widget,create_menu_image_num_g );
}



void
on_ok_button1_clicked         (GtkButton       *button,
			       gpointer         user_data)
{  
  const char *file=
    gtk_file_selection_get_filename    ( fileselection_g);

  g_assert(fileselection1_for_image_num_g > 0);

  if( //e.g.   fileselection_hook=load_image_from_file
     fileselection_hook  (fileselection1_for_image_num_g,
			  file))
    {
      //gtk_widget_hide(GTK_WIDGET(fileselection_g));
      //gtk_file_selection_complete     (fileselection_g,"");
      gtk_widget_show( sp-> im_widget[fileselection1_for_image_num_g]);  

      guide_callback("file",fileselection1_for_image_num_g);
      // lets be nasty
      fileselection1_for_image_num_g=-2;
      fileselection_hook=NULL;      
      gtk_widget_destroy(GTK_WIDGET(fileselection_g));
      fileselection_g=NULL;
    }
}




















#if HAVE_WAILI
#include "wavelet.hh"
GNode *l2_warped_stats[MAX_WINS],  *l2_subimage_stats[MAX_WINS];
#endif

#ifndef IS_PLYMORPH
void
on_wavelet_equalize_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
#if HAVE_WAILI
 int lp;
 GNode *l2_tmp=NULL;
 double fact=0;
 for ( lp =1; lp <= MAX_WINS; lp++)      
   if(sp->im_widget[lp] != NULL && sp-> im_subimage_pixbuf[lp] ) 
     fact +=1;
 fact = 1/fact;
 for ( lp =1; lp <= MAX_WINS; lp++)      
   if(sp->im_widget[lp] != NULL && sp-> im_subimage_pixbuf[lp] ) {
     g_debug(" ----- wavelet L2 energies for subimage image %d\n",lp);
     l2_subimage_stats[lp]=wavelet_stats( sp-> im_subimage_pixbuf[lp]);
     if(!l2_tmp)
       l2_tmp=wavelet_stats_clone(l2_subimage_stats[lp]);
     wavelet_stats_sum(l2_tmp,l2_subimage_stats[lp],fact);
   }

 for ( lp =1; lp <= MAX_WINS; lp++)      
   if(sp->im_widget[lp] != NULL && sp-> im_subimage_pixbuf[lp] ) {
     g_debug(" --- equalizing wavelet L2 energies for image %d\n",lp);     
     wavelet_equalize(sp-> im_subimage_pixbuf[lp],l2_tmp);
   }
#else
 show_error(_("gtkmorph was not linked with Waili wavelet library"));
#endif
}
#endif

static int round_and_dither( double v)
{
  static double delta=0; double nv;
  v=v+delta;
  nv=floor(v+0.5);
  delta=v-nv;
  return (int)nv;
}

void
on_do_mixing_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
  int lp;
  double  totdiss=tot_mixing_factors_nice();
  guint         w=sp->resulting_width, h=sp->resulting_height;    
  int effective_wins=0, max_win=0;
  guchar *data[MAX_WINS+2];
 
  g_debug("blending textures of warped images");  
  
  for ( lp =1; lp <= MAX_WINS; lp++)   {
    if(sp->im_widget[lp] != NULL &&	  
#ifndef IS_PLYMORPH  
        !sp->im_widget_is_difference_mesh[lp] &&
#endif
       ABS(sp->mf.im_dissolve_factor[lp]) >= 0.0001 && lp != MAIN_WIN ) {
      effective_wins++; max_win=lp;
      data[lp] =	gdk_pixbuf_get_pixels(sp-> im_warped_pixbuf[lp]);
      g_assert( data[lp] != NULL);
    } else
      data[lp]=NULL;
  }

  if( effective_wins <=1) {
    //simply copy
    if(sp-> im_warped_pixbuf[MAIN_WIN])
      gdk_pixbuf_unref(sp-> im_warped_pixbuf[MAIN_WIN]);
    sp-> im_warped_pixbuf[MAIN_WIN]=
        gdk_pixbuf_copy(sp-> im_warped_pixbuf[max_win]);
#ifndef IS_PLYMORPH
    render_pixmap(MAIN_WIN, PIXWARPED);    
    set_editview(MAIN_WIN,  settings_get_value( "show warp after warp")); 
    MY_GTK_DRAW(sp->im_widget[MAIN_WIN] );      
#endif
    return;
  }

  {
    /* we precompute some data */
    double fact[max_win+1];
    guint rowstride[max_win+1];
    guint channels[max_win+1];
#if HAVE_WAILI
    GNode       *l2_average_stat=NULL;
#endif
    
    for ( lp =1; lp <= max_win; lp++)   
      if(data[lp]) {
	fact[lp] = sp->mf.im_dissolve_factor[lp] / totdiss;
	rowstride[lp]= gdk_pixbuf_get_rowstride(sp-> im_warped_pixbuf[lp]);
      channels[lp]= gdk_pixbuf_get_n_channels(sp-> im_warped_pixbuf[lp]);
      }
    
#if HAVE_WAILI
    if(   settings_get_value ("wavelet equalization")) {
      g_message("collecting wavelet statistics");
      for ( lp =1 ; lp <= max_win ; lp++) {
	if (data[lp] != NULL){	  
	  if(NULL==l2_warped_stats[lp]) {
	    g_debug(" ----- wavelet L2 energies for image %d\n",lp);
	    l2_warped_stats[lp]=wavelet_stats( sp-> im_warped_pixbuf[lp]);
	  }
	  if(NULL==l2_average_stat)
	    l2_average_stat=wavelet_stats_clone(l2_warped_stats[lp]);
	  wavelet_stats_sum(l2_average_stat, l2_warped_stats[lp] , fact[lp] );
	}
      }}
#endif
    {
      GdkPixbuf *dpb = sp-> im_warped_pixbuf[MAIN_WIN];
      if(dpb==NULL) {
#ifdef IS_PLYMORPH	
	dpb = sp-> im_warped_pixbuf[MAIN_WIN]=
	  gdk_pixbuf_new(GDK_COLORSPACE_RGB,//GdkColorspace colorspace,
		       TRUE,//gboolean has_alpha,
		       8,//int bits_per_sample,
		       w,h);//int width,    int height);
	
#else
	create__pixbuf(MAIN_WIN, PIXWARPED);
	dpb = sp-> im_warped_pixbuf[MAIN_WIN];
#endif

      }
      guchar  *ddata = gdk_pixbuf_get_pixels(dpb);	
      guint drowstride= gdk_pixbuf_get_rowstride (dpb) ;    
      guint dchannels= gdk_pixbuf_get_n_channels(dpb);
      guint i,j;	
      double val; int v;
      guint dps[max_win+1];      long dp=0; /* data position */	
      //we clear the old image
      gdk_pixbuf_clear(dpb);
      
      //FIXME 
      for ( lp =max_win ; lp >= 1 ; lp--)
	if (data[lp] != NULL)
	  if( dchannels != channels[lp]  ) {
	    show_error( _("the input images have different numbers of channels: cannot blend"));
	    return;
	  }
      
      for( j=0; j<  h ; j++) {
	dp= drowstride * j;
	for ( lp =max_win ; lp >= 1 ; lp--)
	  if (data[lp] != NULL) 
	    dps[lp]=rowstride[lp] * j;
	/* this interpolates also the alpha value, if any  */
	for( i=0; i< dchannels*w ; i++) {
	  val = 0 ;   
	  for ( lp =max_win ; lp >= 1 ; lp--) {
	    if (data[lp] != NULL) {
	      val +=  fact[lp] * (double) (data[lp])[dps[lp] + i ]  ;
	    }}
	  /* FIXME if the image is not 8 bit */ 
	  v=round_and_dither(val);
	  ddata[dp + i] = CLAMP(v, 0, 255);
	}
      }
#if HAVE_WAILI
      if(   settings_get_value ("wavelet equalization")) {
	g_message("wavelet equalization");
	wavelet_equalize(dpb,l2_average_stat);      
      }
#endif
    }
  }
#ifndef IS_PLYMORPH
  render_pixmap(MAIN_WIN, PIXWARPED);
  set_editview(MAIN_WIN,  settings_get_value( "show warp after warp")); 
#endif
  MY_GTK_DRAW(sp->im_widget[MAIN_WIN] );  
}

void do_mesh_factors_equal()
{
  int lp;
  for ( lp =1; lp <= MAX_WINS; lp++)
    if( sp->im_widget[lp] != NULL)
      sp->mf.im_warp_factor[lp]=1.0/ (double)sp->max_wins;
}






/*
**********************************************************************

************** resulting image,  spinbuttons      ********************

**********************************************************************
*/

#include "movies.h"


void
on_resulting_apply_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
  double w=sp->resulting_width,h=sp->resulting_height ;
  sp->resulting_width =  sp->resulting_width_sp;
  sp->resulting_height = sp->resulting_height_sp;
  {
    GtkWidget* hb= lookup_widget  ( sp->im_widget[MAIN_WIN]  ,
				    "handlebox_res_size");
    g_assert(hb);
    gtk_widget_hide(hb);
  }
  int lp;
#ifndef IS_PLYMORPH
#ifdef  ANIMATE_INTERNALLY
  int movie_pixmap_free();
#endif
  for( lp =1; lp <= MAIN_WIN; lp++)
   if ( sp->im_widget[lp] )      {
	destroy_image_win_pixbufs(lp,1);
       //alloc_image_win_data(lp,1);
       {
	 int j ; 
	 for (j=2;j>=1;j--) {
	   create__pixbuf(lp,j);   
	   // can't do this now-- need a window --create__pixmap(lp,j);
	 }}
       {
	 int j ; 
	 for (j=2;j>=1;j--) {
	   //  do this now-- need a window --
	   create__pixmap(lp,j);
	 }
       }
       subimage2affines(lp);
       reload_and_scale_image(lp);
       meshScaleFreeformat( &(sp->im_mesh[lp]),
			    (double)sp->resulting_width/w, 
			    (double)sp->resulting_height/h);
     }
  guide_callback("resulting size",MAIN_WIN);
#endif
  for( lp =1; lp <= MAIN_WIN; lp++)
    if ( sp->im_widget[lp] ) {
       drawingarea_configure(lp);
       MY_GTK_DRAW( sp-> im_widget[lp]);  
    }
}


void spinbutton_res_set()
{
  GtkWidget *widget=lookup_widget(sp->im_widget[MAIN_WIN],
				  "spinbutton_reswidth");
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(widget),
			     sp->resulting_width_sp );
  widget=lookup_widget(sp->im_widget[MAIN_WIN],"spinbutton_resheight");
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(widget),
			     sp->resulting_height_sp );
}

void
on_spinbutton_reswidth_changed         (my_GtkSpinButton   *spinbutton,//GtkEditable     *editable,
                                        gpointer         user_data)
{
  //  GtkSpinButton *spin = GTK_SPIN_BUTTON (editable);
  sp->resulting_width_sp =gtk_spin_button_get_value_as_float (spinbutton);
}


void
on_spinbutton_resheight_changed        (my_GtkSpinButton   *spinbutton,//GtkEditable     *editable,
                                        gpointer         user_data)
{
  //  GtkSpinButton *spin = GTK_SPIN_BUTTON (editable);
  sp->resulting_height_sp =gtk_spin_button_get_value_as_float (spinbutton);
}





void
on_double_size_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  if( sp->resulting_height_sp <= 5000 &&  sp->resulting_width_sp <= 5000 ) {
    sp->resulting_height_sp *= 2;
    sp->resulting_width_sp *=2;
    spinbutton_res_set();
    MY_GTK_DRAW( sp-> im_widget[MAIN_WIN]);  
  }
}


void
on_halve_size_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
  if( sp->resulting_height_sp > 8 &&  sp->resulting_width_sp > 8 ) {
    sp->resulting_height_sp /= 2;
    sp->resulting_width_sp /= 2;
    spinbutton_res_set();
     MY_GTK_DRAW( sp-> im_widget[MAIN_WIN]); 
  }
}



/***********************************************************************
**********************************************************************

image windows callbacks


 note: many callbacks are shared with the "resulting image window"

**********************************************************************
*/


gboolean
on_image_win_1_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{  
  int i=
    GPOINTER_TO_UINT(gtk_widget_get_data_top(widget,"imagenum")); 

#ifndef IS_PLYMORPH
  mag_xy_track(MAIN_WIN,0,0);
#endif

  destroy_image_win_data(i);

  sp->mf.im_warp_factor[i]=0;
  sp->mf.im_dissolve_factor[i]=0;

  return FALSE;
}


gboolean
on_diff_window_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  int i=
    GPOINTER_TO_UINT(gtk_widget_get_data_top(widget,"imagenum")); 
  
  destroy_image_win_data(i);

  sp->mf.im_warp_factor[i]=0;
  sp->mf.im_dissolve_factor[i]=0;

  return FALSE;
}

















void
on_load_example_session_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)

{
  int lp;
  for(lp=1;lp<=MAX_WINS; lp++) {
    if(   sp->im_widget[lp] && 
#ifdef IS_PLYMORPH
	  sp->im_ply_labels_unsaved[lp]
#else
	  sp->im_mesh[lp].changed>0
#endif
	  ) {
      show_error(_("Loading the example session may ruin your current session. Please properly save all meshes and retry."));
      return;
    }  
  }
  gchar *P=NULL;
#ifdef  __WIN32__ 
  extern gchar *program_install_dir;
  gchar *pp[]={".",program_install_dir,NULL};
  gchar *ee[]={"example/ad/gtkmorph.session",
	       "example/AD/gtkmorph.session",NULL};
  lp=0; int le=0;
  while(P == NULL && ee[le]) {
    while(P == NULL && pp[lp]) {
      if(P==NULL) {
	P= g_build_filename(pp[lp],ee[le],NULL);
	if(!g_file_test(P,  G_FILE_TEST_IS_REGULAR)) {	
	  g_free(P);P=NULL;
	}
      }
      lp++;
    }
    le++;
  }    
#else    
  if(P==NULL &&
     g_file_test(DATADIR "/" PACKAGE "/example/AD/gtkmorph.session",  G_FILE_TEST_IS_REGULAR))
    P=g_strdup(DATADIR "/" PACKAGE "/example/AD/gtkmorph.session");
  if(P==NULL) {
    gchar *s;
    if(g_file_test("/etc/debian_version", G_FILE_TEST_IS_REGULAR)) 
	
      s=g_strdup_printf(_("The 'example' directory should be in '%s' but is not.\nInstall the 'gtkmorph-example' debian package."),
			DATADIR "/" PACKAGE );
    else
      s=g_strdup_printf(_("The 'example' directory should be in '%s' but is not."),
			DATADIR "/" PACKAGE);
    show_error(s); g_free(s);
  }
#endif
  if(P) {
    g_debug("loading example session from '%s'",P);
    load_session(0,P);
    g_free(P);
  } else g_warning("could not locate example session!");    
}
