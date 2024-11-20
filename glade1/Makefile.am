## Process this file with automake to produce Makefile.in

INCLUDES =

if COND_GTKMORPH
noinst_PROGRAMS = gtkmorph
endif

AM_CPPFLAGS = $(GTK_CFLAGS) -I../intl -DLOCALEDIR=\"$(localedir)\" -DDATADIR=\"$(datadir)\"

gtkmorph_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h

gtkmorph_LDADD = $(GTK_LIBS) $(INTLLIBS)

