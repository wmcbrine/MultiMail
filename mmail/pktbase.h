/*
 * MultiMail offline mail reader
 * Packet base class

 Copyright (c) 2003 William McBrine <wmcbrine@users.sf.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#ifndef PKT_H
#define PKT_H

#include "mmail.h"

class pktbase : public specific_driver
{
 protected:
	struct bodytype {
		long pointer, msgLength;
	} **body;

	struct AREAs {
		char *name;
		int num, nummsgs;
		unsigned long attr;
		char numA[10];  // padded to deal with alignment bug (EMX)
	} *areas;

	struct ndx_fake {
		int confnum;
		long pointer, length;
		bool pers;
		ndx_fake *next;
	};

	mmail *mm;
	letter_body *bodyString;
	file_header **bulletins;

	FILE *infile;
	char packetBaseName[9];
	char *LoginName, *AliasName, *BBSName, *SysOpName, *DoorProg, *BBSProg;
	char *hello, *goodbye;
	int maxConf, numMsgs, ID, currentArea, currentLetter;
	unsigned long hasOffConfig;
	bool hasPers;

	void cleanup();
	void initBody(ndx_fake *, int);
	int getYNum(int, unsigned long);
	void checkLatin(letter_header &);
	const char *getHidden(const char *, char *&);
	void fidocheck(letter_header &);
	void listBulletins(const char [][13], int, int = 2);
	char *nextLine();

	virtual void prefirstblk();
	virtual void getblk(int, long &, long, unsigned char *&,
		unsigned char *&);
	virtual void postfirstblk(unsigned char *&, letter_header &);
	virtual void endproc(letter_header &);
 public:
	pktbase(mmail *);
	~pktbase();
	int getXNum(int);
	int getNoOfAreas();
	virtual int getNoOfLetters();
	void selectArea(int);
	void resetLetters();
	bool hasPersArea();
	virtual bool hasPersonal();
	virtual bool isLatin();
	virtual const char *oldFlagsName();
	virtual bool readOldFlags();
	virtual bool saveOldFlags();
	virtual letter_body *getBody(letter_header &);
	const char *getLoginName();
	const char *getAliasName();
	const char *getBBSName();
	const char *getSysOpName();
	const char *getBBSProg();
	const char *getDoorProg();
	file_header *getHello();
	file_header *getGoodbye();
	file_header **getBulletins();
	virtual const char *getTear(int);
	const char *getBaseName();
};

class pktreply : public reply_driver
{
 protected:
	class upl_base {
	 public:
		char *fname;
		upl_base *nextRecord;
		long msglen;

		upl_base(const char *);
		~upl_base();
	} *uplListHead, *uplListCurrent;

	mmail *mm;
	pktbase *baseClass;
	file_list *upWorkList;
	letter_body *replyText;

	char replyPacketName[13], replyInnerName[13];
	int currentLetter, noOfLetters;
	bool replyExists;

	void uncompress();
	virtual void getReplies(FILE *) = 0;
	void readRep();
	virtual void repFileName() = 0;
	void addUpl(upl_base *);
	virtual void addRep1(FILE *, upl_base *, int) = 0;
	virtual void addHeader(FILE *) = 0;
	virtual const char *repTemplate(bool) = 0;
 public:
	pktreply(mmail *, specific_driver *);
	~pktreply();
	bool checkForReplies();
	void init();
	int getNoOfAreas();
	void selectArea(int);
	int getNoOfLetters();
	void resetLetters();
	letter_body *getBody(letter_header &);
	bool hasPersArea();
	bool hasPersonal();
	bool isLatin();
	const char *oldFlagsName();
	bool readOldFlags();
	bool saveOldFlags();
	const char *getLoginName();
	const char *getAliasName();
	const char *getBBSName();
	const char *getSysOpName();
	const char *getBBSProg();
	const char *getDoorProg();
	file_header *getHello();
	file_header *getGoodbye();
	file_header *getFileList();
	file_header **getBulletins();
	const char *getTear(int);
	void killLetter(int);
	area_header *refreshArea();
	bool makeReply();
	void deleteReplies();
};

#endif
