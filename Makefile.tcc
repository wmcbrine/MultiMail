#-----------------------------------------------
# MultiMail Makefile (top) for Borland/Turbo C++
#-----------------------------------------------

!include version

#--------------------------------------------------------------
# Turbo C++ 3.0:

#CURS_INC = \\"/tc/pdcurs24/curses.h\\"
#LIBS = spawnl.lib \tc\pdcurs24\dos\pdcurses.lib
CURS_INC = \\"/pdcurs27/curses.h\\"
LIBS = spawnl.lib \pdcurs27\dos\pdcurses.lib
LIST = tclist
COMPILER = "$(CC) -c -P"

#--------------------------------------------------------------
#--------------------------------------------------------------

all:	mm

mm-main:
	cd mmail
	$(MAKE) -fMakefile.bcc -DCOMPILER=$(COMPILER) -DMM_MAJOR=$(MM_MAJOR) \
		-DMM_MINOR=$(MM_MINOR) mm-main
	cd ..

intrfc:
	cd interfac
	$(MAKE) -fMakefile.bcc -DCOMPILER=$(COMPILER) -DMM_MAJOR=$(MM_MAJOR) \
		-DMM_MINOR=$(MM_MINOR) -DCURS_INC="$(CURS_INC)" intrfc
	cd ..

mm:	mm-main intrfc
	$(CC) -emm @$(LIST) $(LIBS)

clean:
	del interfac\*.obj
	del mmail\*.obj
	del mm.exe

modclean:
	cd mmail
	$(MAKE) -fMakefile.bcc modclean
	cd ..
