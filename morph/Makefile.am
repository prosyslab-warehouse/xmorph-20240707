
## Process this file with automake to produce Makefile.in

include ../Makefile.conf


INCLUDES = 	-I$(top_srcdir)/libmorph -I$(top_srcdir)/intl 

bin_PROGRAMS = morph

morph_LDADD =   $(INTLLIBS)  ../libmorph/libmorph.la $(NON_GUI_LIBS)

morph_SOURCES =    main.c

morph_CFLAGS = $(CFLAGS)  -DNDEBUG_LIBMORPH_REF_COUNT
