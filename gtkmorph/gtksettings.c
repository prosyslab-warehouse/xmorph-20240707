#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include "gtk-meta.h"
#include <string.h>

/*
 * Standard gettext macros.
 */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif


#include "gtktopdata.h"
#include "gtksettings.h"

/*header in the settings (the template has no header)*/
#define GTKSETTINGS_HEADERSIZE 2


static void call_callback(GtkMenuItem     *menuitem)
{
  void (*callback)(GtkMenuItem* thismenuitem, gpointer userdata);
  gpointer   *settings =
    gtk_widget_get_data_top(GTK_WIDGET(menuitem),"settings");
  gpointer   *userdata =
    gtk_widget_get_data_top(GTK_WIDGET(menuitem),"userdata");
  g_assert(settings);
  callback =gtk_widget_get_data_top(GTK_WIDGET(menuitem),"callback");
  g_assert(callback==settings[0]);
  g_assert(userdata==settings[1]);
  if(callback)
    (*callback)(menuitem, settings[1]);
}

static void
on_setting_activate    (GtkMenuItem     *menuitem,
			gpointer         thisitemsetting)
{  
  int oldval = GPOINTER_TO_INT( *((gpointer *)thisitemsetting+1) );

  gpointer p=gtk_object_get_data(GTK_OBJECT(menuitem),"supermenu"); 
  if (p) {    
    GSList *g=gtk_object_get_data(p,"group"); 
    while(g) {
      struct _GtkCheckMenuItem * z=g->data;
#if GTK_MAJOR_VERSION >= 2
      int a=gtk_check_menu_item_get_active(g->data);
#else
      int a=z->active;
#endif
      if(a )	{
	int newval=GPOINTER_TO_INT
	  (gtk_object_get_data(GTK_OBJECT(z),"setting_value"));
	*((gpointer *)thisitemsetting+1 ) =  GINT_TO_POINTER(newval);
	g_debug(" %s( '%s') from %d to %d '%s' ", __FUNCTION__,
		*(char **)thisitemsetting,oldval,newval,
		gtk_widget_get_name(g->data));      
	if( newval != oldval ) call_callback(p);
	break;
      }
      g=g_slist_next(g); 
    }
  } else {
    int newval = GTK_CHECK_MENU_ITEM(menuitem)->active;
    *((gpointer *)thisitemsetting+1 ) =  GINT_TO_POINTER(newval);

    g_debug(" %s( '%s') from %d to %d", __FUNCTION__,
	    *(char **)thisitemsetting,	    oldval, newval );
    
    /* now we dont deal with accels */
    g_assert(  *((gpointer *)thisitemsetting+2)==NULL );
    if( newval != oldval ) call_callback(menuitem);
  }
}

/* actually this is never called ... */
/* static void on_setting_toggle    (GtkMenuItem     *menuitem, */
/* 			gpointer         thisitemsetting) */
/* {  */
/*   g_debug(" toggle '%s' from %d to %d\n", */
/* 	    *(char **)thisitemsetting, */
/* 	    GPOINTER_TO_INT(*((gpointer *) */
/* 			      thisitemsetting+1 )), */
/* 	    GTK_CHECK_MENU_ITEM(menuitem)->active); */	 
/*   *( ((gpointer *)thisitemsetting) +1)= */
/*     GINT_TO_POINTER(GTK_CHECK_MENU_ITEM(menuitem)->active); */
/*   call_callback(menuitem); */
/* } */





//static inline
int
gtk_settings_get_value(char *name,gpointer thesettings[])
{
  int lp=GTKSETTINGS_HEADERSIZE;
  while( thesettings[lp]) { 
    if(strcmp(name,thesettings[lp])==0) {
      //GtkWidget  *item = thesettings[lp+4];

/*       if ( GTK_CHECK_MENU_ITEM(item)->active  */
/* 	   != GPOINTER_TO_INT(thesettings[lp+1])) */
/* 	g_warning(" the item %s has value %d but the menu has value %d!\n", */
/* 		  *(char **)(thesettings+lp), */
/* 		  GPOINTER_TO_INT(thesettings[lp+1]), */
/* 		  GTK_CHECK_MENU_ITEM(item)->active */
/* 		); */
      

      return GPOINTER_TO_INT(thesettings[lp+1]);

    }
    lp+=GTKSETTINGS_BLOCKS_SIZE;
  }
  g_warning("gtk_settings_get_value: setting '%s' not found", name);

  return 0;
}

void
gtk_settings_set_sensitive(char *name,gpointer thesettings[], gboolean value)
{
  int lp=GTKSETTINGS_HEADERSIZE;
  while( thesettings[lp]) { 
    if(strcmp(name,thesettings[lp])==0)
      {
	GtkObject  *item = thesettings[lp+4];
	gtk_widget_set_sensitive(item ,(value)?TRUE:FALSE);
	return ;
      }
    lp+=GTKSETTINGS_BLOCKS_SIZE;
  }
  g_warning("gtk_settings_set_sensitive: setting '%s' not found", name);
}
//static inline
gboolean
gtk_settings_set_value(char *name,gpointer thesettings[], int value)
{
  int lp=GTKSETTINGS_HEADERSIZE;
  while( thesettings[lp]) { 
    if(strcmp(name,thesettings[lp])==0)
      {
	GtkObject  *item = thesettings[lp+4];
	thesettings[lp+1] = GINT_TO_POINTER(value);
	/* FIXME is this relevant? 
	   if(!GTK_WIDGET_REALIZED(item))
	    g_warning(" item %s is not realized, while initializing to %d\n",
	              (char *)(thesettings[lp]),value);
	   else */
	{
	  gpointer p=gtk_object_get_data(item,"submenu"); 
	  if (p) {
	    GList* L= gtk_container_get_children(p);
	    int len=g_slist_length (L);
	    g_return_val_if_fail(value>=0 && value< len, FALSE);
	    GSList* nth=g_slist_nth(L,value);
	    GtkWidget* radio_menu_item= nth->data;
	    g_return_val_if_fail(radio_menu_item, FALSE);
	    gtk_check_menu_item_set_active
	      (GTK_CHECK_MENU_ITEM (radio_menu_item),TRUE);
	  } else	  
	    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item) ,
					   (value)?TRUE:FALSE);
	}
	return TRUE;
      }
    lp+=GTKSETTINGS_BLOCKS_SIZE;
  }
  g_warning("gtk_settings_set_value: setting '%s' not found", name);

  return FALSE;
}




int
gtk_settings_template_size(/* template for the settings */
		   gpointer template[])
{
  /*find size */
  int lp=0;
  while(template[lp]) { 
    lp+=GTKSETTINGS_BLOCKS_SIZE;
  }  
  /* includes the final NULL and a HACK (FIXME)*/
  return lp+2;
}

gpointer
gtk_settings_alloc(/* template for the settings */
		   gpointer template[])
{
  int lp, size;
  /*find size */
  lp=gtk_settings_template_size(template); 
  /* includes   the header*/
  size=lp+GTKSETTINGS_HEADERSIZE;
  return g_malloc(size * sizeof(gpointer));
}

GtkWidget*
gtk_settings_create(gpointer template[],
		    gpointer *thesettings,
		    void (*callback)(),
		    gpointer userdata)
{
  GtkWidget *menuSettings;
  //GtkAccelGroup *menuSettings_accels;
  GtkWidget *item;
  GtkTooltips *tooltips;
  int lp, size;

#if GTK_MAJOR_VERSION < 2
  //FIXME
  //  menuSettings_accels = gtk_menu_ensure_uline_accel_group     (GTK_MENU (menuSettings));
#endif


  g_assert(thesettings);

  /*find size */
  size= gtk_settings_template_size(template); 

  /* copy template */
  memcpy(thesettings+GTKSETTINGS_HEADERSIZE,template,
	 size*sizeof(gpointer) );
  /*translate*/
  lp=0;
  while(template[lp]) { 
    /* menu item IS NOT TRANSLATED here but below */
    /* thesettings[lp+GTKSETTINGS_HEADERSIZE]= (template[lp]); */
    /* tooltip */
    thesettings[lp+3+GTKSETTINGS_HEADERSIZE]= _(template[lp+3]);
    lp+=GTKSETTINGS_BLOCKS_SIZE;
  } 

  menuSettings = gtk_menu_new ();

  {
    //void (*callback)(GtkWidget* thismenu, gpointer userdata);
    //callback = thesettings[0];
    //g_assert(callback == callbac);
    thesettings[0]=callback;
    thesettings[1]=userdata;
    gtk_widget_set_data_top(menuSettings,"callback", callback);
    gtk_widget_set_data_top(menuSettings,"userdata", userdata);
    gtk_widget_set_data_top(menuSettings,"settings", thesettings);
  }

  tooltips = gtk_tooltips_new ();

  gtk_widget_set_name (menuSettings, "menuSettings");
  // I Think this is useless gtk_object_set_data (GTK_OBJECT (menuSettings),"menuSettings", menuSettings);

  lp=GTKSETTINGS_HEADERSIZE;
  while(thesettings[lp]) {
    char * name = ( char *)thesettings[lp],
      * tooltip = ( char *)thesettings[lp+3];
    if(template[lp+4-GTKSETTINGS_HEADERSIZE]) 
      thesettings[lp+4]= item = /* translate menu label */
	gtk_menu_item_new_with_label ( _(name) );
    else
      thesettings[lp+4]= item = /* translate menu label */
	gtk_check_menu_item_new_with_label ( _(name) );
    
    gtk_widget_set_name (item,name);
    // I Think this is useless gtk_widget_ref (item );
    gtk_widget_show (item);
    gtk_container_add (GTK_CONTAINER (menuSettings), item);
    if  ( tooltip[0] )
      gtk_tooltips_set_tip (tooltips, item, tooltip, NULL);
   
    /* this needs a submenu */
    if( template[lp+4-GTKSETTINGS_HEADERSIZE]) {
      GtkWidget *m=gtk_menu_new ();
      char **v=template[lp+4-GTKSETTINGS_HEADERSIZE];
      int p=0,len=0;
      GSList *group = NULL;
      gtk_widget_show(m);
      /* this creates a submenu */
      while (v[len]) len++;
      while (p < len) {
	GtkWidget *i; 
	i =gtk_radio_menu_item_new_with_label (group, _(v[p]) );
	gtk_widget_set_name(i,v[p]);
#if GTK_MAJOR_VERSION < 2
	group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (i));
#else
	group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (i));
#endif
	gtk_object_set_data(GTK_OBJECT(i),"setting_value",GINT_TO_POINTER(p));
	//USELESS ?? gtk_widget_ref (i);
	gtk_widget_show (i);
	if  ( tooltip[0] )
	  gtk_tooltips_set_tip (tooltips, i, tooltip, NULL);
	gtk_container_add (GTK_CONTAINER (m), i);
	gtk_object_set_data(GTK_OBJECT(i),"supermenu", item);     
	if (p == GPOINTER_TO_INT(thesettings[lp+1]))
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (i), TRUE);
	gtk_signal_connect (GTK_OBJECT (i), "activate",
			    GTK_SIGNAL_FUNC (on_setting_activate ),
			    thesettings+lp);
	p++;
      }
      gtk_menu_item_set_submenu (GTK_MENU_ITEM(item),m);
      gtk_object_set_data(GTK_OBJECT(item),"group", group);
      gtk_object_set_data(GTK_OBJECT(item),"submenu", m);    
    }
    else {
      /* this needs a check button */
      if(thesettings[lp+1])
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), TRUE);
/*       gtk_signal_connect (GTK_OBJECT (item), "toggle", */
/* 			  GTK_SIGNAL_FUNC (on_setting_toggle ), */
/* 			  thesettings+lp); */
      
      gtk_signal_connect (GTK_OBJECT (item), "activate",
			  GTK_SIGNAL_FUNC (on_setting_activate ),
			  thesettings+lp);
    }
    lp+=GTKSETTINGS_BLOCKS_SIZE;
  }
  
  return menuSettings;
}
