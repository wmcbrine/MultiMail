#-----------------------------------
# MultiMail Makefile (top) for RSXNT
#-----------------------------------

include version

# General options (passed to mmail/Makefile and interfac/Makefile):

# With debug:
#OPTS = -g -Wall -pedantic -Zwin32

# Optimized, no debug:
OPTS = -O2 -Wall -pedantic -Zwin32

#--------------------------------------------------------------
# RSXNT (Win32), with GNU make, and PDCurses 2.3:
# Note: If you get "g++: Command not found", then type "set cxx=gcc"
# before running make.

CURS_INC = \\\"/PDCurses-2.3/curses.h\\\"
CURS_DIR = /PDCurses-2.3
CURS_LIB = .
LIBS = /PDCurses-2.3/os2/pdcurses.a -llibmain -lvideont -Zwin32 -Zsys
RM = del
POST = ntbind mm

#--------------------------------------------------------------
#--------------------------------------------------------------

all:	mm

mm-main:
	${MAKE} -C mmail MM_MAJOR="${MM_MAJOR}" MM_MINOR="${MM_MINOR}" \
		OPTS="${OPTS}" mm-main

intrfc:
	${MAKE} -C interfac MM_MAJOR="${MM_MAJOR}" MM_MINOR="${MM_MINOR}" \
		OPTS="${OPTS} -I${CURS_DIR}" \
		CURS_INC="${CURS_INC}" intrfc

mm:	mm-main intrfc
	${CXX} -o mm interfac/*.o mmail/*.o -L${CURS_LIB} ${LIBS}
	${POST}

dep:
	${MAKE} -C interfac CURS_INC="${CURS_INC}" dep
	${MAKE} -C mmail dep

clean:
	${MAKE} -C interfac RM="${RM}" clean
	${MAKE} -C mmail RM="${RM}" clean
	${RM} mm
	${RM} mm.exe

modclean:
	${MAKE} -C mmail RM="${RM}" modclean
