#define MM_NAME "MultiMail"
#define MM_TOPHEADER "%s offline reader v%d.%d"

#define USE_SHADOWS     // "Shadowed" windows
#define VANITY_PLATE    // Author info -- undefine for longer packet list

/* With the default tar/gz compress command, leave this on. It causes the
   entire file to be rearchived when using that format, instead of just
   the .red file. If you replace the tar/gz command with something 
   smarter (e.g., via a script), you can disable the kludge.
*/
#define TAR_KLUDGE

#define HAS_BOOL
#define HAS_OFFT
#define HAS_HOME

/* ----- Platform-specific defines ----- */

/* This could be improved on. Currently, it's only written to deal with
   GCC, DJGPP, EMX, RSX/NT, BCC 5.5, and Turbo C++ 3.0.
*/

/* HAS_UNAME determines whether the "System" part of the tearline is taken
   from the uname() function, or is hardwired. Turbo and Borland C++ don't
   have it, and it's broken in RSX/NT.
*/
#if !defined (__RSXNT__) && !defined (__TURBOC__)
# define HAS_UNAME
#endif

/* MS-DOS and DOS-like systems. Unfortunately, EMX doesn't define an
   "__OS2__" symbol, so I just check for "__EMX__". (RSX/NT also defines
   it, so if you wanted to distinguish them, you'd also have to check for
   the presence or absence of "__WIN32__".)
*/
#if defined (__MSDOS__) || defined (__WIN32__) || defined (__EMX__)
# define DOSCHARS
# define DOSNAMES
# define USE_SHELL
# undef HAS_HOME

/* Assume the use of PDCurses on these platforms (can't check it
   explicitly until curses.h is included), and so:

   In PDCurses, the A_REVERSE attribute resets the colors, so I avoid
   using it.
*/
# define NOREVERSE

/* Some characters are unprintable on standard Unix terminals, even with
   A_ALTCHARSET set. But PDCurses will handle them.
*/
# define ALLCHARSOK

/* With the RSXNT port, the keyboard check after printing each line makes
   output very slow, unless line break optimization is disabled. It also
   causes problems with the BCC port when the mouse is used.
*/
# ifdef __WIN32__
#  define NOTYPEAHEAD
# endif

/* For the ugly cursor-toggling routines -- no longer needed with PDCurses
   2.4?
*/
# if 0
#  define PDCURSKLUDGE
# endif

#else

/* Not a DOS-like system -- enable "Transparency" keyword */

# define HAS_TRANS

#endif

/* Also, see the NCURSES_SIGWINCH definition in interfac/interfac.h -- it
   should be fine as is, but may need manual adjustment in some cases.
*/

/* I use strcasecmp() and strncasecmp() throughout, but some systems call
   these functions stricmp() and strincmp(). I haven't yet dealt with the
   case where neither is defined. TEMP_RELATIVE is for tmpnam() implemen-
   tations that return a relative path rather than an absolute one.
*/
#if defined (__EMX__) || defined (__TURBOC__)
# define USE_STRICMP
# define TEMP_RELATIVE
#endif

/* unistd.h is the POSIX header file. Borland/Turbo C doesn't have it.
   The sleep() function is also defined there.
*/
#ifndef __TURBOC__
# define HAS_UNISTD
# define HAS_SLEEP
#endif

/* For the 16-bit MS-DOS version compiled with Turbo C++ 3.0, I've added
   the SPAWNO library by Ralf Brown to get more memory when shelling.
   Also, this version lacks the "bool" and "off_t" types. (Check should be
   more restrictive.)
*/
#if defined (__TURBOC__) && defined (__MSDOS__)
# define USE_SPAWNO
# define LIMIT_MEM
# define MAXBLOCK 0x0FFE0L
# define USE_SETDISK
# undef HAS_BOOL
# undef HAS_OFFT
#else
# define MAXBLOCK 0x07FFFFFE0L
#endif

/* Borland C++ 5.5 barfs on time_t = 0, which appears as the timestamp of
   the top-level directory. Also, utime(), though implemented, doesn't work
   right.
*/
#if defined (__TURBOC__) && defined (__WIN32__)
# define TIMEKLUDGE
# define USE_SETFTIME
#endif

/* Some lines in the code serve no purpose but to supress the GCC warning
   "might be used uninitialized in this function". Borland C++ 5.5, on the
   other hand, complains "is assigned a value that is never used" if I
   leave these lines _in_.
*/
#ifndef __TURBOC__
# define BOGUS_WARNING
#endif

/* In Borland/Turbo C++ and in DJGPP, using findfirst()/findnext() is
   faster than using readdir()/stat().
*/
#if defined (__MSDOS__) || defined (__TURBOC__)
# define USE_DIRH
# define USE_FINDFIRST
#endif

/* ----- End of platform-specific defines ----- */

#ifndef HAS_BOOL
typedef unsigned char bool;
# define true 1
# define false 0
#endif

#ifndef HAS_OFFT
typedef long off_t;
#endif
