/*
 * MultiMail offline mail reader
 * error-reporting class

 Copyright 1998-2015 William McBrine <wmcbrine@gmail.com>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

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
