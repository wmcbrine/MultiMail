MultiMail compilation and installation procedure
================================================

These instructions assume that you're compiling MultiMail from source. For
precompiled binaries, see the README files that accompany them instead.

1. Make sure any needed packages are installed --
    In addition to the MultiMail package itself, you'll also need InfoZip
    or PKZIP (and/or LHA, ARJ, etc.) to uncompress the packets and
    compress the replies. InfoZip is available from:

    http://infozip.sf.net/

    (PKZIP is the default for DOS; InfoZip is the default for other
    platforms.) The programs should be installed somewhere in the PATH;
    otherwise, the full path must be specified in ~/.mmailrc.

    To compile MultiMail, you'll need curses -- either ncurses, SysV
    curses (e.g., Solaris curses), or PDCurses. You can get ncurses from:

    http://invisible-island.net/ncurses/

    PDCurses is available at:

    http://pdcurses.org/

    (If you're using Linux, you probably already have ncurses and
    InfoZip.)

    If using PDCurses, MultiMail now requires version 3.6 or later.

    The 16-bit MS-DOS Turbo C++ port also uses Ralf Brown's SPAWNO
    library:

    http://www.cs.cmu.edu/afs/cs.cmu.edu/user/ralf/pub/WWW/files.html

2. Configure it (for compilation) --
    Check the options and paths in the Makefile. If curses.h isn't in
    the include path, change CURS_DIR as appropriate. You may also need
    to change LIBS. These can be set on the command line, e.g. "make
    CURS_DIR=/pdcurses".

3. Compile MultiMail --
    At the base directory, type: `make`

4. Run it --
    Type: `./mm`
    (For DOS, OS/2 or Windows, set the MMAIL or HOME variable, then run mm.)

5. (Optional:) Configure it (for end user) --
    Edit the ~/.mmailrc file. (For DOS, OS/2 or Windows, mmail.rc.)

6. (Optional:) Install it system-wide --
    Type: `make install`
    to install the manual and binary under /usr/local
    (requires root access). (This doesn't work in DOS, OS/2 or Windows.)

See the man page (mmail.1) and README for more information.

This package includes some example color schemes, in the "colors"
directory. To select one, use the "ColorFile" keyword in .mmailrc .


Support for XCurses (PDCurses)
------------------------------

When MultiMail is compiled with XCurses, you can use the X resource
database to set certain startup options. Here are some example resources:

    XCurses*normalFont: 9x15
    XCurses*boldFont:   9x15bold
    XCurses*lines:      30
    XCurses*cols:       80

For details, see the PDCurses documentation.

If you're using a non-X text editor with an XCurses version of MultiMail,
it will work better if you set MultiMail's editor variable to "xterm -e
$EDITOR" instead of just "$EDITOR" (the default).


Compile notes: Windows, MS-DOS, and OS/2
----------------------------------------

In the MultiMail source, separate makefiles are provided for these ports.

    Makefile     - GCC (including DJGPP and MinGW)
    Makefile.bcc - Borland C++ (Windows, MS-DOS)
    Makefile.vc  - Microsoft Visual C++ (Windows)
    Makefile.wcc - Watcom (All platforms -- Windows by default)

Point to your installation of PDCurses and compile with, e.g.:

    make -f Makefile.bcc CURS_DIR=/pdcurs38 SYS=DOS

(Use "wmake" instead of "make" for Watcom; "nmake" for MSVC.)
