#-----------------------------------------------
# MultiMail Makefile (top) for Open Watcom C++
#-----------------------------------------------

!include version

#--------------------------------------------------------------
# For Windows:

CURS_DIR = /pdcurses

LIBS = $(CURS_DIR)/wincon/pdcurses.lib
COMPILER = "wpp386 -zq -bt=nt -D__WIN32__ -DWIN32"
LINKER = wlink system nt

#--------------------------------------------------------------
# For 32-bit OS/2:

!ifeq OS2 Y
LIBS = $(CURS_DIR)/os2/pdcurses.lib
COMPILER = "wpp386 -zq -bt=os2v2 -D__OS2__"
LINKER = wlink system os2v2
!endif

#--------------------------------------------------------------
# For 32-bit DOS:

!ifeq DOS32 Y
LIBS = $(CURS_DIR)/dos/pdcurses.lib
COMPILER = "wpp386 -zq -bt=dos4g -mf -D__MSDOS__"
LINKER = wlink system dos4g
!endif

#--------------------------------------------------------------
# For 16-bit DOS:

!ifeq DOS16 Y
LIBS = $(CURS_DIR)/dos/pdcurses.lib
COMPILER = "wpp -zq -bt=dos -ml -D__MSDOS__"
LINKER = wlink system dos
!endif

#--------------------------------------------------------------
#--------------------------------------------------------------

all:	mm.exe

mm-main
	cd mmail
	$(MAKE) -f Makefile.wat COMPILER=$(COMPILER) MM_MAJOR=$(MM_MAJOR) &
	MM_MINOR=$(MM_MINOR) mm-main
	cd ..

intrfc
	cd interfac
	$(MAKE) -f Makefile.wat COMPILER=$(COMPILER) MM_MAJOR=$(MM_MAJOR) &
	MM_MINOR=$(MM_MINOR) CURS_DIR="$(CURS_DIR)" intrfc
	cd ..

mm.exe:	mm-main intrfc
	$(LINKER) name mm.exe file mmail/*.obj,interfac/*.obj libfile $(LIBS)

clean
	del mmail\*.obj
	del interfac\*.obj
	del mm.exe

modclean
	cd mmail
	$(MAKE) -f Makefile.wat modclean
	cd ..
