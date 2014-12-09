# Generated automatically from Makefile.in by configure.

SHELL = /bin/sh

srcdir = /home/zhx/MulticoreCompiler-12月26日星期五/prophet_vmips/doc
top_srcdir = /home/zhx/MulticoreCompiler-12月26日星期五/prophet_vmips
VPATH = /home/zhx/MulticoreCompiler-12月26日星期五/prophet_vmips/doc
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

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
transform = s,x,x,

DOC = README
TOC = toc
OPTDOC = options.texi
OPTIONTBLH = $(top_srcdir)/optiontbl.h
# This is less than optimal. We could consolidate CHAPTERS and DISTFILES
# more than they are currently.
CHAPTERS = $(srcdir)/to-do
CLEANFILES = vmips.log vmips.aux vmips.cp vmips.fn vmips.vr \
 vmips.tp vmips.ky vmips.pg vmips.toc vmips.cps vmips.fns vmips.pgs vmips.pod
DISTCLEANFILES = 
USERDOCS = $(DOC) $(OPTDOC) $(TOC) vmips.info vmips.html \
 vmips.pdf $(MANPAGES)
MAINTAINERCLEANFILES = $(USERDOCS) vmips.pod vmipstool.pod
DISTFILES = Makefile.in to-do vmips.texi invoke.texi \
 $(USERDOCS) options.texi vmipstool.texi texinfo.tex copyright.texi \
 rel-checklist.txt
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
MANSECT = 1
vmips_MANPAGE = vmips.$(MANSECT)
vmipstool_MANPAGE = vmipstool.$(MANSECT)
MANPAGES = $(vmips_MANPAGE) $(vmipstool_MANPAGE)

all: $(DOC) dvi info html man pdf

# Nothing to do here (yet):
check installcheck:

install: vmips.info $(MANPAGES)
	$(mkinstalldirs) $(DESTDIR)$(infodir)
	$(INSTALL_DATA) $(srcdir)/vmips.info $(DESTDIR)$(infodir)/`echo vmips.info|sed '$(transform)'`
	-$(mkinstalldirs) $(DESTDIR)$(mandir)
	-$(mkinstalldirs) $(DESTDIR)$(mandir)/man$(MANSECT)
	-$(INSTALL_DATA) $(srcdir)/$(vmips_MANPAGE) $(DESTDIR)$(mandir)/man$(MANSECT)/`echo vmips.$(MANSECT)|sed '$(transform)'`
	-$(INSTALL_DATA) $(srcdir)/$(vmipstool_MANPAGE) $(DESTDIR)$(mandir)/man$(MANSECT)/`echo vmipstool.$(MANSECT)|sed '$(transform)'`

pdf: vmips.pdf

vmips.pdf: $(srcdir)/vmips.texi $(OPTDOC) texinfo.tex
	cd $(srcdir) && texi2dvi -p vmips.texi

man: $(MANPAGES)

$(vmips_MANPAGE): invoke.texi options.texi
	perl $(srcdir)/texi2pod.pl $< > vmips.pod
	pod2man --release="vmips "`cat $(top_srcdir)/VERSION` \
		--center="VMIPS Programmer's Manual" vmips.pod > $@

$(vmipstool_MANPAGE): vmipstool.texi
	perl $(srcdir)/texi2pod.pl $< > vmipstool.pod
	pod2man --release="vmips "`cat $(top_srcdir)/VERSION` \
		--center="VMIPS Programmer's Manual" vmipstool.pod > $@

ps: vmips.ps

vmips.ps: vmips.dvi
	cd $(srcdir) && dvips -o vmips.ps vmips.dvi

dvi: vmips.dvi

vmips.dvi: $(srcdir)/vmips.texi $(OPTDOC) texinfo.tex
	cd $(srcdir) && texi2dvi vmips.texi

info: vmips.info

vmips.info: $(srcdir)/vmips.texi $(OPTDOC)
	cd $(srcdir) && makeinfo --no-split vmips.texi

html: vmips.html

vmips.html: $(srcdir)/vmips.texi $(OPTDOC)
	cd $(srcdir) && makeinfo --no-split --html vmips.texi

$(DOC): $(TOC) $(CHAPTERS)
	cd $(srcdir) && cat $(TOC) $(CHAPTERS) > $(srcdir)/$(DOC)

$(TOC): $(CHAPTERS)
	cd $(srcdir) && ./maketoc -o $(TOC) $(CHAPTERS)

$(OPTDOC): $(OPTIONTBLH)
	cd $(srcdir) && ./makeoptdoc $(OPTDOC)

clean:
	-rm -f $(CLEANFILES)

distclean: clean
	-rm -f $(DISTCLEANFILES)

maintainer-clean: distclean clean
	-rm -f $(MAINTAINERCLEANFILES)

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

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
