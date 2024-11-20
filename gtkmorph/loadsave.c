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

//NO WE USE g_assert for portability
//#include "assert.h"

#include <gtk/gtk.h>

#include "gtk-meta.h"

#ifdef USE_IMLIB
#include <gdk_imlib.h>
#else
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

#include "gdk-pixbuf-extra.h"

#include "gtktopdata.h"


#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "main.h"

#ifdef IS_PLYMORPH
#include "utils__.h"
#else
#include "utils.h"
#endif

//#include "settings.h"
#include "dialogs.h"
#include "loadsave.h"



/************************************************************************

************************************************************************

 load/save hooks 

 see  on_ok_button1_realize ()

************************************************************************

************************************************************************/

inline  void 
showerr(const char *file, const char *msg)
{
  char *s = 
	g_strdup_printf( msg, file, strerror(errno));
			
  show_error(s  );
  g_free(s);
}

gboolean is_null(const gchar *s)
{
  if(strlen(s) == 0)
    {
      show_error(_("please provide a filename"));
      return TRUE;
    }
  return FALSE;
}
/*****************************************************************************/

gboolean save_session(int ignored,
		      const char *file)
{
  FILE *f;
  int lp;
#ifdef HAVE_UNISTD_H
  int c;
  extern gchar *session_name;
  char cwd[PATH_MAX];
  getcwd(cwd,PATH_MAX);
  c=strlen(cwd);
  /* delete the common initial part of cwd and filename */
#define REDUCE_CWD(A) ( (strlen(A)>=(c+1) && 0==strncmp((A),cwd,c) && (A)[c] == '/' ) ? (A)+c+1:(A))
#else
#define REDUCE_CWD(A) (A)
#endif
 
  if(is_null(file)) return FALSE;

  if((f=fopen(file, "w"))==NULL) {
    showerr(file, _("could not open file '%s' for write: %s") );
    return FALSE; 
  }

  if(session_name) g_free(session_name);
  session_name=g_strdup(file);

  fprintf(f,"<gtkmorph session>\n");
#ifdef HAVE_UNISTD_H
  fprintf(f,"<cwd>\n%s\n</cwd>\n",cwd);
#endif
 for(lp=1;lp<=MAX_WINS; lp++)
    if(sp->im_widget[lp] != NULL)
      {
	if(sp->im_filename_in[lp]) {
	  fprintf(f,"<image>\n");
	  fprintf(f,"<image file in>\n%s\n</image file in>\n",
		  REDUCE_CWD(sp->im_filename_in[lp]));
	  fprintf(f,"<preserve aspect>\n%d\n</preserve aspect>\n",
		  image_settings_get_value("preserve aspect ratio",lp));
	  if(sp->im_mesh_filename[lp]) 
	    fprintf(f,"<mesh file>\n%s\n</mesh file>\n",
		    REDUCE_CWD(sp->im_mesh_filename[lp]));
	  fprintf(f,"<morph factors>\n%f %f\n</morph factors>\n",
		  sp->mf.im_warp_factor[lp],
		  sp->mf.im_dissolve_factor[lp]);
	  fprintf(f,"</image>\n");
	}
      }
  fprintf(f,"<resulting_size>\n%d %d\n</resulting_size>\n",
	 sp->resulting_width,sp->resulting_height);
  fprintf(f,"</gtkmorph session>\n");
  fclose(f);
  return TRUE;
}

static inline void de_n(char *s)
{ int l=strlen(s);  if(s[l-1]=='\n') s[l-1]=0; }


/* did the user use it somehow?  */
inline static gboolean image_used(int lp)
{
  return sp->im_widget[lp] && (
#ifdef IS_PLYMORPH
			       sp->im_ply_surface[lp].vlist ||
			       sp->im_ply_labels_unsaved[lp]   ||
#else
			       sp->im_mesh_filename[lp] ||
			       sp->im_mesh[lp].changed>0 ||
#endif
			       sp->im_filename_in[lp] || 
			       sp->im_filename_out[lp]);
}

static inline void close_tag(FILE *f)
{
  char s[PATH_MAX+1];
  fgets(s,PATH_MAX,f);
  if( s[0]!='<' || s[1]!='/') {
    de_n(s);
    g_warning("malformed session: '%s' is not a closing tag",s);
  } 
}


static gboolean load_session_stanza(FILE *f)
{
  int aspect=-3333, equal=0;
  float a=-3333.3,b=-3333.3;
  const  int lens=PATH_MAX;
  char s[lens+1],  imagename[lens+1], meshname[lens+1];

  imagename[0]=meshname[0]=0; 
  fgets(s,lens,f); de_n(s); /* tag? */
  while( !feof(f) && 0!=strcmp(s,"</image>") ) {
    /********* image tags loop *********/
    if( s[0]!='<' || s[1]=='/' ) {
      /* attemp syncronization */
      g_warning("malformed session: '%s' is not an opening tag in image",s);
    } else
      if(0==strcmp(s,"<image file in>")) {
	fgets(imagename,lens,f); /* file name */
	de_n(imagename); close_tag(f);
      } else
	if( 0==strcmp(s,"<mesh file>")) {
	  fgets(meshname,lens,f);
	  de_n(meshname); close_tag(f);
	} else
	  if( 0==strcmp(s,"<preserve aspect>")) {
	    fgets(s,lens,f); 
	    sscanf(s,"%d",&aspect);     close_tag(f);  
	  } else
	    if( 0==strcmp(s,"<morph factors>")) {
	      fgets(s,lens,f); 
	      sscanf(s,"%f %f",&a,&b);  close_tag(f);
	    } else
	      g_warning("session file: line '%s' was unrecognized in image\n",s);
    fgets(s,lens,f); de_n(s);/* tag? */
  }    /********* end image tags loop *********/
      
  if(imagename[0]==0) {
    g_warning("malformed session: no image filename");
    return FALSE;
  }
  {/* look if it has already been loaded */
    int empty=0;
    { 
      int lp;
      /* FIXME < or <= ?? */
      for(lp=1;lp<MAX_WINS; lp++) {
	if(sp->im_widget[lp] != NULL) {	  
	  if(sp->im_filename_in[lp] &&
	     0==strcmp(sp->im_filename_in[lp],imagename)) {
	    equal=lp;	    
	  }
	  /* if the user has not used this image... we reuse it */
	  if(!image_used(lp)) { if(!empty) empty=lp; }
	} else  {
	  if(!empty) empty=lp; 
	}
      }
    }
    /* if not existent, create it now */
    if(equal==0) {
      if(empty==0) {
	show_error(_("can't reload the full session-no more available images"));
	return FALSE;
      } else {
	if(sp->im_widget[empty] == NULL) {
	  sp->max_wins++;
	  create_and_show_image_win(empty);	 
	}
      }
    }
    /* if not loaded, load it now */
    if(equal==0) {
#ifdef IS_PLYMORPH
      load_ply_from_file(empty,imagename);
#else
      load_image_from_file(empty,imagename);
#endif
      equal=empty;
    }
  }
  if(equal==0) return TRUE;
  /* aspect */
  if(aspect!=-3333)
    image_settings_set_value("preserve aspect ratio",equal,aspect);

  /* mesh file */    
  if(strlen(meshname)  > 0 &&
     ( sp->im_mesh_filename[equal]==NULL ||
       0!=strcmp(sp->im_mesh_filename[equal],meshname) )) {
#ifdef IS_PLYMORPH
    load_points_from_file__(equal,meshname);
#else
    load_mesh_from_file(equal,meshname);
#endif
  }

  if(a!=-3333.3) { 
    sp->mf.im_warp_factor[equal]=a;
    sp->mf.im_dissolve_factor[equal]=b;
  }

  gtk_widget_show(sp->im_widget[equal]);
  return TRUE;
}



gboolean load_session(int ignored,
		      const char *file)
{
  FILE *f;  
  const  int lens=PATH_MAX;
  char s[lens+1], cwd[lens+1];

  if(is_null(file)) return FALSE;

  cwd[0]=0;
  if((f=fopen(file, "r"))==NULL) {
    showerr(file, _("could not open file '%s' for read: %s"));
    return FALSE; 
  }
  { int e;
#if GLIB_MAJOR_VERSION == 1
    gchar * dir = g_dirname(file);
#else
    gchar * dir = g_path_get_dirname (file);
#endif
    e=chdir(dir);
    if(e) g_critical("cannot ch dir to %s : %s",dir,strerror(e));
    g_free(dir);
  }
  fgets(s,lens,f);
  if(strcmp(s,"<gtkmorph session>\n")) {
    show_error( _("parsing of session failed at first header!") );
    return FALSE;
  }
  fgets(s,lens,f);  de_n(s);
  while(!feof(f) && strcmp(s,"</gtkmorph session>") ) {
    /* attemp syncronization    */
    if( s[0]!='<' || s[1]=='/' ) {
      /* attemp syncronization */
      g_warning("malformed session: '%s' is not an opening tag",s);
    } else
      if( 0==strcmp(s,"<cwd>") ) {
	fgets(cwd,lens,f); de_n(cwd);      
	if(chdir(cwd)) {
	  g_warning("Cannot change to CWD %s\n   as specified in the session file",cwd);
	  perror("");
	}
	close_tag(f);	
      } else
	if( 0==strcmp(s,"<resulting_size>") ) {
	  fgets(s,lens,f);
	  sscanf(s,"%d %d",
		 &sp->resulting_width_sp,&sp->resulting_height_sp);
	  spinbutton_res_set();
	  on_resulting_apply_clicked(NULL,NULL);
	  close_tag(f);
	} else 
	  if ( 0==strcmp(s,"<image>") ) {
	    load_session_stanza(f);
	  } 
	  else
	    g_warning("session file: line '%s' was unrecognized\n",s);
    fgets(s,lens,f);de_n(s);
  }

  if(strcmp(s,"</gtkmorph session>")) {
    show_error(_("parsing of session failed"));
    return FALSE;
  }
  // WHY? setup_handlebox_factors();
  fclose(f);
  return TRUE;
}







/* it looks bad: it would be nicer if done with a static inline fun...
 but it must return the caller, not itself !*/

#define myfputc(F,C) \
{ \
 if( EOF == fputc(C,F)) { \
      showerr(file,_("\
the attempt to save the image file %s has produced error: %s")); \
      return FALSE; \
    } \
}

gboolean save_as_ppm(gchar *file,GdkPixbuf * pb)
{    
  FILE *f;
  int  w= gdk_pixbuf_get_width(pb);
  int h= gdk_pixbuf_get_height(pb);

    if( gdk_pixbuf_get_n_channels(pb) != 3)  {
      show_error
	(g_strdup_printf
	 (  " cant save as .ppm :-( the image has %d channels", 
	    gdk_pixbuf_get_n_channels(pb)  ));
      return FALSE;
    }

    f = fopen(file, "wb");
    if( f ==NULL)
      {
	showerr(file,_("\
the attempt to save the image file %s has produced error: %s"));
	return FALSE;
      }
  
    /* save as ppm format */
  
    fprintf(f,"\
P6\n\
# CREATOR: " PACKAGE " " VERSION "\n\
%d %d\n\
255\n",
	    w,h);
 
 
    {
      guchar  *data = gdk_pixbuf_get_pixels(pb);	
      int y, stride=gdk_pixbuf_get_rowstride(pb),
	channels=gdk_pixbuf_get_n_channels(pb);
      for (y=0;y<h;y++)
	if ( fwrite(data+y*stride, channels,w, f) != w)
	  {
	    showerr(file,_("\
the attempt to save the image file %s has produced error: %s"));
	  }
    }

    if( 0 != fclose (f))
      showerr(file,_("\
the attempt to close the saved image file %s has produced error: %s"));
    return TRUE;
}

/*******************************************************/
























gboolean save_pixbuf_to_file(const char *file,
			    GdkPixbuf * pb)
{


  //int len=strlen(file);
  char *ext=extension(file);
  gboolean result=TRUE;

  if(is_null(file)) return FALSE;
  
  if( pb ==NULL)    {
    show_error(_("\
internal error: the image doesnt exist!"));
    return FALSE;
  }
  

#ifndef HAVE_GDK_FORMATS
  if (ext == NULL || cmp_extension(file,"ppm")) {
    show_error(_(" this version of gtkmorph can save images only in .ppm format (to save in other formats, compile with GTK2 or higher). Please set the filename extension to '.ppm'"));
    return FALSE;
  } 
  result=save_as_ppm(file,pb);
#else
  {
    extern GSList *writable_formats;
    char *type="ppm";
    int wl=g_slist_length(writable_formats);
    gint sel=-1;
    GSList *F;
#ifdef HAVE_GTK_COMBO
    if(fileselection_g) {
      GtkWidget*  C=lookup_widget (fileselection_g,"file_type_combo");
      //if(!C)      { g_critical("cannot save! internal error"); return TRUE; }
      if(C)
	sel = gtk_combo_box_get_active(C);
    }
#endif
    if(sel<0 || sel >wl) 
       sel= type_by_extension    (file);
    if(sel<0 || sel >wl) {
      show_error(_("please use an allowed image format"));
      return FALSE;
    }
    F=g_slist_nth(writable_formats,sel);
    //g_assert(F || sel==wl);
    if(F) {
      gpointer data=F->data;
      GError *error=NULL;
      // che cavolo ho scritto ?? f=data;
      type=gdk_pixbuf_format_get_name ((GdkPixbufFormat *)data);
      result = gdk_pixbuf_save(pb,file,//const char *filename,
			       type,&error,NULL);
      g_assert ((result  && !error ) || (!result && error ));
      if(error)
	{show_error((error)->message);g_error_free (error);}   
    } else
      result=save_as_ppm(file,pb);
    {
      if ( ext==NULL ||  cmp_extension(file,type)) {
	gchar * s=g_strdup_printf
	  (_("the filename extension is '%s' but the image was saved in format '%s'. This mismatch may prevent you from viewing this image."),
	   (ext?ext:""),type); 
	show_warning(s);
	g_free(s);
      }
    }
  }
#endif
  return result;
}
