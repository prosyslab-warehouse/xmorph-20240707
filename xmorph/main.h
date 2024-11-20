/* main.h : Digital Image Warping for X Window System
//
// A graphical user interface to a mesh warping algorithm
//


   Written and Copyright (C) 1994-1999 by Michael J. Gourlay

This file is part of Xmorph.

Xmorph is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

Xmorph is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Xmorph; see the file LICENSE.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

*/

#ifndef MAIN_H
#define MAIN_H




#define NUM_ORIG_IMAGES 2




extern int verbose;
extern RgbaImageT orig_image[NUM_ORIG_IMAGES];




#ifdef GIMP
# define NEED_GIMP 1

/* xmorph plugin changes for the GIMP
 * Copyright (C) 1997, Gordon Matzigkeit
 * Gordon Matzigkeit <gord@gnu.org>, 1997
 *
 * The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Bump map plug-in --- emboss an image by using another image as a bump map
 * Copyright (C) 1997 Federico Mena Quintero
 * federico@nuclecu.unam.mx
 *
 * xscanimage -- a graphical scanner-oriented SANE frontend
 * Authors:
 * Andreas Beck <becka@sunserver1.rz.uni-duesseldorf.de>
 * Tristan Tarrant <ttarrant@etnoteam.it>
 * David Mosberger-Tang <davidm@azstarnet.com>
 *
 * All GIMP-related code is also distributable under the GNU General
 * Public License.  See the above notice for details.
 *
 * TODO:
 *   - Update destination menu when GIMP drawables change.
 *   - Properly initialize Xt resources when the plugin changes the output
 *     basename and sequence number of steps.
 *   - Parameterize more things so that GIMP scripts can call us.
 */

# include <libgimp/gimp.h>

#endif /* GIMP */




#endif /* MAIN_H */
