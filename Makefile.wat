#-----------------------------------------------
# MultiMail Makefile (top) for Open Watcom C++
#-----------------------------------------------

!include version

#--------------------------------------------------------------
# For Win32:

CURS_DIR = /watcom/pdcurs26
CURS_INC = \"$(CURS_DIR)/curses.h\"

LIBS = $(CURS_DIR)/win32/pdcurses.lib
COMPILER = "wpp386 -bt=nt -D__WIN32__ -DWIN32"
LINKER = wlink system nt

#--------------------------------------------------------------
# For 32-bit OS/2:

#LIBS = $(CURS_DIR)/os2/pdcurses.lib
#COMPILER = "wpp386 -bt=os2v2 -D__OS2__ -I/watcom/h/os2"
#LINKER = wlink system os2v2

#--------------------------------------------------------------
# For 16-bit DOS:

#LIBS = $(CURS_DIR)/dos/pdcurses.lib
#COMPILER = "wpp -bt=dos -ml -D__MSDOS__"
#LINKER = wlink system dos

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
	MM_MINOR=$(MM_MINOR) OPTS="-I$(CURS_DIR)" CURS_INC="$(CURS_INC)" intrfc
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
