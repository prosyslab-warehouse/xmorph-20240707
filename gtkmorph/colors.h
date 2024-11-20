
/* color for selected points */
extern GdkGC   *mps_gc;
/* color for resulting points */
extern GdkGC   *mpr_gc;
/* color for mesh lines */
extern GdkGC   *mpl_gc;
extern int features_max_n;
extern GdkGC  *features_gc[];

void allocate_colors(GtkWidget * widget);
