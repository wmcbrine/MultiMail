/*
 * MultiMail offline mail reader
 * main, error

 Copyright (c) 1996 Kolossvary Tamas <thomas@vma.bme.hu>
 Copyright (c) 2003 William McBrine <wmcbrine@users.sf.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "error.h"
#include "interfac.h"

#include <new.h>

Interface *ui = 0;
const chtype *ColorArray = 0;
time_t starttime;
ErrorType error;
mmail mm;

#ifdef USE_MOUSE
MEVENT mouse_event;
#endif

#ifdef PDCURSKLUDGE
int curs_start, curs_end;
#endif

void memError();
void fatalError(const char *description);

ErrorType::ErrorType()
{
	set_new_handler(memError);
	origdir = mygetcwd();
#ifdef __DJGPP__
	if (!getenv("TMP") && !getenv("DJGPP")) {
		const char *temp = getenv("TEMP");
		if (!temp)
			temp = origdir;
		newtmp = new char[strlen(temp) + 5];
		sprintf(newtmp, "TMP=%s", temp);
		putenv(newtmp);
	} else
		newtmp = 0;
#endif
}

ErrorType::~ErrorType()
{
#ifdef __DJGPP__
	delete[] newtmp;
#endif
	mychdir(origdir);
	delete[] origdir;
}

const char *ErrorType::getOrigDir()
{
	return origdir;
}

#if defined(SIGWINCH) && !defined(XCURSES) && !defined(NCURSES_SIGWINCH)
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

void memError()
{
	fatalError("Out of memory");
}

#ifdef USE_MOUSE

void mm_mouse_get()
{
# ifndef NCURSES_MOUSE_VERSION
	request_mouse_pos();
	mouse_event.x = Mouse_status.x;
	mouse_event.y = Mouse_status.y;
	mouse_event.bstate = (Mouse_status.button[0] ? BUTTON1_CLICKED : 0) |
		(Mouse_status.button[2] ? BUTTON3_CLICKED : 0);
# else
	getmouse(&mouse_event);
# endif
}

#endif

int main(int argc, char **argv)
{
	char **ARGV = argv;
	int ARGC = argc;

	starttime = time(0);
	
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
