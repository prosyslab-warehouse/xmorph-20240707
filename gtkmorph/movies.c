
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "stdio.h"
#include "strings.h"
#include "string.h"
#include <unistd.h>
#include "math.h"

#include <gtk/gtk.h>

#ifdef USE_IMLIB
#include <gdk_imlib.h>
#else
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

#include <gdk/gdk.h>

#include "gtk-meta.h"

#include "gtktopdata.h"

#include "gtk_subimagesel.h"

#include "main.h"

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "dialogs.h"

#include "loadsave.h"

#ifndef IS_PLYMORPH
#include "loadsave_mesh.h"
#else
#include "loadsave_ply.h"
#endif

//#include "mesh-gtk.h"
//#include "utils.h"
//#include "../libmorph/relax.h"
#include "movies.h"


//#define PRINT_MORPH_TIME

/************************************* making movies ******************/





void
on_morph_sequence1_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{  
  static GtkWidget* m=NULL;
 if( sp->max_wins<=1)
    {
      show_warning( _("to morph, you must have at least two input images"));
      return;
    }
 else
   {
     if(m==NULL || !GTK_IS_WIDGET (m))
       m=create_window_movie();     
     gtk_widget_show(m);
   }
}



morph_factors_t morph_factors_saved[2];


int movie_init()
{
  int lp;
  for(lp=MAX_WINS; lp >=0; lp--) {
    morph_factors_saved[0].im_warp_factor[lp]=0;
    morph_factors_saved[1].im_warp_factor[lp]=0;
    morph_factors_saved[0].im_dissolve_factor[lp]=0;
    morph_factors_saved[1].im_dissolve_factor[lp]=0;
  }
  morph_factors_saved[0].im_warp_factor[1]=1;
  morph_factors_saved[1].im_warp_factor[2]=1;
  morph_factors_saved[0].im_dissolve_factor[1]=1;
  morph_factors_saved[1].im_dissolve_factor[2]=1;
  return 0;
}

void
store_morph_factors(int i)
{
  memcpy (& morph_factors_saved[i], &sp->mf,  sizeof(morph_factors_t));
}

#include "callbacks_subimg.h"
void
restore_morph_factors(int i)
{
  memcpy(& sp->mf, &morph_factors_saved[i], sizeof(morph_factors_t));
  redraw_spins(-3);
}

void
on_store_morph_factors_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  int lp;
  for(lp=1;lp<=MAX_WINS; lp++)
    if(sp->im_widget[lp] != NULL)
      setup_handlebox_factor(lp,TRUE);
      //set_editview(lp, EDITVIEW_SHOWMESHES);
}


void
on_restore_morph_factors1_activate     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  int lp;
  for(lp=1;lp<=MAX_WINS; lp++)
    if(sp->im_widget[lp] != NULL)
      setup_handlebox_factor(lp,TRUE);
      //set_editview(lp, EDITVIEW_SHOWMESHES);
}

void
on_restore_equal_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  int lp,i=0;
  for(lp=1;lp<=MAX_WINS; lp++)
    if(
#ifndef IS_PLYMORPH  
        !sp->im_widget_is_difference_mesh[lp] &&
#endif
	sp->im_widget[lp] != NULL ) i++;
  for(lp=1;lp<=MAX_WINS; lp++)
    if(
#ifndef IS_PLYMORPH  
       !sp->im_widget_is_difference_mesh[lp] &&
#endif
       sp->im_widget[lp] != NULL) 
      sp->mf.im_warp_factor[lp]=sp->mf.im_dissolve_factor[lp]=1./(double)i;
  redraw_spins(-3);
}


void
on_restore_start_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  restore_morph_factors(0); 
}

void
on_restore_end_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  restore_morph_factors(1);
}


void
on_store_start_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  store_morph_factors(0);
}


void
on_store_end_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  store_morph_factors(1);
}









#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>

void
movie_names(char *name[],char * basename, char * ext,int first, int tot)
{
  int ima;
  for(ima=0; ima < tot; ima++) 
    name[ima]=g_strdup_printf("%s%0+4d.%s",basename,ima+first,ext);
}

gchar * movie_temp_ext()
{
  char *ext="ppm";
#ifdef HAVE_GDK_FORMATS      
  GSList *FORMAT=NULL;
  {
    extern GSList *writable_formats ;
    if(!writable_formats ) create_list_of_formats();
    FORMAT=writable_formats;
    while(FORMAT) {
      gpointer data=FORMAT->data;
      gchar * type= gdk_pixbuf_format_get_name ((GdkPixbufFormat *)data);
      if (0== strcmp("png",type)) 	break;
      FORMAT=g_slist_next(FORMAT);
    }
    if(FORMAT) {
      gpointer data=FORMAT->data;
      ext=gdk_pixbuf_format_get_name (data);
    }
  }
#endif
  return ext;
}

void
movie_data( GtkButton       *button, int  *first, int *tot, gchar **basename)
{
  GtkWidget* m=NULL, 
    *widget=GTK_WIDGET(button);

  GtkSpinButton *spin;
  m=gtk_widget_get_data_top(widget,"file_base_name");
  g_assert(m);
  //G_CONST_RETURN
  gchar* text=gtk_entry_get_text(GTK_ENTRY(m));
  *basename= (char *) text;

  m=gtk_widget_get_data_top(widget,"spinbutton_first_file");
  g_assert(m);
  spin = GTK_SPIN_BUTTON (m);  
  *first=gtk_spin_button_get_value_as_float (spin);
  
  m=gtk_widget_get_data_top(widget,"spinbutton_n_files");
  g_assert(m);
  spin = GTK_SPIN_BUTTON (m);  
  *tot=gtk_spin_button_get_value_as_float (spin);

}


#ifdef  ANIMATE_INTERNALLY
GdkPixmap **movie_pixmaps =NULL;
int movie_pixmaps_num =0;
int movie_pixmaps_frame=0;
int movie_pixmap_free()
{ 
  GdkPixmap **p =movie_pixmaps;
  if(!movie_pixmaps) 
    return 0;
  movie_pixmaps=NULL;
  int lp=movie_pixmaps_num-1,z=lp;
  while(lp) {gdk_pixmap_unref(p[lp]); p[lp]=0; lp--;}
  g_free(p);
  movie_pixmaps_num=0;   movie_pixmaps_frame=0;
  return z;
}
gboolean    movie_pixmap_free_callback(GtkWidget *widget,
				       GdkEvent *event,
				       gpointer user_data)
{
  movie_pixmap_free();
  return FALSE;
}

gint animator_loop=-4;
static gboolean    draw_frame( gpointer data)
{
  GtkWidget *window=data; 
  if(!movie_pixmaps || !data  ||  !GTK_WIDGET(window)
     //#if GTK_MAJOR_VERSION >= 2
     // ||  !GDK_IS_DRAWABLE(widget->window)
     //#endif
     ) return FALSE;
  
  movie_pixmaps_frame++;
  if(movie_pixmaps_frame>=movie_pixmaps_num)
    movie_pixmaps_frame=0;
  {
    GtkWidget *widget=gtk_widget_get_data_top(window,"drawingarea_warped"); 
#if GTK_MAJOR_VERSION >= 2
    if(GDK_IS_DRAWABLE(widget->window) ) 
#endif
      {
	GdkPixmap *pm = movie_pixmaps[movie_pixmaps_frame];
	int w=sp->resulting_width, h=sp->resulting_height;
	gdk_draw_pixmap(widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			pm,		    0,0,0,0,w,h);
      }
  }
  {
    GtkWidget *widget=gtk_widget_get_data_top(window,"warped_frame"); 
    gchar *label=g_strdup_printf("frame %04d",movie_pixmaps_frame);
    gtk_frame_set_label             (GTK_FRAME(widget),label);
    g_free(label);
  }
  return TRUE;
}

#endif

gboolean should_stop_computation=FALSE;
void
on_stop_clicked                        (GtkButton       *button,
                                        gpointer         user_data)
{
  should_stop_computation=TRUE;
}

static void
__movie_set_sensitives__(GtkButton       *button,gboolean b)
{
  GtkWidget * stop=lookup_widget(button,"stop");
  gtk_widget_set_sensitive(stop,!b);
  gtk_widget_set_sensitive(button,b);
}


void
on_movie_ok_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
 
  char * basename;
  int ima, lp, first, tot;
  gchar *ext=movie_temp_ext();
#ifndef IS_PLYMORPH
  /* stops all background processes that are smoothing the meshes */
  smooth_idle_stop();
#endif
  movie_data( button, &first, &tot, &basename);
  if(tot<2) {
    show_error("total number of images must be >=2");
    return;
  }

  char *name[tot];

  movie_names(name,basename,ext,  first, tot);
  
  {
    gchar *  err= g_strdup ("");
      struct stat buf;
      for(ima=0; ima < tot; ima++) {
	if (0== stat (name[ima],&buf)) {
	  {
	    gchar * err2=g_strdup_printf("%s\n %s ",err,name[ima]);
	    g_free(err);
	    err=err2;
	  }
	  //perror(name[ima]);
	}
      }
      
      if( strlen(err) > 0) {
	{
	  gchar * err2=g_strdup_printf("the following files already exist:\n%s",err);
	  g_free(err);
	  err=err2;
	}
	show_error(err);
	for(ima=0; ima < tot; ima++)
	  g_free(name[ima]);
	g_free(err);
	return ;      
      }
      g_free(err);    
  }
#ifdef  ANIMATE_INTERNALLY
  if(movie_pixmaps) movie_pixmap_free();
  movie_pixmaps = g_new0(GdkPixmap *,tot);
  movie_pixmaps_num =tot;
#endif

  //#ifdef PRINT_MORPH_TIME
  GTimeVal bef,aft;
  g_get_current_time(&bef);
  //#endif

  __movie_set_sensitives__(button,FALSE);

  for(ima=0; ima < tot; ima++) {
    if(should_stop_computation) { 
      for(ima=0; ima < tot; ima++)
	g_free(name[ima]);
      should_stop_computation=FALSE;
      __movie_set_sensitives__(button,TRUE);
      return;
    }

    double a = (double)ima / ((double)tot-1.0),      b=1-a;
    for(lp=MAX_WINS; lp >=0; lp--) {
      sp->mf.im_warp_factor[lp]= 
	morph_factors_saved[0].im_warp_factor[lp] * b +
	morph_factors_saved[1].im_warp_factor[lp] * a;
      sp->mf.im_dissolve_factor[lp]= 
	morph_factors_saved[0].im_dissolve_factor[lp] * b +
	morph_factors_saved[1].im_dissolve_factor[lp] * a;
    }    
    on_interpolate_meshes1_activate (NULL,NULL);
    redraw_spins(-3);
    /* computation going on */
    while (gtk_events_pending())
      gtk_main_iteration();
    /* computation continued */    
    on_morph_images1_activate( NULL, NULL);
    /* computation going on */
#ifdef IS_PLYMORPH
    /* refresh sp->im_pixbuf[MAIN_WIN */
    MY_GTK_DRAW(sp->im_drawingarea_widget[MAIN_WIN]);
#endif
    while (gtk_events_pending())
      gtk_main_iteration();
    /* computation continued */

#ifdef IS_PLYMORPH
      GdkPixbuf *pb=sp->im_pixbuf[MAIN_WIN];
#else
      GdkPixbuf *pb=sp->im_warped_pixbuf[MAIN_WIN];
#endif

#ifdef  ANIMATE_INTERNALLY  
      {
	int w=sp->resulting_width; int h=sp->resulting_height;
	movie_pixmaps[ima] = gdk_pixmap_new(sp->im_widget[MAIN_WIN]->window, w,h,-1);
	gdk_pixbuf_render_to_drawable  
	  (pb,
	   movie_pixmaps[ima],
	   sp->im_widget[MAIN_WIN]->style->black_gc, //GdkGC *gc,
	   0, //int src_x,
	   0, //int src_y,
	   0, //int dest_x,
	   0, //int dest_y,
	   w,h, //width, height,
	   GDK_RGB_DITHER_NORMAL,//GdkRgbDither dither,
	   0, //int x_dither,
	   0 ); //int y_dither);
      }
#endif	    
#ifdef HAVE_GDK_FORMATS       
    if(0 != strcmp(ext,"ppm")) {
      gboolean result=TRUE;
      GError *error=NULL;
      result = gdk_pixbuf_save( pb, name[ima], ext,&error,NULL);
      g_assert ((result  && !error ) || (!result && error ));
      if(error)
	{show_error((error)->message);g_error_free (error);}   
    }    else
#endif
      save_as_ppm(name[ima], pb );    
    {
      char * rem=NULL, *s =NULL;
      //#ifdef PRINT_MORPH_TIME
      g_get_current_time(&aft);
      { 
	double secs=(double)(aft.tv_sec-bef.tv_sec) +
	  (double)(aft.tv_usec -bef.tv_usec) /1e6;	
	if (tot>ima+1) { 
	  secs= ((double)(tot-ima-1))*   secs / (double)(ima+1);
	  int min=floor(secs/60); int sec= secs - min*60;
	  rem=g_strdup_printf("%2dm:%02ds",min,sec);
	} else 	  rem=g_strdup_printf(" 0");
      }
      //#endif      
      GtkWidget* hb= lookup_widget  ( button  ,"information_label");
      if(hb) {
	s=g_strdup_printf("F %d ETA %s",ima,rem);
	gtk_label_set_text(hb,s);
      }   else {
	s=g_strdup_printf("frame %d estimated time %s",ima,rem);
	gtk_label_set_text(hb,s);
      }
      g_free(s); g_free(rem);
    }
  }
  //#ifdef PRINT_MORPH_TIME
  g_get_current_time(&aft);
  {
    double secs=(double)(aft.tv_sec-bef.tv_sec) +
      (double)(aft.tv_usec -bef.tv_usec) /1e6;
    int min=floor(secs/60); int sec= secs - min*60;
    g_message("total movie time %2dm:%02ds",min,sec );
  }
  //#endif

  __movie_set_sensitives__(button,TRUE);

  on_movie_replay_clicked(button, user_data);
  
  for(ima=0; ima < tot; ima++)
    g_free(name[ima]);
}



void
on_movie_help_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{

  char * basename;
  int ima, first, tot;

  movie_data( button, &first, &tot, &basename);
  {
    char *name[tot], *help;
    gchar * movie_temp_ext();
    gchar *ext=movie_temp_ext();
    movie_names(name,basename,ext,  first, tot);
    
    help=g_strdup_printf(_("Help on movie making: when you hit ok, gtkmorph will repeat a loop for %d times. Any time, it will set the warping and image-blending factors to an interpolation between the values that you have stored as 'first' and 'end'. Any time, it will save the morphed image (in format %s), starting from '%s' and ending with '%s'. Then it will create any animation that you have asked to create: the animated gif is called '%s.gif', the mpeg file is called '%s.mpeg' (and mpeg_encode will use '%s.param' for parameters if available: see in %s/mpeg.param an example, or copy the end part of %s.auto.param) and play them. You may recreate animations from preexisting frames by hitting 'replay'" ),
			 tot, ext,name[0], name[tot-1] ,basename,basename,basename, DATADIR "/" PACKAGE,basename );
    show_info(help);


    for(ima=0; ima < tot; ima++)
      g_free(name[ima]);
    g_free(help);
  }
}  


void
on_movie_replay_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
  char * basename;
  int ima,  first, tot;
  movie_data( button, &first, &tot, &basename);

  {
    char *name[tot]; 
    gchar *ext=movie_temp_ext();
    movie_names(name,basename, ext, first, tot);

    char *names=g_strdup("");    
    for(ima=0; ima < tot; ima++) {
      char * newnames=g_strdup_printf("%s '%s'",names,name[ima]);
      g_free(names);
      names=newnames;
    }

    {
      GtkWidget *widget=GTK_WIDGET(button);

      GtkCheckButton *b;
      /******************** animate ******************/
      b=gtk_widget_get_data_top(widget,"animate");
      g_assert(b);
      if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b))) {	
#ifdef  ANIMATE_INTERNALLY
	if(movie_pixmaps_num >0) {
	  GtkWidget* animator=create_window_warped ();
	  /* resizes the viewport so that the scrolling bars will work ok */ 
	  int w=sp->resulting_width, h=sp->resulting_height;	  
	  GtkWidget *g =lookup_widget(animator,"drawingarea_warped");
	  g_assert(g);
	  gtk_widget_set_data_top(animator,"pixmap", movie_pixmaps[0]);	  
#if GTK_MAJOR_VERSION < 2
	  gtk_widget_set_usize(g,w,h);
#else
	  gtk_widget_set_size_request(g,w,h);
#endif
	  gtk_widget_show_all(animator);
	  gtk_signal_connect (GTK_OBJECT (animator), "delete-event",
			      (movie_pixmap_free_callback),
			      NULL);	  
	  animator_loop=g_timeout_add ((int)ceil(3000/tot),draw_frame, g);
	}	else
	  // if the pixmaps are not available internally, fall back 
#endif
	  {
	    gchar *cmd= g_strdup_printf ("animate -delay %d %s &",(int)ceil(300/tot),names);  
	    system(cmd);
	    g_free(cmd);
	  }
      }

      /******************* avi *********************/
      b=gtk_widget_get_data_top(widget,"avi");
      //g_assert(b);
      if(b && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b))) {
	if(strcmp(ext,"png")) 
	  show_error(_("Sorry : temporary files can only be saved as png, and not as ppm; but 'mencoder' does not read ppm files. You must compile gtkmorph using a newer version of libGTK."));
	else {
	  if ( strstr(basename,","))
	    show_error(_("sorry : cannot create AVI if a comma ',' is in the filename (blame this on 'mencoder')"));
	  else {
	    int w=sp->resulting_width, h=sp->resulting_height;
	    char *commanames=g_strdup("");    
	    for(ima=0; ima < tot; ima++) {
	      char * n=g_strdup_printf("%s,'%s'",commanames,name[ima]);
	      g_free(commanames);
	      commanames=n;
	    }
	    

	    gchar *cmd= g_strdup_printf ("\
mencoder  mf://%s -mf  w=%d:h=%d:fps=%d:type=png -ovc lavc  -o '%s.avi' && \
mplayer -noconsolecontrols -loop 10 -fixed-vo '%s.avi' ",
					 commanames,w,h,
					 MAX(25,2+2/tot), basename,basename);
	    system(cmd);
	    g_free(cmd);
	    g_free(commanames);
	  }
	}
      }

      /******************* gif *********************/
      b=gtk_widget_get_data_top(widget,"animated_gif");
      g_assert(b);
      if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b))) {
	gchar *cmd= g_strdup_printf ("\
convert %s '%s.gif' && animate  -delay %d '%s.gif' &",
			      names,basename,
			      1+200/tot, basename);	
	system(cmd);
	g_free(cmd);
      }
      
      /***************** mpeg **********************/
      b=gtk_widget_get_data_top(widget,"animated_mpeg");
      g_assert(b);
      if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b))) {
	char *tmpname=g_strdup_printf("%s.auto.param",
				      basename),
	  *parmname=g_strdup_printf ("%s.param",basename);
	FILE *tmp_file=fopen(tmpname,"w"),
	  *parmfile;
	char *cmd= g_strdup_printf ("\
mpeg_encode -quiet 3 '%s' && { \
 if [ -r /usr/bin/X11/mpeg_play ] ; then mpeg_play -quiet '%s.mpeg' ; \
  else xanim  '%s.mpeg' ; fi ;  }  &", tmpname,basename,basename);
	if(tmp_file) {
	  fprintf(tmp_file,"\
#this file is automatically generated and will be overwritten\n");

	  if(tmpname[0]=='/')
	    fprintf(tmp_file,"\
OUTPUT %s.mpeg\n\
INPUT_DIR \n\
INPUT\n",basename);
	  else
	    fprintf(tmp_file,"\
OUTPUT %s.mpeg\n\
INPUT_DIR .\n\
INPUT\n",basename);
	  for(ima=0; ima < tot; ima++)
	    fprintf(tmp_file,"%s\n", name[ima]);
	  fprintf(tmp_file,"\
END_INPUT\n\
BASE_FILE_FORMAT PPM\n\
INPUT_CONVERT *\n");
	  parmfile=fopen(parmname,"r");
	  if( NULL!=  parmfile) {
	    char s[500];
	    while(!feof(parmfile)) {
	      fgets(s,500,parmfile);
	      fputs(s,tmp_file); }
	      fclose(parmfile);
	  } else {
	    int slices=16;
	    while ( slices >1 && ( sp-> resulting_height %slices) != 0 )
	      slices --;
	    fprintf(tmp_file,"\
### these parameters will be read from file %s\n\
### if you create it\n\
### explanation of parameters is in the help of mpeg_encode\n\
PATTERN IBBPBBPBBPBBPBB\n\
GOP_SIZE 30\n\
SLICES_PER_FRAME %d\n\
PIXEL HALF\n\
RANGE 10\n\
FRAME_RATE 24\n\
PSEARCH_ALG TWOLEVEL\n\
BSEARCH_ALG CROSS2\n\
IQSCALE 1\n\
PQSCALE 1\n\
BQSCALE 1\n\
REFERENCE_FRAME DECODED\n",parmname,slices);
	  }
	  fclose(tmp_file);
	  system(cmd);
	  g_free(cmd);
	  g_free(parmname);
	}
	else
	  show_error(g_strdup_printf("\
can't open the temporary file %s for writing: %s", tmpname,strerror(errno)));
      }      
    }

    for(ima=0; ima < tot; ima++)
      g_free(name[ima]);
  }
}
