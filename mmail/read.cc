/*
 * MultiMail offline mail reader
 * main_read_class, reply_read_class

 Copyright (c) 1996 Toth Istvan <stoty@vma.bme.hu>
 Copyright (c) 2001 William McBrine <wmcbrine@users.sourceforge.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "mmail.h"
#include "compress.h"

/* read_class -- virtual */

read_class::~read_class()
{
}

/* main_read_class -- for regular areas */

main_read_class::main_read_class(mmail *mmA, specific_driver *driverA) :
	mm(mmA), driver(driverA)
{
	ro = mm->resourceObject;

	noOfAreas = driver->getNoOfAreas();
	noOfLetters = new int[noOfAreas];
	readStore = new int *[noOfAreas];

	for (int c = 0; c < noOfAreas; c++) {
		driver->selectArea(c);

		int numlett = driver->getNoOfLetters();
		noOfLetters[c] = numlett;
		readStore[c] = numlett ? new int[numlett] : 0;

		for (int d = 0; d < numlett; d++)
			readStore[c][d] = 0;
	}

	hasPersArea = driver->hasPersArea();
	hasPersNdx = !(!(mm->workList->exists("personal.ndx")));
}

main_read_class::~main_read_class()
{
	while(noOfAreas--)
		delete[] readStore[noOfAreas];
	delete[] readStore;
	delete[] noOfLetters;
}

void main_read_class::init()
{
	// If basename.red not found, look for any .red file;
	// then look for an old-style file, and use the most recent:

	file_header *redfile, *oldfile;
	file_list *wl = mm->workList;

	redfile = wl->existsF(readFilePath(ro->get(PacketName)));
	if (!redfile)
		redfile = wl->existsF(".red");

	const char *oldFileN = driver->oldFlagsName();
	oldfile = oldFileN ? wl->existsF(oldFileN) : 0;

	bool oldused = (oldfile && (!redfile || (oldfile->getDate() >
		redfile->getDate())));

	if (oldused) {
		int oldsort = lsorttype;
		lsorttype = LS_MSGNUM;

		oldused = driver->readOldFlags();

		lsorttype = oldsort;
	}

	if (!oldused) {
		FILE *readFile;
		const char *readFileN = redfile ? redfile->getName() : 0;

		readFile = readFileN ? wl->ftryopen(readFileN) : 0;

		if (readFile) {
			// Don't init personal area, unless using QWK
			// personal.ndx (this is for backwards
			// compatibility):

			int skip = hasPersArea && !hasPersNdx;

			for (int c = skip; c < noOfAreas; c++)
			    for (int d = 0; d < noOfLetters[c]; d++)
				readStore[c][d] = fgetc(readFile);

			fclose(readFile);
		}
	}
}

void main_read_class::setRead(int area, int letter, bool value)
{
	if (value)
		readStore[area][letter] |= MS_READ;
	else
		readStore[area][letter] &= ~MS_READ;
}

bool main_read_class::getRead(int area, int letter)
{
	return !(!(readStore[area][letter] & MS_READ));
}

void main_read_class::setStatus(int area, int letter, int value)
{
	readStore[area][letter] = value;
}

int main_read_class::getStatus(int area, int letter)
{
	return readStore[area][letter];
}

int main_read_class::getNoOfUnread(int area)
{
	int tmp = 0;

	for (int c = 0; c < noOfLetters[area]; c++)
		if (!(readStore[area][c] & MS_READ))
			tmp++;
	return tmp;
}

int main_read_class::getNoOfMarked(int area)
{
	int tmp = 0;

	for (int c = 0; c < noOfLetters[area]; c++)
		if (readStore[area][c] & MS_MARKED)
			tmp++;
	return tmp;
}

bool main_read_class::saveAll()
{
	const char *readFileN = 0, *oldFileN = driver->oldFlagsName();

	bool oldused = !(!oldFileN);

	if (mychdir(ro->get(WorkDir)))
		fatalError("Unable to change to work directory");

	if (oldused) {
		int oldsort = lsorttype;
		lsorttype = LS_MSGNUM;

		oldused = driver->saveOldFlags();

		lsorttype = oldsort;
	}

	if (!oldused) {
		FILE *readFile;

		readFileN = readFilePath(ro->get(PacketName));
		readFile = fopen(readFileN, "wb");

		for (int c = (hasPersArea && !hasPersNdx); c < noOfAreas; c++)
			for (int d = 0; d < noOfLetters[c]; d++)
				fputc(readStore[c][d], readFile);

		fclose(readFile);
	}

	// add the .red file to the packet
	return !compressAddFile(ro, ro->get(PacketDir),
		ro->get(PacketName), oldFileN ? oldFileN : readFileN);
}

const char *main_read_class::readFilePath(const char *FileN)
{
	static char tmp[13];

	sprintf(tmp, "%.8s.red", findBaseName(FileN));
	return tmp;
}

/* reply_read_class -- for reply areas */
/* (Formerly known as dummy_read_class, because it does almost nothing) */

reply_read_class::reply_read_class(mmail *, specific_driver *)
{
}

reply_read_class::~reply_read_class()
{
}

void reply_read_class::init()
{
}

void reply_read_class::setRead(int, int, bool)
{
}

bool reply_read_class::getRead(int, int)
{
	return true;
}

void reply_read_class::setStatus(int, int, int)
{
}

int reply_read_class::getStatus(int, int)
{
	return 1;
}

int reply_read_class::getNoOfUnread(int)
{
	return 0;
}

int reply_read_class::getNoOfMarked(int)
{
	return 0;
}

bool reply_read_class::saveAll()
{
	return true;
}
