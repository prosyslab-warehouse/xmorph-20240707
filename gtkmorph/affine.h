

static void affine_X_vector(double *dst,double *src, double *mat)
{
  int i,j;
  for(i=0; i<2; i++) {
      dst[i] =0;
      for(j=0; j<2; j++) {
	dst[i] += mat[i*3+j] * src[j];
      }
      dst[i] += mat[i*3+2];
  }
}

static void p_affine_X_vector(GdkPoint p,double x, double y, double *affine)
{
  p.x =  x*affine[0]+y*affine[1] +affine[2];    
  p.y =  x*affine[3]+y*affine[4] +affine[5];
}

static void invert(double *b,double *f)
{
  /* invert */
  b[0]=1/f[0]; b[4]=1/f[4]; b[2]=-f[2]/f[0]; b[5]=-f[5]/f[4];
}

static void mesh_X_affine(MeshT *dstmesh,MeshT *mesh,double *inv)
{
  int xi,yi; double x,y;
  const double *xsP = mesh->x;
  const double *ysP = mesh->y;
  for(xi=0; xi < mesh->nx; xi++) {
     for(yi=0; yi < mesh->ny; yi++) {
       x=xsP[yi*mesh->nx + xi];  y=ysP[yi*mesh->nx + xi];
       meshSetNoundo( dstmesh, xi,yi,
		      x*inv[0]+y*inv[1] +inv[2],
		      x*inv[3]+y*inv[4] +inv[5]);
     }
   }
}
