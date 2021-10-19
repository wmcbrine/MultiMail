#--------------------------
# MultiMail Makefile (top)
#--------------------------

msrc = mmail
isrc = interfac

# General options (passed to mmail/Makefile and interfac/Makefile). POST
# is for any post-processing that needs doing.

ifeq ($(DEBUG),Y)
	OPTS = -g -Wall -pedantic
else
	OPTS = -O2 -Wall -pedantic
	POST = strip mm$(E)
endif

# INSTALL_PREFIX is the base directory under which to install the binary and man
# page; generally either /usr/local or /usr (or perhaps /opt...).
# It can be changed when make is run (e.g. 'make INSTALL_PREFIX=/tmp/pkg install'
# (or when using 'uninstall', respectively))

INSTALL_PREFIX = /usr/local

#--------------------------------------------------------------
# Defaults are for the standard curses setup:

# CURS_DIR specifies the directory with curses.h, if it's not in the
# include path. LIBS lists any "extra" libraries that need to be linked
# in, including the curses library. RM is the Delete command ("rm" or
# "del", as appropriate), and SEP is the separator for multi-statement
# lines... some systems require ";", while others need "&&".

ifeq ($(OS),Windows_NT)
	CURS_DIR = /pdcurses
	LIBS = $(CURS_DIR)/wincon/pdcurses.a
	RM = del
	SEP = &&
	E = .exe
	BUILD = $(CXX) -static
else
	CURS_DIR = .
	LIBS = -lcurses
	RM = rm -f
	SEP = ;
	E =
	BUILD = $(CXX)
endif

#--------------------------------------------------------------
# With PDCurses for X11:

ifeq ($(SYS),X11)
	CURS_DIR = $(shell xcurses-config --cflags | cut -c 3-)
	LIBS = $(shell xcurses-config --libs)
endif

#--------------------------------------------------------------
# With PDCurses for SDL:

ifeq ($(SYS),SDL)
	CURS_DIR = /Users/wmcbrine/pdsrc/PDCurses
	LIBS = $(CURS_DIR)/sdl2/pdcurses.a $(shell sdl2-config --libs)
endif

#--------------------------------------------------------------
# For DJGPP:

ifeq ($(SYS),DOS)
	CURS_DIR = /pdcurses
	LIBS = $(CURS_DIR)/dos/pdcurses.a
	RM = del
	SEP = ;
	E = .exe
	POST = strip mm.exe; exe2coff mm.exe; copy /b \
		c:\djgpp\bin\cwsdstub.exe+mm mm.exe; del mm
endif

HELPDIR = $(INSTALL_PREFIX)/man/man1
O = o

.SUFFIXES: .cc
.PHONY: clean dep install

all:	mm$(E)

MOBJS = misc.o resource.o mmail.o filelist.o netadd.o area.o letter.o \
read.o compress.o pktbase.o bw.o qwk.o omen.o soup.o opx.o

IOBJS = mmcolor.o mysystem.o isoconv.o basic.o interfac.o packet.o \
arealist.o letterl.o letterw.o lettpost.o ansiview.o addrbook.o \
tagline.o help.o main.o

$(MOBJS) : %.o: $(msrc)/%.cc
	$(CXX) $(OPTS) -c $<

$(IOBJS) : %.o: $(isrc)/%.cc
	$(CXX) $(OPTS) -I$(CURS_DIR) -c $<

mm$(E):	$(MOBJS) $(IOBJS)
	$(BUILD) -o mm$(E) $(MOBJS) $(IOBJS) $(LIBS)
	$(POST)

dep:
	$(CXX) -MM $(msrc)/*.cc | sed s/"\.o"/"\.\$$(O)"/ > depend
	$(CXX) -I$(CURS_DIR) -MM $(isrc)/*.cc | sed s/"\.o"/"\.\$$(O)"/ >> depend

clean:
	$(RM) *.o
	$(RM) mm$(E)

install:
	install -D -s mm -t $(INSTALL_PREFIX)/bin
	install -D -m 644 mm.1 -t $(HELPDIR)
	if test -f $(HELPDIR)/mmail.1; then	\
		$(RM) $(HELPDIR)/mmail.1;	\
	fi
	ln $(HELPDIR)/mm.1 $(HELPDIR)/mmail.1

uninstall:
	for file in $(INSTALL_PREFIX)/bin/mm $(HELPDIR)/mm.1 $(HELPDIR)/mmail.1; do \
		if test -f $$file; then	\
			$(RM) $$file;	\
		fi;	\
	done

include depend
