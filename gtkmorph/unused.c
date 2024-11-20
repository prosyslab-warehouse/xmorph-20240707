

/************************** pixbuf <-> r g b
 *  these copy data 
 */

void
pixbuf_to_rrrgggbb(GdkPixbuf *pb, guint8 *r, guint8 *g, guint8 *b)
  /* copy pixbuf data  to RGB data for morphing	   */
{ 
  int i,j,
    width =  gdk_pixbuf_get_width(pb),
    height = gdk_pixbuf_get_height(pb);
  long //size = width *  height,
    pos=0, dp=0;

  int rowstride= gdk_pixbuf_get_rowstride (pb) ;
  guchar  *data = gdk_pixbuf_get_pixels(pb);

  int ch= gdk_pixbuf_get_n_channels(pb);

  g_assert ( r != NULL);  g_assert ( g != NULL);  g_assert ( b != NULL);  
  g_assert ( pb != NULL);

  /* FIXME I am not absolutely sure
     that the data are RGB with lines strictly packed!!
  */
  g_assert(gdk_pixbuf_get_colorspace(pb) == 
	   GDK_COLORSPACE_RGB);
	
  g_assert( pb != NULL);

  for( j=0; j<  height ; j++)	  
    {
      dp= rowstride * j;
      for( i=0; i<  width ; i++)
	{
		
	  r[pos]= data[dp++];
	  g[pos]=data[dp++];
	  b[pos]=data[dp++];
	  pos++; 
	  if (ch==4)	 dp ++;
	}
    }
	
}   

static
void
rrrgggbbb_add_to_pixbuf(GdkPixbuf *pb, /* destination */
			const guint8 *r, const  guint8 *g, const guint8 *b,
			/* source */
			double factor /* dissolving factor */
			)
{ 
  int i,j,
    width =  gdk_pixbuf_get_width(pb),
    height = gdk_pixbuf_get_height(pb);
  long //size =  width *  height,
    pos=0, dp=0;
  
  guchar  *data = gdk_pixbuf_get_pixels(pb);	

  int rowstride= gdk_pixbuf_get_rowstride (pb) ;

  int ch= gdk_pixbuf_get_n_channels(pb);

  g_assert ( r != NULL);  g_assert ( g != NULL);  g_assert ( b != NULL);  
  g_assert ( pb != NULL);	


    /* FIXME I am not absolutely sure
     that the data are RGB with lines strictly packed!!
  */
  g_assert(gdk_pixbuf_get_colorspace(pb) == 
	   GDK_COLORSPACE_RGB);
	
  for( j=0; j<  height; j++)	      
    {
      dp= rowstride * j;
      for( i=0; i< width; i++)
	{
	  data[dp++] += (double)r[pos] * factor;
	  data[dp++] += (double)g[pos] * factor;
	  data[dp++] += (double)b[pos] * factor;
	  pos++; 
	  if ( 4 == ch)	 dp ++;
	}
    }
}

static
void
pixbuf_add_to_pixbuf(GdkPixbuf *src,
		     GdkPixbuf *dst,
		     double factor /* dissolving factor */
		     )
{ 
  int i,j,
    d_width =  gdk_pixbuf_get_width(dst),
    d_height = gdk_pixbuf_get_height(dst),
    s_width =  gdk_pixbuf_get_width(src),
    s_height = gdk_pixbuf_get_height(src);
  long     s=0, d=0;
  
  guchar  *srcdata = gdk_pixbuf_get_pixels(src);	
  guchar  *dstdata = gdk_pixbuf_get_pixels(dst);	

  int s_rowstride= gdk_pixbuf_get_rowstride (src) ;
  int d_rowstride= gdk_pixbuf_get_rowstride (dst) ;

  int s_ch= gdk_pixbuf_get_n_channels(src);
  int d_ch= gdk_pixbuf_get_n_channels(dst);
  int ii=MIN(s_width,d_width)*MIN(s_ch,d_ch);

  j=MIN(d_height,s_height);
  while(j>=0)
    {
      for(i=0;i<ii;i++)
	{
	  dstdata[d+i] += (double)srcdata[s+i] * factor;	  
	}
      s+= s_rowstride ; d+=d_rowstride;
      j--;
    }
}


static
void
do_warp_an_image_old(int lp) //, char * r, char *g, char * b)
{
  int   nx=sp-> im_mesh[MAIN_WIN].nx,
    ny=sp-> im_mesh[MAIN_WIN].ny;
  GdkPixbuf *pb = sp-> im_warped_pixbuf[lp];
  MeshT *dstmesh = (&(sp->im_mesh[MAIN_WIN]));

  /* geometry of the resulting image */
  int w=sp->resulting_width, h=sp->resulting_height;
  long size = w * h ; 
  guint8 * r , *b, *g; 
  r = g_malloc( size );
  g = g_malloc( size );
  b = g_malloc( size );
  
 if(pb==NULL) {
    create__pixbuf(lp,PIXWARPED);
    pb = sp-> im_warped_pixbuf[lp];
  }


#define warpthis(IN,OUT) \
	warp_image_inv_old(IN,OUT, \
		   sp->resulting_width,	 sp->resulting_height, \
		   sp->im_mesh[lp].x,	   sp->im_mesh[lp].y, \
		   dstmesh->x, dstmesh->y, \
		   nx,ny); 

  gdk_pixbuf_clear(pb);

  if( nx != sp->im_mesh[lp].nx  || ny != sp->im_mesh[lp].ny)
    {
      show_error
	(g_strdup_printf
	 (_("\
The image %d has an %ld by %ld mesh while the resulting mesh is %d by %d!\n\
I cant warp it!\n\
I suggest that you add lines so that both meshes are %d by %d"),
	  lp, sp->im_mesh[lp].nx, sp->im_mesh[lp].ny , nx, ny,
	  sp->meshes_x,
	  sp->meshes_y));

    }
  else
    {

      warpthis(sp->red[lp], r);
      warpthis(sp->green[lp], g);
      warpthis(sp->blue[lp], b);
    
      //g_message("writing pixels for %d\n",lp);
          
      rrrgggbbb_add_to_pixbuf(pb,     r,g,b,     1.0 );
    }

  g_free(r);   g_free(g);   g_free(b); 

  __after_warp(lp);

}






















/* save also the viewport widget address, so we can resize it easily 
   
   actually "glade" does the same thing, and I didnt know it
*/

void
on_viewport3_realize                   (GtkWidget       *widget,
                                        gpointer         user_data)
{
  gtk_widget_set_data_top(widget, "viewport widget", widget);
}









void
on_optionmenu_editview_released        (GtkButton       *button,
                                        gpointer         user_data)
{
  /* this signal is (almost) never emitted
  printf("the choice is %s \n",
	 gtk_widget_get_name(gtk_menu_get_active (GTK_MENU(user_data)))); 
   */
}


void
on_optionmenu_editview_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
  /* this signal is never emitted
     printf("the click choice is %s \n",
     gtk_widget_get_name(gtk_menu_get_active (GTK_MENU(user_data)))); 
  */
}


void
on_optionmenu_editview_pressed         (GtkButton       *button,
                                        gpointer         user_data)
{
  /* this signal is never emitted
  g_message("at %s %d PRESSED" ,__FILE__ , __LINE__);
  */
}














/**********
	   pixbufs and rr gg bb buffers are alloced here,
	   whilst the im_pixmap_subimage and im_pixwarped are set elsewhere 
*/

void
alloc_image_win_data(int lp, int what)
{ 

  g_assert(what==0 || what == 1);

 /* there is no image associated to this window */
#ifdef USE_IMLIB
  sp->im_image[lp]=NULL;
  sp->im_imlib[lp]=NULL;
#endif

#ifdef STORE_RRGGBB
  {       
    long size = sp->resulting_width *  sp->resulting_height ;
    sp->red[lp] = g_malloc( size );
    sp->blue[lp]  = g_malloc( size );
    sp->green[lp] = g_malloc( size ) ;
  }
#endif

  {
    int j ; 
    for (j=2;j>=what;j--) {
      create__pixbuf(lp,j);   
      // can't do this now-- need a window --create__pixmap(lp,j);
  }}

}










#if (0)
  {
    GtkWidget *b=gtk_button_new_with_label("filter");
    GtkWidget *z=gtk_button_new_with_label("che faccio");
    //fileselection_g->fileop_dialog=gtk_dialog_new();
    g_return_if_fail(fileselection_g->fileop_c_dir);
    gtk_widget_hide(fileselection_g->fileop_c_dir);
    //g_return_if_fail(fileselection_g->fileop_dialog);
    gtk_widget_show(b);    gtk_widget_show(z);
    //gtk_widget_show(fileselection_g->fileop_dialog);
    gtk_container_add (GTK_CONTAINER 
		       (GTK_DIALOG(fileselection_g)->action_area),
		       b);
    gtk_container_add (GTK_CONTAINER 
		       (GTK_DIALOG(fileselection_g)->vbox),
		       z);
  }
  // gtk_dialog_has_separator(fileselection_g->fileop_dialog,TRUE);
  //gtk_dialog_add_button(fileselection_g,"NEW",999);
#endif






/******************************************************************/


void adjust_point(MeshT *dstmesh, GdkPixbuf   *dst , int i)
  {
    MeshT *dstmesh=&(sp->im_mesh[i]);
      
    GdkPixbuf   *dst = (sp->im_subimage_pixbuf[i]);

    int x,y,dx,dy; int *mi, *mj;
    long d;
    GdkModifierType state;
    gdk_window_get_pointer (event->window, &x, &y, &state);
    d=meshPointNearest(dstmesh , 
		       x,y, //event->x, event->y, //int px, int py, 
		       mi, mj, 
		       &dx, &dy);
    gtk_widget_update_rect (widget, x, y,90);     
  }






////////////////////// time
/* gchar * elapsed_time_as_string( double secs) */
/* { */
/*   int sec= (double)tot /	((double)(tot-ima-1))* */
/*     ((double)(aft.tv_usec -bef.tv_usec) /1e6 ); */
/*   int min=sec/60; sec= sec % 60; */
/*   rem=g_strdup_printf(" %2dm:%02ds",min,sec); */

/*   g_get_current_time(&aft); */
/* } */
