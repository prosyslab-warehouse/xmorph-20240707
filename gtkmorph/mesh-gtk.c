/* by A Mennucci, Aug 2000
 *

 this code is the interface between the GTK+ toolkit and libmorph

 it is an intermediate step in between, 
 the mesh code and the complexity of the whole GUI

 it should be part of libmorph, but  then libmorph would need 
 to be linked against GTK+, and this is unwanted


*/


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "../libmorph/spl-array.h"
#include "../libmorph/mesh.h"
#include "../libmorph/relax.h"

#include "gtk/gtk.h"
#include <glib.h>

#ifdef USE_IMLIB
#include <gdk_imlib.h>
#else
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

#ifdef HAVE_X
#include <gdk/gdkx.h>
#endif
#include <gdk/gdkrgb.h>


#include "gtk-meta.h"

#include "gtktopdata.h"

#ifdef HAVE_LIBART
#include <libart_lgpl/libart.h>
#endif

#include "main.h"
#include "callbacks.h"
#include "mesh-gtk.h"
#include "interface.h"
#include "feature.h"
#include "utils.h"
#include "support.h"


#ifdef  G_THREADS_ENABLED
#if GLIB_MAJOR_VERSION >= 2
//FIXME
// for some strange reasons, 
//the windows are not drawn while the thread is runnig
// so I disable this code
//#define SMOOTH_THREADS
#else
#warning  the code for glib-1.2 threads is incomplete and malfunctioning
#endif
#endif

  
double ortho=0.1;

static double  smooth_mesh_once(MeshT *mesh,int keepborder)
{
  double change;

  g_return_val_if_fail(mesh->nx>=2 && mesh->ny>=2,0);
#ifdef USELESS
  struct mesh_variance_s  aft,bef;
  const int anylabel=0;
  struct mesh_variance_s ref=
    {sp->resulting_width*sp->resulting_width/8,
     sp->resulting_height *sp->resulting_height/8,
     0,// am I stupid? YES sp->resulting_width*sp->resulting_height/8,
     sp->resulting_width/2,sp->resulting_height/2};

  MeshT copymesh;
  if(!keepborder) {
    meshInit(&copymesh);
    meshAlloc(&copymesh,mesh->nx,mesh->ny);
    meshCopy(&copymesh,mesh);
    bef=mesh_variance(mesh, anylabel); 
    //avoid crisis
    bef.vx=CLAMP(bef.vx,400,ref.vx);
    bef.vxy=CLAMP(bef.vxy,0,ref.vxy);
    bef.vy=CLAMP(bef.vy,400,ref.vy);
    bef.mx=CLAMP(bef.mx,ref.mx-100,ref.mx+100);
    bef.my=CLAMP(bef.my,ref.my-100,ref.my+100);
  }
#endif
  change=smooth_energy_mesh
    (settings_get_value("energy for mesh smoothing"),
     mesh,  
     settings_get_value("mesh cant overlap"),
     keepborder,
     ortho,  sp->resulting_width, sp->resulting_height);
#ifdef USELESS
  if(!keepborder) {
    aft=mesh_variance(mesh, anylabel);

    //mesh_normalize_variance(mesh,anylabel,bef,aft);
    change= meshDistance(mesh,&copymesh,1);
    //  meshFreeReally(&copymesh);
    meshUnref(&copymesh);
  }
#endif
  return change;
}



static gboolean    smooth_idle(gpointer data);
#ifdef SMOOTH_THREADS
static gpointer    smooth_thread(gpointer data);
#endif

GHashTable *idle_data_by_mesh=NULL;
struct idle_data_t {
  MeshT *mesh;GtkWidget *widget;
  double total_change ; // , partial_change; 
  int keepborder;
  GTimeVal time;
#ifdef SMOOTH_THREADS
  GThread* thread;
#endif
  gint idle_pid;
  //  gint width;gint height
};


static double  smooth_mesh(MeshT *mesh,GtkWidget *widget)
{
  struct idle_data_t *i;
  if(idle_data_by_mesh==NULL)
    idle_data_by_mesh = g_hash_table_new(NULL,NULL);
  i=  g_hash_table_lookup  (idle_data_by_mesh,//GHashTable *hash_table,
			    mesh); //gconstpointer key
  if(i){
    i->total_change=0;
    //g_debug("resetting  smoothing mesh 0x%lx",(long)mesh);
  }else{
    i=g_malloc0(sizeof(struct idle_data_t));
    i->mesh=mesh;
    i->widget=widget;
    i->total_change=0;
    g_get_current_time(&(i->time));
    i-> keepborder= settings_get_value("preserve border");

    g_hash_table_insert (idle_data_by_mesh,//GHashTable *hash_table,
			 mesh, //                  gpointer key,
			 i );//value
#ifdef SMOOTH_THREADS
    if(g_thread_supported ()) {
      GError *err=NULL;
      i->thread= g_thread_create(smooth_thread,//GThreadFunc func,
				 i,//gpointer data,
				 FALSE,//gboolean joinable,
				 &err);
      g_debug("starting thread %lx for smoothing mesh 0x%lx",
	      (long)i->thread,(long)mesh);
      if (err)	{ g_warning(  err->message ); g_error_free (err); }
      } else
#endif      
	{
	  i->idle_pid=(g_idle_add(smooth_idle,i));//gpointer value
	  g_debug("starting idle_smoother %d for smoothing mesh 0x%lx",
		  i->idle_pid,  (long)mesh);
	}

  } 
  return smooth_mesh_once(mesh,i-> keepborder);
}

static gboolean  smooth_idle_stop_one  //(*GHRFunc)
(gpointer key,
 gpointer value,
 gpointer user_data)
{
  struct idle_data_t *i=value;
 /* quick exit */
  i->total_change=1e100;
  return TRUE;
}

gint smooth_idle_stop()
{
  if(idle_data_by_mesh)
    return g_hash_table_foreach_remove(idle_data_by_mesh,//GHashTable 
				       smooth_idle_stop_one,// GHRFunc func,
				       NULL);//gpointer user_data);
  else return 0;
}



gboolean  smooth_idle_stop_by_mesh(MeshT *mesh)
{
  struct idle_data_t *i;
  if(idle_data_by_mesh){
    i=  g_hash_table_lookup  (idle_data_by_mesh,//GHashTable *hash_table,
			      mesh); //gconstpointer key
    /* quick exit */
    if(i)
      i->total_change=1e100;
    else
      return FALSE;
    return TRUE;
  } else
    return TRUE;
}



static gboolean  smooth_idle(gpointer data)
{
  struct idle_data_t *i=data;
  MeshT *mesh=i->mesh;

  double change=0, maxchange=sp->resulting_width * mesh->nx * mesh->ny,
    elapsed;

  GTimeVal bef=i->time,aft;

  /* 1e100 is the quick exit flag (when window is deleted)*/
  if(mesh->x && i->total_change < 1e100) {    
    change=smooth_mesh_once(mesh,i-> keepborder);
    i->total_change+=change;
    //i->partial_change+=change;
  }

  g_get_current_time(&aft);
  elapsed=(double)(aft.tv_sec-bef.tv_sec) +
    (double)(aft.tv_usec -bef.tv_usec) /1e6 ;

  if( i->total_change < 1e100  && elapsed > 0.4 // && i->partial_change>20 
      &&  i->widget && GTK_WIDGET_REALIZED(i->widget)) {
    //if(i->partial_change>20)      i->partial_change=0;  
    i->time=aft;
    MY_GTK_DRAW(i->widget);
  }

  if (mesh->x==NULL || change < 0.001 || i->total_change > maxchange)   {
    g_hash_table_remove (idle_data_by_mesh, mesh);
    //NOO! RECURSIVE GTK!!!  while (gtk_events_pending())gtk_main_iteration ();
    g_free(data);
    g_debug("stop smoothing mesh 0x%lx, change %g",(long)mesh,i->total_change);
    return FALSE;
  }
  return TRUE;
}


  //  fprintf(stderr,"\r change %f total_change %f mesh %ld",    
//	  change,i->total_change, i->mesh); 


#ifdef SMOOTH_THREADS
static gpointer    smooth_thread(gpointer data)
{
  struct idle_data_t *i=data;
  MeshT *mesh=i->mesh;
  while ( smooth_idle(data)  ) { g_thread_yield () ; g_usleep(10000) ; } //
  g_debug("stopping thread %lx for smoothing mesh %lx",
	  (long)i->thread,(long)mesh);
  return data;
}
#endif



/*********************************************************************/

/*changes to point to point to upper left corner point, not nearest */
static void
point_to_upper_left(MeshT *mesh,
		    int *mi, int *mj, /* the mesh point affected */
		    GdkEventButton  *event)
{
  if ( meshGetxClamp(mesh, *mi,*mj) > event->x //&&       *mi>0 //
       //meshMaxx(mesh) < *mi 
       )
    { *mi -= 1;}

  if ( meshGetyClamp(mesh, *mi,*mj) > event->y //&&       *mj > 0 //
	       //meshMaxy(mesh) < *mj 
       )
    {*mj -= 1; }
}

/*********************************************************************
 these callbacks are used when editing a mesh

 if they return TRUE, something was done to point *mi *mj
 (and often the mesh is smootheed)
********************************************************************/



enum tools active_tool=0;

static int last_mi, last_mj, last_label;
static int last_x=0, last_y=0;
enum tools last_action;
GdkEventButton  last_event;
MeshT *last_mesh=NULL;
GtkWidget *last_widget=NULL;



void
on_unselect_point_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  last_action=tool_unselect;
  if(meshGetLabel(last_mesh, last_mi, last_mj)== 0 
     // ||sp->im_editview[MAIN_WIN] != EDITVIEW_FEATURES
     )
    {gdk_beep();return;}
  //if(   meshGetLabel(last_mesh, last_mi, last_mj) != 0 )
  {
    if (settings_get_value("mesh auto sync")) {
      int lp=MAIN_WIN;
      for(; lp>=0; lp--) 
	if( sp->im_widget[lp] != NULL) {
	  meshSetLabel(&sp->im_mesh[lp], last_mi, last_mj, 0);
	  MY_GTK_DRAW (sp->im_widget[lp]);
	}} else
	  meshSetLabel(last_mesh, last_mi, last_mj, 0);
  }
  smooth_mesh(last_mesh,last_widget);
}


void
on_assign_point_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  extern int feature_n;
  last_action=tool_assign;
  if( feature_n < 0
     //meshGetLabel(last_mesh, last_mi, last_mj) ==     feature_n ||
     //|| sp->im_editview[MAIN_WIN] != EDITVIEW_FEATURES
     )
    {gdk_beep();return;}
  //if(meshGetLabel(last_mesh, last_mi, last_mj)!=-1) 
  {
    if (settings_get_value("mesh auto sync")) {
      int lp=MAIN_WIN;
      for(; lp>=0; lp--) 
	if( sp->im_widget[lp] != NULL) {
	  meshSetLabel(&sp->im_mesh[lp], last_mi, last_mj, feature_n+1);
	  MY_GTK_DRAW(sp-> im_widget[lp]);  
	}} 
    else
      meshSetLabel(last_mesh, last_mi, last_mj, feature_n+1);
  }
}


void
on_select_point_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  last_action=tool_select;
  if(meshGetLabel(last_mesh, last_mi, last_mj)==-1 
     //||sp->im_editview[MAIN_WIN] != EDITVIEW_FEATURES
     )
    {gdk_beep();return;}
  //if(meshGetLabel(last_mesh, last_mi, last_mj)!=-1) 
  {
    if (settings_get_value("mesh auto sync")) {
      int lp=MAIN_WIN;
      for(; lp>=0; lp--) 
	if( sp->im_widget[lp] != NULL) {
	  meshSetLabel(&sp->im_mesh[lp], last_mi, last_mj, -1);
	  MY_GTK_DRAW(sp-> im_widget[lp]);  
	}} 
    else
      meshSetLabel(last_mesh, last_mi, last_mj, -1);
  }
}



void
on_menu_smooth_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  smooth_mesh(last_mesh,last_widget);
}

void
on_add_horizontal_line_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  point_to_upper_left(last_mesh,&last_mi,&last_mj,&last_event);
  last_action=4;
  if (settings_get_value("mesh auto sync")) {
    int lp=MAIN_WIN;
      for(; lp>=0; lp--) 
	if( sp->im_widget[lp] != NULL)  {
	    meshLineAdd(&sp->im_mesh[lp], last_mj, 0.5, 2);
	    MY_GTK_DRAW(sp->im_widget[lp]);
	}
  } else
  meshLineAdd(last_mesh, last_mj, 0.5, 2);

  sp->meshes_x= MAX(sp->meshes_x, last_mesh->nx);
  sp->meshes_y= MAX(sp->meshes_y, last_mesh->ny);
  smooth_mesh(last_mesh,last_widget);
  redraw_images();
}


void
on_add_vertical_line_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  point_to_upper_left(last_mesh,&last_mi,&last_mj,&last_event);  
  last_action=5;
  if (settings_get_value("mesh auto sync"))    {
      int lp=MAIN_WIN;
      for(; lp>=0; lp--) 
	if( sp->im_widget[lp] != NULL)  {
	    meshLineAdd(&sp->im_mesh[lp], last_mi, 0.5, 1);
	    MY_GTK_DRAW(sp->im_widget[lp]);//    gtk_widget_draw (sp->im_widget[lp] , NULL);
	}  
  }  else
    meshLineAdd(last_mesh, last_mi, 0.5, 1);
  sp->meshes_x= MAX(sp->meshes_x, last_mesh->nx);
  sp->meshes_y= MAX(sp->meshes_y, last_mesh->ny);
  smooth_mesh(last_mesh,last_widget);
  redraw_images();
}


void
on_del_horizontal_line_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  //point_to_upper_left(last_mesh,&last_mi,&last_mj,&last_event);  
  last_action=6;
  if (settings_get_value("mesh auto sync")) {
    int lp=MAIN_WIN;
    for(; lp>=0; lp--) 
	if( sp->im_widget[lp] != NULL)  {
	    meshLineDelete(&sp->im_mesh[lp], last_mj,  2);
	    MY_GTK_DRAW(sp->im_widget[lp]);// gtk_widget_draw (sp->im_widget[lp] , NULL);
	}
  } else
  meshLineDelete(last_mesh, last_mj,  2);
  smooth_mesh(last_mesh,last_widget);
  redraw_images();
}


void
on_del_vertical_line_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  //point_to_upper_left(last_mesh,&last_mi,&last_mj,&last_event);  
  last_action=7;
  if (settings_get_value("mesh auto sync"))    {
      int lp=MAIN_WIN;
      for(; lp>=0; lp--) 
	if( sp->im_widget[lp] != NULL)  {
	    meshLineDelete(&sp->im_mesh[lp], last_mi,  1);
	    MY_GTK_DRAW(sp->im_widget[lp]);//gtk_widget_draw (sp->im_widget[lp] , NULL);
	}  
  }  else
    meshLineDelete(last_mesh, last_mi,  1);
  smooth_mesh(last_mesh,last_widget);
  redraw_images();
}




void
on_adjust_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  extern  int last_clicked_image ;
  int  lp = last_clicked_image ;

  MeshT *mesh=&(sp->im_mesh[lp]);
  GdkPixbuf   *dst= (sp->im_warped_pixbuf[lp]);

#if 0
  int x,y,dx,dy; int *mi, *mj;
  long d;
  GdkModifierType state;
  gdk_window_get_pointer (event->window, &x, &y, &state);
  d=meshPointNearest(mesh , 
		     x,y, //event->x, event->y, //int px, int py, 
		     mi, mj, 
		     &dx, &dy);
#endif
  adjust_1_point_wrt_morph(mesh,dst ,  last_mi, last_mj);
  redraw_images();
  //gtk_widget_update_rect (widget, x, y,90);     
}


/******************************************************************/



/*********************************************************************/

MeshT rubbermesh;

gboolean dragging=FALSE;




gboolean
gdk_mesh_key_press_event         (GtkWidget       *widget,
				  GdkEventKey     *event,
				  gpointer         user_data,
				  MeshT *mesh ,
				  /* the mesh point affected */
				  int *mi, int *mj, int *mlabel,
				  enum tools  *action )
{
  if ((event->keyval >=48 && event->keyval <58 )
      || (event->keyval >=105 && event->keyval <= 108  )) {
    {
      int x,y,dx,dy;
      long d;
      //GdkModifierType state;
      // THIS IS OFF:  it is computed w/o attention to scrolling
      //gdk_window_get_pointer (event->window, &x, &y, &state);
      //dont exist; x=event->x ; y=event->y;
      x=last_x; y=last_y;
      d=meshPointNearest(mesh , 
			 x,y, //event->x, event->y, //int px, int py, 
			 mi, mj, 
			 &dx, &dy);
      my_gtk_widget_update_rect (widget, x, y,90); 
    }
    last_mi=*mi; last_mj=*mj;
    last_label=*mlabel=meshGetLabel(mesh, *mi, *mj);
    //last_event=*event;
    last_mesh=mesh;
    last_widget=widget;

    if(event->keyval ==48)
      on_unselect_point_activate(NULL,NULL);
    if(event->keyval >48 && event->keyval <58) {
      meshSetLabel(mesh, last_mi, last_mj, event->keyval-48);  
    }
    if (event->keyval >=105 && event->keyval <= 108  ) {
      double x,y;
      x=meshGetx(mesh, *mi, *mj);
      y=meshGety(mesh, *mi, *mj);
      if (event->keyval ==105)
	y-=.333;

      if (event->keyval ==106)
	x-=.333;

      if (event->keyval ==107)
	y+=.333;

      if (event->keyval ==108)
	x+=.333;

      meshSet(mesh, *mi, *mj, x, y);
    }
    return TRUE;
  } else   return FALSE;
}


#define NORMALIZE_X_Y \
      if(settings_get_value("preserve border")){\
	if(*mi==0)\
	  x=meshMinx(mesh);\
	else\
	  if(*mi == mesh->nx-1)\
	    x=meshMaxx(mesh);\
          else  x=CLAMP(x,meshMinx(mesh) ,meshMaxx(mesh));\
	if( *mj==0 )\
	  y=meshMiny(mesh);\
	else\
	  if( *mj == mesh->ny-1)\
	      y=meshMaxy(mesh);\
          else y=CLAMP(y,meshMiny(mesh) ,meshMaxy(mesh));\
}

      

gboolean
gdk_mesh_button_press_event  (GtkWidget       *widget,
			      GdkEventButton  *event,
			      gpointer         user_data,
			      MeshT *mesh,
			      /* the mesh point affected */
			      int *mi, int *mj, int *mlabel,
			      enum tools  *action, /* what was done: */
			      gboolean readonly)
{
  //int mi; int mj;  
  
  int x; int y;
  long d;
  int e=settings_get_value("edit features");

  //g_message("press");
  *action=active_tool;
  x=event->x; y= event->y;  
  x-= PIXEDITBORDER;
  y-= PIXEDITBORDER;

  {
    int dx; int dy;    
    d=meshPointNearest(mesh , 
		       x, y, //int px, int py, 
		       mi, mj, 
		       &dx, &dy);
  }

  last_mi=*mi; last_mj=*mj;
  last_label=*mlabel=meshGetLabel(mesh, *mi, *mj);
  last_event=*event;
  last_mesh=mesh;
  last_widget=widget;

  if(readonly) {
    last_label=*mlabel=meshGetLabel(mesh, *mi, *mj);
    return TRUE;
  }
  
  if(event->button == 1) 
    {
      //g_message("press");


      NORMALIZE_X_Y

      dragging=TRUE;

      if(active_tool == tool_edit &&( last_label!=0 || e )) {
	if(e && last_label==0)
	  on_select_point_activate(NULL,NULL);
	meshSet(mesh, *mi, *mj, x,y);
	smooth_mesh(last_mesh,last_widget);
      } else if (active_tool == tool_move ||active_tool == tool_stretch) {
	if(last_label==0)
	  { gdk_beep(); dragging = FALSE; return FALSE; }
      } else if(e) {
	if(active_tool == tool_unselect )
	  on_unselect_point_activate(NULL,NULL);
	if(active_tool == tool_select )
	  on_select_point_activate(NULL,NULL);
	if(active_tool == tool_assign ) 
	  on_assign_point_activate(NULL,NULL);	
      } else { gdk_beep(); dragging = FALSE; return FALSE; }

      //prepare for rubbermesh dragging
      meshCopy(&rubbermesh,mesh);

      last_label=*mlabel=meshGetLabel(mesh, *mi, *mj);

      //FIXME 180 is a wild guess
      //gtk_widget_update_rect (widget, x, y,180); 
      MY_GTK_DRAW(widget);//gtk_widget_draw(widget,NULL);
      return TRUE;
    }
  else
  if(event->button == 2 ) 
    {
      smooth_idle_stop_by_mesh(mesh);
      return FALSE;
    }
  else
    if(event->button==3)
      {
	static GtkWidget* m=NULL;
	smooth_idle_stop_by_mesh(mesh);
	if(m==NULL) 
	    m=create_menuEditMesh();
	if(e) {
	  gtk_widget_set_data_top(m,"mesh",mesh);
	  gtk_widget_set_data_top(m,"drawable",widget);
	  gtk_menu_popup (GTK_MENU(m),//GtkMenu *menu,
			  NULL,//GtkWidget *parent_menu_shell,
			  NULL,//GtkWidget *parent_menu_item,
			  NULL,//GtkMenuPositionFunc func,
			  NULL,//gpointer data,
			  3,//guint button,
			  0);//guint32 activate_time);
	  return TRUE;
	} else { gdk_beep(); return FALSE;}
      }
    else
    {
      dragging=FALSE;
      return FALSE;
    }
}


gboolean
gdk_mesh_motion_notify_event   (GtkWidget       *widget,
				GdkEventMotion  *event,
				gpointer         user_data,
				MeshT *mesh,
				int *mi, int *mj/* the mesh point affected */ 
				)
{
  GdkModifierType state;
  int x,y;
  //track point position
  {
    if (event->is_hint)
      gdk_window_get_pointer (event->window, &x, &y, &state);
    else
      {
	x = event->x;
	y = event->y;
	state = event->state;
      }
    x-= PIXEDITBORDER;
    y-= PIXEDITBORDER;
    last_x=x; last_y=y;
    {
      char s[200];
      sprintf(s,"x %3d y %3d",x,y);
      set_info_label(s);
    }
  }
  
  if(!dragging)
    return FALSE;

  if (state & GDK_BUTTON1_MASK)
    {  
      //int mi; int mj;       
#ifndef LOSE_POINTS_WHILE_MOVING
      if(last_mi == -1 )
	{ int dx; int dy;	
	  g_warning("lost point while editing the mesh: WHY?!?");
	  /* I have to track a strange bug : sometimes I lose the point
	   it is as if this signal is executed BEFORE the button press!, 
	  or after the button RELEASE*/  
	  //prepare for rubbermesh dragging FIXME memory leak
	  meshCopy(&rubbermesh,mesh);

	  meshPointNearest(mesh , 
			   x, y, //int px, int py, 
			   mi, mj, 
			   &dx, &dy);
	  last_mi=*mi; last_mj=*mj;
	  last_mi=*mi; last_mj=*mj;
	  last_label=meshGetLabel(mesh, *mi, *mj);
	  last_mesh=mesh;
	  last_widget=widget;
	}
      else
	{      *mi=last_mi; *mj=last_mj; }
#endif

      NORMALIZE_X_Y

      if( active_tool == tool_edit) {
	meshSet(mesh, *mi, *mj, x, y);
	smooth_mesh(last_mesh,last_widget);
      } else
      if( active_tool == tool_move ) {
	meshSet(mesh, *mi, *mj, x, y);
	smooth_mesh_rubber(mesh,&rubbermesh,0,*mi,*mj,last_label,
			   settings_get_value("mesh cant overlap"),
			   settings_get_value("preserve border"),ortho,
			   sp->resulting_width, sp->resulting_height );
	smooth_mesh(mesh,last_widget);
      } else
      if(active_tool == tool_stretch ) {
	double rubberish=0.2;
	if(event->state&GDK_SHIFT_MASK )
	  rubberish+=0.2;
	if(event->state&GDK_CONTROL_MASK )
	  rubberish+=0.4;
	meshSet(mesh, *mi, *mj, x, y);
	smooth_mesh_rubber(mesh,&rubbermesh,rubberish,*mi,*mj,last_label,
			   settings_get_value("mesh cant overlap"),
			   settings_get_value("preserve border"),ortho ,
			   sp->resulting_width, sp->resulting_height);
	smooth_mesh(mesh,last_widget);
      }

      ////FIXME the rectangle size is a wild guess
      //gtk_widget_update_rect (widget, x, y, 200);
      MY_GTK_DRAW(widget);//gtk_widget_draw(widget,NULL);

      return TRUE;
    } 
  else
    return FALSE;
}




gboolean
gdk_mesh_button_release_event    (GtkWidget       *widget,
				  GdkEventButton  *event,
				  gpointer         user_data,
				  MeshT *mesh,
				  int *mi, int *mj
				  /* the mesh point affected */ 
				  )
{
  {
    int x,y;
    x = event->x;
    y = event->y;
    x-= PIXEDITBORDER;
    y-= PIXEDITBORDER;
    last_x=x; last_y=y;
  }
  
  if(!dragging)
    return FALSE;  
  //g_message("release");
  dragging=FALSE;
  last_mesh=mesh;
  smooth_mesh(mesh,widget);

  if(event->button==1) {
    /* I have to track a strange bug : lets be nasty */
    // last_mi=-1;
    //g_message("release");
    meshUnref(&rubbermesh);
  }
  *mi=last_mi; *mj=last_mj;
  MY_GTK_DRAW(widget);//gtk_widget_draw (widget , NULL);
  return TRUE;
}




/************************************************************
 * the code following is based on:
 *
 * diw_map.c: Digital Image Warping X graphical user interface
//
// Written and Copyright (C) 1994-1999 by Michael J. Gourlay
//
// PROVIDED AS IS.  NO WARRANTEES, EXPRESS OR IMPLIED.
//
*/







/* Meshpoint picking and drawing parameters */

/* MP_PICK_DIST: farthest you may be from a Mesh point to pick it 
   currently ignored
*/
//#define MP_PICK_DIST 15

//#define MP_SIZE 16

#define MP_ARC  (360*64)



#define MY_CALLOC(A,B) (g_malloc0((A)*sizeof(B)))
#define FREE(A) (g_free(A))





/* --------------------------------------------------------------- */

/* NAME
//   gdk_draw_mesh: Draw mesh lines and points for one mesh
//
//
//
//
// DESCRIPTION
//   Draw mesh lines and points.
//   Also draw the selected meshpoint.
//
//  if subimage !=NULL, rescale the mesh to stay in the subimage
//
// SEE ALSO
//   DrawMeshes
*/

typedef struct _GdkArc GdkArc;
struct _GdkArc {
  gint          filled;
  gint          x;
  gint          y;
  gint          width;
  gint          height;
  gint          angle1;
  gint          angle2;
} ;

#define USE_SPLINES

gboolean
gdk_draw_mesh(GdkDrawable  *drawable,
	      gboolean draw_lines_p,
	      GdkGC        *lines_gc, //lines GC
	      unsigned int draw_points_p,
	      GdkGC        *points_gc[], //points GC
	      gboolean draw_features_p, // FIXME unimplemented
	      MeshT *mesh,
	      //GdkRectangle *subimage,
	      //int height, //viewport height
	      //int width, //viewport width
	      const double affine[6]
	      )
{
  g_return_val_if_fail( meshAllocated(mesh) , FALSE );
  GdkPoint *xpoints;  
  int xi, yi;
  const double *xsP = mesh->x;
  const double *ysP = mesh->y;
  double *x_tmp; //= mesh->x;
  double *y_tmp; //= mesh->y;
  // this is not defined currently in gdk, so we define it above
  GdkArc     xarc;
  /* intelligent point size */
  int point_size= 6;//MIN (width / mesh-> nx, height / mesh->ny)/3;

#ifdef USE_SPLINES
  double *xrow;
  double *yrow;
  gint width,    height;
#if GTK_MAJOR_VERSION == 1
  gdk_window_get_size(drawable,&width,&height);
#else
  gdk_drawable_get_size(drawable,&width,&height);
#endif
#endif

  if ( ! draw_points_p && ! draw_lines_p && ! draw_features_p)
    return TRUE;

  g_return_val_if_fail(mesh && mesh->x && mesh->y,FALSE);

  xarc.width = point_size;
  xarc.height = point_size ;   
  xarc.angle1=0;
  xarc.angle2=MP_ARC;  
  { 
    int s=MAX(mesh->nx,mesh->ny);
    if(!(x_tmp=MY_CALLOC(s, double))) {
      return(FALSE);
    }
    if(!(y_tmp=MY_CALLOC(s, double))) {
      return(FALSE);
    }
#ifdef USE_SPLINES
    s=MAX(s,MAX(width/4,height/4))+1;
    if(!(xrow=MY_CALLOC(s, double))) {
      return(FALSE);
    }
    if(!(yrow=MY_CALLOC(s, double))) {
      return(FALSE);
    }
#endif
    if(!(xpoints=MY_CALLOC(s, GdkPoint))) {
      return(FALSE);
    }
  }
  /* Draw meshpoints and vertical meshlines */
  for(xi=0; xi < mesh->nx; xi++) {
    /* Draw vertical mesh lines */
    /* Create 1D array of mesh points knots */
    {
      double x,y;
      for(yi=0; yi<mesh->ny; yi++) {
	x=xsP[yi*mesh->nx + xi];  y=ysP[yi*mesh->nx + xi];
	x_tmp[yi] =  x*affine[0]+y*affine[1] +affine[2];    
	y_tmp[yi] =  x*affine[3]+y*affine[4] +affine[5];
      }
    }
    if(draw_lines_p) {
#ifdef USE_SPLINES
      /* Create abcissas of interpolant to make a smoothly drawn mesh line */
      /*       for(yi=0; yi<(height/4); yi++) */
      /* 	yrow[yi] = yi*4; */
      /* Make sure last abcissa is set */
      /*       yrow[yi-1] = height-1; */
      /* Create the interpolated line segments */
      hermite3_array2(y_tmp, x_tmp, mesh->ny, 0,4, xrow, height/4,0);
      /* Create an array of XPoints to draw smoothly interpolated mesh line */
      for(yi=0; yi<(height/4); yi++) {
	xpoints[yi].x = (int) xrow[yi];
	xpoints[yi].y = (int) yi*4;
      }
      /* Draw the smoothly interpolated mesh line */
      gdk_draw_lines(drawable,lines_gc , xpoints,height/4);
#else // USE_SPLINES
      for(yi=0; yi< mesh->ny; yi++) {
	xpoints[yi].x = (int) x_tmp[yi];
	xpoints[yi].y = (int) y_tmp[yi];
      }
      /* Draw the mesh lines */
      gdk_draw_lines(drawable,lines_gc , xpoints, mesh->ny);
#endif
    }

    /* Draw mesh points */
    if(draw_points_p)	{
      int l;
      for(yi=0; yi<mesh->ny; yi++) {
	xarc.x =  x_tmp[yi]   ;
	xarc.y =  y_tmp[yi]  ;	
	l=meshGetLabel(mesh,xi,yi);
	if (l && (l+1) < draw_points_p)
	  gdk_draw_arc  (drawable, points_gc[l+1],
			 TRUE,
			 xarc.x - point_size/2,
			 xarc.y - point_size/2,
			 xarc.width,
			 xarc.height,
			 xarc.angle1,
			 xarc.angle2 );
#ifdef DRAW_UNSELECTED_POINTS
	else
	  gdk_draw_arc  (drawable, lines_gc,
			 TRUE,
			 xarc.x - point_size/2,
			 xarc.y - point_size/2,
			 xarc.width/2,
			 xarc.height/2,
			 xarc.angle1,
			 xarc.angle2 );	
#endif
      }
    }
  }

  /* Draw horizontal meshlines */
  if(draw_lines_p) {  
    for(yi=0; yi<mesh->ny; yi++) {
      {
	double x,y;
	for(xi=0; xi<mesh->nx; xi++) {
	  x=xsP[yi*mesh->nx + xi];  y=ysP[yi*mesh->nx + xi];
	  x_tmp[xi] =  x*affine[0]+y*affine[1] +affine[2];    
	  y_tmp[xi] =  x*affine[3]+y*affine[4] +affine[5];
	}
      }
#ifdef USE_SPLINES 
/*       for(xi=0; xi<(width/4); xi++) */
/* 	xrow[xi] = xi*4; */
/*       xrow[xi-1] = width-1;	 */
      hermite3_array2(x_tmp, y_tmp, mesh->nx, 0,4, yrow, width/4,0);
      for(xi=0; xi<(width/4); xi++) {
	xpoints[xi].x = (int) xi*4;
	xpoints[xi].y = (int) yrow[xi];
      }
      gdk_draw_lines (drawable, lines_gc,
		      xpoints, width/4); 
#else
      for(xi=0; xi<mesh->nx; xi++) {
	xpoints[xi].x = (int) x_tmp[xi];
	xpoints[xi].y = (int) y_tmp[xi];
      }    
      gdk_draw_lines (drawable, lines_gc,
		      xpoints, mesh->nx);      
#endif
    }
  }

#ifdef USE_SPLINES
  g_free(xrow);
  g_free(yrow);
#endif
  FREE(x_tmp);
  FREE(y_tmp);
  g_free(xpoints);

  return(TRUE);
}

gboolean
gdk_draw_mesh_as_arrows(GdkDrawable  *drawable,
			GdkGC        *gc, 
			MeshT *mesh,
			MeshT *base_mesh,
			gboolean is_difference_mesh,
			const double affine[6])
{
  int xi, yi;
  g_return_val_if_fail( meshAllocated(mesh) && meshAllocated(base_mesh) , FALSE);
  const double c=cos(3.141 * 0.8), s=sin(3.141 * 0.8);
  const int nx=mesh->nx, ny=mesh->ny ;
  g_return_val_if_fail(nx==base_mesh->nx && ny==base_mesh->ny,FALSE);
  const double *xsP = mesh->x;
  const double *ysP = mesh->y;
  const double *xbP = base_mesh->x;
  const double *ybP = base_mesh->y;
  double xt, yt, xbt, ybt , xd, yd ;
  for(xi=0; xi < nx; xi++) { 
    for(yi=0; yi < ny; yi++) {
      if(meshGetLabel(mesh,xi,yi)>0 || meshGetLabel(base_mesh,xi,yi)>0)   {
	{
	  double xb, yb;
	  xb=xbP[yi*nx + xi];  yb=ybP[yi*nx + xi];
	  xbt =  xb*affine[0]+yb*affine[1] +affine[2];    
	  ybt =  xb*affine[3]+yb*affine[4] +affine[5];	  	  

	  if(!is_difference_mesh) {
	    double x,y ;
	    x =xsP[yi*nx + xi];  y =ysP[yi*nx + xi];
	    xt =  x*affine[0]+y*affine[1] +affine[2];    
	    yt =  x*affine[3]+y*affine[4] +affine[5];	    
	    xd=xt-xbt; yd=yt-ybt; 
	  } else {
	    double x,y ;
	    x =xsP[yi*nx + xi];  y =ysP[yi*nx + xi];
	    xt =  x*affine[0]+y*affine[1];    
	    yt =  x*affine[3]+y*affine[4];
	    xd=xt; yd=yt; xt=xd+xbt; yt=yd+ybt;
	  }
	}
	double xdr = xd * c - yd *s  ,  ydr =   xd * s + yd * c ,
     	       xdl = xd * c + yd *s  ,  ydl = - xd * s + yd * c ;
	gdk_draw_line(drawable,gc, xbt, ybt,  xt, yt);
	gdk_draw_line(drawable,gc, xt, yt,  xt+xdr/4., yt+ydr/4.);
	gdk_draw_line(drawable,gc, xt, yt,  xt+xdl/4., yt+ydl/4.);
	//gdk_draw_line(drawable,gc, xt+xdr/10, yt+ydr/10,  xt+xdl/10, yt+ydl/10);
      }}}
  return TRUE;
}


/************************************************************************/


void
on_tool_edit_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
  active_tool=tool_edit;
}


void
on_tool_move_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
  active_tool=tool_move;
}


void
on_tool_stretch_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
  active_tool=tool_stretch;
}


void
on_tool_select_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  active_tool=tool_select;
}


void
on_tool_unselect_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
  active_tool=tool_unselect;
}


void
on_tool_assign_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  active_tool=tool_assign;
}

