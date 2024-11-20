#ifndef _SETTINGS_H_
#define _SETTINGS_H_


//typedef enum _editview_t editview_t;


/* this reflects the optionmenu by the same name */

typedef enum   {  
  EDITVIEW_EDIT=0, /* edit mesh on loaded image */
  EDITVIEW_SHOW=1, /* show warped image without meshes
                      dont edit (unless we arein the resulting image)*/
  //EDITVIEW_FEATURES=4, /* edit features */
  EDITVIEW_SHOWMESHES=2, /* show warped image with both meshes
                            dont edit*/
  //EDITVIEW_SHOWANIM=5,
  /* animate mesh from normal
     to warped, on fixed  images;
     dont edit (NOT IMPLEMENTED)*/

  /* this one has a different meaning in the "resulting image" window */
  EDITVIEW_EYES=3  /*change the position of the eyes points, or, 
		    select subimage*/
} editview_t;


GtkWidget*
create_gtkmorph_menuSettings (void);

int
settings_get_value(char *name);
int
settings_set_value(char *name, int val);

GtkWidget*
create_image_menu_settings (int i);

int
image_settings_get_value(char *name, int i);

int
image_settings_set_value(char *name, int i, int value);

void
gtkmorph_settings_callback(GtkWidget* thismenu, gpointer userdata);

#endif //_SETTINGS_H_
