
//draw event and expose event are different
#if GTK_MAJOR_VERSION < 2
#define __EXPOSE__(A)  (A)
#else
#define __RETURN__(A)  (A)
#endif
