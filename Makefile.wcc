#-----------------------------------------------
# MultiMail Makefile (top) for Open Watcom C++
#-----------------------------------------------

# For Windows:

CURS_DIR = /pdcurses

LIBS = $(CURS_DIR)/wincon/pdcurses.lib
COMPILER = wpp386 -zq -bt=nt -D__WIN32__ -DWIN32
LINKER = wlink system nt

#--------------------------------------------------------------
# For 32-bit OS/2:

!ifeq SYS OS2
LIBS = $(CURS_DIR)/os2/pdcurses.lib
COMPILER = wpp386 -zq -bt=os2v2 -D__OS2__
LINKER = wlink system os2v2
!endif

#--------------------------------------------------------------
# For 32-bit DOS:

!ifeq SYS DOS32
LIBS = $(CURS_DIR)/dos/pdcurses.lib
COMPILER = wpp386 -zq -bt=dos4g -mf -D__MSDOS__
LINKER = wlink system dos4g
!endif

#--------------------------------------------------------------
# For 16-bit DOS:

!ifeq SYS DOS16
LIBS = $(CURS_DIR)/dos/pdcurses.lib
COMPILER = wpp -zq -bt=dos -ml -D__MSDOS__
LINKER = wlink system dos
!endif

!ifdef __LOADDLL__
! loaddll wlink  wlinkd
! loaddll wlib   wlibd
! loaddll wpp    wppdi86
! loaddll wpp386 wppd386
!endif

O = obj

CPPFLAGS = -I$(CURS_DIR)

.cc:	mmail;interfac
.cc.obj:	.autodepend
	$(COMPILER) $(CPPFLAGS) $<

MOBJS = misc.$(O) resource.$(O) mmail.$(O) driverl.$(O) filelist.$(O) &
area.$(O) letter.$(O) read.$(O) compress.$(O) pktbase.$(O) bw.$(O) &
qwk.$(O) omen.$(O) soup.$(O) opx.$(O)

IOBJS = mmcolor.$(O) mysystem.$(O) isoconv.$(O) basic.$(O) interfac.$(O) &
packet.$(O) arealist.$(O) letterl.$(O) letterw.$(O) lettpost.$(O) &
ansiview.$(O) addrbook.$(O) tagline.$(O) help.$(O) main.$(O)

all:	mm.exe

mm.exe:	$(MOBJS) $(IOBJS)
	$(LINKER) name mm.exe file *.obj libfile $(LIBS)

clean
	del *.obj
	del mm.exe
