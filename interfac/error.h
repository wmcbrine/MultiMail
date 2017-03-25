/*
 * MultiMail offline mail reader
 * error-reporting class

 Copyright 1998-2017 William McBrine <wmcbrine@gmail.com>
 Distributed under the GNU General Public License, version 3 or later. */

class ErrorType
{
    char *origdir;
#ifdef __DJGPP__
    char *newtmp;
#endif
 public:
    ErrorType();
    ~ErrorType();
    const char *getOrigDir();
};

extern ErrorType error;
