/* file_menu.h : file menu widgets and callbacks
//
// Written and Copyright (C) 1994-1999 by Michael J. Gourlay
//
// NO WARRANTEES, EXPRESS OR IMPLIED.
*/

/* create_file_menu: Create a file menu and its button
** returns the widget of the menu button
*/
extern Widget create_file_menu(Widget parent, Widget toplevel, Widget left_w);

#ifdef NEED_GIMP
extern Widget create_dest_menu(Widget parent, Widget toplevel, Widget left_w);
#endif
