/*
 * MultiMail offline mail reader
 * Blue Wave

 Copyright (c) 1996 Toth Istvan <stoty@vma.bme.hu>
 Copyright (c) 2003 William McBrine <wmcbrine@users.sf.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#ifndef BW_H
#define BW_H

#include "pktbase.h"

#ifndef BIG_ENDIAN
#define BIG_ENDIAN
#endif

#include "bluewave.h"

class bluewave : public pktbase
{
	FILE *ftiFile;
	INF_HEADER infoHeader;
	INF_AREA_INFO *areas;
	MIX_REC *mixRecord;

	struct perstype {
		int area, msgnum;
	} *persNdx;

	long personal;

	int *mixID;
	int noOfInfRecs, noOfMixRecs;
	int from_to_len;
	int subject_len;
	unsigned infoHeaderLen, infoAreainfoLen, mixStructLen, ftiStructLen;

	FILE *openFile(const char *);
	void findInfBaseName();
	void initInf();
	void initMixID();

	void getblk(int, long &, long, unsigned char *&, unsigned char *&);
	void endproc(letter_header &);
 public:
	bluewave(mmail *);
	~bluewave();
	bool hasPersonal();
	area_header *getNextArea();
	int getNoOfLetters();
	letter_header *getNextLetter();
	const char *getTear(int);
	const char *oldFlagsName();
	bool readOldFlags();
	bool saveOldFlags();
	INF_HEADER &getInfHeader();
};

class bwreply : public pktreply
{
	class upl_bw : public upl_base {
	 public:
		UPL_REC uplRec;
		char *msgid, *newsgrps, *extsubj;

		upl_bw(const char * = 0);
		~upl_bw();
	};

	UPL_HEADER *uplHeader;

	int getAreaFromTag(const char *);
	bool getRep1(FILE *, upl_bw *, int);
	void getReplies(FILE *);
	const char *freeFileName();
	void addRep1(FILE *, upl_base *, int);
	void addHeader(FILE *);
	void repFileName();
	const char *repTemplate(bool);
	char *nextLine(FILE *);
 public:
	bwreply(mmail *, specific_driver *);
	~bwreply();
	area_header *getNextArea();
	letter_header *getNextLetter();
	void enterLetter(letter_header &, const char *, long);
	bool getOffConfig();
	bool makeOffConfig();
};

/* To ensure correct operation where alignment padding is used, when
   reading from or writing to disk, use the _SIZE defines given here
   rather than "sizeof":
*/

#define UPI_HEAD_SIZE 55
#define PDQ_REC_SIZE 21

#endif
