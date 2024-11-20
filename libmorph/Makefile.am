## Process this file with automake to produce Makefile.in

include ../Makefile.conf

AM_CFLAGS = @CFLAGS@  $(CFLAGS_CONF)

#noinst_LIBRARIES =  libmorph.a

#LIBTOOL support
lib_LTLIBRARIES = libmorph.la

libmorph_la_LDFLAGS = -version-info 1:0:0

pkginclude_HEADERS = mesh.h mesh_t.h mesh-getext.h  RgbaImage.h    my_malloc.h  spl-array.h  spline.h  tga.h  warp.h warp2.h relax.h resample.h  warp-gtk.h  

libmorph_la_SOURCES =   my_malloc.c     warp.c   \
  mesh.c         spline.c   my_malloc.h  tga.c   tga.h  \
  RgbaImage.c         spl-array.c   warp2.c \
  relax.c  resample.c braindead_msvc.h sinc_256.h resample_snippet.h \
  $(pkginclude_HEADERS)


EXTRA_DIST = README.libmorph
