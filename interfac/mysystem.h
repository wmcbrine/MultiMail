/*
 * MultiMail offline mail reader
 * protos for mysystem.cc

 Copyright (c) 2001 William McBrine <wmcbrine@users.sourceforge.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#ifndef MYSYSTEM_H
#define MYSYSTEM_H

extern "C" {
#include <sys/types.h>
}

class mystat;

char *myfgets(char *, size_t, FILE *);
int mysystem(const char *);
int mysystem2(const char *, const char *);
char *mytmpnam();
void edit(const char *);
int mychdir(const char *);
int mymkdir(const char *);
void myrmdir(const char *);
char *mygetcwd();
const char *sysname();
bool myopendir(const char *);
const char *myreaddir(mystat &);
void clearDirectory(const char *);
time_t touchFile(const char *);

#ifdef LIMIT_MEM
long limitmem(long);
#else
# define limitmem(x) x
#endif

char *canonize(char *);

#ifdef HAS_HOME
const char *homify(const char *);
#else
# define homify(x) x
#endif

#ifdef USE_SHELL
class Shell
{
	char *prompt;
 public:
	Shell();
	~Shell();
	void out();
};
#endif

class mystat
{
	int mode;
	off_t size;
	time_t date, adate;
 public:
	mystat(const char *);
	mystat();

	bool init(const char *);
#ifdef USE_FINDFIRST
	void init(long, long, char);
#endif
	void init();
	bool isdir();
	bool readable();
	bool writeable();
	off_t fsize();
	time_t fdate();
	void reset_date(const char *);
};

#ifdef USE_STRICMP
# define strcasecmp stricmp
# define strncasecmp strnicmp
#endif

#endif
