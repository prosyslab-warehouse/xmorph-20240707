## Process this file with automake to produce Makefile.in

include Makefile.conf

#all other files that are distributed

EXTRA_DIST= config.rpath   AUTHORS INSTALL README.html README.old gimp-README TODO HISTORY \
  configure  config.rpath  po/gtkmorph.glade.c po/gtkmorph.glade2.c \
  Makefile.conf gtkmorph1.glade  gtkmorph2.glade  gtkmorph2.gladep

# it is copied by some glade autoconf stuff:  pixmaps/howmany.xpm  \
# it is obsolete: strings.gettext ; now we use po/gtkmorph.glade.c



if COND_MORPH
SUBDIRS1= morph
endif

if COND_GTK2
SUBDIRS4= glade2 gtkmorph 
else
SUBDIRS4= glade1 gtkmorph
endif

if COND_TKMORPH
SUBDIRS5= tkmorph
endif

SUBDIRS= po doc libmorph $(SUBDIRS1) xmorph $(SUBDIRS3) $(SUBDIRS4) $(SUBDIRS5)


install-data-local:
	@$(NORMAL_INSTALL)
	  $(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/pixmaps; \
	  for pixmap in $(srcdir)/pixmaps/* ; do \
	    if test -f $$pixmap; then \
	      $(INSTALL_DATA) $$pixmap $(DESTDIR)$(pkgdatadir)/pixmaps; \
	    fi \
	  done 
	cd example; for i in * ;\
	 	do $(INSTALL) -d $(DESTDIR)$(pkgdatadir)/example/$$i ;\
		for j in $$i/* ;\
		do  $(INSTALL_DATA) $$j $(DESTDIR)$(pkgdatadir)/example/$$i;\
		done;\
	done

dist-hook:
	cp -a example pixmaps debian $(distdir)/
	cd $(distdir)/. && find . -type d -name .svn -print0 | xargs -0  rm -r


clean-local:
	rm -fr .xvpics */.xvpics example/frame.auto.param example/frame*.ppm

ACLOCAL_AMFLAGS = -I m4
