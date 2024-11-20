
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "stdio.h"

#include <gtk/gtk.h>

#ifdef USE_IMLIB
#include <gdk_imlib.h>
#else
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

#include "support.h"

#include "gtk-meta.h"

#include "main.h"
#include "utils.h"
#include "colors.h"
#include "interface.h"
#include "string.h"

#include "feature.h"


/**********************************************************
FEATURES
*/

// to change from feature_n to feature_gc you have to add 2
// to change from feature_n to mesh label you have to add 1


#define FEAT_COLOR_SIZE 24

gboolean on_feat_drawinga_expo(GtkWidget       *widget,
			       GdkEventExpose  *event,
			       gpointer         user_data)
{
  GdkGC *gc;
  GdkDrawable *d=widget->window;
  gc=user_data;
  gdk_gc_set_clip_rectangle (gc,    &event->area);
  gdk_draw_rectangle              (d,	   gc,
				   1,0,0,FEAT_COLOR_SIZE,FEAT_COLOR_SIZE); 
  //gdk_draw_line                   (d,widget->style->white_gc,
  //2,2,FEAT_COLOR_SIZE,FEAT_COLOR_SIZE);
  gdk_gc_set_clip_rectangle (gc,    NULL);
  return TRUE;
}


/*********** adds widget in the combo ****************/
void feat_widget_add(gchar *t,GtkCombo *combo, GdkGC *gc)
{
    GtkWidget  *item, *hbox,  *label, *d;
    g_return_if_fail(gc);    g_return_if_fail(combo);     g_return_if_fail(t);
    item = gtk_list_item_new();
    gtk_widget_show (item);

    hbox = gtk_hbox_new (FALSE, 4);
    gtk_container_add (GTK_CONTAINER (item), hbox);
    gtk_widget_show (hbox);

    d=gtk_drawing_area_new();
    gtk_drawing_area_size(GTK_DRAWING_AREA(d),FEAT_COLOR_SIZE,FEAT_COLOR_SIZE);
    gtk_widget_show (d);
    gtk_box_pack_start (GTK_BOX (hbox), d, FALSE, FALSE, 0);
    gtk_signal_connect (GTK_OBJECT (d), "expose_event", 
			GTK_SIGNAL_FUNC (on_feat_drawinga_expo), 
			gc);
 
    label = gtk_label_new (t);
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

    /* You must set the string to display in the entry field when the item is
       selected. */
    gtk_combo_set_item_string (GTK_COMBO (combo), GTK_ITEM (item), t);

    /* Now we simply add the item to the combo's list. */
    gtk_container_add (GTK_CONTAINER (GTK_COMBO (combo)->list), item);
}


static
gchar *feature_array[100];

int feature_n=-10;
static
int feature_last_n=-10;
static
int feature_len=0;



/********************************************/

/* cache, sort of */
static GtkEditable     *combo_entry_feature_itself=NULL;
GtkEditable * combo_entry_lookup()
{
  if(NULL==combo_entry_feature_itself)
    combo_entry_feature_itself=GTK_EDITABLE 
      (lookup_widget(sp->im_widget[MAIN_WIN],
		     "combo_entry_feature"));
  return combo_entry_feature_itself;
}

/* cache, sort of */
static GtkCombo     *combo_feature_itself=NULL;
GtkCombo * combo_lookup()
{
  if(NULL==combo_feature_itself)
    combo_feature_itself=GTK_COMBO (lookup_widget(sp->im_widget[MAIN_WIN],
						  "combo_feature"));
  return combo_feature_itself;
}


static void _remove(GtkWidget *widget,
		    gpointer data)
{
  GtkContainer * con=data; 
  gtk_container_remove(con,widget);

}
void feature_init()
{
  GtkCombo *combo=combo_lookup();
  GList*  ch=gtk_container_get_children (GTK_CONTAINER (combo->list));
  if(0<g_list_length(ch)) {
    gtk_container_foreach(GTK_CONTAINER(combo->list),_remove,
			  GTK_CONTAINER(combo->list));
    g_critical("a. fix combo glade");
  }
  g_list_free(ch);
}


/* cache, sort of */
static GtkWidget     *dac_itself=NULL;
GtkWidget * dac_lookup()
{
  if(NULL==dac_itself)
    dac_itself=lookup_widget(sp->im_widget[MAIN_WIN],  "drawingarea_color");
  return dac_itself;
}

/********************************************/



void sensitivity()
{

  GtkWidget * w=sp->im_widget[MAIN_WIN];
  set_sensitive(lookup_widget(w,"delete_feature"),(feature_n>=0));
  set_sensitive(lookup_widget(w,"pack_feature"),(feature_n>=0));
  set_sensitive(lookup_widget(w,"unpack_feature"),(feature_n>=0));

  set_sensitive(lookup_widget(w,"rename_feature"),(feature_n<0));
}







gboolean
on_drawingarea_color_expose_event      (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
  GdkGC *gc;
  if(feature_n>=0)
    gc=features_gc[feature_n+2];
  else
    gc=widget->style->black_gc;
  return on_feat_drawinga_expo(widget,event,gc);;
}



/******* sets the current feature to number n ********/
void feat_entry_set(int n)
{
  gint position=0;
  gchar *t;
  GtkEditable *e=combo_entry_lookup(sp->im_widget[MAIN_WIN]);
  g_return_if_fail(e );
  g_return_if_fail(n>=0);
  while(n>=feature_len )
    if(!add_feature(NULL))
      break;
  if(n>=feature_len)
    t=_("(unavailable feature name)");
  else {
    t=feature_array[n];  
    feature_n=n;
  }
  sensitivity();
  gtk_editable_delete_text        (e,0,-1);
  gtk_editable_insert_text        (e,t,strlen(t),&position);
  MY_GTK_DRAW(dac_lookup());
}




int feat_string_find(gchar *t)
{
  int lp=0;
  for(;lp<feature_len;lp++)
    if(feature_array[lp] && 0==strcmp(t,feature_array[lp]))
      return lp;
  return -1;
}


int feat_entry_find(GtkWidget *w)
{
  GtkCombo *combo=combo_lookup(GTK_WIDGET(w));
  GtkEditable *entry=combo_entry_lookup(w);

  if(combo && entry)
    { 
      gchar*   t=gtk_editable_get_chars   (entry,
					   0,//      gint start_pos,
					   -1);//      gint end_pos);
      return  feat_string_find(t);
    }
  else
    {
      g_warning("can t find feature cokmbo entry or kombo");
      return -1;
    }
}

static
gboolean rename_feature(int i, GtkCombo *combo,gchar*   t)
{
  {
    int A= feat_string_find(t);
    
    if(A>=0) /*cant re-add the same or an already existing one!*/
      { gdk_beep();  return FALSE;}
  }
  if(feature_array[i]) {
    g_free(feature_array[i]); 
  }
  feature_array[i]=g_strdup(t);  
  {
    GList*  ch=gtk_container_get_children (GTK_CONTAINER (combo->list));
    GList *li=g_list_nth(ch,i);
    if(! li) return FALSE;
    {
      GtkWidget *item=li->data;
      GtkWidget *hbox=GTK_BIN(item)->child;
      GList*  chhbox=gtk_container_get_children (GTK_CONTAINER (hbox));
      GList *lili=g_list_nth(chhbox,1);
      if(!lili)return FALSE;
      {
	GtkLabel *label=lili->data;
	gtk_combo_set_item_string (combo, GTK_ITEM (item), t);
	gtk_label_set_text(label,t);
      }	  
      g_list_free(chhbox);
    }
    g_list_free(ch);
  }
  return TRUE;
}


void
on_combo_entry_feature_activate        (GtkEditable     *editable,
                                        gpointer         user_data)
{
  GtkCombo *combo=combo_lookup(GTK_WIDGET(editable));
  gchar*   t=gtk_editable_get_chars   (editable,
                                       0,//      gint start_pos,
                                       -1);//      gint end_pos);

  // WHY???
  //gtk_combo_disable_activate (combo);

  if(feature_last_n>=0 && feature_last_n<feature_len) {
    /*rename */
    if(!rename_feature(feature_last_n,combo,t))
      gdk_beep();
    else
      feature_n=feature_last_n;
  } else {
    /*add as new */
    if(!add_feature(t))
      gdk_beep();
    else
      feature_n=feature_last_n=feature_len-1;
  }
  sensitivity();
  MY_GTK_DRAW(dac_lookup());
  //gtk_widget_draw (sp->im_widget[MAIN_WIN] , NULL);
}

void
on_rename_feature_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{

  GtkCombo *combo=combo_lookup(GTK_WIDGET(button));
  GtkEditable *entry=combo_entry_lookup(GTK_WIDGET(button));

  g_assert(entry && combo);
  {
    gchar*   t=gtk_editable_get_chars   (entry,
					 0,//      gint start_pos,
					 -1);//      gint end_pos);
    if(feature_last_n>=0 && feature_last_n<feature_len) {
      if(!rename_feature(feature_last_n,combo,t))
	gdk_beep();
      else
	feature_n=feature_last_n;
    } else gdk_beep();
  }
  sensitivity();
  MY_GTK_DRAW(dac_lookup());
}




void
on_combo_entry_feature_changed         (GtkEditable     *editable,
                                        gpointer         user_data)
{
  //FIXME this assumes that there is only one
  combo_entry_feature_itself=editable;

  feature_n=feat_entry_find(GTK_WIDGET(editable));
  if(feature_n>=0) {
    feature_last_n=feature_n;
  }
  
  sensitivity();
  MY_GTK_DRAW(dac_lookup());

  //gtk_widget_draw(dac_lookup(GTK_WIDGET(editable)),NULL);
  //printf("CHANGED %d %d\n",feature_n,feature_last_n);
}


gboolean add_feature(gchar *t)
{
  GtkCombo *combo=combo_lookup(sp->im_widget[MAIN_WIN]);
  gchar *tc=NULL;
  if (feature_len>= features_max_n) 
      return FALSE;
  if(feature_len<0)
    feature_len=0;
  if(!t)
    tc=t=g_strdup_printf("%s %d",_("feature"),feature_len);
  feature_array[feature_len] = g_strdup(t);
  feat_widget_add(t,combo,features_gc[feature_len+2]);
  feature_len++;
  if(feature_n<0) 
    feat_entry_set(feature_len-1);
  //gtk_widget_draw (sp->im_widget[MAIN_WIN] , NULL);
  if(tc) g_free(tc);
  return TRUE;
}

void
on_new_feature_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  gchar*   t;
  //GtkCombo *combo=combo_lookup(GTK_WIDGET(button));
  GtkEditable *entry=combo_entry_lookup(GTK_WIDGET(button));
  if(!entry || feature_len<0 ||
     feat_entry_find(GTK_WIDGET(button))>=0)
    t=g_strdup_printf("%s %d",_("feature"),feature_len);
  else
    t=gtk_editable_get_chars   (entry,
				0,//      gint start_pos,
				-1);//      gint end_pos);

  if(!add_feature(t))
    gdk_beep();
  else
    {
      //gtk_widget_draw (sp->im_widget[MAIN_WIN] , NULL);
      feat_entry_set(feature_len-1);
    }
}



void
on_delete_feature_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkCombo *combo=combo_lookup(GTK_WIDGET(button));
  // WE JUST DELETE THE LAST ONE
  // OTHERWISE IT MESSES THE ORDER OF COLORS
  int N= feature_len-1;  //N=feature_n
  
  if(N>=0 &&feature_len>0 ) {
    //int lp=feature_len;
    feature_len--;
    if(feature_n>=feature_len)
      feature_n=feature_last_n=-1;

    //for(;lp<=feature_len;lp++)
    //  feature_array[lp]=      feature_array[lp+1];
    {
      GList*  ch=gtk_container_children (GTK_CONTAINER (combo->list));
      g_return_if_fail(ch );
      {
	GList* li=g_list_nth(ch,N);
	g_return_if_fail(li );
	gtk_container_remove (GTK_CONTAINER(combo->list),
			      li->data);
      }
      g_list_free(ch);
    }
    //gtk_widget_draw (sp->im_widget[MAIN_WIN] , NULL);
  }
  else { feature_n=feature_last_n=-1; gdk_beep(); }
}


void
on_pack_feature_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
  int lp=MAIN_WIN,xi,yi;
  if(feature_n<0) {gdk_beep();return;}
  for(; lp>=0; lp--) {
    if ((settings_get_value("mesh auto sync") || lp==MAIN_WIN)
	&& sp->im_widget[lp] != NULL) {
      MeshT *mesh=&sp->im_mesh[lp];	
      for(xi=0; xi < mesh->nx; xi++) {
	for(yi=0; yi<mesh->ny; yi++) {
	  if( meshGetLabel(mesh,xi,yi)==-1)
	    meshSetLabel(mesh, xi,yi, feature_n+1);
	}}
      //gtk_widget_draw (sp->im_widget[lp] , NULL);
    }
  } 
}

void
on_unpack_feature_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
  int lp=MAIN_WIN,xi,yi;
  if(feature_n<0) {gdk_beep();return;}
  for(; lp>=0; lp--) {
    if ((settings_get_value("mesh auto sync") || lp==MAIN_WIN)
	&& sp->im_widget[lp] != NULL) {
      MeshT *mesh=&sp->im_mesh[lp];	
      for(xi=0; xi < mesh->nx; xi++) {
	for(yi=0; yi<mesh->ny; yi++) {
	  if( meshGetLabel(mesh,xi,yi)==(feature_n+1))
	    meshSetLabel(mesh, xi,yi, -1);
	}}
      //gtk_widget_draw (sp->im_widget[lp] , NULL);
    }
  } 
}


void
on_color_feature_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
  if(feature_n>=0) {
    GtkWidget *csd=create_colorselectiondialog ();
    gtk_widget_show(csd);
    gtk_object_set_data(GTK_OBJECT (csd),
			"GC", &(features_gc[feature_n+2]));
  } else gdk_beep();
}


void
on_ok_button_color_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
  //GdkGC *gc=user_data;
  //GdkGC *gc=gtk_object_set_data_top (GTK_OBJECT (button), "GC");
  GtkColorSelectionDialog *csd=GTK_COLOR_SELECTION_DIALOG 
    (lookup_widget (GTK_WIDGET(button), 
		    "colorselectiondialog"));
  if(feature_n>=0) {
#if GTK_MAJOR_VERSION < 2
    //gdouble c[6];
    //gtk_color_selection_get_color   (csd->colorsel,c);
    show_warning(_("sorry this functions is as yet unimplemented"));
#else
    GdkColor color;
    gtk_color_selection_get_current_color(csd->colorsel, &color);
    if(gdk_colormap_alloc_color (gdk_colormap_get_system (),
				 & color,FALSE ,TRUE)) {
      gdk_gc_set_foreground ( features_gc[feature_n+2], &color); 
      MY_GTK_DRAW(dac_lookup());
    }
    GtkWidget *csd=lookup_widget(button,"colorselectiondialog");
    if(csd) gtk_widget_destroy(csd);
#endif
  } else gdk_beep();
}






void feature_save(FILE *f)
{
  int lp;
  for(lp=0;lp<feature_len;lp++) {
/*     GdkColor color; */
/*     gdk_gc_get_foreground ( features_gc[lp+2], &color);  */
/* THE ABOVE DOES NOT EXIST ALAS */
/*     fprintf(f, "<color>\n%d %d %d\n</color>\n" */
/* 	    color.red,color.green,color.blue) */
    fprintf(f,"<name>\n%s\n</name>\n",	  feature_array[lp]);
  }
}

static inline void de_n(char *s)
{ int l=strlen(s);  if(s[l-1]=='\n') s[l-1]=0; }

void feature_load(FILE *f)
{
  int lp;
  GtkCombo *combo=combo_lookup();
  gchar s[401]; s[0]=0;
  for(lp=0;;lp++) {
    if(feof(f)) break;
    fgets(s,400,f);  de_n(s);
    if(0==strcmp(s,"</features>")) 
      break;
    if(0!=strcmp(s,"<name>")) {
      g_warning("malformed session: '%s' is not an opening tag in features",s);
      break;
    }
    fgets(s,400,f);  de_n(s);
    if(lp<features_max_n) {
      if(lp<feature_len) {
	if(0!=strcmp(s,feature_array[lp])) {
	  if (!rename_feature(lp, combo,s))
	    g_warning(" cant set feature %d to '%s'",lp,s);
	}
      } else add_feature(s);
    }
    fgets(s,400,f);  de_n(s);
    if(0!=strcmp(s,"</name>")) {
      g_warning("malformed session: '%s' is not a closing tag for <name>",s);
      break;
    } 
  }
  if(lp>feature_len)   feature_len=MIN(features_max_n-1,lp);
  feat_entry_set(0);
}
