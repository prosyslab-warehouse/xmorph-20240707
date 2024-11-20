
void show_fs(//GtkWidget *widget,
	     char *title /* fileselection title */
#if GTK_MAJOR_VERSION >= 2 
	     ,	     GtkFileChooserAction action
#endif
	     );

typedef gboolean (*fileselection_hook_t)      (int lp,  const char *file);
extern fileselection_hook_t fileselection_hook;

int      type_by_extension    (const char *file);

void create_fs();
