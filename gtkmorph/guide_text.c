#include <gtk/gtk.h>
#include <string.h>

#include "guide.h"
#include "guide_text.h"

#include "support.h"


/* the first letter in the title is ';'  if it only applies to warp,
  and ':' for morph; otherwise it is '-' .
  Any other choice will print a warning
*/
char **pane_text=NULL;


void guide_init_text()
{ 
  char *p[] = {    
    _("-Introduction"),
    
    _("     Welcome to GtkMorph\n\n\
GtkMorph is a powerful program that permits you to warp or morph images.\
\n\n\
   Using this guide.\
\n\n\
Hit 'next' to skip a topic, or 'do it' to do it; when you have done a topic,\
 hit 'Guide' in the main window to come back to the guide.\n\
(Note that, when you hit 'do it', the guide will iconify and deiconify\
 other windows to highlight your task: with some dumb window managers,\
 this will not work ok).\n\
 If you are not satisfied by the way you have complied with the guide\
 requests, hit 'prev' and 'do it' to retry the task.\n\
As a first task, you may get acquainted with the main gtkmorph window:\
 if you want to give it a look, hit 'do it': the main window will pop up!\
 Note that, if you keep the mouse still on a menu voice or on a button\
 for a moment, a tip shows up. When you are done,\
 hit 'Guide' to come back to the guide.\n\
If you know the program main window, simply hit 'next'.\
\n\n\
If you are fed up with the guide, you may\
 stop it at any time by closing this window:\
 the normal gtkmorph program will pop up.\n\
\n\
If you want to test the program, you may want to load the example session:\
look into the 'Help' menu."),

    _("-Select subimage size"),

    _("Now you select the size that the output image(s) \
will have. To this end, in the lower part of the main window, there is a \
pair of spins; select your preferred size and hit 'apply'.\n\
After that, you will be brought back to the guide.\n\
You may change the size again by selecting 'resulting size...' \
in the Edit menu."),

    _("-Morph or warp"),

    _("You may `warp' or `morph'.\n\n\
'Warp' means 'to deform, contort, distort, wring, bend, or otherwise twist'.\n\
If you have one single input image and you want to make \
a distortion of it, then you want to warp it. In this case, hit 'next'\n\n\
'Morphing' images instead means 'blending together warped versions\
 of the images'. So\
 to morph you need two or more input images.\n\
In this case, press 'do it', and, in the File menu, choose 'add image'\
 as many times as\
 to reach the required number of input images,\
 then hit 'Guide' in the main window.\
\n\n\
Note that at startup the program is set for ``warping'':\
 there is only one input image."),

    _("-Load image(s)"),

    _("Now you need to load the input images.\n\
In each input image window, load the input image \
using the big 'load image' button."),

    _(";Load reference image"),

    _("If you wish, you may load an image in the main window; it may help you \
if you are warping images and you want to have a reference to use as a \
target. This is not really necessary, and you may do as well without.\n\n\
If in this reference image there are 3 important features, like the eyes \
and mouth in face, then you should drag the 3 white points on these \
features.\n\n\
Similarly, if you have a reference mesh that is associated to this image, \
you may load it now, using the 'load mesh' button\n\n\
When done, click 'Guide'."),

    _("-Select subimage(s)"),

    _("Now you select a subimage of each input image. You do this by either\
 moving the selection rectangle (using the spins at bottom of the\
 window) or by dragging the 3 white feature points. (see [1] below)\
\n\n\
When you have chosen the preferred subimage, hit 'apply'\
 (and the window will close).\
 If later you find out that are not satisfied of your choice, you may\
 click in the option menu (at center top) and select 'choose subimage' to\
 repeat this process.\
\n\n\
Alternatively,\
 if you have already a mesh for this image, you may load it now; since the\
 subimage selection is saved with the mesh; when you load the saved mesh,\
 the subimage is reselected for you.\
\n\n\
 [1] Note that you cannot position the 3 points freely: their mutual\
 position is copied from the position of the 3 reference points in the\
 resulting image; so you will need to be patient.\n\
Note also that the selection rectangle has currently a fixed aspect\
 (see [2]); if you want to freely change the aspect, change the\
 'preserve aspect ratio' setting in the 'settings'.\n\
 [2] 'aspect' is the ratio between width and height."),
    
    _("-Adjust mesh"), 
    g_strdup_printf("%s\n\n%s\n\n%s\n\n%s",
		    _("\
Now you adjust the mesh in the input image(s) so that they best fit\
 to the features of those images. If you hit the right mouse key while on\
 the mesh, you will get a menu.\n\
It is very important that points in different meshes are syncronized, that is,\
 that point (i,j) in every mesh is associated to the same type of feature;\
 for this reason, when you drag a point in a mesh, the same point flashes in\
 all other meshes.\n\
When you have put points on a feature (say, for example, the right eye),\
 you may hit 'pack' to pack all this point into a 'feature'."),
		    _(FEATURE_HELP),
		    _("From time to time, you better save the mesh(es)."),
		    _(TIPS_HELP)),
    _(";Warp"),
    g_strdup_printf("%s\n\n%s",
		    _("\
Hit the 'do warp' button to view the warp. If you are not satisfied, \
choose 'edit mesh' in the option menu (top center) and edit the mesh further.\
From time to time, you better save the mesh."),
		    _(TIPS_HELP)),
    _(":Morph"),
    _("To morph your images, you must decide how much each image should\
 influence the resulting morph. To this end, you adjust the morph factors\
 to your need; the 'mesh interpolation' slide tells how much this mesh\
 will influence the resulting mesh, and the 'image blending' tells how much\
 this image is visible in the resulting image.\n\n\
Then hit the 'do morph' button in the main window to view the morph.\n\
 If you are not satisfied,\
 choose 'edit mesh' in the option menu (top center) and edit the mesh further;\
 or, choose 'view warp&mesh' and edit the morph factors.\
\n\n\
If you want to change the morph factors further, choose ``morph factors..''\
 in the 'edit' menu\
\n\n\
Tip: set all factors to equal values and\
 do a morph: if the images do not superimpose well, you may try to select\
 a subimage in each image so that they superimpose better"),
    
    _(":Movie"),
    _("To make a movie of your morph, choose 'morph sequence' in the Morph menu.\
 A window will pop up. Click on help there for further help."),
    /* END OF LIST */
    _("-End"),
    _("Here ends the guide.\n\nGoodbye and thank you"),
    NULL,
    NULL
  };
  pane_text=g_malloc(sizeof(p));
  memcpy(pane_text,p,sizeof(p));
}
