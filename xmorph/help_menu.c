/* help_menu.c : help menu widgets and callbacks
//
// Written and Copyright (C) 1994-2000 by Michael J. Gourlay
//
// PROVIDED AS IS.  NO WARRANTEES, EXPRESS OR IMPLIED.
//
// These routines are used for the X Window System only and are not
// needed for tkmorph.
*/

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>

#include <stdio.h>
#include <stdlib.h>

#include "xmorph.h"
#include "mjg_dialog.h"
#include "help_menu.h"




/* help strings:
// Make sure each line ends with a backslash-n-backslash.
// Be sure that there are no spaces at the end of lines after the last
// backslashes.
*/




const char *about_s=
"\n\
                            About XMorph:\n\
\n\
XMorph is a digital image warp program and graphical user interface\n\
written and copyright (C) 1994-2000 by Michael J. Gourlay,\n\
gourlay@colorado-research.com.\n\
\n\
XMorph performs morphological changes to images based on an algorithm\n\
described in Douglas B. Smythe's article ``A Two-Pass Mesh Warping\n\
Algorithm for Object Transformation and Image Interpolation'', ILM\n\
Technical Memo #1030, Computer Graphics Department, Lucasfilm Ltd.,\n\
1990.\n\
\n\
\n\
The basic method of the program goes like this:\n\
\n\
  First, the user loads in a pair of images, one ``source'' image,\n\
and one ``destination'' image.  These are the start and finish images.\n\
\n\
  Next, the user uses XMorph to shape the ``source'' and ``destination''\n\
meshes over the respective images.  This is done by picking meshpoints\n\
and moving them, with the mouse.\n\
\n\
  After the meshes have been defined (and saved to files) the user\n\
can warp the images.  Images can be either warped manually, if\n\
single images are desired, or an entire sequence of images can be\n\
generated automatically, for an animated sequence.  These images can\n\
be saved to files for later use.\n\
\n\
\n\
Please read the other help pages for details on how this process can\n\
be carried out.\n\
\n\
Xmorph is part of GNU software.\n\
\n\
-----------------------------------------------------\n\
\n\
                       DIW Image Panel images:\n\
\n\
The digital image warp (DIW) panels are the main features of XMorph.\n\
They are the boxes with the images and the meshes.  The meshes are\n\
drawn overtop the images.  Notice that the images in the DIW image\n\
panels are somewhat ``grainy''.  This is a side-effect of the way they\n\
are displayed, which dithers the images using only 64 colors, even\n\
though the images are full-color images (possibly with thousands of\n\
colors).  This graininess is a necessary evil to have XMorph not take\n\
control over a large number of the colors available on a typical\n\
display.  Don't worry though-- even though the images look grainy in\n\
XMorph's DIW image panels, they images warped and saved to (or loaded\n\
from) disk files are in full color, and do not retain the graininess\n\
shown in the DIW image panels.\n\
\n\
";




/* =============================================================== */

/*
// The following code was contributed by PW:
//
// This is a kludge to work with Apollo's compiler, which does not accept
// quoted strings longer than 4095 bytes.  The idea is to create two macro
// definitions, and then insert code to sprintf them into the mmp_s
// definition, before the string is installed as a menu.  For non-Apollo
// compilers, the definition at the end will use the ANSI string
// concatenation feature to give the same result as the older code.
*/

#define MMP_S_STRING_1 \
"\n\
\n\
\n\
                      Manipulating the meshes:\n\
\n\
\n\
There are two meshes concurrently active that the user can manipulate:\n\
the ``source'' mesh and the ``destination'' mesh.  Each is a grid of\n\
mesh points and mesh lines, which are usualy superimposed over the\n\
``source'' and ``destination'' images.  The ``source'' mesh is colored\n\
green, and the ``destination'' mesh red.\n\
\n\
There is a third mesh, called the ``tween'' mesh, which is colored\n\
yellow, but it depends on the ``source'' and ``destination'' meshes.\n\
Specifically, the ``tween'' mesh is the mesh part-way between the\n\
``source'' and ``destination'' meshes.  When you first run XMorph, the\n\
``tween'' mesh will be exactly the same as the ``source'' mesh in the\n\
first DIW image panel, and exactly equal to the ``destination'' mesh in\n\
the second DIW image panel.  You can use the slide-bar below each image\n\
panel to determine what shape the ``tween'' mesh takes-- it is always\n\
somewhere between the ``source'' and ``destination'' meshes.  The\n\
purpose of the ``tween'' mesh is to indicate how much digital image\n\
warping is to be applied to each image when the images are warped.\n\
See the information about ``Warping'' for more details.\n\
\n\
Note that the meshes are drawn with splines connecting the mesh\n\
points, instead of straight line segments.  The kind of spline used\n\
was designed (by me :) to maintain smoothness, yet prevent splines\n\
from overshooting the mesh points.  The same spline used to draw the\n\
mesh points is used in the actual digital image warp algorithm.\n\
\n\
-----------------------------------------------------\n\
\n\
                      Moving mesh points:\n\
\n\
Each mesh can be manipulated by grabbing a mesh point, dragging it to\n\
the desired location, and dropping it.\n\
\n\
To grab a mesh point, move the mouse pointer over-top the mesh point,\n\
and press and hold the left mouse button (button 1).  The meshes will\n\
redraw themselves, giving the selected meshpoint a highlight in both\n\
DIW image panels.  Also, the cursor will change shape and color to\n\
indicate that the mesh point is being dragged.  The color of the\n\
cursor indicates which mesh is being manipulated.  If the cursor is\n\
green, then a mesh point from the ``source'' mesh is being dragged.\n\
If the cursor is red, then a mesh point from the ``destination'' mesh\n\
is being dragged.  The way XMorph decides which mesh to choose from\n\
depends on the setting of the mesh slide-bar below each DIW image panel.\n\
If the slide-bar is closer to the ``source mesh'' side then XMorph will\n\
grab ``source'' mesh points.  If the slide-bar is closer to the\n\
``destination mesh'' side then XMorph will grab ``destination'' mesh\n\
points.  XMorph will never choose mesh points from the ``tween'' mesh.\n\
Note that XMorph will grab points from a mesh regardless of whether that\n\
mesh is currently visible.\n\
\n\
To drag a mesh point that has been grabbed, simply move the mouse.\n\
The mesh will not change shape immediately, since on most computers,\n\
the computation required to constantly update the image makes\n\
redrawing that often too slow.  You can drag the mesh point anywhere\n\
within the outer boundaries of the mesh (i.e. the edges of the image),\n\
but the digital image warping algorithm requires that the mesh does\n\
not overlap itself.  That is, vertical mesh lines must not cross\n\
over other vertical mesh lines, and horizontal mesh lines must not\n\
cross over other horizontal mesh lines.  However, since it is usually\n\
much easier to be allowed to violate this rule, at least temporarily\n\
while creating the mesh, this rule is not enforced in XMorph.  It is up\n\
to the user to keep the meshes valid.  A valid mesh is called\n\
``functional'' since each line can be represented with a mathematical\n\
function in Cartesian coordinates.\n\
\n\
To drop a mesh point, release the mouse button.  The meshes will\n\
be redrawn, and the cursor will return to the ``mesh point selection''\n\
cursor.\n\
\n"




#define MMP_S_STRING_2 "\
=====================================================\n\
\n\
                      Changing the number of mesh lines:\n\
\n\
Often, the number of mesh lines established by XMorph is not the right\n\
number.  It is usually close, though, and they start out at convenient\n\
locations most of the time.  The best approach to creating a mesh is\n\
this:\n\
\n\
First, delete any obviously useless or extraneous mesh lines.  Then\n\
save both meshes to files.\n\
\n\
Next, move the mesh points around from where they started to the\n\
places most near them where mesh points are desired.  Mesh lines\n\
should not deviate too far from being straight, unless you really know\n\
what you're doing.  A good way to imaging how a warp will look (other\n\
than simply trying it!) is to look the shape of the quarangles the\n\
meshpoints make up.  Each quadrangle in the ``source'' image maps to\n\
an analogous quadrangle in the ``destination'' image.  Work on one\n\
mesh line at a time, and save both meshes to files often.\n\
\n\
Next, if you decide that you need more mesh lines, add them one at a\n\
time.  Save both meshes to files after adding the mesh line.  Each\n\
time you add a mesh line, move all of its mesh points to where you\n\
want them, on both the ``source'' and ``destination'' meshes.  After\n\
they're in position, save the meshes to files.\n\
\n\
-----------------------------------------------------\n\
\n\
                      Adding mesh lines:\n\
\n\
To add a mesh line, move the mouse pointer to a place on the image\n\
where the mesh line should pass through.  Then, to add a vertical mesh\n\
line, press the middle mouse button (button 2).  To add a horizontal\n\
mesh line, press the right mouse button (button 3).  The mesh line\n\
will be added to both the ``source'' and ``destination'' meshes.  Be\n\
sure to save both meshes immediately.  If you save only one mesh and\n\
not the other, the two meshes will not be allowed to be loaded back\n\
in, since their geometries will not match.\n\
\n\
-----------------------------------------------------\n\
\n\
                      Deleting mesh lines:\n\
\n\
To delete a mesh line, move the mouse pointer to a mesh point on the\n\
line to be deleted.  Now, press and hold the ``Shift'' button on the\n\
keyboard.  Click on the middle mouse button (button 2) to delete a\n\
vertical mesh line, or click on the right mouse button (button 3) to\n\
delete a horizontal mesh line.  Save both meshes to files immediately.\n\
\n\
"




/* apollo: The Apollo Domain/OS BSD C compiler does not allow long
// constant strings to be used as initializers so the long strings have to
// be broken up and then concatenated at run-time.  The HPUX 7.0
// compiler used to have the same sort of problem.
//
// This code was contributed by PW.
*/
#ifdef apollo
char mmp_s [sizeof(MMP_S_STRING_1) + sizeof(MMP_S_STRING_2) - 1];
#else
const char *mmp_s = MMP_S_STRING_1 MMP_S_STRING_2 ;
#endif




const char *file_help_s =
"\n\
\n\
\n\
                      File menu button descriptions:\n\
\n\
\n\
Note that many functions apply to either ``source'' or ``destination''.\n\
Digital image warping is generally applied to ``morph'' one image into\n\
another.  That is, one image is reshaped to fit into the shape of the\n\
other.  The first image is called the ``source'' image and the second\n\
image is called the ``destination'' image.  The meshes which define the\n\
shaped of these images are also called the ``source'' and ``destination''\n\
meshes.\n\
\n\
-----------------------------------------------------\n\
\n\
Open source image...\n\
Open destination image...\n\
\n\
  Open and load a Targa (TGA) image from a file into memory to be\n\
  used as the current image.  A dialog box pops up to prompt the user\n\
  for the filename.\n\
\n\
If the loaded image has a different geometry than the previous image,\n\
XMorph will resize or add scrollbars in the DIW image panel.\n\
\n\
NOTE:  XMorph will not allow images of different geometries to be loaded\n\
at the same time.  This is because the process of warping between\n\
images of different size is ambiguous.  If an image is loaded by XMorph,\n\
and the alternate image does not have the same geometry as the newly\n\
loaded image, the alternate image is replaced with an image that XMorph\n\
generates, which will have the geometry of the newly loaded image.\n\
\n\
WARNING:  If the loaded image has a different geometry than the previous\n\
image, then the mesh geometries will also not match.  Because of this,\n\
XMorph will reset the mesh to fit the new image geometry.  Make sure you\n\
have saved the mesh before loading in a new image, if the mesh is\n\
important to you.\n\
\n\
\n\
-----------------------------------------------------\n\
\n\
Open source mesh...\n\
Open destination mesh...\n\
\n\
  Open and load a mesh from a file into memory to be used as the mesh.\n\
  A dialog box pops up to prompt the user for the filename.  The\n\
  source mesh is the mesh that establishes the starting points for the\n\
  digital image warp.  The source mesh is represented by green dots\n\
  and lines overtop the images in the DIW image panels.  The destination\n\
  mesh is the mesh that establishes the finishing points for the digital\n\
  image warp.  The destination mesh is represented by red dots and\n\
  lines overtop the images in the DIW image panels.\n\
\n\
WARNING:  If the loaded mesh has a different geometry or a different\n\
number of lines than the alternate mesh, then the meshes will not\n\
match.  Because of this, XMorph will reset the alternate mesh to fit the\n\
newly loaded mesh geometry.  Make sure you have saved the alternate\n\
mesh before loading in a new mesh, if the alternate mesh is important\n\
to you.\n\
\n\
-----------------------------------------------------\n\
\n\
Save source mesh...\n\
Save destination mesh...\n\
\n\
  Save the current mesh to a file.  A dialog box pops up to\n\
  prompt the user for the filename.  The mesh file format is an all\n\
  text format, and can be viewed and modified with a text editor.\n\
\n\
-----------------------------------------------------\n\
\n\
Quit\n\
\n\
  Exit the program.\n\
\n\
";




const char *mesh_help_s =
"\n\
\n\
\n\
                      Mesh menu button descriptions:\n\
\n\
\n\
Note that many functions apply to either ``source'' or ``destination''.\n\
Digital image warping is generally applied to ``morph'' one image into\n\
another.  That is, one image is reshaped to fit into the shape of the\n\
other.  The first image is called the ``source'' image and the second\n\
image is called the ``destination'' image.  The meshes which define the\n\
shaped of these images are also called the ``source'' and ``destination''\n\
meshes.\n\
\n\
-----------------------------------------------------\n\
\n\
\n\
Reset source mesh:\n\
Reset destination mesh:\n\
\n\
  Reset the mesh to a regularly spaced rectangular grid.\n\
\n\
-----------------------------------------------------\n\
\n\
Functionalize source mesh:\n\
Functionalize destination mesh:\n\
\n\
  Force the mesh to have no horizontal or vertical overlaps.  This\n\
  is a requirement for the digital image warp algorithms to perform\n\
  correctly.  The user should not create meshes that have vertical\n\
  lines crossing other vertical lines, or horizontal lines that cross\n\
  other horizontal lines.  This button makes an attemp to correct any\n\
  such discrepencies in the currently loaded meshes.\n\
\n\
";




const char *morph_help_s =
"\n"
"\n"
"\n"
"                      Morph Sequence menu button descriptions:\n"
"\n"
"\n"
"\n"
"The ``Morph Sequence'' functions are used to generate a sequence of\n"
"warped and dissolved images automatically, which can be placed on\n"
"video or film to make an animated morph sequence.\n"
"\n"
"Morphing involves two separate image manipulations:  First, the source\n"
"and destination (i.e. start and finish) images are warped to a common\n"
"shape.  Then, these warped images are dissolved (i.e. faded) into each\n"
"other.  The combination of these image effects, if done with the\n"
"proper timing, creates the illusion that one object becomes another.\n"
"\n"
"\n"
"-----------------------------------------------------\n"
"\n"
"Set sequence file name...\n"
"\n"
"  Set the base file name for the warp sequence.  A dialog box pops up\n"
"  to prompt the user for this file name.  This file name is the prefix\n"
"  for each of the images in the sequence.  Each sequential image is\n"
"  given a number, starting at 0000 and ending at the number of images\n"
"  in the sequence, minus one (since the numbering starts at zero).\n"
"\n"
"  The images are stored in the TrueVision Targa (TGA) image file\n"
"  format.\n"
"\n"
"  Xmorph only reads and writes images with the TARGA Truevision\n"
"  image file format.  This image file format does not have a magic\n"
"  number at the beginning, so many image manipulation programs find it\n"
"  difficult to identify these image files unless they have a filename\n"
"  suffix of '.tga'.  You should therefore always make sure the image\n"
"  files written by Xmorph have '.tga' at the end of the filename.\n"
"  When writing an image sequence, Xmorph automatically appends the\n"
"  '.tga' suffix so you do not have have to do this yourself, but\n"
"  this filename requirement is a good thing to keep in mind.\n"
"\n"
"Set sequence number of steps...\n"
"\n"
"  Set the total number of images in the warp sequence.  If these\n"
"  images are to be placed onto video, remember tha NTSC uses 30\n"
"  frames, or 60 fields, per second.  Standard motion film uses 24\n"
"  frames per second.  A typical cartoon on film used 12 distinct\n"
"  images per second.  A good looking, convincing morph usually takes\n"
"  place within the duration of one second or less, depending on the\n"
"  coherency of the start and finish images.\n"
"\n"
"Warp sequence\n"
"\n"
"  Begin the automatic warp sequence.  Once this has begun, it can not\n"
"  be interrupted without exitting the program, so first make sure all\n"
"  of your parameters are set correctly, and be sure to do some manual\n"
"  test warps at the extremes and middle of the warp, in the image\n"
"  panels.\n"
"\n"
"  Each image in the sequence is saved to a file, as described above.\n"
"\n"
"  The first DIW image panel shows the progress of the warp sequence.\n"
"  The DIW image panel itself shows the current image, both dissolved\n"
"  and warped.  The scrollbars below the image show the relative\n"
"  progress of the warping and dissolve.  The warp and dissolve are\n"
"  done at different rates since morphing tends to look better if the\n"
"  dissolve happens less rapidly at the start and finish of the warp\n"
"  sequence.  The warping is done linearly with time, and the dissolve\n"
"  is done sigmoidally with time.\n"
"\n";




const char *dpm_help_s =
"\n\
\n\
\n\
          DIW Properties menu button descriptions:\n\
\n\
\n\
A digital image warp (DIW) panel is an area that contains an image,\n\
meshes, and the associated graphical user interface widgets that\n\
control the image and meshes.\n\
\n\
The DIW panel has a set of properties that control the way it looks.\n\
These properties are independant for each DIW panel, which is why each\n\
DIW panel has its own properties menu.\n\
\n\
The meshes are drawn so that you can see where they are with respect\n\
to the images.  However, when grabbing and dragging mesh points, keep\n\
in mind that which mesh is being manipulated does not depend on which\n\
meshes are visible.  That means that it is possible to modify a mesh\n\
without being able to see it.  The reason for this possible source of\n\
confusion is that both source and destination meshes can be drawn\n\
overtop an image at the same time (which is a useful thing to be able\n\
to to) but when both are drawn (or neither), the choice of which mesh\n\
to manipulate should not depend on which is visible since that would\n\
be ambiguous.  See the help pages on ``Manipulating the mesh'' for\n\
more information about this.\n\
\n\
-----------------------------------------------------\n\
\n\
Show source mesh points:\n\
Show source mesh lines:\n\
Show destination mesh points:\n\
Show destination mesh lines:\n\
Show tween mesh points:\n\
Show tween mesh lines:\n\
\n\
  These are toggle buttons which control which meshes and mesh points\n\
  are drawn.  By default, when XMorph starts, the first DIW panel draws\n\
  ``source'' mesh lines and points, the ``tween'' mesh lines;  The\n\
  second DIW panel draws ``destination'' mesh lines and points, and\n\
  the ``tween'' mesh lines.  The ``source'' mesh is green, the\n\
  ``destination'' mesh is red, and the ``tween'' mesh is yellow.\n\
\n\
\n\
-----------------------------------------------------\n\
\n\
Dim image:\n\
\n\
  Toggle button to dim the image.  Often, having a mesh drawn overtop\n\
  an image obscures the visibility of the mesh.  Since at this stage,\n\
  the mesh is more important than the image, it is often useful to dim\n\
  the image so that the mesh is more clearly visible.  The setting of\n\
  this toggle does not effect the image itself, only the way it is\n\
  drawn in the DIW panel (i.e. when you save the image, it is saved at\n\
  its normal brightness, not dimmed).\n\
\n\
";




const char *dcm_help_s =
"\n"
"\n"
"\n"
"          DIW Commands menu button descriptions:\n"
"\n"
"\n"
"A digital image warp (DIW) panel is an area that contains an image,\n"
"meshes, and the associated graphical user interface widgets that\n"
"control the image and meshes.\n"
"\n"
"Each DIW panel image can be somewhat manipulated independantly of the\n"
"images in another DIW panel.  For this reason, each DIW panel has its\n"
"own Commands menu.\n"
"\n"
"-----------------------------------------------------\n"
"\n"
"Save Image...\n"
"\n"
"  Save the image currently displayed in the DIW panel.\n"
"  A dialog box will pop up to prompt you for a file name\n"
"  under which to save the image.  The image saved is dissovled as much\n"
"  as is shown in the DIW image panel, and warped as much as the most\n"
"  recent warp applied in that image panel.  The ``Dim image'' setting\n"
"  has no effect on the image being saved.  Also, the dithering done on\n"
"  the image is not saved-- that is done simply to display the image,\n"
"  so the saved image will not look grainy, as it appears in the DIW\n"
"  image panel.\n"
"\n"
"  Xmorph only reads and writes images with the TARGA Truevision\n"
"  image file format.  This image file format does not have a magic\n"
"  number at the beginning, so many image manipulation programs find it\n"
"  difficult to identify these image files unless they have a filename\n"
"  suffix of '.tga'.  You should therefore always make sure the image\n"
"  files written by Xmorph have '.tga' at the end of the filename.\n"
"\n"
"Warp Image\n"
"\n"
"  Apply the digital image warp algorithm to the images in the DIW\n"
"  panel.  The ``source'' and ``destination'' images are warped\n"
"  separately.  The amount (and direction) of warping applied depends\n"
"  on the setting of the ``mesh'' slide-bar below the DIW panel, which\n"
"  controls the ``tween'' mesh for that DIW panel.  The ``source''\n"
"  image image is warped forward from the ``source'' mesh to the\n"
"  ``tween'' mesh.  The ``destination'' image is warped backwards from\n"
"  the ``destination'' mesh to the ``tween'' mesh.  After the warping\n"
"  algorithm has been applied, the warped image remain warped at that\n"
"  point until the next time the warping algorithm is applied.  Note\n"
"  that this implies that the images can be dissolved independantly of\n"
"  the warping.  That means that after you apply the warp, you can then\n"
"  diddle with the dissolve slide-bar until you get a composed image\n"
"  that looks the way you want it to look.  In general, the best\n"
"  looking morphed image has different setting for the warp and\n"
"  dissolve parameters.  In general, a morph looks better if the\n"
"  warping is applied more than the dissolve near the start and finish\n"
"  of the warp.  That is, while the images are warped closer to the\n"
"  shape of the ``source'' image, the amount of dissolve should\n"
"  strongly prefer the ``source'' image, and vice versa.\n"
"\n"
"  While the digital image warp algorithm is working, the cursor in the\n"
"  DIW panel changes to indicate the level of progress.  The shape of\n"
"  the cursor is a watch, to indicate that XMorph is busy.  The color of\n"
"  the cursor indicates which channel of which image is being warped.\n"
"  If the watch has a white background, the the ``source'' image is\n"
"  being warped.  A black background indicated the ``destination''\n"
"  image is being warped.  A red watch means that the red channel is\n"
"  being warped, a green watch for the green channel, and a blue watch\n"
"  for the blue channel.\n"
"\n"
"Redither Image\n"
"\n"
"  This button activates the dissolve, but it is redundant, since the\n"
"  dissolve is automatically activated whenever the dissolve slide-bar\n"
"  is moved.  This button is present so that slow computers can\n"
"  deactivate the slide-bar's automatic dissolve behaviour, which can\n"
"  be time-consuming sometimes.\n"
"\n"
"  While the image is being redithered, the cursor turns into a white\n"
"  watch to indicate that XMorph is busy.\n"
"\n";




/* create_help_menu: Create a help menu and its button
// returns the widget of the menu button
*/
Widget
create_help_menu(Widget parent, Widget toplevel, Widget left_w)
{
  Widget help_menu_button;
  Widget help_menu;
  Widget   hm_about_sme;   /* about xmorph */

  Widget   hm_mmp_sme;     /* moving mesh points */

  Widget   hm_file_sme;    /* file help */
  Widget   hm_mesh_sme;    /* mesh help */
  Widget   hm_warp_sme;    /* warp help */

  Widget   hm_dpm_sme;     /* diw properties menu help */
  Widget   hm_dcm_sme;     /* diw commands menu help */

  Widget   hm_quit_sme;    /* quit */

  dialog_apdx_t *hm_about_daP;
  dialog_apdx_t *hm_mmp_daP;
  dialog_apdx_t *hm_file_daP;
  dialog_apdx_t *hm_mesh_daP;
  dialog_apdx_t *hm_warp_daP;
  dialog_apdx_t *hm_dpm_daP;
  dialog_apdx_t *hm_dcm_daP;

  /* ========================= */
  /* Create the Dialog widgets */
  /*  - - - - - - - - - - - - - - - - -  */
  /* Create the "about dialog */
  hm_about_daP = create_mjg_dialog(toplevel, "h_about");
  hm_about_daP->client_data = NULL;
  hm_about_daP->callback = NULL;
  XtVaSetValues(hm_about_daP->dialog, XtNvalue, about_s, NULL);
  XtVaSetValues(hm_about_daP->text,XtNscrollVertical,XawtextScrollAlways, NULL);

  /*  - - - - - - - - - - - - - - - - -  */
  /* Create the "moving mesh points" dialog */
  hm_mmp_daP = create_mjg_dialog(toplevel, "h_mmp");
  hm_mmp_daP->client_data = NULL;
  hm_mmp_daP->callback = NULL;

#ifdef apollo
   sprintf(mmp_s, "%s%s", MMP_S_STRING_1, MMP_S_STRING_2);
#endif

  XtVaSetValues(hm_mmp_daP->dialog, XtNvalue, mmp_s, NULL);
  XtVaSetValues(hm_mmp_daP->text,XtNscrollVertical,XawtextScrollAlways,NULL);

  /*  - - - - - - - - - - - - - - - - -  */
  /* Create the "file help" dialog */
  hm_file_daP = create_mjg_dialog(toplevel, "h_file");
  hm_file_daP->client_data = NULL;
  hm_file_daP->callback = NULL;
  XtVaSetValues(hm_file_daP->dialog, XtNvalue, file_help_s, NULL);
  XtVaSetValues(hm_file_daP->text, XtNscrollVertical,XawtextScrollAlways, NULL);

  /*  - - - - - - - - - - - - - - - - -  */
  /* Create the "mesh help" dialog */
  hm_mesh_daP = create_mjg_dialog(toplevel, "h_mesh");
  hm_mesh_daP->client_data = NULL;
  hm_mesh_daP->callback = NULL;
  XtVaSetValues(hm_mesh_daP->dialog, XtNvalue, mesh_help_s, NULL);
  XtVaSetValues(hm_mesh_daP->text, XtNscrollVertical,XawtextScrollAlways, NULL);

  /*  - - - - - - - - - - - - - - - - -  */
  /* Create the "morph help" dialog */
  hm_warp_daP = create_mjg_dialog(toplevel, "h_morph");
  hm_warp_daP->client_data = NULL;
  hm_warp_daP->callback = NULL;
  XtVaSetValues(hm_warp_daP->dialog, XtNvalue, morph_help_s, NULL);
  XtVaSetValues(hm_warp_daP->text, XtNscrollVertical,XawtextScrollAlways, NULL);

  /*  - - - - - - - - - - - - - - - - -  */
  /* Create the "diw properties menu help" dialog */
  hm_dpm_daP = create_mjg_dialog(toplevel, "h_dpm");
  hm_dpm_daP->client_data = NULL;
  hm_dpm_daP->callback = NULL;
  XtVaSetValues(hm_dpm_daP->dialog, XtNvalue, dpm_help_s, NULL);
  XtVaSetValues(hm_dpm_daP->text, XtNscrollVertical,XawtextScrollAlways, NULL);

  /*  - - - - - - - - - - - - - - - - -  */
  /* Create the "diw commands menu help" dialog */
  hm_dcm_daP = create_mjg_dialog(toplevel, "h_dcm");
  hm_dcm_daP->client_data = NULL;
  hm_dcm_daP->callback = NULL;
  XtVaSetValues(hm_dcm_daP->dialog, XtNvalue, dcm_help_s, NULL);
  XtVaSetValues(hm_dcm_daP->text, XtNscrollVertical,XawtextScrollAlways, NULL);

  /* ----------------------------- */
  /* Create a simple menu for Command */
  help_menu = XtVaCreatePopupShell("help_menu", simpleMenuWidgetClass,
    parent, NULL);

    /* - - - - - - - - - - - - - - - - - - - -  */
    /* Create a menu entry for "about" */
    hm_about_sme = XtVaCreateManagedWidget("hm_about_sme", smeBSBObjectClass,
      help_menu, NULL);
    XtAddCallback(hm_about_sme, XtNcallback, popup_dialog_cb, hm_about_daP);

    /* - - - - - - - - - - - - - -  */
    /* Create a menu item separator */
    XtVaCreateManagedWidget("hm_sep_sme", smeLineObjectClass,
      help_menu, NULL);

    /* - - - - - - - - - - - - - - - - - - - -  */
    /* Create a menu entry for "moving mesh points" */
    hm_mmp_sme = XtVaCreateManagedWidget("hm_mmp_sme", smeBSBObjectClass,
      help_menu, NULL);
    XtAddCallback(hm_mmp_sme, XtNcallback, popup_dialog_cb, hm_mmp_daP);

    /* - - - - - - - - - - - - - -  */
    /* Create a menu item separator */
    XtVaCreateManagedWidget("hm_sep_sme", smeLineObjectClass, help_menu, NULL);

    /* - - - - - - - - - - - - - - - - - - - -  */
    /* Create a menu entry for "file help" */
    hm_file_sme = XtVaCreateManagedWidget("hm_file_sme", smeBSBObjectClass,
      help_menu, NULL);
    XtAddCallback(hm_file_sme, XtNcallback, popup_dialog_cb, hm_file_daP);

    /* - - - - - - - - - - - - - - - - - - - -  */
    /* Create a menu entry for "mesh help" */
    hm_mesh_sme = XtVaCreateManagedWidget("hm_mesh_sme", smeBSBObjectClass,
      help_menu, NULL);
    XtAddCallback(hm_mesh_sme, XtNcallback, popup_dialog_cb, hm_mesh_daP);

    /* - - - - - - - - - - - - - - - - - - - -  */
    /* Create a menu entry for "warp help" */
    hm_warp_sme = XtVaCreateManagedWidget("hm_warp_sme", smeBSBObjectClass,
      help_menu, NULL);
    XtAddCallback(hm_warp_sme, XtNcallback, popup_dialog_cb, hm_warp_daP);

    /* - - - - - - - - - - - - - -  */
    /* Create a menu item separator */
    XtVaCreateManagedWidget("hm_sep_sme", smeLineObjectClass, help_menu, NULL);

    /* - - - - - - - - - - - - - - - - - - - -  */
    /* Create a menu entry for "diw properties menu help" */
    hm_dpm_sme = XtVaCreateManagedWidget("hm_dpm_sme", smeBSBObjectClass,
      help_menu, NULL);
    XtAddCallback(hm_dpm_sme, XtNcallback, popup_dialog_cb, hm_dpm_daP);

    /* - - - - - - - - - - - - - - - - - - - -  */
    /* Create a menu entry for "diw commands menu help" */
    hm_dcm_sme = XtVaCreateManagedWidget("hm_dcm_sme", smeBSBObjectClass,
      help_menu, NULL);
    XtAddCallback(hm_dcm_sme, XtNcallback, popup_dialog_cb, hm_dcm_daP);

    /* - - - - - - - - - - - - - -  */
    /* Create a menu item separator */
    XtVaCreateManagedWidget("hm_sep_sme", smeLineObjectClass, help_menu, NULL);

    /* - - - - - - - - - - - - - -  */
    /* Create a menu item separator */
    XtVaCreateManagedWidget("hm_sep_sme", smeLineObjectClass, help_menu, NULL);

    /* - - - - - - - - - - - - - - - - - - - -  */
    /* Create a menu entry for "quit" */
    hm_quit_sme = XtVaCreateManagedWidget("hm_quit_sme", smeBSBObjectClass,
      help_menu, NULL);
    XtAddCallback(hm_quit_sme, XtNcallback, exit_callback, NULL);

  /* ------------------------- */
  /* Create a help menu button */
  if(left_w != NULL) {
    help_menu_button = XtVaCreateManagedWidget("help_menu_button",
      menuButtonWidgetClass, parent, XtNmenuName, "help_menu",
      XtNfromHoriz, left_w, NULL);
  } else {
    help_menu_button = XtVaCreateManagedWidget("help_menu_button",
      menuButtonWidgetClass, parent, XtNmenuName, "help_menu", NULL);
  }

  /* button tells where the popup goes */
  hm_about_daP->button = help_menu_button;
  hm_mmp_daP->button = help_menu_button;
  hm_file_daP->button = help_menu_button;
  hm_mesh_daP->button = help_menu_button;
  hm_warp_daP->button = help_menu_button;
  hm_dpm_daP->button = help_menu_button;
  hm_dcm_daP->button = help_menu_button;

  return(help_menu_button);
}
