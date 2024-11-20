
#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 4
#define HAVE_GTK_COMBO
#endif

#if GTK_MAJOR_VERSION >= 2
#define HAVE_GDK_FORMATS
#endif

#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION < 4
typedef enum
{
  GTK_FILE_CHOOSER_ACTION_OPEN,
  GTK_FILE_CHOOSER_ACTION_SAVE,
  GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
  GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER
} GtkFileChooserAction;
#endif


#if GTK_MAJOR_VERSION >= 2 
#define  MY_GTK_DRAW(A)  gdk_window_invalidate_rect((A)->window,NULL,TRUE);
#else
#define  MY_GTK_DRAW(A)  gtk_widget_draw((A),NULL);
#endif


inline static void 
my_gtk_widget_update_rect (GtkWidget *widget, gdouble x, gdouble y, int span) 
{ 
  
  static GdkRectangle update_rect; 
  
  update_rect.x = (long)(x - span); 
  update_rect.y = (long)(y - span) ;
  update_rect.width = 2*span; 
  update_rect.height = 2*span;
#if GTK_MAJOR_VERSION >= 2 
  gdk_window_invalidate_rect((widget)->window,&update_rect,TRUE);
#else
  gtk_widget_draw (widget, &update_rect); 
#endif
} 

/****************   VARARG for  glib 1.2          ************/

#if GLIB_MAJOR_VERSION == 1
#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define G_HAVE_ISO_VARARGS 1
#elif defined (__GNUC__)
#define G_HAVE_GNUC_VARARGS 1
#endif
#endif
/* gcc-2.95.x supports both gnu style and ISO varargs, but if -ansi
 * is passed ISO vararg support is turned off, and there is no work
 * around to turn it on, so we unconditionally turn it off.
 */
#if __GNUC__ == 2 && __GNUC_MINOR__ == 95
#  undef G_HAVE_ISO_VARARGS
#endif


/****************     g_debug         ************/


#ifdef NDEBUG
//
#ifdef G_HAVE_ISO_VARARGS
#define g_debug(...)  
#elif defined(G_HAVE_GNUC_VARARGS)
#define g_debug(format...) 
#else   /* no varargs macros */
static void g_debug (const gchar *format,         ...){}
#endif
//
#else  //  NDEBUG
//
#ifdef G_HAVE_ISO_VARARGS
#define g_debug(...)    g_log (G_LOG_DOMAIN,         \
                               G_LOG_LEVEL_DEBUG,    \
                               __VA_ARGS__)
#elif defined(G_HAVE_GNUC_VARARGS)
#define g_debug(format...)      g_log (G_LOG_DOMAIN,         \
                                       G_LOG_LEVEL_DEBUG,    \
                                       format)
#else   /* no varargs macros */
static void
g_debug (const gchar *format,         ...)
{
  va_list args;
  va_start (args, format);
  g_logv (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, format, args);
  va_end (args);
}
#endif
//
#endif //  NDEBUG






#if GTK_MAJOR_VERSION == 1
#define my_GtkSpinButton GtkEditable
#else
#define my_GtkSpinButton GtkSpinButton
#endif
