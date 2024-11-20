/* mesh_cb.c : X Callbacks for Digital Image Warp mesh related widgets
//
// Written and Copyright (C) 1994-1999 by Michael J. Gourlay
//
// PROVIDED AS IS.  NO WARRANTEES, EXPRESS OR IMPLIED.
//
//
// These routines are used for the X Window System only and are not
// needed for tkmorph.
*/

#include <stdio.h>
#include <stdlib.h>

#include "diw_map.h"
#include "mjg_dialog.h"
#include "mesh.h"
#include "mesh_cb.h"




/* reset_mesh_cb: callback to reset a mesh
// Accesses global_diw_map[0] mesh information
// Changes global_diw_map.mesh_[xy][s|d]
// Fakes an expose event to all diw_map image regions to redraw meshes
*/
/*ARGSUSED*/
void
reset_mesh_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  int type = (int) client_data;

  switch(type) {
    case 1:
      meshReset(&global_diw_map[0].mesh_src,
        global_diw_map[0].width, global_diw_map[0].height);
      break;

    case 2:
      meshReset(&global_diw_map[0].mesh_dst,
        global_diw_map[0].width, global_diw_map[0].height);
      break;

    default:
      fprintf(stderr, "reset_mesh_cb: Bad Value: type: %i\n", type);
      return;
      /*NOTREACHED*/
      break;
  }

  /* Now redraw the meshes */
  FakeAllExpose(0, NULL, NULL, NULL);
}




/* functionalize_mesh_cb: callback to functionalize a mesh
// Accesses global_diw_map[0] mesh information
// Changes global_diw_map.mesh_[xy][s|d]
// Fakes an expose event to all diw_map image regions to redraw meshes
*/
/*ARGSUSED*/
void
functionalize_mesh_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  int type = (int) client_data;
  int chg;

  switch(type) {
    case 1:
    chg=meshFunctionalize(&global_diw_map[0].mesh_src,
        global_diw_map[0].width, global_diw_map[0].height);
      break;

    case 2:
    chg=meshFunctionalize(&global_diw_map[0].mesh_dst,
        global_diw_map[0].width, global_diw_map[0].height);
      break;

    default:
      fprintf(stderr, "functionalize_mesh_cb: Bad Value: type: %i\n", type);
      return;
      /*NOTREACHED*/
      break;
  }

  /* If the mesh changed, redraw the meshes */
  if(chg) FakeAllExpose(0, NULL, NULL, NULL);
}




/* load_mesh_cb: callback to load mesh from file
//
// Makes sure that both meshes have the same geometry after loading
// the new mesh.
//
// client_data (in): tyecast to int, indicates whether src or dst mesh
//   is being loaded
//
// call_data (in): pointer to dialog box, used to get string.
//
// Accesses global_diw_map[0] mesh and image information
//
// Side effects:
//   Changes global_diw_map.mesh_(src,dst)
//
//   via meshRead:
//     Frees memory for old mesh
//     Allocates memory for new mesh
//
//   Fakes an expose event to all diw_map image regions to redraw meshes
*/
/*ARGSUSED*/
void
load_mesh_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  dialog_apdx_t *daP = (dialog_apdx_t *)call_data;
  int type = (int) client_data;
  String filename;

#ifdef DEBUG
  printf("load_mesh_cb:\n");
#endif

  filename = XawDialogGetValueString(daP->dialog);
  if(filename==NULL) {
    return;
  }

  switch(type) {
    case 1:
      if (meshRead(&global_diw_map[0].mesh_src, filename)) {
        fprintf(stderr, "load_mesh_cb: meshRead failed\n");
        return;
      }
      meshMatch(&global_diw_map[0].mesh_dst, &global_diw_map[0].mesh_src);
#ifdef GIMP
      set_src_mesh_name (filename);
#endif
      meshScale(&global_diw_map[0].mesh_src,
                global_diw_map[0].src_img.ncols,
                global_diw_map[0].src_img.nrows);
      break;


    case 2:
      if(meshRead(&global_diw_map[0].mesh_dst, filename)) {
        fprintf(stderr, "load_mesh_cb: meshRead failed\n");
        return;
      }
      meshMatch(&global_diw_map[0].mesh_src, &global_diw_map[0].mesh_dst);
#ifdef GIMP
      set_dst_mesh_name (filename);
#endif
      meshScale(&global_diw_map[0].mesh_dst,
                global_diw_map[0].dst_img.ncols,
                global_diw_map[0].dst_img.nrows);
      break;

    default:
      fprintf(stderr, "load_mesh_cb: Bad Value: type: %i\n", type);
      return;
      /*NOTREACHED*/
      break;
  }

  /* Propagate the mesh changes to all of the diw_maps */
  diw_map_of_widget(NULL);

  /* Redraw the meshes */
  FakeAllExpose(0, NULL, NULL, NULL);

#ifdef DEBUG
  printf("load_mesh_cb: leaving\n");
#endif
}




/* save_mesh_cb: callback to save mesh to file
// Accesses global_diw_map[0] mesh information
*/
/*ARGSUSED*/
void
save_mesh_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  dialog_apdx_t *daP = (dialog_apdx_t *)call_data;
  int type = (int) client_data;
  String filename;
  int save_mesh_rv;

  filename = XawDialogGetValueString(daP->dialog);
  if(filename==NULL) {
    return;
  } else {
    switch(type) {
      case 1:
        if( (save_mesh_rv = meshWrite(&global_diw_map[0].mesh_src, filename)) )
        {
          fprintf(stderr, "meshWrite returned %i\n", save_mesh_rv);
          return;
        }
#ifdef GIMP
        set_src_mesh_name (filename);
#endif
        break;

      case 2:
        if( (save_mesh_rv = meshWrite(&global_diw_map[0].mesh_dst, filename)) )
        {
          fprintf(stderr, "save_mesh returned %i\n", save_mesh_rv);
          return;
        }
#ifdef GIMP
        set_dst_mesh_name (filename);
#endif
        break;

      default:
        fprintf(stderr, "save_mesh_cb: Bad Value: type: %i\n", type);
        return;
        /*NOTREACHED*/
        break;
    }
  }
}
