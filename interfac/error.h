/*
 * MultiMail offline mail reader
 * error-reporting class

 Copyright (c) 2000 William McBrine <wmcbrine@users.sourceforge.net>

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
