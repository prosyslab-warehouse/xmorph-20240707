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

#include "libmorph/warp.h"
#include "libmorph/warp2.h"
#include "libmorph/warp-gtk.h"
#include "libmorph/resample.h"
#include "libmorph/mesh.h"

#include "mesh-gtk.h"

#include "support.h"

#include "utils.h"
//#include "utils-mesh.h"
#include "main.h"
#include "settings.h"
#include "interface.h"
#include "dialogs.h"
#include "loadsave.h"
#include "utils.h"
#include "feature.h"


/***************************************************************
 synch mesh labels
********/




/* we want the negative labels to be the highest, as if casting to unsigned */
#define MESHLABELMAP(A)   (((A)<0)? 20000-(A) : (A))
#define MESHLABELDEMAP(A) (((A)>20000)? 20000-(A) : (A))
#define MAXLABEL(A,B) MESHLABELDEMAP( MAX( MESHLABELMAP(A),MESHLABELDEMAP(B)))



/***************************************************************
 load mesh from file
*/



static gboolean cmp_mesh_name(int lp,const char *file)
{
  int l;
  for ( l =1; l <= MAX_WINS; l++)
    if( sp->im_widget[l] != NULL && sp->im_mesh_filename[l] != NULL)
      if( l != lp && 
	  0==strcmp (file, sp->im_mesh_filename[l])) {
	gchar *s=
	  g_strdup_printf
	  (_("this filename is already used by the mesh of image %d") ,l);
	show_error(s);
	g_free(s);
	return FALSE;
      }

  return TRUE;
}

static inline void de_n(char *s)
{ int l=strlen(s);  if(s[l-1]=='\n') s[l-1]=0; }

gboolean load_mesh_from_file(int lp, const char * file)
{
  FILE *fP;
  //int c;
  if(is_null(file)) return FALSE;

  if(!cmp_mesh_name(lp,file))
    return FALSE;

  if((fP=fopen(file, "r"))==NULL) {
    showerr(file, "could not open file '%s' for read: %s");
    return FALSE; 
  }

  if (meshRead_stream( &(sp-> im_mesh[lp]), fP))   {
    show_error(_("the attempt to load mesh from file has produced an error\n(either this is not a mesh file, or the mesh file is corrupted)"));
    fclose(fP);
    return FALSE;
  }
  
  if(sp->im_mesh_filename[lp]) g_free(sp->im_mesh_filename[lp]);
  sp->im_mesh_filename[lp] = g_strdup(file);

#ifdef NO_BORDER_HACK
  {
    MeshT *m =&sp->im_mesh[lp];
    meshSetLabel(m,0,0,MESHPOINTSELECTED);
    meshSetLabel(m,0,m->ny-1,MESHPOINTSELECTED);
    meshSetLabel(m,m->nx-1,0,MESHPOINTSELECTED);
    meshSetLabel(m,m->nx-1,m->ny-1,MESHPOINTSELECTED);
  }
#endif
  
/* #ifdef THE_MESH_IS_NOW_FREE   */
/*   meshScaleFreeformat???( &(sp->im_mesh[lp]), */
/* 	     sp->resulting_width, sp->resulting_height); */
/* #endif */
  //MY_GTK_DRAW(sp-> im_widget[lp]);  
  
  sp->meshes_x= MAX(sp->meshes_x, sp->im_mesh[lp].nx);
  sp->meshes_y= MAX(sp->meshes_y, sp->im_mesh[lp].ny);
  
  if (settings_get_value("mesh auto sync")) {
    int xi,yi,t=0;
    MeshT *this, *res;
    promote_meshes();
    this=&(sp-> im_mesh[lp]);
    res=&(sp-> im_mesh[MAIN_WIN]);

    for(yi=0; yi < this->ny; yi++) {
      for(xi=0; xi < this->nx; xi++) {
	t+= meshGetLabel(this,xi,yi);
	/* this does a sort of fuzzy union */
	meshSetLabel(res, xi,yi,
		     MAXLABEL(meshGetLabel(this,xi,yi),
			      meshGetLabel(res ,xi,yi)));
      }}
    
    if(t==0) {
      /* this is an old style mesh, with no labels:
	 then we select all points so that they cant be smoothed
      */
      g_message(" this mesh %s had no labels: autoselecting all point",file);
	  for(yi=0; yi < this->ny; yi++) {
	    for(xi=0; xi < this->nx; xi++) {
	      meshSetLabel(this , xi,yi, -1);
	    }}}
  }


  if(gtk_subimasel_load(&(sp-> subimasel[lp]), fP)==0) {
    char imagename[PATH_MAX+1];   imagename[0]=0;
    subimages_spinbuttons_set(lp);
    subimage2affines(lp);
    {
      char s[401];    s[0]=0;     
      fgets(s,400,fP); de_n(s);
      if( 0==strcmp("<image file name>",s)) {
	fgets(imagename,PATH_MAX,fP); de_n(imagename);
	fgets(s,400,fP);        fgets(s,400,fP); de_n(s);
      }
      {
	if( 0==strcmp("<resulting image size>",s)) {
	  int old_res_w, old_res_h;
	  fgets(s,400,fP);
	  sscanf(s,"%d %d",&old_res_w, &old_res_h);
	  meshScaleFreeformat( &(sp->im_mesh[lp]),  
			       sp->resulting_width/(double)old_res_w,
			       sp->resulting_height/(double)old_res_h); 
	}
	fgets(s,400,fP);        fgets(s,400,fP); de_n(s);	
      }
      if( 0==strcmp("<features>",s)) {
	feature_load(fP); //eats the </features>
	//enable this if there is another part: fgets(s,400,fP); de_n(s);	
      }
    }
    fclose(fP);fP=NULL;

    if(sp-> im_filename_in[lp] && imagename[0] &&  
       0!=strcmp(sp-> im_filename_in[lp],imagename)) {
      char z[601];
      sprintf(z,_("this mesh was created on image '%s' and not on this image '%s'"),imagename,sp-> im_filename_in[lp]);
      show_warning(z);
      if(sp-> im_filename_in[lp] && imagename[0] ) {
	gchar * b1, *b2;
#if GLIB_MAJOR_VERSION == 1
	b1=g_basename(sp-> im_filename_in[lp]);   b2=g_basename(imagename); 
#else
	b1=g_path_get_basename(sp-> im_filename_in[lp]);       
	b2=g_path_get_basename(imagename); 
#endif
	if(b1 && b2 && 0==strcmp(b1,b2))       {
	  set_editview( lp, EDITVIEW_EDIT);
	  scale_loaded_pixbuf_et_rrggbb(lp);}
	else set_editview( lp, EDITVIEW_EYES);
#if GLIB_MAJOR_VERSION >= 2
	g_free(b1); g_free(b2);
#endif
      } 
    } else {
    if(sp-> subimasel[lp].orig_width != sp-> im_width[lp] || 
       sp-> subimasel[lp].orig_height != sp->im_height[lp]) {
      char z[601];
      sprintf(z,_("the size of the image w=%d h=%d and the size recorded in the mesh  w=%d h=%d  do not match"),
	      sp-> im_width[lp],sp->im_height[lp],
	      sp-> subimasel[lp].orig_width,sp-> subimasel[lp].orig_height);
      show_warning(z);
      set_editview( lp, EDITVIEW_EYES);
      } else {
	set_editview( lp, EDITVIEW_EDIT);
	scale_loaded_pixbuf_et_rrggbb(lp);
      }
    }
  }
  else
    show_warning("could not read subimage information from this file.");

  return TRUE;
}



/****************************************************************/


gboolean save_mesh_to_file(int lp, const char * file)
{
  FILE *fP;
/*   int c; */
/*   c=strlen(file); */
/*   /\* delete the common initial part of cwd and filename *\/ */
/* #define REDUCE__(A) ((0==strncmp(A,file,c)) ? (A)+c:(A)) */
  if(is_null(file)) return FALSE;

  if(lp==MAIN_WIN && settings_get_value("automatic mesh interpolation"))
    {
      if((fP=fopen(file, "r"))!=NULL)
	{
	  show_error(_("This mesh is automatically generated. You don't want to overwrite another mesh with this one!"));
	  fclose(fP);
	  return FALSE;
	}
    }

  if(!cmp_mesh_name(lp,file))
    return FALSE;
  
  if((fP=fopen(file, "w"))==NULL) {
    showerr(file, "could not open file '%s' for write: %s");
    return FALSE; 
  }
  {
    //MeshT copy;    meshInit(&copy);    meshCopy(&copy, &(sp-> im_mesh[lp])) ;
    //meshScaleFreeformat(copy,?????);
    if (meshWrite_stream( &(sp-> im_mesh[lp]) , fP) ==0)  {
      if(sp->im_mesh_filename[lp]) g_free(sp->im_mesh_filename[lp]);
      sp->im_mesh_filename[lp] = g_strdup(file);
      sp-> subimasel[lp].orig_width = sp-> im_width[lp];
      sp-> subimasel[lp].orig_height = sp->im_height[lp];
      gtk_subimasel_save(&(sp-> subimasel[lp]), fP);
      if(sp->im_filename_in[lp])
	fprintf(fP,"<image file name>\n%s\n</image file name>\n",
		(sp->im_filename_in[lp]));
      fprintf(fP,"<resulting image size>\n%d %d\n</resulting image size>\n",
	      sp->resulting_width, sp->resulting_height);
      fprintf(fP,"<features>\n");
      feature_save(fP);
      fprintf(fP,"</features>\n");
    }  else   {      
      char *s = 
	g_strdup_printf("\
the attempt to save the mesh in file %s has produced an internal error",
			file);
			
      show_error(s);	  
      g_free(s);
      fclose(fP);
      //meshUnref(&copy);
      return FALSE;
    }
    //meshUnref(&copy);
  }
  fclose(fP);  
  return TRUE;
}


gboolean save_diff_mesh_to_file(int lp, const char * file)
{
  if(is_null(file)) return FALSE;
  FILE *fP; 
  if((fP=fopen(file, "w"))==NULL) {
    showerr(file, "could not open file '%s' for write: %s");
    return FALSE; 
  }
  MeshT * mesh = sp->im_mesh_diff[lp], *base_mesh =  &(sp->im_mesh[lp]);
  g_return_val_if_fail(mesh,FALSE);
  const int nx=mesh->nx, ny=mesh->ny ;
  g_return_val_if_fail( nx==base_mesh->nx && ny==base_mesh->ny,FALSE);
  MeshT copy; meshInit(&copy);   meshAlloc(&copy,nx,ny);
  {int xi, yi;
    for(xi=0; xi < nx; xi++) { 
      for(yi=0; yi < ny; yi++) {
	meshSetNoundo(&copy, xi,yi,
		      meshGetx(mesh,xi,yi)-meshGetx(base_mesh,xi,yi),
		      meshGety(mesh,xi,yi)-meshGety(base_mesh,xi,yi));
	meshSetLabel(&copy, xi,yi,meshGetLabel(mesh,xi,yi));
      }}}
  if (meshWrite_stream( &copy , fP) ==0) {
    fclose(fP);
    meshUnref(&copy);
  } else   {      
      char *s = 
	g_strdup_printf("\
the attempt to save the differential mesh in file %s has produced an internal error",
			file);
      show_error(s);	  
      g_free(s);
      fclose(fP);
      meshUnref(&copy);
      return FALSE;
    }
  return TRUE;
}











gboolean load_diff_from_file(int lp, const char * file)
{

  if(!cmp_mesh_name(lp,file))
    return FALSE;

  FILE *fP;
   if((fP=fopen(file, "r"))==NULL) {
    showerr(file, "could not open file '%s' for read: %s");
    return FALSE; 
  }

  if (meshRead_stream( &(sp-> im_mesh[lp]), fP))   {
    show_error(_("the attempt to load mesh from file has produced an error\n(either this is not a mesh file, or the mesh file is corrupted)"));
    fclose(fP);
    return FALSE;
  }
  fclose(fP);

  if(sp->im_mesh_filename[lp]) g_free(sp->im_mesh_filename[lp]);
  sp->im_mesh_filename[lp] = g_strdup(file);

  /* we set this before, 
     otherwise the "factor_normalize" goes berseker */
  sp->max_wins++;
  GtkWidget *window=sp->im_widget[lp]=create_diff_window(); 
  gtk_widget_set_data_top(window,"imagenum",
			  GUINT_TO_POINTER(lp));
  sp->mf.im_warp_factor[lp]=0;
  sp->mf.im_dissolve_factor[lp]=0;
  sp->im_drawingarea_widget[lp] = NULL;
  sp->im_widget_is_difference_mesh[lp]=TRUE;
  {
    gchar * N=compute_title (lp);
    gtk_window_set_title (GTK_WINDOW(sp->im_widget[lp]),N);
    g_free(N);
  }
  gtk_widget_show(window);
  return TRUE;
}






gboolean load_image_from_file_not_ply(int lp,
				      const char *file)
{
  GdkPixbuf      *impixfile;

#if GTK_MAJOR_VERSION < 2  
  impixfile= gdk_pixbuf_new_from_file(file);
  if (impixfile ==NULL)    {
      showerr(file,_("\
the attempt to load the image file %s has produced error: %s"));
      return FALSE;
    }
#else
  {
    GError *err=NULL;
    impixfile= gdk_pixbuf_new_from_file(file,&err);        
    if(err)      {
	show_error((err)->message);g_error_free (err);
	return FALSE;
      }
  }
#endif

    {
      sp->im_width[lp]= gdk_pixbuf_get_width(impixfile);
      sp->im_height[lp]= gdk_pixbuf_get_height(impixfile);
      gtk_subimasel_reset( &(sp->subimasel[lp]) ,
			   sp->im_width[lp],
			   sp->im_height[lp]);

#ifndef RESCALE_RELOAD_LESS_MEM
      destroy_pixbuf(lp,PIXLOADED );
#endif      
      destroy_pixmap(lp,PIXLOADED );

      subimages_spinbuttons_set(lp);
      subimage2affines(lp);

      sp->im_loaded_pixbuf[lp]=impixfile;
      create__pixmap(lp,PIXLOADED );
      render_pixmap(lp, PIXLOADED);

      scale_loaded_pixbuf_et_rrggbb(lp);
      render_pixmap(lp, PIXSUBIMAGE);
      //FIXME do we do this here?
      //render_pixmap(lp, PIXWARPED);

#ifdef RESCALE_RELOAD_LESS_MEM
      destroy_pixbuf(lp,PIXLOADED );
      impixfile=NULL;
#endif


      if(sp->im_filename_in[lp]!=NULL)
	g_free(sp->im_filename_in[lp]);
      
      sp->im_filename_in[lp]=g_strdup(file);
      {
	gchar * N = compute_title (lp);
	gtk_window_set_title ( GTK_WINDOW(sp->im_widget[lp]),   N);
	g_free(N);
      }
      set_editview( lp, EDITVIEW_EYES); 

      drawingarea_configure(lp);
      MY_GTK_DRAW( sp-> im_widget[lp]);  
      return TRUE;
    }  
}


gboolean load_image_from_file(int lp,
			      const char *file)
{
  int len=strlen(file);
  g_assert(lp>0);
  if (strcmp(file+len-4,".ply"))
    return load_image_from_file_not_ply(lp,file);
  else
    {
#if HAVE_LIBPLY_H
    if(!read_ply_) {
      show_error(_("gtkmorph could not find libply, so it cannot load PLY surfaces. Read documentations to install libply."));
      return FALSE;    
    }
    if(!read_ply_(file,&sp->im_ply_surface[lp])) {
      showerr(file,_("error while loading PLY file"));
      return FALSE;
    }
    return TRUE;
#else
    show_error(_("gtkmorph was NOT compiled with PLY support"));
    return FALSE;
#endif
  } 
}




gboolean save_image_to_file(int lp,
			    const char *file)
{
  GdkPixbuf * pb=*(which_pixbuf_is_visible(lp));
  if(sp->im_filename_out[lp] == NULL ||
     0!=strcmp(sp->im_filename_out[lp],file)) {
    if( g_file_test(file, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR ))
      {
	showerr(file,_("\
This file already exists! If you really want to overwrite it, delete it."));
	return FALSE;
      }
  }
  gboolean result=  save_pixbuf_to_file(file,pb);
  if(result)
    sp->im_filename_out[lp]=g_strdup(file);
  return result;  
}



/* reloads image , if necessary,  and rescales it */
void
reload_and_scale_image(int i)
{
  GdkPixbuf **impixfile = which_pixbuf(i,PIXLOADED);
  g_assert(i > 0);

  if(*impixfile == NULL) {
#ifndef RESCALE_RELOAD_LESS_MEM
    g_warning("lost original pixbuf %d... why???\n try reloading\n",i);
#endif
    if ( sp-> im_filename_in[i] == NULL
	 || strlen( sp-> im_filename_in[i]) == 0)	
      {
	show_error
	  ( g_strdup_printf(_("can't resize %dth image-no filename"),i));
	return;
      }
    /* reload */
#if GTK_MAJOR_VERSION < 2 
    *impixfile= gdk_pixbuf_new_from_file(sp-> im_filename_in[i]);
#else
    {
      GError *err=NULL;
      *impixfile= gdk_pixbuf_new_from_file(sp-> im_filename_in[i],&err);
      //FIXME should use err
    }
#endif
    if (*impixfile ==NULL)
      {
	showerr(sp-> im_filename_in[i], _("the attempt to reload the image file %s to resize it has produced error: %s"));
	create__pixbuf(i, PIXLOADED);
	return;
      }
  }
#ifdef RESCALE_RELOAD_LESS_MEM
  else {
    g_warning("the original pixbuf %d was not deallocated, reusing it\n",i);
    //FIXME why?? gdk_pixbuf_ref(*impixfile);
  }
#endif

  g_assert(*impixfile);
  g_assert(&(sp->im_loaded_pixbuf[i])==impixfile);
  render_pixmap(i, PIXLOADED);
  scale_loaded_pixbuf_et_rrggbb(i);
  render_pixmap(i, PIXSUBIMAGE);
#ifdef RESCALE_RELOAD_LESS_MEM
  delete_pixbuf(i, PIXLOADED);
#endif
}


