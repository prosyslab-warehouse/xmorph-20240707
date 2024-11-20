#include "stdio.h"
#include <string.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#define GTK_ENABLE_BROKEN
#include <gtk/gtk.h>
#include "gtk-meta.h"

#ifdef USE_IMLIB
#include <gdk_imlib.h>
#else
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

#ifdef HAVE_X
#include <gdk/gdkx.h>
#endif
#include <gdk/gdkrgb.h>


#include <gdk/gdk.h>

#include "interface.h"
#include "guide.h"
#include "support.h"
#include "callbacks.h"


#include "main.h"

/* trick to have a translation that is not immediate 
//#ifdef ENABLE_NLS 
//#undef _
//#define _(S) (g_strdup(dgettext(PACKAGE,(S))))
////#define _(S) (g_strdup(S))
//#endif
*/

#if GTK_MAJOR_VERSION >= 2
#define SHOW(A)      gtk_widget_show(A);gtk_window_deiconify(GTK_WINDOW(A));
#define HIDE(A)      gtk_window_iconify(GTK_WINDOW(A));
#else
#define SHOW(A)      gtk_widget_show(A);
#define HIDE(A)      gtk_widget_hide(A);
#endif


extern char **pane_text;

int guide_step=0,  guide_tot_steps=-1;
GtkWidget *guide_widget=NULL;

#if GTK_MAJOR_VERSION >= 2
GtkTextView *guide_widget_text=NULL;
#else
GtkText *guide_widget_text=NULL;
GtkEditable *guide_widget_editable=NULL;
#endif




gboolean guide_callback(char *what,
		    int imagenum)
{
  int lp=0;
  if(!guide_widget)
    return FALSE;

  if(!(sp->im_widget[imagenum])) {
    g_critical("guide cannot auto hide %d",imagenum);
    return FALSE;
  }
    
  if ( guide_step!= 6 && ! 
       (strcmp(what,"file")==0 && imagenum==MAIN_WIN)
       )  {
    HIDE(sp->im_widget[imagenum]);
    

    for(lp=0;lp<MAX_WINS+2;lp++)
      if(sp->im_widget[lp])
	if( GTK_WIDGET_VISIBLE (sp->im_widget[lp])) {
	  g_debug("guide:image %d is visible",lp);
	  return TRUE;
	}
    on_back_to_guide_activate(NULL,NULL);
  }
  return TRUE;
}




void guide_set_text()
{
  char s[6000];

  g_return_if_fail(guide_widget_text);

  if(!pane_text || !pane_text[guide_step*2] || !pane_text[guide_step*2+1]) {
    g_critical("no guide text %d!!!",guide_step);
    return;
  }

  { 
    char c=pane_text[2*guide_step][0];
    if (!(c == '-' || c == ';' ||c == ':') )      
	g_warning("The guide pane %d has a title '%s' that does not"
		  "start with character '-',';',':'. "
		  "Please correct the program or the translation.",
		  guide_step,pane_text[2*guide_step]);
  }

  sprintf(s,"gtkmorph guide %d:%s",guide_step,pane_text[guide_step*2]+1);
  gtk_window_set_title(GTK_WINDOW (guide_widget), s);  
  sprintf(s,"                                            [task %d:%s]\n",
	  guide_step,pane_text[guide_step*2]+1);

#if GTK_MAJOR_VERSION >= 2
 {

  GtkTextBuffer *buffer;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (guide_widget_text));

  gtk_text_buffer_set_text (buffer, s,-1);
  gtk_text_buffer_insert_at_cursor (buffer,
				    pane_text[guide_step*2+1],-1);
  /* Now you might put the view in a container and display it on the
   * screen; when the user edits the text, signals on the buffer
   * will be emitted, such as "changed", "insert_text", and so on.
   */
 }
#else
 {
   gint position;
   gtk_text_freeze (guide_widget_text);
  if(gtk_text_get_length (guide_widget_text)>0)
    gtk_editable_delete_text(guide_widget_editable,//GtkEditable *editable,
			     0,//gint start_pos,
			     gtk_text_get_length (guide_widget_text));
  gtk_text_set_point(guide_widget_text,0);
  gtk_editable_insert_text(guide_widget_editable,
			   s,strlen(s),   &position);
  gtk_editable_insert_text(guide_widget_editable,//GtkEditable *editable,
			   pane_text[guide_step*2+1],//const gchar *new_text,
			   strlen(pane_text[guide_step*2+1]),
			   //gint new_text_length,
			   &position);//gint *position);

  gtk_text_thaw (guide_widget_text);
 }
#endif
}



static void show_all(int step)
{ 
  int lp=0,m=MAIN_WIN;

  while (gtk_events_pending())
    gtk_main_iteration ();


  if(step==0 || step==1 || step == 4) {
    lp=m;
  }  else
    if(step==3||step==5||step==6)
      m--;

  for(;lp<=m;lp++)
    if(sp->im_widget[lp]) {
      SHOW(sp->im_widget[lp]);
    }
}

static void hide_all()
{ 
  int lp;

  while (gtk_events_pending())
    gtk_main_iteration ();

  for(lp=0;lp<MAX_WINS+2;lp++)
    if(sp->im_widget[lp]) {
      HIDE(sp->im_widget[lp]);
    }
}





void guide_initialize()
{
  int lp=0;

  if(guide_widget)
    return;

  guide_init_text();
  
  while(pane_text[lp])
    lp++;
  lp--;
  guide_tot_steps=lp/2;
  
  guide_widget=create_guide();


#if GTK_MAJOR_VERSION >= 2
  guide_widget_text=GTK_TEXT_VIEW(lookup_widget(guide_widget,"text"));
  g_assert(guide_widget_text);
#else
  guide_widget_text=GTK_TEXT(lookup_widget(guide_widget,"text"));
  guide_widget_editable=GTK_EDITABLE(lookup_widget(guide_widget,"text"));
  g_assert(guide_widget_text && guide_widget_editable);
#endif



  guide_set_text();
  gtk_window_set_default_size(GTK_WINDOW(guide_widget),700,300);
  gtk_widget_show(guide_widget);
  
  hide_all();

  settings_set_value("no warnings",1);
}

void guide_next()
{
  guide_step++;
  while(guide_step<guide_tot_steps && 
	(    ( pane_text[2*guide_step][0] == ':' && sp->max_wins==1 )
	  || ( pane_text[2*guide_step][0] == ';' && sp->max_wins> 1 )
	     ))
    guide_step++;
}


/*************************** callbacks **********************************/
void
on_back_to_guide_activate              (GtkButton     *button,
                                        gpointer         user_data)
{
  guide_initialize();
  guide_next();
  if(guide_step<guide_tot_steps) {
    guide_set_text();
    hide_all();
    SHOW(guide_widget);
  } else
    gtk_object_destroy(GTK_OBJECT(guide_widget)); 
}


void
on_guide_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  int lp;
  for(lp=0;lp<MAX_WINS+2;lp++)
    if(sp->im_widget[lp])
      {
	GtkWidget *w=(lookup_widget(sp->im_widget[lp],"back_to_guide"));
	if(w) gtk_widget_show(w);
      }
  guide_initialize();
  guide_set_text();
  hide_all();
  gtk_widget_show(guide_widget);
  SHOW(guide_widget);
}


gboolean
on_guide_text_expose_event             (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
  return FALSE;
}


void
on_guide_prev_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
  if(guide_step>0)
    guide_step--;
  guide_set_text();
}


void
on_guide_do_it_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  int lp=0;
#ifndef IS_PLYMORPH
  int z=EDITVIEW_EYES;
  if(guide_step>5)
    z=EDITVIEW_EDIT;
  if(guide_step==8)
    z=EDITVIEW_SHOWMESHES;

  for(lp=0;lp<=MAIN_WIN;lp++)
    if(sp->im_widget[lp]) {
      set_editview(lp,z);
    }
#endif
  show_all(guide_step);
}


void
on_guide_next_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
  guide_next();
  if(guide_step<=guide_tot_steps) {
    guide_set_text();
  } else {
    guide_step=0;
    gtk_object_destroy(GTK_OBJECT(guide_widget)); 
  }
}


gboolean
on_guide_delete_event                  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  //destroy will be called as well
  return FALSE;
}


void
on_guide_destroy  (GtkObject       *object,
		   gpointer         user_data)
{
  //GtkWidget *widget = GTK_WIDGET(object);
  int lp;
  show_all(-1);
  guide_widget=NULL;
  guide_widget_text=NULL;
#if GTK_MAJOR_VERSION < 2
  guide_widget_editable=NULL;
#endif
  settings_set_value("no warnings",0);
  for(lp=0;lp<MAX_WINS+2;lp++)
    if(sp->im_widget[lp])
      {
	GtkWidget *w=(lookup_widget(sp->im_widget[lp],"back_to_guide"));
	if(w) gtk_widget_hide(w);
      }
}
