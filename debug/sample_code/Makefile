# Generated automatically from Makefile.in by configure.
SHELL = /bin/sh

srcdir = /home/zhx/MulticoreCompiler-12月26日星期五/prophet_vmips/sample_code
top_srcdir = /home/zhx/MulticoreCompiler-12月26日星期五/prophet_vmips
VPATH = /home/zhx/MulticoreCompiler-12月26日星期五/prophet_vmips/sample_code
prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/vmips
pkglibdir = $(libdir)/vmips
pkgincludedir = $(includedir)/vmips

top_builddir = ..

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs

SUBDIRS=. xmboot
VMIPSTOOL = $(top_builddir)/vmipstool -n -F $(top_builddir)/vmipsrc --ld-script=$(srcdir)/ld.script
CLEANFILES=setup.o
DISTCLEANFILES=
DISTFILES= Makefile.in asm_regnames.h ld.script setup.S
#------

all: 

.PHONY: build_xmboot

build_xmboot:
	@cd xmboot && $(MAKE)

# Nothing to do here:
dvi check installcheck:

install: ld.script
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir) $(DESTDIR)$(pkgincludedir)
	$(INSTALL_DATA) $(srcdir)/ld.script $(DESTDIR)$(pkgdatadir)/ld.script
	$(INSTALL_DATA) $(srcdir)/asm_regnames.h $(DESTDIR)$(pkgincludedir)/asm_regnames.h

clean:
	-rm -f $(CLEANFILES)
	@cd xmboot && $(MAKE) clean

distclean: clean
	-rm -f $(CLOBBERFILES)
	@cd xmboot && $(MAKE) distclean

maintainer-clean: distclean
	@cd xmboot && $(MAKE) maintainer-clean

%.o: %.s
	$(VMIPSTOOL) --assemble -o $@ $<

%.o: %.S
	$(VMIPSTOOL) --assemble -o $@ $<

# Snarfed from automake and modified.
distdir: $(DISTFILES)
	-rm -rf $(distdir)
	mkdir $(distdir)
	-chmod 777 $(distdir)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(distdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$d/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done
	for subdir in $(SUBDIRS); do \
	  if test "$$subdir" = .; then :; else \
	    test -d $(distdir)/$$subdir \
	    || mkdir $(distdir)/$$subdir \
	    || exit 1; \
	    chmod 777 $(distdir)/$$subdir; \
	    (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) top_distdir=../$(distdir) distdir=../$(distdir)/$$subdir distdir) \
	    || exit 1; \
	  fi; \
	done

