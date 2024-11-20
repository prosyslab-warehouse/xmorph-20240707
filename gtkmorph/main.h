#ifndef _XMORPH_GTK_MAIN_H_
#define _XMORPH_GTK_MAIN_H_

#define MAX_WINS 105
#define MAIN_WIN (MAX_WINS+1)

extern GtkWidget *menuFile_g;
extern GtkWidget *menuEdit_g;
extern GtkWidget *menuMorph_g;
//extern GtkWidget *menuSettings_g;
extern GtkWidget *menuHelp_g;


extern GtkFileSelection *fileselection_g; 

extern int fileselection1_for_image_num_g;
extern GtkWidget *dialogwarning_g; 
/* the text of the warning */
/* extern char dialogwarning_text[]; */


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#if HAVE_LIBPLY_H
#include "libply.h" 
#endif

#ifdef IS_PLYMORPH
#include <gtk/gtkgl.h>
#include "libply.h"
#include "mytypes.h"
#include "topologic.h"
#include "eikonal.h"
#include "gl-utils.h"
#include "bind.h"
#include "settings_.h"
#else
#include "../libmorph/mesh.h"
#include "settings.h"
#endif

#include "gtk_subimagesel.h"

/********** the current status is
  in a "structure of arrays " (see main.h)

  why not an "array of structures"?
  because many subroutines work on the
  arrays, and they would become unnecessarily complicated

  on the other end, a  "array of structures"
  may be easily modified to add new images... this
  way, the program will have to be recompiled to add new images;
  but hopefully MAX_WINS may be set to a value that is sufficient 
  in 99.9% of cases

  */

typedef struct _morph_factors_t morph_factors_t;

struct  _morph_factors_t {
  double im_warp_factor[MAX_WINS+2];
  double im_dissolve_factor[MAX_WINS+2];
};

 
typedef struct _gtkmorph_status_t gtkmorph_status_t;

/** affine transforms **/
typedef struct affine_transforms_ affine_transforms;
struct affine_transforms_ {
  double loaded2subimage[6];
  double subimage2loaded[6];
};

struct  _gtkmorph_status_t {
 
  unsigned int max_wins;

  unsigned int resulting_width;
  unsigned int resulting_height;

  /* standard size of meshes; it is augmented when other meshes are loaded */
  unsigned int meshes_x;
  unsigned int meshes_y;

  /* what is in the spinbuttons */
  unsigned int resulting_width_sp;
  unsigned int resulting_height_sp;

  /* the filename of the loaded image (or PLY file) */
  char *im_filename_in[MAX_WINS+2];
  char *im_filename_out[MAX_WINS+2];

  char *im_mesh_filename[MAX_WINS+2];
  
#ifndef IS_PLYMORPH
  /* the size of the loaded image (and not of the subimage) */
  unsigned int im_width[MAX_WINS+2];
  unsigned int im_height[MAX_WINS+2];
  
  MeshT im_mesh[MAX_WINS+2];

  /* show mesh differences, if this is non null : */
  MeshT *im_mesh_diff[MAX_WINS+2];
  /* either the above mesh contains a real mesh, or a diff-mesh  */
  gboolean im_mesh_diff_is_difference_mesh[MAX_WINS+2];
#endif
  /* is this a usual window, or the simplified window associated to a difference mesh*/
  gboolean im_widget_is_difference_mesh[MAX_WINS+2];

  morph_factors_t mf;

  /* record the value of the "edit mesh/ show warp" choice
   */

  editview_t im_editview[MAX_WINS+2];

  /** this is the position of the 'eyes&mouth' : 3 points that we use
      to put all images in the same positions
  **/

  gtk_subimage_sel_t  subimasel[MAX_WINS+2];

  //private

  GtkWidget * im_widget[MAX_WINS+2];
  GtkWidget * im_drawingarea_widget[MAX_WINS+2];

  /* special window for warped images*/
  GtkWidget * im_warped_widget[MAX_WINS+2];

  /*** this records the settings for the image */
  gpointer * im_settings[MAX_WINS+2];
  GtkWidget * im_menu_settings[MAX_WINS+2];

  /* which image are we displaying? 
     0 loaded
     1 subimage
     2 warped
   */
  int which_pix[MAX_WINS+2];

  /* affine transformation */
  affine_transforms transforms[MAX_WINS+2];


  /* stores the loaded PLY surface */
#ifdef IS_PLYMORPH
  PlySurface im_ply_surface[MAX_WINS+2];
  PlySurface im_ply_surface_warped[MAX_WINS+2];
  labels_t *im_ply_labels[MAX_WINS+2];
  GArray *im_ply_labels_as_array[MAX_WINS+2];
  gboolean im_ply_labels_unsaved[MAX_WINS+2];
  /* rendering of the PLY
      has size    resulting_width × resulting_height
   */
  GdkPixbuf  *im_pixbuf[MAX_WINS+2]; 
  /* backing pixmap of the PLY
      has size    resulting_width × resulting_height
   */
  //  GdkPixmap * im_pixmap[MAX_WINS+2];

  /* warped flat out version of the image */
  GdkPixbuf * im_warped_pixbuf[MAX_WINS+2]; 
#else
  /* stores the loaded image, (and not not its subimage). */
  /*  To save memory, we should not use it , but reload the image when
   * a new subimage is requested ; see #ifdef RESCALE_RELOAD_LESS_MEM */
  GdkPixbuf  *im_loaded_pixbuf[MAX_WINS+2]; 
  /*     it would have size    im_width[lp]  ×  im_height[lp]  */

  /* and its rendered version */
  GdkPixmap  *im_loaded_pixmap[MAX_WINS+2]; 

  /* stores the zoomed subimage. 
      has size    resulting_width × resulting_height
   */
  GdkPixbuf  *im_subimage_pixbuf[MAX_WINS+2]; 
  /* backing pixmap of subimage image 
      has size    resulting_width × resulting_height
   */
  GdkPixmap * im_subimage_pixmap[MAX_WINS+2];

  /* stores the warped version 
      has size    resulting_width × resulting_height
  */
  GdkPixbuf * im_warped_pixbuf[MAX_WINS+2]; 

  /* backing pixmap, the warped version 
     has size    resulting_width × resulting_height
  */
  GdkPixmap * im_warped_pixmap[MAX_WINS+2];
#endif
}  ;



//struct gtkmorph_undo {
//  gtkmorph_status_t undos[1000];
//};

extern gtkmorph_status_t settings, *sp;

#endif // _XMORPH_GTK_MAIN_H_
