
/* load/save hooks  */ 

gboolean save_session(int lp,
		      const char *file);

gboolean load_session(int lp,
		      const char *file);

gboolean save_as_ppm(gchar *file,GdkPixbuf * pb);

gboolean save_image_to_file(int lp,
			    const char *file);

gboolean save_pixbuf_to_file(const char *file,
			     GdkPixbuf * pb);


static inline gchar *extension(const gchar *file )
{
  int len=strlen(file);
  len--;
  while(len>0) {
    if(file[len]=='.')      
      return ((gchar *)file)+len+1;
    len--;
  }
  return NULL;
}
static inline int cmp_extension(const gchar *file,
				const gchar *ext)
{
  int len=strlen(file), l=strlen(ext);
  if(len<l)
    return 2;
  if(file[len-l-1] != '.')
    return 3;
  return strcmp(file+len-l,ext);
}
