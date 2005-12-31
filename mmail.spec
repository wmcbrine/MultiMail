# Spec file originally by Greg Wildman <greg@techno.co.za>

%define	name	mmail
%define	version	0.47
%define	release	1
%define	prefix	/usr

Summary:	A QWK/Blue Wave offline mail reader for Linux
Name:		%{name}
Version:	%{version}
Release:	%{release}
Copyright:	GPL
Group:		Applications/Mail
URL:		http://multimail.sourceforge.net
Source:		%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-root

%description
MultiMail is an offline mail packet reader for Unix/Linux, MS-DOS, OS/2,
Windows, Mac OS X, and other systems, using a curses-based interface. It
supports the Blue Wave, QWK, OMEN, SOUP and OPX formats.

%prep
%setup -q 

%build
make

%install
rm -rf %{buildroot}
install -D mm %{buildroot}%{_bindir}/mm
install -D mm.1 %{buildroot}%{_mandir}/man1/mm.1

%clean
rm -rf %{buildroot}
rm -rf $RPM_BUILD_DIR/%{name}-%{version}

%post
ln -s %{_mandir}/man1/mm.1.gz %{_mandir}/man1/mmail.1.gz

%files
%defattr(-, root, root)
%doc README COPYING HISTORY INSTALL TODO FAQ colors/*
%{_bindir}/mm
%{_mandir}/man1/*
