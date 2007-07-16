/*
 * MultiMail offline mail reader
 * main, error

 Copyright (c) 1996 Kolossvary Tamas <thomas@vma.bme.hu>
 Copyright (c) 2007 William McBrine <wmcbrine@users.sf.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "error.h"
#include "interfac.h"

#ifdef USE_NEWHANDLER
# include <new.h>
#endif

#include <locale.h>

Interface *ui = 0;
const chtype *ColorArray = 0;
time_t starttime;
ErrorType error;
mmail mm;

#ifdef USE_MOUSE
MEVENT mm_mouse_event;
#endif

#ifdef USE_NEWHANDLER
void memError()
{
	fatalError("Memory allocation error");
}
#endif

ErrorType::ErrorType()
{
	starttime = time(0);
	srand((unsigned) starttime);

#ifdef USE_NEWHANDLER
	set_new_handler(memError);
#endif
	origdir = mygetcwd();
}

ErrorType::~ErrorType()
{
	mychdir(origdir);
	delete[] origdir;
}

const char *ErrorType::getOrigDir()
{
	return origdir;
}

#if defined(SIGWINCH) && !defined(PDCURSES) && !defined(NCURSES_SIGWINCH)
extern "C" void sigwinchHandler(int sig)
{
	if (sig == SIGWINCH)
		ungetch(KEY_RESIZE);
	signal(SIGWINCH, sigwinchHandler);
}
#endif

void fatalError(const char *description)
{
	delete ui;
	fprintf(stderr, "\n\n%s\n\n", description);
	exit(EXIT_FAILURE);
}

void pauseError(const char *description)
{
	fprintf(stderr, "\n\n%s\n\n", description);
	napms(2000);
}

#ifdef USE_MOUSE
void mm_mouse_get()
{
# ifdef NCURSES_MOUSE_VERSION
	getmouse(&mm_mouse_event);
# else
	nc_getmouse(&mm_mouse_event);
# endif
}
#endif

int main(int argc, char **argv)
{
	char **ARGV = argv;
	int ARGC = argc;

	setlocale(LC_ALL, "");

	while ((ARGC > 2) && ('-' == ARGV[1][0])) {
		char *resName = ARGV[1] + 1;
		char *resValue = ARGV[2];

		if ('-' == *resName)
			resName++;

		mm.resourceObject->processOneByName(resName,
			resValue);

		ARGV += 2;
		ARGC -= 2;
	}

	ui = new Interface();
	ui->init();
	if (ARGC > 1)
		for (int i = 1; (i < ARGC) &&
			ui->fromCommandLine(ARGV[i]); i++);
	else
		ui->main();
	delete ui;
	ui = 0;		// some destructors, executed after this, may
			// check for this
	return EXIT_SUCCESS;
}
