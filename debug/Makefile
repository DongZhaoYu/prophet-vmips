# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.4-p5 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999, 2001 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


SHELL = /bin/sh

srcdir = /home/zhx/MulticoreCompiler-12月26日星期五/prophet_vmips
top_srcdir = /home/zhx/MulticoreCompiler-12月26日星期五/prophet_vmips
VPATH = /home/zhx/MulticoreCompiler-12月26日星期五/prophet_vmips
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

top_builddir = .

ACLOCAL = aclocal
AUTOCONF = autoconf
AUTOMAKE = automake
AUTOHEADER = autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
build_alias = i686-pc-linux-gnu
build_triplet = i686-pc-linux-gnu
host_alias = i686-pc-linux-gnu
host_triplet = i686-pc-linux-gnu
target_alias = i686-pc-linux-gnu
target_triplet = i686-pc-linux-gnu
AS = 
ASFLAGS = 
AWK = mawk
BIGENDIAN = nobigendian
CC = gcc
CCAS = 
CCASFLAGS = 
CPP = gcc -E
CXX = c++
EXEEXT = 
LDFLAGS =  -s
LN_S = ln -s
MAKEINFO = makeinfo
MIPSTOOLPREFIX = mipstoolprefix=''
PACKAGE = vmips
RANLIB = ranlib
SAMPLE_CODE_TARGETS = 
SOLARIS_DL_HACK = @SOLARIS_DL_HACK@
TESTCODE_DIR = 
VERSION = 1.3.2
mipsbin = /opt/mips/bin

SUBDIRS = libopcodes_mips . sample_code  doc
REDHAT_PKG_FILES = vmips.spec vmips.spec.in
DEBIAN_PKG_FILES = debian/changelog debian/conffiles debian/control  debian/copyright debian/dirs debian/docs debian/info debian/rules

EXTRA_DIST = VERSION $(REDHAT_PKG_FILES)  $(sysconf_DATA) $(DEBIAN_PKG_FILES)


# What are we building here?
bin_PROGRAMS = vmips vmipstool

# Defaults! Defaults for everyone!
CXXFLAGS = -I$(top_srcdir)/libopcodes_mips -O0 -g3 -Wall -fno-strict-aliasing  -DSYSCONFDIR=\"$(sysconfdir)\" -DPKGDATADIR=\"$(pkgdatadir)\"

MOSTLYCLEANFILES = errs memdump.bin
CLEANFILES = romfile.rom vmips.out

# How to build the simulator:
vmips_SOURCES = cpu.cc cpzero.cc devicemap.cc   mapper.cc options.cc range.cc intctrl.cc   spimconsole.cc stub-dis.cc vmips.cc   deviceint.cc debug.cc remotegdb.cc clockdev.cc error.cc   clock.cc terminalcontroller.cc haltdev.cc decrtc.cc deccsr.cc   decstat.cc decserial.cc rommodule.cc fileutils.cc exeloader.cc speculativecpu.cc speculativelogic.cc prophetlog.cc speculativecpu.h speculativelogic.h prophetlog.h predefine.h   cpu.h cpzero.h cpzeroreg.h deviceint.h   devicemap.h intctrl.h mapper.h memorymodule.h options.h optiontbl.h   range.h spimconsole.h spimconsreg.h   tlbentry.h vmips.h debug.h remotegdb.h   accesstypes.h deviceexc.h clockdev.h excnames.h error.h   clock.h task.h terminalcontroller.h haltdev.h devreg.h clockreg.h   haltreg.h wipe.h stub-dis.h decrtc.h decrtcreg.h deccsr.h deccsrreg.h   decstat.h decserial.h decserialreg.h rommodule.h gccattr.h mmapglue.h   types.h endiantest.h fileutils.h

vmips_LDADD = libopcodes_mips/libopcodes_mips.a

# Huh, huhuh, it says tool.
vmipstool_SOURCES = vmipstool.cc options.cc error.cc stub-dis.cc   fileutils.cc options.h error.h stub-dis.h endiantest.h fileutils.h

vmipstool_LDADD = libopcodes_mips/libopcodes_mips.a

# Install a sample vmipsrc.
sysconf_DATA = vmipsrc
ACLOCAL_M4 = $(top_srcdir)/aclocal.m4
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = config.h
CONFIG_CLEAN_FILES =  vmips.spec vmipsrc
bin_PROGRAMS =  vmips$(EXEEXT) vmipstool$(EXEEXT)
PROGRAMS =  $(bin_PROGRAMS)


DEFS = -DHAVE_CONFIG_H -I. -I$(srcdir) -I.
CPPFLAGS = 
LIBS = 
vmips_OBJECTS =  cpu.o cpzero.o devicemap.o mapper.o options.o range.o \
intctrl.o spimconsole.o stub-dis.o vmips.o deviceint.o debug.o \
remotegdb.o clockdev.o error.o clock.o terminalcontroller.o haltdev.o \
decrtc.o deccsr.o decstat.o decserial.o rommodule.o fileutils.o \
exeloader.o
vmips_DEPENDENCIES =  libopcodes_mips/libopcodes_mips.a
vmips_LDFLAGS = 
vmipstool_OBJECTS =  vmipstool.o options.o error.o stub-dis.o \
fileutils.o
vmipstool_DEPENDENCIES =  libopcodes_mips/libopcodes_mips.a
vmipstool_LDFLAGS = 
CXXCOMPILE = $(CXX) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)
CXXLD = $(CXX)
CXXLINK = $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@
CFLAGS = -g -O2 -Wall -fno-strict-aliasing
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@
DATA =  $(sysconf_DATA)

DIST_COMMON =  README ./stamp-h.in AUTHORS COPYING ChangeLog INSTALL \
Makefile.am Makefile.in NEWS THANKS acinclude.m4 aclocal.m4 \
config.guess config.h.in config.sub configure configure.in install-sh \
missing mkinstalldirs vmips.spec.in vmipsrc.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = gnutar
GZIP_ENV = --best
SOURCES = $(vmips_SOURCES) $(vmipstool_SOURCES)
OBJECTS = $(vmips_OBJECTS) $(vmipstool_OBJECTS)

all: all-redirect
.SUFFIXES:
.SUFFIXES: .S .c .cc .o .s
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu --include-deps Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status
	cd $(top_builddir) \
	  && CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

$(ACLOCAL_M4):  configure.in  acinclude.m4
	cd $(srcdir) && $(ACLOCAL)

config.status: $(srcdir)/configure $(CONFIG_STATUS_DEPENDENCIES)
	$(SHELL) ./config.status --recheck
$(srcdir)/configure: $(srcdir)/configure.in $(ACLOCAL_M4) $(CONFIGURE_DEPENDENCIES)
	cd $(srcdir) && $(AUTOCONF)

config.h: stamp-h
	@if test ! -f $@; then \
		rm -f stamp-h; \
		$(MAKE) stamp-h; \
	else :; fi
stamp-h: $(srcdir)/config.h.in $(top_builddir)/config.status
	cd $(top_builddir) \
	  && CONFIG_FILES= CONFIG_HEADERS=config.h \
	     $(SHELL) ./config.status
	@echo timestamp > stamp-h 2> /dev/null
$(srcdir)/config.h.in: $(srcdir)/stamp-h.in
	@if test ! -f $@; then \
		rm -f $(srcdir)/stamp-h.in; \
		$(MAKE) $(srcdir)/stamp-h.in; \
	else :; fi
$(srcdir)/stamp-h.in: $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOHEADER)
	@echo timestamp > $(srcdir)/stamp-h.in 2> /dev/null

mostlyclean-hdr:

clean-hdr:

distclean-hdr:
	-rm -f config.h

maintainer-clean-hdr:
vmips.spec: $(top_builddir)/config.status vmips.spec.in
	cd $(top_builddir) && CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status
vmipsrc: $(top_builddir)/config.status vmipsrc.in
	cd $(top_builddir) && CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

mostlyclean-binPROGRAMS:

clean-binPROGRAMS:
	-test -z "$(bin_PROGRAMS)" || rm -f $(bin_PROGRAMS)

distclean-binPROGRAMS:

maintainer-clean-binPROGRAMS:

install-binPROGRAMS: $(bin_PROGRAMS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	@list='$(bin_PROGRAMS)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo "  $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`"; \
	     $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	  else :; fi; \
	done

uninstall-binPROGRAMS:
	@$(NORMAL_UNINSTALL)
	list='$(bin_PROGRAMS)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	done

.c.o:
	$(COMPILE) -c $<

.s.o:
	$(COMPILE) -c $<

.S.o:
	$(COMPILE) -c $<

mostlyclean-compile:
	-rm -f *.o core *.core

clean-compile:

distclean-compile:
	-rm -f *.tab.c

maintainer-clean-compile:

vmips$(EXEEXT): $(vmips_OBJECTS) $(vmips_DEPENDENCIES)
	@rm -f vmips$(EXEEXT)
	$(CXXLINK) $(vmips_LDFLAGS) $(vmips_OBJECTS) $(vmips_LDADD) $(LIBS)

vmipstool$(EXEEXT): $(vmipstool_OBJECTS) $(vmipstool_DEPENDENCIES)
	@rm -f vmipstool$(EXEEXT)
	$(CXXLINK) $(vmipstool_LDFLAGS) $(vmipstool_OBJECTS) $(vmipstool_LDADD) $(LIBS)
.cc.o:
	$(CXXCOMPILE) -c $<

install-sysconfDATA: $(sysconf_DATA)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(sysconfdir)
	@list='$(sysconf_DATA)'; for p in $$list; do \
	  if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(sysconfdir)/$$p"; \
	    $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(sysconfdir)/$$p; \
	  else if test -f $$p; then \
	    echo " $(INSTALL_DATA) $$p $(DESTDIR)$(sysconfdir)/$$p"; \
	    $(INSTALL_DATA) $$p $(DESTDIR)$(sysconfdir)/$$p; \
	  fi; fi; \
	done

uninstall-sysconfDATA:
	@$(NORMAL_UNINSTALL)
	list='$(sysconf_DATA)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(sysconfdir)/$$p; \
	done

# This directory's subdirectories are mostly independent; you can cd
# into them and run `make' without going through this Makefile.
# To change the values of `make' variables: instead of editing Makefiles,
# (1) if the variable is set in `config.status', edit `config.status'
#     (which will cause the Makefiles to be regenerated when you run `make');
# (2) otherwise, pass the desired values on the `make' command line.



all-recursive install-data-recursive install-exec-recursive \
installdirs-recursive install-recursive uninstall-recursive  \
check-recursive installcheck-recursive info-recursive dvi-recursive:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	dot_seen=no; \
	target=`echo $@ | sed s/-recursive//`; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    dot_seen=yes; \
	    local_target="$$target-am"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done; \
	if test "$$dot_seen" = "no"; then \
	  $(MAKE) $(AM_MAKEFLAGS) "$$target-am" || exit 1; \
	fi; test -z "$$fail"

mostlyclean-recursive clean-recursive distclean-recursive \
maintainer-clean-recursive:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	dot_seen=no; \
	rev=''; list='$(SUBDIRS)'; for subdir in $$list; do \
	  rev="$$subdir $$rev"; \
	  test "$$subdir" != "." || dot_seen=yes; \
	done; \
	test "$$dot_seen" = "no" && rev=". $$rev"; \
	target=`echo $@ | sed s/-recursive//`; \
	for subdir in $$rev; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    local_target="$$target-am"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"
tags-recursive:
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  test "$$subdir" = . || (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) tags); \
	done

ID: $(HEADERS) $(SOURCES) $(LISP)
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	here=`pwd` && cd $(srcdir) \
	  && mkid -f$$here/ID $$unique $(LISP)

TAGS: tags-recursive $(HEADERS) $(SOURCES) config.h.in $(TAGS_DEPENDENCIES) $(LISP)
	tags=; \
	here=`pwd`; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
   if test "$$subdir" = .; then :; else \
	    test -f $$subdir/TAGS && tags="$$tags -i $$here/$$subdir/TAGS"; \
   fi; \
	done; \
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	test -z "$(ETAGS_ARGS)config.h.in$$unique$(LISP)$$tags" \
	  || (cd $(srcdir) && etags $(ETAGS_ARGS) $$tags config.h.in $$unique $(LISP) -o $$here/TAGS)

mostlyclean-tags:

clean-tags:

distclean-tags:
	-rm -f TAGS ID

maintainer-clean-tags:

distdir = $(PACKAGE)-$(VERSION)
top_distdir = $(distdir)

# This target untars the dist file and tries a VPATH configuration.  Then
# it guarantees that the distribution is self-contained by making another
# tarfile.
distcheck: dist
	-rm -rf $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) zxf $(distdir).tar.gz
	mkdir $(distdir)/=build
	mkdir $(distdir)/=inst
	dc_install_base=`cd $(distdir)/=inst && pwd`; \
	cd $(distdir)/=build \
	  && ../configure --srcdir=.. --prefix=$$dc_install_base \
	  && $(MAKE) $(AM_MAKEFLAGS) \
	  && $(MAKE) $(AM_MAKEFLAGS) dvi \
	  && $(MAKE) $(AM_MAKEFLAGS) check \
	  && $(MAKE) $(AM_MAKEFLAGS) install \
	  && $(MAKE) $(AM_MAKEFLAGS) installcheck \
	  && $(MAKE) $(AM_MAKEFLAGS) dist
	-rm -rf $(distdir)
	@banner="$(distdir).tar.gz is ready for distribution"; \
	dashes=`echo "$$banner" | sed s/./=/g`; \
	echo "$$dashes"; \
	echo "$$banner"; \
	echo "$$dashes"
dist: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) chozf $(distdir).tar.gz $(distdir)
	-rm -rf $(distdir)
dist-all: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) chozf $(distdir).tar.gz $(distdir)
	-rm -rf $(distdir)
distdir: $(DISTFILES)
	-rm -rf $(distdir)
	mkdir $(distdir)
	-chmod 777 $(distdir)
	$(mkinstalldirs) $(distdir)/debian
	@for file in $(DISTFILES); do \
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
clock.o: clock.cc clock.h task.h types.h config.h error.h gccattr.h \
	wipe.h
clockdev.o: clockdev.cc clockdev.h clock.h task.h types.h config.h \
	deviceint.h intctrl.h devicemap.h range.h accesstypes.h \
	devreg.h
cpu.o: cpu.cc cpu.h deviceexc.h accesstypes.h types.h config.h cpzero.h \
	tlbentry.h cpzeroreg.h debug.h mapper.h range.h vmips.h \
	options.h excnames.h error.h gccattr.h remotegdb.h stub-dis.h \
	libopcodes_mips/bfd.h libopcodes_mips/ansidecl.h \
	libopcodes_mips/symcat.h libopcodes_mips/dis-asm.h \
	libopcodes_mips/bfd.h
cpzero.o: cpzero.cc cpzero.h tlbentry.h config.h cpzeroreg.h types.h \
	mapper.h range.h accesstypes.h excnames.h cpu.h deviceexc.h \
	intctrl.h error.h gccattr.h
debug.o: debug.cc debug.h deviceexc.h accesstypes.h types.h config.h \
	remotegdb.h cpu.h mapper.h range.h excnames.h cpzeroreg.h \
	vmips.h options.h
deccsr.o: deccsr.cc deccsrreg.h deccsr.h devicemap.h range.h \
	accesstypes.h types.h config.h deviceint.h intctrl.h
decrtc.o: decrtc.cc devicemap.h range.h accesstypes.h types.h config.h \
	clock.h task.h decrtcreg.h decrtc.h deviceint.h intctrl.h
decserial.o: decserial.cc cpu.h deviceexc.h accesstypes.h types.h \
	config.h deccsr.h devicemap.h range.h deviceint.h intctrl.h \
	decserial.h decserialreg.h terminalcontroller.h devreg.h task.h \
	mapper.h vmips.h
decstat.o: decstat.cc deviceexc.h accesstypes.h types.h config.h cpu.h \
	decstat.h devicemap.h range.h mapper.h vmips.h
deviceint.o: deviceint.cc deviceint.h intctrl.h types.h config.h \
	options.h vmips.h
devicemap.o: devicemap.cc accesstypes.h range.h types.h config.h \
	devicemap.h
error.o: error.cc error.h gccattr.h config.h
exeloader.o: exeloader.cc vmips.h types.h config.h cpzeroreg.h \
	memorymodule.h range.h accesstypes.h error.h gccattr.h
fileutils.o: fileutils.cc
haltdev.o: haltdev.cc haltdev.h devicemap.h range.h accesstypes.h \
	types.h config.h vmips.h
intctrl.o: intctrl.cc deviceint.h intctrl.h types.h config.h
mapper.o: mapper.cc cpu.h deviceexc.h accesstypes.h types.h config.h \
	devicemap.h range.h error.h gccattr.h excnames.h mapper.h \
	memorymodule.h options.h vmips.h
options.o: options.cc error.h gccattr.h config.h fileutils.h options.h \
	types.h optiontbl.h
range.o: range.cc range.h accesstypes.h types.h config.h error.h \
	gccattr.h
remotegdb.o: remotegdb.cc remotegdb.h types.h config.h
rommodule.o: rommodule.cc rommodule.h range.h accesstypes.h types.h \
	config.h vmips.h mmapglue.h
spimconsole.o: spimconsole.cc clock.h task.h types.h config.h mapper.h \
	range.h accesstypes.h spimconsole.h deviceint.h intctrl.h \
	devicemap.h terminalcontroller.h devreg.h spimconsreg.h vmips.h
stub-dis.o: stub-dis.cc stub-dis.h types.h config.h \
	libopcodes_mips/bfd.h libopcodes_mips/ansidecl.h \
	libopcodes_mips/symcat.h libopcodes_mips/dis-asm.h \
	libopcodes_mips/bfd.h
terminalcontroller.o: terminalcontroller.cc clock.h task.h types.h \
	config.h error.h gccattr.h terminalcontroller.h devreg.h
speculativecpu.o: speculativecpu.cc speculativecpu.h cpu.h predefine.h prophetlog.h
speculativelogic.o: speculativelogic.cc speculativelogic.h 
prophetlog.o: prophetlog.cc prophetlog.h
vmips.o: vmips.cc clock.h task.h types.h config.h clockdev.h deviceint.h \
	intctrl.h devicemap.h range.h accesstypes.h devreg.h clockreg.h \
	cpzeroreg.h debug.h deviceexc.h error.h gccattr.h endiantest.h \
	haltreg.h haltdev.h spimconsole.h terminalcontroller.h mapper.h \
	memorymodule.h cpu.h cpzero.h tlbentry.h spimconsreg.h vmips.h \
	options.h decrtc.h decrtcreg.h deccsr.h deccsrreg.h decstat.h \
	decserial.h decserialreg.h stub-dis.h libopcodes_mips/bfd.h \
	libopcodes_mips/ansidecl.h libopcodes_mips/symcat.h \
	libopcodes_mips/dis-asm.h libopcodes_mips/bfd.h rommodule.h
vmipstool.o: vmipstool.cc endiantest.h types.h config.h fileutils.h \
	stub-dis.h libopcodes_mips/bfd.h libopcodes_mips/ansidecl.h \
	libopcodes_mips/symcat.h libopcodes_mips/dis-asm.h \
	libopcodes_mips/bfd.h error.h gccattr.h options.h

info-am:
info: info-recursive
dvi-am:
dvi: dvi-recursive
check-am: all-am
check: check-recursive
installcheck-am:
installcheck: installcheck-recursive
all-recursive-am: config.h
	$(MAKE) $(AM_MAKEFLAGS) all-recursive

install-exec-am: install-binPROGRAMS install-sysconfDATA
install-exec: install-exec-recursive

install-data-am:
install-data: install-data-recursive

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-recursive
uninstall-am: uninstall-binPROGRAMS uninstall-sysconfDATA
uninstall: uninstall-recursive
all-am: Makefile $(PROGRAMS) $(DATA) config.h
all-redirect: all-recursive-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs: installdirs-recursive
installdirs-am:
	$(mkinstalldirs)  $(DESTDIR)$(bindir) $(DESTDIR)$(sysconfdir)


mostlyclean-generic:
	-test -z "$(MOSTLYCLEANFILES)" || rm -f $(MOSTLYCLEANFILES)

clean-generic:
	-test -z "$(CLEANFILES)" || rm -f $(CLEANFILES)

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-hdr mostlyclean-binPROGRAMS \
		mostlyclean-compile mostlyclean-tags \
		mostlyclean-generic

mostlyclean: mostlyclean-recursive

clean-am:  clean-hdr clean-binPROGRAMS clean-compile clean-tags \
		clean-generic mostlyclean-am

clean: clean-recursive

distclean-am:  distclean-hdr distclean-binPROGRAMS distclean-compile \
		distclean-tags distclean-generic clean-am

distclean: distclean-recursive
	-rm -f config.status

maintainer-clean-am:  maintainer-clean-hdr maintainer-clean-binPROGRAMS \
		maintainer-clean-compile maintainer-clean-tags \
		maintainer-clean-generic distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-recursive
	-rm -f config.status

.PHONY: mostlyclean-hdr distclean-hdr clean-hdr maintainer-clean-hdr \
mostlyclean-binPROGRAMS distclean-binPROGRAMS clean-binPROGRAMS \
maintainer-clean-binPROGRAMS uninstall-binPROGRAMS install-binPROGRAMS \
mostlyclean-compile distclean-compile clean-compile \
maintainer-clean-compile uninstall-sysconfDATA install-sysconfDATA \
install-data-recursive uninstall-data-recursive install-exec-recursive \
uninstall-exec-recursive installdirs-recursive uninstalldirs-recursive \
all-recursive check-recursive installcheck-recursive info-recursive \
dvi-recursive mostlyclean-recursive distclean-recursive clean-recursive \
maintainer-clean-recursive tags tags-recursive mostlyclean-tags \
distclean-tags clean-tags maintainer-clean-tags distdir info-am info \
dvi-am dvi check check-am installcheck-am installcheck all-recursive-am \
install-exec-am install-exec install-data-am install-data install-am \
install uninstall-am uninstall all-redirect all-am all installdirs-am \
installdirs mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean


# This library contains the subset of libbfd/libopcodes that we use.
# We build libopcodes_mips before trying to link vmips or vmipstool.
libopcodes_mips/libopcodes_mips.a:
	cd libopcodes_mips && $(MAKE) all

# Miscellaneous targets.
tags: TAGS
	ctags *.cc

gcov:
	@for f in $(vmips_SOURCES); do echo $$f; gcov $$f; done

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
