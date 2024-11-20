


#ifdef __cplusplus
extern "C" {
#endif

  void wavelet_init();

  GNode *  wavelet_stats(GdkPixbuf *src);
  
  void wavelet_stats_sum(GNode *this, GNode *other,double factor);

  GNode * wavelet_stats_copy(GNode *this);

  GNode * wavelet_stats_clone(GNode *this);

  void wavelet_stats_free(GNode *this);

  void wavelet_equalize(GdkPixbuf *dpb,GNode *l2_stat);

#ifdef __cplusplus
}
#endif


  
