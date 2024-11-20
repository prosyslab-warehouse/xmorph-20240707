
#ifdef _
#error "Error in the code: this header unsets the macro _() , it must be included before support.h"
#endif
#define _(A) A
/* otherwise this string does not end up in the message catalog */
#define FEATURE_HELP _("\
'Features' are sets of grid points; they are represented\
 by the same color in the grid. They can be used to more\
 easily edit a complex grid: for example, when preparing\
 a grid on a face, each facial feature (eyes,\
 mouth..) can be outlined by using a proper set of points.\n\
Points can be added or subtracted from features using\
 the tools 'assign' and 'unselect'; whole features can be\
 moved all together, using the 'move' and 'stretch' tools;\
 the 'pack' button transforms all selected points into\
 a feature, and 'unpack' does the opposite.")
#define TIPS_HELP _("\
Tips:\n\
1) try to keep the mesh lines as linear as possible:\
 add new lines (with right mouse button) if this helps!\n\
2) to have a better morph, for each image, set the `morph factors' to\
 a maximum, hit `do warp' and try to adjust the mesh until this warp\
 looks right\n\
3) if you are fighting with small details, then you should increase\
 the resulting image size until you have fixed things. (Use the\
 the 'x2' button.)")
#undef _

