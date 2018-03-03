#--------------------------
# MultiMail Makefile (top)
#--------------------------

include version

# General options (passed to mmail/Makefile and interfac/Makefile):

ifeq ($(DEBUG),Y)
	OPTS = -g -Wall -Wextra -pedantic -Wno-char-subscripts
else
	OPTS = -O2 -Wall -pedantic -Wno-char-subscripts
endif

# PREFIX is the base directory under which to install the binary and man
# page; generally either /usr/local or /usr (or perhaps /opt...):

PREFIX = /usr/local

# Delete command ("rm" or "del", as appropriate):

RM = rm -f

# The separator for multi-statement lines... some systems require ";",
# while others need "&&":

SEP = ;

# Any post-processing that needs doing:

POST =

#--------------------------------------------------------------
# Defaults are for the standard curses setup:

# CURS_DIR specifies the directory with your curses header file, if it's
# not /usr/include/curses.h:

CURS_DIR = .

# CURS_LIB specifies the directory where the curses libraries can be found,
# if they're not in the standard search path:

CURS_LIB = .

# LIBS lists any "extra" libraries that need to be linked in:

LIBS = -lcurses

#--------------------------------------------------------------
# With PDCurses for X11:

ifeq ($(X11),Y)
	CURS_DIR = /usr/local/include/xcurses
	LIBS = -lXCurses
endif

#--------------------------------------------------------------
# With PDCurses for SDL:

ifeq ($(SDL),Y)
	CURS_DIR = /Users/wmcbrine/pdsrc/PDCurses
	CURS_LIB = /Users/wmcbrine/pdsrc/PDCurses/sdl2
	LIBS = -lpdcurses `sdl2-config --libs`
endif

#--------------------------------------------------------------
#--------------------------------------------------------------

HELPDIR = $(PREFIX)/man/man1

all:	mm

mm-main:
	cd mmail $(SEP) $(MAKE) MM_MAJOR="$(MM_MAJOR)" \
		MM_MINOR="$(MM_MINOR)" OPTS="$(OPTS)" mm-main $(SEP) cd ..

intrfc:
	cd interfac $(SEP) $(MAKE) MM_MAJOR="$(MM_MAJOR)" \
		MM_MINOR="$(MM_MINOR)" OPTS="$(OPTS)" \
		CURS_DIR="$(CURS_DIR)" intrfc $(SEP) cd ..

mm:	mm-main intrfc
	$(CXX) -o mm mmail/*.o interfac/*.o -L$(CURS_LIB) $(LIBS)
	$(POST)

dep:
	cd interfac $(SEP) $(MAKE) CURS_DIR="$(CURS_DIR)" dep $(SEP) cd ..
	cd mmail $(SEP) $(MAKE) dep $(SEP) cd ..

clean:
	cd interfac $(SEP) $(MAKE) RM="$(RM)" clean $(SEP) cd ..
	cd mmail $(SEP) $(MAKE) RM="$(RM)" clean $(SEP) cd ..
	$(RM) mm

install::
	install -c -s mm $(PREFIX)/bin
	install -c -m 644 mm.1 $(HELPDIR)
	$(RM) $(HELPDIR)/mmail.1
	ln $(HELPDIR)/mm.1 $(HELPDIR)/mmail.1
