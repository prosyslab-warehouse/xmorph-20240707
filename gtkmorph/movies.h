
#ifdef  __WIN32__
#define ANIMATE_INTERNALLY
#endif

#define ANIMATE_INTERNALLY

#ifdef  ANIMATE_INTERNALLY
int movie_pixmap_free();
#endif

int movie_init();
