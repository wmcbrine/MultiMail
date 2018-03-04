#--------------------------
# MultiMail Makefile (top)
#--------------------------

msrc = mmail
isrc = interfac

# General options (passed to mmail/Makefile and interfac/Makefile). POST 
# is for any post-processing that needs doing.

ifeq ($(DEBUG),Y)
	OPTS = -g -Wall -Wextra -pedantic -Wno-char-subscripts
else
	OPTS = -O2 -Wall -pedantic -Wno-char-subscripts
	POST = strip mm$(E)
endif

# PREFIX is the base directory under which to install the binary and man 
# page; generally either /usr/local or /usr (or perhaps /opt...).

PREFIX = /usr/local

#--------------------------------------------------------------
# Defaults are for the standard curses setup:

# CURS_DIR specifies the directory with your curses header file, if it's 
# not /usr/include/curses.h. CURS_LIB specifies the directory where the 
# curses libraries can be found, if they're not in the standard search 
# path. LIBS lists any "extra" libraries that need to be linked in. RM 
# is the Delete command ("rm" or "del", as appropriate), and SEP is the 
# separator for multi-statement lines... some systems require ";", while 
# others need "&&".

ifeq ($(OS),Windows_NT)
	CURS_DIR = /pdcurses
	CURS_LIB = .
	LIBS = /pdcurses/wincon/pdcurses.a
	RM = del
	SEP = &&
	E = .exe
else
	CURS_DIR = .
	CURS_LIB = .
	LIBS = -lcurses
	RM = rm -f
	SEP = ;
	E =
endif

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

HELPDIR = $(PREFIX)/man/man1
CPPFLAGS = $(OPTS) -I$(CURS_DIR)
O = o

.SUFFIXES: .cc
.PHONY: clean dep install

all:	mm$(E)

MOBJS = misc.o resource.o mmail.o driverl.o filelist.o area.o letter.o \
read.o compress.o pktbase.o bw.o qwk.o omen.o soup.o opx.o

IOBJS = mmcolor.o mysystem.o isoconv.o basic.o interfac.o packet.o \
arealist.o letterl.o letterw.o lettpost.o ansiview.o addrbook.o \
tagline.o help.o main.o

$(MOBJS) : %.o: $(msrc)/%.cc
	$(CXX) $(CPPFLAGS) -c $<

$(IOBJS) : %.o: $(isrc)/%.cc
	$(CXX) $(CPPFLAGS) -c $<

mm$(E):	$(MOBJS) $(IOBJS)
	$(CXX) -o mm$(E) $(MOBJS) $(IOBJS) -L$(CURS_LIB) $(LIBS)
	$(POST)

dep:
	$(CXX) -MM $(msrc)/*.cc | sed s/"\.o"/"\.\$$(O)"/ > depend
	$(CXX) -I$(CURS_DIR) -MM $(isrc)/*.cc | sed s/"\.o"/"\.\$$(O)"/ >> depend

clean:
	$(RM) *.o
	$(RM) mm$(E)

install::
	install -c -s mm $(PREFIX)/bin
	install -c -m 644 mm.1 $(HELPDIR)
	$(RM) $(HELPDIR)/mmail.1
	ln $(HELPDIR)/mm.1 $(HELPDIR)/mmail.1

include depend
