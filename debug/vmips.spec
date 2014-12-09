# vmips RPM spec file.
Name: vmips
Summary: A MIPS-based virtual machine simulator.
Version: 1.3.2
Release: 1
Copyright: GPL
Group: Emulators
URL: http://www.dgate.org/vmips/
Source: ftp://ftp.dgate.org/pub/vmips/releases/vmips-1.3.2/vmips-1.3.2.tar.gz
Packager: VMIPS Maintainers <vmips@dgate.org>
BuildRoot: %{_tmppath}/vmips-buildroot

%description
VMIPS is a software simulator for a virtual machine based on a MIPS R3000 CPU.

%prep
%setup -q
%configure --sysconfdir=/etc

%build
make

%install
%makeinstall sysconfdir=$RPM_BUILD_ROOT/etc

%files
%doc AUTHORS COPYING ChangeLog INSTALL NEWS README THANKS VERSION
%{_bindir}/vmips
%{_bindir}/vmipstool
%{_infodir}/vmips.info.gz
%{_mandir}/man1/vmips.1.gz
%{_mandir}/man1/vmipstool.1.gz
%{_includedir}/vmips/asm_regnames.h
%{_datadir}/vmips/ld.script
%config /etc/vmipsrc

%changelog
* Fri Oct 08 2004 VMIPS Maintainers <vmips@dgate.org>
- Use _variables instead of hardcoded paths in files section
- Fix source url

* Mon Jun 21 2004 VMIPS Maintainers <vmips@dgate.org>
- Use a buildroot and standard 'configure' and 'makeinstall' macros.
- Get rid of mipsel-ecoff configuration flags.
- Gzip man pages & info

* Fri Jun 18 2004 VMIPS Maintainers <vmips@dgate.org>
- Add vmipstool man page.

* Fri Feb 21 2003 VMIPS Maintainers <vmips@dgate.org>
- Add configuration information for mipsel-ecoff.

* Mon Jul 22 2002 VMIPS Maintainers <vmips@dgate.org>
- Update hostnames.

* Sun Jun 17 2001 VMIPS Maintainers <vmips@dgate.org>
- Move vmipsrc to /etc, and make it a config file

* Sun Jun 17 2001 VMIPS Maintainers <vmips@dgate.org>
- Add changelog to RPM spec file.
- Change maintainer address to vmips@dgate.org everywhere.

* Sun Jun 17 2001 VMIPS Maintainers <vmips@dgate.org>
- Add man page to installation.

* Sun Jun 17 2001 VMIPS Maintainers <vmips@dgate.org>
- Get rid of some directories, to quash the "File listed twice" errors from RPM.

* Sat Jun 16 2001 VMIPS Maintainers <vmips@dgate.org>
- Add include/vmips, include/vmips/asm_regnames.h to package.

* Tue Jun 05 2001 VMIPS Maintainers <vmips@dgate.org>
- Add pkgdatadir files to package: ld.script and vmipsrc.

* Mon Jun 04 2001 VMIPS Maintainers <vmips@dgate.org>
- Set prefix to /usr (as with other RPMs.)
- Add "doc" files to %doc line: AUTHORS, COPYING, ChangeLog, INSTALL,
- NEWS, README, THANKS, and VERSION.
- Add vmips.info to package.

* Tue May 29 2001 VMIPS Maintainers <vmips@dgate.org>
- Add vmipstool to package.
- Moved up from package directory to toplevel.

* Tue Aug 15 2000 VMIPS Maintainers <vmips@dgate.org>
- Original RPM spec file for vmips. 
- Needs to be munged with autoconf to contain correct VERSION.

