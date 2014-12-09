## Process this file with automake to produce Makefile.in.

SUBDIRS = libopcodes_mips . sample_code @TESTCODE_DIR@ doc
REDHAT_PKG_FILES = vmips.spec vmips.spec.in
DEBIAN_PKG_FILES = debian/changelog debian/conffiles debian/control \
 debian/copyright debian/dirs debian/docs debian/info debian/rules
EXTRA_DIST = VERSION $(REDHAT_PKG_FILES) \
 $(sysconf_DATA) $(DEBIAN_PKG_FILES)

# What are we building here?
bin_PROGRAMS = prophet_vmips vmipstool

# Defaults! Defaults for everyone!
AM_CXXFLAGS = -I$(top_srcdir)/libopcodes_mips @CXXFLAGS@ \
 -DSYSCONFDIR=\"$(sysconfdir)\" -DPKGDATADIR=\"$(pkgdatadir)\"
MOSTLYCLEANFILES = errs memdump.bin
CLEANFILES = romfile.rom vmips.out

# How to build the simulator:
prophet_vmips_SOURCES = accesstypes.h clock.cc clock.h clockdev.cc clockdev.h \
	clockreg.h cpu.cc cpu.h cpzero.cc cpzero.h cpzeroreg.h debug.cc debug.h deccsr.cc \
	deccsr.h deccsrreg.h decrtc.cc decrtc.h decrtcreg.h decserial.cc decserial.h \
	decserialreg.h decstat.cc decstat.h deviceexc.h deviceint.cc deviceint.h devicemap.cc \
	devicemap.h devreg.h endiantest.h error.cc error.h excnames.h exeloader.cc \
	fileutils.cc fileutils.h gccattr.h haltdev.cc haltdev.h haltreg.h intctrl.cc \
	intctrl.h mapper.cc mapper.h memorymodule.h mmapglue.h options.cc options.h \
	optiontbl.h predefine.h prophetlog.cc prophetlog.h range.cc range.h remotegdb.cc \
	remotegdb.h rommodule.cc rommodule.h speculativecpu.cc speculativecpu.h \
	speculativelogic.cc speculativelogic.h spimconsole.cc spimconsole.h spimconsreg.h \
	stub-dis.cc stub-dis.h task.h terminalcontroller.cc terminalcontroller.h tlbentry.h \
	prophetfpu.cc prophetfpu.h \
	prophetconsole.cc prophetconsole.h prophetxmlelement.cc prophetxmlelement.h \
	prophetxmldoc.cc prophetxmldoc.h prophetstatistic.cc prophetstatistic.h \
	types.h vmips.cc vmips.h wipe.h prophet_vmips.cc prophet_vmips.h
prophet_vmips_LDADD = $(top_builddir)/libopcodes_mips/libopcodes_mips.a \
	libopcodes_mips/libopcodes_mips.a

# This library contains the subset of libbfd/libopcodes that we use.
# We build libopcodes_mips before trying to link prophet_vmips or vmipstool.
libopcodes_mips/libopcodes_mips.a:
	cd libopcodes_mips && $(MAKE) all

# Huh, huhuh, it says tool.
vmipstool_SOURCES = vmipstool.cc options.cc error.cc stub-dis.cc \
  fileutils.cc options.h error.h stub-dis.h endiantest.h fileutils.h
vmipstool_LDADD = libopcodes_mips/libopcodes_mips.a

# Install a sample vmipsrc.
sysconf_DATA = vmipsrc inst_cost.ini

# Miscellaneous targets.
tags: TAGS
	ctags *.cc

gcov:
	@for f in $(prophet_vmips_SOURCES); do echo $$f; gcov $$f; done
noinst_HEADERS =
