## Process this file with automake to produce Makefile.in

include ../Makefile.conf

EXTRA_DIST = select.xbm unselect.xbm


#AM_CFLAGS = $(CFLAGS)  $(CFLAGS_CONF)  -DX_GUI -DNDEBUG_LIBMORPH_REF_COUNT

INCLUDES = -I$(top_srcdir)/libmorph -I$(top_srcdir)/intl 

if COND_XMORPH
bin_PROGRAMS = xmorph
endif

#keep this first and hope they are processed in order

#main.h : ../morph/main.h ; ln -s ../morph/main.h .

diw_map.o : diw_map.c main.h

main-x.o : ../morph/main.c ; $(COMPILE) -DX_GUI -DNDEBUG_LIBMORPH_REF_COUNT  -c  ../morph/main.c -o main-x.o

xmorph_CFLAGS =  $(X_CFLAGS) $(CFLAGS)  $(CFLAGS_CONF)  -DNDEBUG_LIBMORPH_REF_COUNT

xmorph_SOURCES =  main.h \
 help_menu.c       diw_map.c    mjg_dialog.c  file_menu.c \
 xmorph.c     sequence.c  warp_menu.c  mesh_menu.c \
 diw_map.h  warp_menu.h  mjg_dialog.h  \
 image_diw.h image_diw.c \
 help_menu.h  \
 xmorph.h   sequence.h   mesh_menu.h  \
  image_cb.h  image_cb.c \
  file_menu.h \
  mesh_cb.h mesh_cb.c


xmorph_LDADD =  main-x.o  $(X_PRE_LIBS) $(X_LIBS) $(XLIBS) $(X_EXTRA_LIBS) \
	 $(GIMPLIBS) $(NON_GUI_LIBS) $(INTLLIBS)  ../libmorph/libmorph.la 
