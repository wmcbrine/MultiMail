/*
 * MultiMail offline mail reader
 * file_header and file_list

 Copyright (c) 1996 Toth Istvan <stoty@vma.bme.hu>
 Copyright (c) 2002 William McBrine <wmcbrine@users.sourceforge.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "mmail.h"

// ----------------------------------------------------------------
// file_header methods
// ----------------------------------------------------------------
file_header::file_header(const char *nameA, time_t dateA, off_t sizeA) :
	date(dateA), size(sizeA)
{
	name = strdupplus(nameA);
	next = 0;
}

file_header::~file_header()
{
	delete[] name;
}

const char *file_header::getName() const
{
	return name;
}

time_t file_header::getDate() const
{
	return date;
}

void file_header::setDate()
{
	date = touchFile(name);
}

off_t file_header::getSize() const
{
	return size;
}

// --------------------------------------------------------------
// file_list methods
// --------------------------------------------------------------

file_list::file_list(const char *FileDir, bool sorttypeA, bool dirlistA) :
	sorttype(sorttypeA), dirlist(dirlistA)
{
	DirName = strdupplus(FileDir);
	filter = 0;
	relist();
}

file_list::~file_list()
{
	cleanup();

	delete[] DirName;
	delete[] filter;
}

void file_list::cleanup()
{
	while (noOfFiles)
		delete files[--noOfFiles];
	delete[] files;

	while (noOfDirs)
		delete dirs[--noOfDirs];
	delete[] dirs;
}

void file_list::relist()
{
	if (!myopendir(DirName))
		fatalError("There is no Packet Dir!");

	noOfFiles = noOfDirs = 0;

	file_header head("", 0, 0), dirhead("", 0, 0);
	file_header *filept = &head, *dirpt = &dirhead;

	const char *fname;
	mystat st;

	while ((fname = myreaddir(st)) != 0)
	    if (!filter || searchstr(fname, filter))
		if (dirlist || !st.isdir())
		    if (strcmp(fname, "."))
			if (st.readable()) {
				file_header *trec = new file_header(fname,
					st.fdate(), st.fsize());
				if (st.isdir()) {
					dirpt->next = trec;
					dirpt = dirpt->next;
					noOfDirs++;
				} else {
					filept->next = trec;
					filept = filept->next;
					noOfFiles++;
				}
			}

	int c;

	if (noOfDirs > 0) {
		dirs = new file_header *[noOfDirs];
		dirpt = dirhead.next;

		c = 0;
		while (dirpt) {
			dirs[c++] = dirpt;
			dirpt = dirpt->next;
		}

		if (noOfDirs > 1)
			qsort(dirs, noOfDirs, sizeof(file_header *),
				fnamecomp);
	} else
		dirs = 0;

	files = new file_header *[noOfFiles];
	filept = head.next;

	c = 0;
	while (filept) {
		files[c++] = filept;
		filept = filept->next;
	}
	sort();
}

void file_list::sort()
{
	if (noOfFiles > 1)
		qsort(files, noOfFiles, sizeof(file_header *), sorttype ?
			ftimecomp : fnamecomp);
}

void file_list::resort()
{
	sorttype = !sorttype;
	sort();
}

int fnamecomp(const void *a, const void *b)
{
	int d;

	const char *p = (*((file_header **) a))->getName();
	const char *q = (*((file_header **) b))->getName();

	d = strcasecmp(p, q);
	if (!d)
		d = strcmp(q, p);

	return d;
}

int ftimecomp(const void *a, const void *b)
{
	long result = (*((file_header **) b))->getDate() -
		(*((file_header **) a))->getDate();
	return (result > 0) ? 1 : (result < 0) ? -1 : 0;
}

const char *file_list::getDirName() const
{
	return DirName;
}

int file_list::getNoOfDirs() const
{
	return noOfDirs;
}

int file_list::getNoOfFiles() const
{
	return noOfFiles;
}

void file_list::gotoFile(int fileNo)
{
	if (fileNo < (noOfFiles + noOfDirs))
		activeFile = fileNo;
}

char *file_list::changeDir(const char *newpath)
{
	char *newdir = 0;

	if (dirlist) {
		if (!newpath)
			newpath = getName();

		mychdir(DirName);
		if (!mychdir(newpath))
			newdir = mygetcwd();
	}
	return newdir;
}

int file_list::changeName(const char *newname)
{
	mychdir(DirName);
	return rename(getName(), newname);
}

file_header *file_list::base() const
{
	return (activeFile < noOfDirs) ? dirs[activeFile] :
		files[activeFile - noOfDirs];
}

file_header *file_list::base(int i) const
{
	return (i < noOfDirs) ? dirs[i] : files[i - noOfDirs];
}

const char *file_list::getName() const
{
	return base()->getName();
}

time_t file_list::getDate() const
{
	return base()->getDate();
}

void file_list::setDate()
{
	base()->setDate();
}

off_t file_list::getSize() const
{
	return base()->getSize();
}

const char *file_list::getNext(const char *fname)
{
	int c, len;
	const char *p, *q;
	bool isExt;

	if (fname) {
		isExt = (*fname == '.');

		for (c = activeFile + 1; c < (noOfFiles + noOfDirs); c++) {
			q = base(c)->getName();

			if (isExt) {
				len = strlen(q);
				if (len > 5) {
				p = q + len - 4;
					if (!strcasecmp(p, fname)) {
						activeFile = c;
						return q;
					}
				}
			} else
				if (!strncasecmp(q, fname, strlen(fname))) {
					activeFile = c;
					return q;
				}
        	}
	}
        return 0;
}

file_header *file_list::getNextF(const char *fname)
{
	return getNext(fname) ? base() : 0;
}

const char *file_list::exists(const char *fname)
{
	gotoFile(-1);
	return getNext(fname);
}

file_header *file_list::existsF(const char *fname)
{
	return exists(fname) ? base() : 0;
}

void file_list::addItem(file_header **list, const char *q, int &filecount)
{
	file_header *p;
	int x;

	gotoFile(-1);
	while ((p = getNextF(q)) != 0) {
		for (x = 0; x < filecount; x++)
			if (list[x] == p)
				break;
		if (x == filecount) {
			list[x] = p;
			filecount++;
		}
	}
}

char *file_list::expandName(const char *fname)
{
	return fullpath(DirName, fname);
}

FILE *file_list::ftryopen(const char *fname)
{
	FILE *f;
	
	const char *p = exists(fname);
	if (p) {
		char *q = expandName(p);
		f = fopen(q, "rb");
		delete[] q;
	} else
		f = 0;

	return f;
}

void file_list::kill()
{
	if (activeFile >= noOfDirs) {
		int i = activeFile - noOfDirs;

		char *fname = expandName(getName());
		remove(fname);
		delete[] fname;

		delete files[i];
		noOfFiles--;
		for (; i < noOfFiles; i++)
			files[i] = files[i + 1];
	}
}

int file_list::nextNumExt(const char *baseName)
{
	int retval = -1;
	const char *nextName;

	gotoFile(-1);
	do {
		nextName = getNext(baseName);
		if (nextName) {
			int newval = getNumExt(nextName);

			if (newval > retval)
				retval = newval;
		}
	} while (nextName);

	if (retval == 999)
		retval = -1;

	return ++retval;
}

const char *file_list::getFilter() const
{
	return filter;
}

void file_list::setFilter(const char *newfilter)
{
	delete[] filter;
	filter = (newfilter && *newfilter) ? strdupplus(newfilter) : 0;

	cleanup();
	relist();

	if (filter && !(noOfDirs + noOfFiles)) {
		delete[] filter;
		filter = 0;
		
		cleanup();
		relist();
	}
}
