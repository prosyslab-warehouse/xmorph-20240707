#define GTKSETTINGS_BLOCKS_SIZE 5


gpointer
gtk_settings_alloc(/* template for the settings */
		   gpointer template[]);

/* returns: the menu widget for GUI to the settings */
GtkWidget*
gtk_settings_create (/* template for the settings */
		     gpointer template[],
		     /* actual array that will record the settings;
		      (note that the settings' names
		      are translated using gettext)
		     It must be preallocated by using the above routine */
		     gpointer *settings,
		     /* the callback that is called when any menu item is
			changed by the user*/
		     void (*callback)(GtkWidget* thismenuitem,
				      gpointer userdata),
		     gpointer userdata
		     );

/* there is also a callback that will be called (if non NULL)
   when the user changes a setting; it is of form 
   void callback();
   and is the first element of the template. 
*/


//static inline
int
gtk_settings_get_value(char *name,gpointer thesettings[]);

//static inline
int
gtk_settings_set_value(char *name,gpointer thesettings[], int value);



void
gtk_settings_set_sensitive(char *name,gpointer thesettings[], gboolean value);
