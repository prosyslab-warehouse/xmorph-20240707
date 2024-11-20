## Process this file with automake to produce Makefile.in

include ../Makefile.conf

AM_CFLAGS = $(CFLAGS_CONF) 


someCPPFLAGS= -I../intl $(GDK_PIXBUF_CFLAGS) $(GTK_CFLAGS)  -DLOCALEDIR=\"$(localedir)\" -DDATADIR=\"$(datadir)\"

if COND_GTK2
AM_CPPFLAGS = $(someCPPFLAGS) $(GTHREAD_CFLAGS)  -I../glade2 
else
AM_CPPFLAGS = $(someCPPFLAGS) -I../glade1 
endif

############################


#this is a sort of extra gtk library
# I may decide to compile it as such ...
# but now it is too intertwingled with gtkmorph
gtkextra = gtksettings.c gtksettings.h \
	gdk-pixbuf-extra.c gdk-pixbuf-extra.h  \
	gtk_subimagesel.c gtk_subimagesel.h \
	gtktopdata.h gtktopdata.c 

#lib_LTLIBRARIES =  libmorph-gtk.la libgtk-am-extra.la
#libgtk_am_extra_la_LDFLAGS = -version-info 1:0:0
#libgtk_am_extra_la_CFLAGS = @CFLAGS@ @PACKAGE_CFLAGS@ $(CFLAGS_CONF) 
#libgtk_am_extra_la_SOURCES = $(gtkextra)


#####################################
if COND_GTKMORPH
bin_PROGRAMS = gtkmorph
noinst_PROGRAMS =  testfourier
endif

#this is the gtkmorph GUI
gtkmorphsources = main.c main.h\
	callbacks_main.c callbacks_help.c  \
	callbacks_img.c  \
	callbacks_edit.c \
	callbacks_fs.c callbacks_fs_mesh.c callbacks_fs.h \
	callbacks_subimg.c callbacks_subimg.h \
	 callbacks_warped.c \
	dialogs.c dialogs.h \
	movies.c movies.h colors.c colors.h \
	utils.c utils.h utils-mesh.c loadsave.c loadsave.h \
	loadsave_mesh.c loadsave_mesh.h \
	settings.c settings.h \
	feature.c feature.h \
	guide.c guide_text.c guide.h guide_text.h mag.c mag.h \
	mesh-gtk.c mesh-gtk.h \
	gtk-meta.h	gtk-meta.c affine.h

#	support.c support.h 
#	interface.c interface.h 
#	pixmaps.c pixmaps.h 
#	pixmaps.c pixmaps.h 	application.hh 

EXTRA_DIST=mpeg.param utils2_c

pkgdata_DATA=mpeg.param


#if COND_LIBPLY
#p = 	bind.c bind.h gtk_widget.h gtk_widget.c
#else
p =
#endif

if COND_WAILI
w  = wavelet.cc wavelet.hh
wl = -lwaili
else
w  = 
wl = 
endif




gtkmorph_SOURCES = $(gtkextra) $(gtkmorphsources) $(p) $(w) $(f)

testfourier_SOURCES = testfourier.cc
#testfourier_CFLAGS = -g
testfourier_CXXFLAGS = -g  -Wall
if COND_FFTW
f = fourier.cc fourier.hh  
fl = -lfftw3
else
#there is a backup for lib_fftw
f = fourier.cc fourier.hh  
fl =  
endif

testfourier_LDADD = $(GTK_LIBS) $(GDK_PIXBUF_LIBS) $(fl)

testfourier_run1: testfourier
	for i in 200.0 200.2 200.4 200.6 200.8 201.0 201.2 201.4 201.6 201.8 202.0 ; do ./testfourier -c  testfourier-2.tif 200 150  testfourier-1.tif  $$i 150 /tmp/x_$$i.png ; done


if COND_GTK2
# HACK $(GTHREAD_LIBS) 
gtkmorph_LDADD =  $(GTK_LIBS) $(GTHREAD_LIBS) $(GDK_PIXBUF_LIBS)  $(INTLLIBS) \
 $(NON_GUI_LIBS)  ../libmorph/libmorph.la 	 \
    ../glade2/interface.o ../glade2/support.o $(wl) $(fl)
else
gtkmorph_LDADD = $(GTK_LIBS) $(GDK_PIXBUF_LIBS) $(INTLLIBS) \
 $(NON_GUI_LIBS)  ../libmorph/libmorph.la 	 \
    ../glade1/interface.o ../glade1/support.o $(wl) $(fl)
endif


