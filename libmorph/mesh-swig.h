

/* SWIG: Simplified Wrapper Interface Generator */
#ifdef SWIG
%module mesh
%{
#include "mesh.h"



/* mesh_tcl_result: TCL Result string for mesh functions
//
// DESCRIPTION
// This is a sneaky way to return character string results to TCL while
// not having to worry about memory leaks, and while still using SWIG's
// wrapping abilities.  I.e., I don't have to worry about writing my
// own TCL wrapper code and registering it with TCL.
//
// NOTES
// The drawback to this method is potentially that returning a pointer to
// a string could cause problems if that pointer is abused, i.e., if the
// contents of that string are assumed to remain constant by the TCL code.
*/
#define MESH_TCL_RESULT_MAX_LEN 2048
static char mesh_tcl_result[MESH_TCL_RESULT_MAX_LEN];



/* NAME
//   meshLine: return a line of mesh point coordinate pairs as a string
//
//
// ARGUMENTS
//   this (in): mesh pointer
//
//   line_index (in): index of the row or column, depending on 'direction'
//
//   direction (in): 1 for row (horizontal line), 2 for column (vertical line)
//
//
// DESCRIPTION
//   meshLine returns a list of coordinate pairs of all of the mesh
//   points along a line of the given line_index and direction.
//
//
// RETURN VALUE
//   Returns the address of the result string, which is assumed to be
//   passed back to the TCL interpretter as a TCL list.
//   The string used is the global string mesh_tcl_result.  Using a
//   constant address eliminated potential memory leak problems.
//
//
// SEE ALSO
//   mesh_tcl_result
*/
//FIXME mennucci: why is this in mesh.h instead of mesh.c?
// anyway it is protected by ifdef SWIG
char *
meshLine(const MeshT *this, const int line_index, const int direction)
{
  int vi;
  int nv;
  int str_len = 0;
  char float_string[64];
  int point_index = -10000;

  if(line_index < 0) {
    /* line_index is out of range */
    return NULL;
  }

  if(1 == direction) {
    nv = this->nx;
    if(line_index >= this->ny) {
      /* line_index is out of range */
      return NULL;
    }
  } else if (2 == direction) {
    nv = this->ny;
    if(line_index >= this->nx) {
      /* line_index is out of range */
      return NULL;
    }
  } else {
    /* Invalid value for direction */
    return NULL;
  }

  /* Empty the result string */
  mesh_tcl_result[0] = '\0';

  for(vi = 0; vi < nv; vi++) {
    if(1 == direction) {
      point_index = line_index * this->nx + vi;
    } else if (2 == direction) {
      point_index = vi * this->nx + line_index;
    }
    sprintf(float_string, " %.0f %.0f",
      this->x[point_index], this->y[point_index]);
    str_len += strlen(float_string);
    if(str_len >= MESH_TCL_RESULT_MAX_LEN) {
      fprintf(stderr, "meshLine: mesh_tcl_result length exceeded\n");
      return NULL;
    }
    strcat(mesh_tcl_result, float_string);
  }
  return mesh_tcl_result;
}
%}
#endif /* SWIG */


#ifdef SWIG
%addmethods MeshT {
  MeshT(void)           { return meshNew(0, 0); }

  ~MeshT()              { meshDelete(self); }

  int alloc(int nx, int ny)
                        { return meshAlloc(self, nx, ny); }

  void free()           { meshFree(self); }

  void print()          { meshPrint(self); }

  void interpolate(const MeshT *m1P, const MeshT *m2P, float tween_param)
                        {meshInterpolate(self, m1P, m2P, tween_param);}

  void reset(int img_width, int img_height)
                        { meshReset(self, img_width, img_height); }

  void scale(int img_width, int img_height)
                        { meshScale(self, img_width, img_height); }

  char *row(int line_index)
                        { return meshLine(self, line_index, 1); }

  char *col(int line_index)
                        { return meshLine(self, line_index, 2); }

  int pick(int mouse_x, int mouse_y, int component, float proximity)
        { return meshPick(self, mouse_x, mouse_y, component, proximity); }

  void store()          { meshStore(self); }

  void recover()        { meshRetrieve(self); }

  int lineModify(MeshT *other, int mouse_x, int mouse_y, char line_type, char action)
        { return meshLineMouseModify(self, other, mouse_x, mouse_y,
                                     line_type, action);
        }

  int read(const char *filename)
                        { return meshRead(self, filename); }

  int write(const char *filename)
                        { return meshWrite(self, filename); }

  void match(const MeshT *other)
                        { meshMatch(self, other); }

  float pointGet(int xi, int yi, int component)
        { if(0 == component) return self->x[yi * self->nx + xi];
          else if(1 == component) return self->y[yi * self->nx + xi];
          /* Invalid value for component */
          else return -1.0;
        }

  void set(int xi, int yi, float new_x, float new_y)
        {  meshSet(self, xi, yi, new_x, new_y); }
};
#endif
