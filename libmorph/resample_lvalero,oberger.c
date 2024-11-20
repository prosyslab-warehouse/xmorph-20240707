
/************************ extra stuff   *************************************/

#ifdef ARTIFACTS

/* A Mennucc: this routine that follows was provided by lvalero and oberger;
it uses  integers for every double variable,
and directly accesses the sinc table; so it is very very fast;
unfortunately if x and dx are integers, then warping is not precise
to the subpixel , and there are visible artifacts

I have rewritten resample_array_inv_conv (above)
so that it directly accesses the sinc_table , and it uses integers,
but so that it is precise to subpixel

in my tests I could achieve these results

old routine     9.55 sec
new routine     3.50 sec
lvalero,oberger 2.86 sec 

note that this routine needs the old style sinc table which is not provided
any more 
*/
#include "sinc_table.h"




/* this test was provided by lvalero, oberger 05/05/2004 :*/

/*With one image 2048*2048 in 16 bits, Athlon 1.4Ghz + parhelia 128 Mo + 785 Mo RAM
 we have :
for resample_array_inv_conv (optimised) 3s
for resample_array_inv_bilin 2.5s
for resample_array_inv_noaa_ 1.4s*/     



//VERSION OPTIMISEE MODIF lvalero, oberger 05/05/2004
static void resample_array_inv_conv (const double *F, const PIXEL_TYPE *src, int s_len, int s_stride, PIXEL_TYPE *dst, int d_len, int d_stride)
{
  int i, p = 0, j, lastj ;
  int x, nx, px, dx ;
  double c, s, v ;
  int firstj,index_stride,index,increment ;
  
  for( i = 0 ; i < d_len ; ++i ) 
  {
   v=0; c=0;
   x=(int)F[i];
   nx=(int)F[i+1];
   px=(int)F[i-1];
   //dx=ABS(nx-px)/2.;
   dx=ABS(nx-px) >> 1 ;

   if(dx<1) dx=1;

   //lastj=ceil(x+dx+dx);
   lastj = x+dx+dx ;
   
   // optimisation pour v+=s* (double)src[j*s_stride]; => on economise une multiplication
   // par defaut, g met floor pour passer en int, mais ce peut etre mauvais
   //firstj=floor(x-dx-dx);
   firstj = x-dx-dx;

   //index_stride = floor(firstj*s_stride) ;
   index_stride = firstj*s_stride ;
   // en deroulant la boucle avec j = x-dx-dx on a :
   //
   // (x-j)/dx = (x-x+dx+dx)/dx = (2*dx)/dx = 2  <- etape 0
   // (x-(j+1))/dx = (x-(x+1)+dx+dx)/dx  = 2-(1/dx)  <- etape 1
   // (x-(j+2))/dx = (x-(x+2)+dx+dx)/dx  = 2-(2/dx)  <- etape 2
   // (x-(j+3))/dx = (x-(x+3)+dx+dx)/dx  = 2-(3/dx)  <- etape 3
   // ...
   // (x-lastj)/dx = (x-x-dx-dx)/dx  = -2  <- etape fin
   //
   // Donc on a pour l'etape n :
   //
   // ((2 - (n/dx)) + 4)*100 = (2+4)*100 - (n/dx)*100
   //           = (2+4)*100 - n*(1/dx)*100
   //           = (2+4)*100 - n*(100/dx)
   
   // avec le commentaire precedent, on s'apercoit que la premiere valeur est :
   //index = (2+4)*100 ;
   index = 600 ;
   // et l'increment devient :
   //increment = floor(100./dx) ;//  correspond a (1/dx)*100, mais on racle une multiplication
   increment = 100/dx ;
   
   for ( j = firstj ; j <= lastj  ; ++j )
   {
    // avec cette optimisation il convient de changer le test : laisser les operations
    // comme elle sont ecrites, le compilo fera le calcul
    //if ( index < ((-2+4)*100) /*|| index > ((2+4)*100)*/ )
    if ( index < (200) /*|| index > ((2+4)*100)*/ )
     s = 0 ;    
    else
     s=sinc_table [index];
    if ( j>=0 && j<s_len)
     v+=s* (double)src[index_stride];
     c+=s;
    
    index -= increment ;
    ++index_stride ;
   }
   if (!(c<0.0001 && c > -0.0001)) v /=c ;
   dst[p]=(PIXEL_TYPE)CLAMP(v,0,255);
   p+= d_stride;    
  }
}



#endif
