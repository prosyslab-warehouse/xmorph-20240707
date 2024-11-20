

#ifdef __cplusplus
extern "C" { 
#endif
  gboolean detect_translation(GdkPixbuf *src, double sx, double sy,
			  GdkPixbuf *dst, double dx, double dy,
			  // new suggested destination 
			  double *nx,double *ny);
#ifdef __cplusplus
}
#endif
