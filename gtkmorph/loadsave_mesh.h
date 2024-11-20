

gboolean save_mesh_to_file(int lp, const char * file);

gboolean save_diff_mesh_to_file(int lp, const char * file);

gboolean load_diff_from_file(int lp, const char * file);

gboolean load_mesh_from_file(int lp, const char * file);


gboolean load_image_from_file(int lp,
			      const char *file);

void
reload_and_scale_image(int i);
