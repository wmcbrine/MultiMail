/*
 * MultiMail offline mail reader
 * main, error

 Copyright 1996-1997 Kolossvary Tamas <thomas@vma.bme.hu>
 Copyright 1997-2021 William McBrine <wmcbrine@gmail.com>
 Distributed under the GNU General Public License, version 3 or later. */

#include "error.h"
#include "interfac.h"

#include <locale.h>

const chtype *ColorArray = 0;
time_t starttime;
ErrorType error;
mmail mm;
Interface ui;

#ifdef USE_MOUSE
MEVENT mm_mouse_event;
#endif

ErrorType::ErrorType()
{
    starttime = time(0);
    srand((unsigned) starttime);

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
    if (ui.on && !isendwin())
        ui.close();
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
    setlocale(LC_ALL, "");

    while ((argc > 2) && ('-' == argv[1][0])) {
        char *resName = argv[1] + 1;
        char *resValue = argv[2];

        if ('-' == *resName)
            resName++;

        mm.res.processOneByName(resName, resValue);

        argv += 2;
        argc -= 2;
    }

    ui.init();
    if (argc > 1)
        for (int i = 1; (i < argc) &&
            ui.fromCommandLine(argv[i]); i++);
    else
        ui.main();
    ui.close();

    return EXIT_SUCCESS;
}
