/*
 * MultiMail offline mail reader
 * error-reporting class

 Copyright 1998-2018 William McBrine <wmcbrine@gmail.com>
 Distributed under the GNU General Public License, version 3 or later. */

class ErrorType
{
    char *origdir;
 public:
    ErrorType();
    ~ErrorType();
    const char *getOrigDir();
};

extern ErrorType error;
