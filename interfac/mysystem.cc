/*
 * MultiMail offline mail reader
 * some low-level routines common to both sides

 Copyright (c) 2005 William McBrine <wmcbrine@users.sf.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

/* Most non-ANSI, non-curses stuff is here. */

#include "interfac.h"
#include "error.h"

extern "C" {
#if !defined(USE_FINDFIRST) && !defined(__WATCOMC__)
# include <dirent.h>
#endif

#ifdef USE_SPAWNO
# include <spawno.h>
#endif

#ifdef LIMIT_MEM
# include <alloc.h>
#endif

#ifdef HAS_UNISTD
# include <unistd.h>
#endif

#ifdef USE_DIRH
# include <dir.h>
#endif

#include <sys/stat.h>

#ifdef HAS_UNAME
# include <sys/utsname.h>
#endif

#ifdef USE_SETFTIME
# include <fcntl.h>
# include <io.h>
#else
# ifdef __WATCOMC__
#  include <sys/utime.h>
# else
#  include <utime.h>
# endif
#endif

#ifdef __EMX__
int _chdir2(const char *);
char *_getcwd2(char *, int);
#endif

#ifdef __WATCOMC__
# include <dos.h>
# include <direct.h>
#endif
}

#ifndef S_IREAD
# define S_IREAD S_IRUSR
#endif

#ifndef S_IWRITE
# define S_IWRITE S_IWUSR
#endif

#ifndef USE_FINDFIRST
static DIR *Dir;
#endif

void fatalError(const char *description);

char *myfgets(char *s, size_t size, FILE *stream)
{
	char *end = 0;

	if (!feof(stream) && fgets(s, size, stream)) {

		end = s + strlen(s) - 1;

		// Skip any leftovers:
		if (*end != '\n')
			while (!feof(stream) && (fgetc(stream) != '\n'));

	}
	return end;
}

int mysystem(const char *cmd)
{
	if (ui && !isendwin()) {
			endwin();
#ifdef PDCURSKLUDGE
			// Restore original cursor
			PDC_set_cursor_mode(curs_start, curs_end);
#endif
	}

#ifdef USE_SPAWNO
	int result = mm.resourceObject->getInt(swapOut) ?
		systemo(mm.resourceObject->get(BaseDir), cmd) : -1;

	if (-1 == result)
		result = system(cmd);
#else
	int result = system(cmd);
#endif

#ifdef HAS_SLEEP
	// Non-zero result = error; pause so it can (maybe) be read
	if (result)
		sleep(2);
#endif

	if (ui) {
#ifdef __PDCURSES__
# ifdef __WIN32__
		// Force scroll bars off in Windows 2000, XP
		resize_term(LINES, COLS);
# endif
# if defined(__WIN32__) || defined(XCURSES)
		PDC_set_title(MM_NAME);
# endif
		typeahead(-1);
#endif
		keypad(stdscr, TRUE);
	}

	return result;
}

int mysystem2(const char *cmd, const char *args)
{
	int lencmd = strlen(cmd);
	char *qargs = canonize(quotespace(args));
	char *cmdline = new char[lencmd + strlen(qargs) + 2];

	sprintf(cmdline, "%s %s", cmd, qargs);

	int result = mysystem(cmdline);

	delete[] cmdline;
	delete[] qargs;
	return result;
}

char *mytmpdir(const char *home)
{
	mystat st;
	char name[9];

	if (mychdir(home))
		fatalError("Could not change to temp dir");
	
	do
		sprintf(name, "work%04x", (rand() & 0xffff));
	while (st.init(name));
	
	return canonize(fullpath(home, name));
}

char *mytmpnam()
{
	static long tcount = 1;
	char name[13];

	if (tcount > 99999L)
		fatalError("Out of temporary filenames");

	sprintf(name, "tmp%05ld.txt", tcount++);

	return canonize(fullpath(mm.resourceObject->get(BaseDir), name));
}

void edit(const char *reply_filename)
{
        mysystem2(mm.resourceObject->get(editor), reply_filename);
}

int mychdir(const char *pathname)
{
#ifdef USE_SETDISK
	if (':' == pathname[1])
		setdisk(toupper(pathname[0]) - 'A');
#endif
	return
#ifdef __EMX__
		_chdir2(pathname);
#else
		chdir(pathname);
#endif
}

int mymkdir(const char *pathname)
{
#ifdef HAS_UNISTD
	return mkdir(pathname, S_IRWXU);
#else
	return mkdir(pathname);
#endif
}

void myrmdir(const char *pathname)
{
	rmdir(pathname);
}

char *mygetcwd()
{
	char pathname[256];
#ifdef __EMX__
	_getcwd2(pathname, 255);
#else
	getcwd(pathname, 255);
#endif
	return strdupplus(pathname);
}

// system name -- results of uname()
const char *sysname()
{
#ifdef HAS_UNAME
	static struct utsname buf;

	if (!buf.sysname[0])
		uname(&buf);

	return buf.sysname;
#else
# ifdef __WIN32__
	return "Win32";
# else
	return "XT";
# endif
#endif
}

bool myopendir(const char *dirname)
{
#ifdef USE_FINDFIRST
	return !mychdir(dirname);
#else
	return ((Dir = opendir((char *) dirname)) != 0) ?
		!mychdir(dirname) : false;
#endif
}

const char *myreaddir(mystat &st)
{
#ifdef USE_FINDFIRST
	static struct ffblk blk;
	static bool first = true;
	int result;

	if (first) {
		result = findfirst("*.*", &blk, FA_DIREC);
		first = false;
	} else
                result = findnext(&blk);

	if (result) {
# ifdef __WIN32__
		findclose(&blk);
# endif
		first = true;
		return 0;
	} else {
		st.init(blk.ff_fsize, ((long) blk.ff_ftime << 16) +
			(long) blk.ff_fdate, blk.ff_attrib);
		return blk.ff_name;
	}
#else
	static dirent *entry;
	const char *result = 0;

	entry = readdir(Dir);
	if (entry)
		result = entry->d_name;
	else
		closedir(Dir);

	if (result)
		st.init(result);
	return result;
#endif
}

void clearDirectory(const char *DirName)
{
	mystat st;
	const char *fname;

	if (myopendir(DirName)) {
		while ((fname = myreaddir(st)) != 0)
			if (!st.isdir())
				remove(fname);
	} else {
		char tmp[512];
		sprintf(tmp, "Could not change to %.491s", DirName);
		fatalError(tmp);
	}
}

#ifdef USE_SETFTIME
void myutime(const char *fname, time_t now)
{
	struct ftime ut;
	struct tm tmnow = *localtime(&now);

	ut.ft_tsec = tmnow.tm_sec >> 1;
	ut.ft_min = tmnow.tm_min;
	ut.ft_hour = tmnow.tm_hour;
	ut.ft_day = tmnow.tm_mday;
	ut.ft_month = tmnow.tm_mon + 1;
	ut.ft_year = tmnow.tm_year - 80;

	int f = open(fname, O_RDWR | O_BINARY);
	if (f != -1) {
		setftime(f, &ut);
		close(f);
	}
}
#endif

time_t touchFile(const char *fname)
{
	time_t now = time(0);
#ifdef USE_SETFTIME
	myutime(fname, now);
#else
	struct utimbuf ut;
	ut.actime = ut.modtime = now;
	utime((char *) fname, &ut);
#endif
	return now;
}

#ifdef LIMIT_MEM

/* Constrain memory allocation according to maximum block size and free
   memory remaining. Currently used only in the 16-bit MS-DOS port.
*/

long limitmem(long wanted)
{
	long maxavail = (long) coreleft();

	// Give it a 25% margin
	maxavail -= (wanted >> 2);

	//if (maxavail > MAXBLOCK)
	//	maxavail = MAXBLOCK;

	if (wanted > maxavail)
		wanted = maxavail;

	return wanted;
}

#endif

/* Convert pathnames to "canonical" form (change slashes to backslashes).
   The "nospace" stuff leaves any parameters unconverted.
*/

char *canonize(char *sinner)
{
#ifdef DOSNAMES
	int i;
	bool nospace = true;
	bool inquotes = false;

	for (i = 0; sinner[i] && nospace; i++) {
		if ('\"' == sinner[i])
			inquotes = !inquotes;
		else
			if ('/' == sinner[i])
				sinner[i] = '\\';
			else
				if ((' ' == sinner[i]) && !inquotes)
					nospace = false;
	}
#endif
	return sinner;
}

#ifdef HAS_HOME

/* Recognize '~' as a substitute for the home directory path, on Unix-like
   systems.
*/

const char *homify(const char *raw)
{
	static const char *home = getenv("HOME");

	if (home && raw && (raw[0] == '~') &&
	    ((raw[1] == '/') || (raw[1] == '\0'))) {
		static char expanded[512];

		sprintf(expanded, "%.255s/%.255s", home, raw + 1);
		return expanded;
	} else
		return raw;
}

#endif

#ifdef USE_SHELL

/* Command shell routine -- currently only used in the DOSish ports */

Shell::Shell()
{
	const char *oldprompt = getenv("PROMPT");
	if (!oldprompt)
		oldprompt = "$p$g";

	int len = strlen(oldprompt) + 13;
	prompt = new char[len];

	sprintf(prompt, "PROMPT=%s[MM] ", oldprompt);
	putenv(prompt);
}

Shell::~Shell()
{
	delete[] prompt;
}

void Shell::out()
{
	mychdir(error.getOrigDir());
	touchwin(stdscr);
	refresh();
	mysystem(getenv("COMSPEC"));

	ui->redraw();
}

#endif

mystat::mystat(const char *fname)
{
	init(fname);
}

mystat::mystat()
{
	init();
}

bool mystat::init(const char *fname)
{
#ifdef USE_FINDFIRST
	struct ffblk blk;
	bool retval = !findfirst(fname, &blk, FA_DIREC);

	if (retval)
		init(blk.ff_fsize, ((long) blk.ff_ftime << 16) +
			(long) blk.ff_fdate, blk.ff_attrib);
	else
		init();

# ifdef __WIN32__
	findclose(&blk);
# endif
#else
	struct stat fileStat;
	bool retval = !stat((char *) fname, &fileStat);

	if (retval) {
		size = fileStat.st_size;
		date = fileStat.st_mtime;
		adate = fileStat.st_atime;
		mode = fileStat.st_mode;
	} else
		init();
#endif
	return retval;
}

#ifdef USE_FINDFIRST

void mystat::init(long sizeA, long dateA, char ff_attrib)
{
	size = sizeA;
	date = mktime(getdostime(dateA));
	adate = date;
	mode = S_IREAD | ((ff_attrib & FA_RDONLY) ? 0 : S_IWRITE) |
		((ff_attrib & FA_DIREC) ? S_IFDIR : 0);
}

#endif

void mystat::init()
{
	size = -1;
	date = (time_t) -1;
	adate = date;
	mode = 0;
}

bool mystat::isdir()
{
	return !(!(mode & S_IFDIR));
}

bool mystat::readable()
{
	return !(!(mode & S_IREAD));
}

bool mystat::writeable()
{
	return !(!(mode & S_IWRITE));
}

off_t mystat::fsize()
{
	return size;
}

time_t mystat::fdate()
{
	return date;
}

void mystat::reset_date(const char *fname)
{
	if (date != (time_t) -1)
#ifdef USE_SETFTIME
		myutime(fname, date);
#else
	{
		struct utimbuf ut;
		ut.actime = adate;
		ut.modtime = date;
		utime((char *) fname, &ut);
	}
#endif
}
