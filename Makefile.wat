#-----------------------------------------------
# MultiMail Makefile (top) for Open Watcom C++
# (Currently set up only for Win32)
#-----------------------------------------------

!include version

#--------------------------------------------------------------
# Open Watcom 1.3:

CURS_INC = \"/watcom/pdcurs26/curses.h\"
CURS_DIR = /watcom/pdcurs26
LIBS = /watcom/pdcurs26/win32/pdcurses.lib

#--------------------------------------------------------------
#--------------------------------------------------------------

all:	mm.exe

mm.exe:
	cd mmail
	$(MAKE) -f Makefile.wat MM_MAJOR=$(MM_MAJOR) MM_MINOR=$(MM_MINOR) &
	mm-main
	cd ../interfac
	$(MAKE) -f Makefile.wat MM_MAJOR=$(MM_MAJOR) MM_MINOR=$(MM_MINOR) &
	OPTS="-I$(CURS_DIR)" CURS_INC="$(CURS_INC)" intrfc
	cd ..
	wlink system nt name mm.exe file mmail/*.obj,interfac/*.obj libfile &
	$(LIBS)

clean:	.symbolic
	del mmail\*.obj interfac\*.obj mm.exe

modclean:	.symbolic
	cd mmail
	$(MAKE) -f Makefile.wat modclean
	cd ..
