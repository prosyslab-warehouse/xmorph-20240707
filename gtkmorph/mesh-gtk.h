
enum tools {
  tool_edit,tool_move,tool_stretch,tool_select,tool_unselect,tool_assign,
  tool_error
};

/* stops all background processes that are smoothing the meshes */
gint smooth_idle_stop();
/* stops all background processes that are smoothing this mesh */
gboolean  smooth_idle_stop_by_mesh(MeshT *mesh);

gboolean
gdk_mesh_key_press_event         (GtkWidget       *widget,
				  GdkEventKey     *event,
				  gpointer         user_data,
				  MeshT *mesh ,
				  /* the mesh point affected */
				  int *mi, int *mj, int *mlabel,
				  enum tools  *action);


gboolean
gdk_mesh_button_press_event(GtkWidget       *widget,
			    GdkEventButton  *event,
			    gpointer         user_data,
			    MeshT *mesh,
			    /* the mesh point affected */
			    int *mi, int *mj, int *mlabel,
			    enum tools *action  //FIXME WHAT FOR?
			    /* what was done: 0==point moved
			       1== added hor line
			       2== added ver line
			    */
			    //MeshT meshes[] /* array , to replicate actions*/
			    , gboolean readonly );



gboolean
gdk_mesh_motion_notify_event(GtkWidget       *widget,
			     GdkEventMotion  *event,
			     gpointer         user_data,
			     MeshT *mesh,
			     int *mi, int *mj /*the mesh point affected */
			     //MeshT meshes[] 
			     /* array , to replicate actions*/
			     );




gboolean
gdk_mesh_button_release_event(GtkWidget       *widget,
			      GdkEventButton  *event,
			      gpointer         user_data,
			      MeshT *mesh,
			      int *mi, int *mj
			      /* the mesh point affected */ 
			      //MeshT meshes[] /*array , to replicate actions*/
			      );


gboolean
gdk_draw_mesh(GdkDrawable  *drawable,
	      gboolean draw_lines_p, 
	      GdkGC        *lines_gc, //lines GC
	      unsigned int draw_points_p, //also: how many points GC are passed
	      GdkGC        *points_gc[], //points GC
	      gboolean draw_features_p,
	      MeshT *mesh,
	      // GdkRectangle *subimage,
	      //int height, //viewport height
	      //int width, //viewport width
	      const double affine[6]
	      );

gboolean
gdk_draw_mesh_as_arrows(GdkDrawable  *drawable,
			GdkGC        *arrows_gc, 
			MeshT *mesh,
			MeshT *base_mesh,
			gboolean as_difference,
			const double affine[6]);
