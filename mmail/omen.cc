/*
 * MultiMail offline mail reader
 * OMEN

 Copyright (c) 2003 William McBrine <wmcbrine@users.sf.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "omen.h"
#include "compress.h"

// Area attributes in SYSTEMxy.BBS
enum {OM_WRITE = 1, OM_SYSOP = 2, OM_PRIVATE = 4, OM_PUBLIC = 8,
	OM_NETMAIL = 0x10, OM_ALIAS = 0x20, OM_ACTIVE = 0x40};

// -----------------------------------------------------------------
// The OMEN methods
// -----------------------------------------------------------------

omen::omen(mmail *mmA) : pktbase(mmA)
{
	strcpy(extent, defExtent());

	readSystemBBS();

	infile = mm->workList->ftryopen("newmsg");
	if (!infile)
		fatalError("Could not open NEWMSGxy.TXT");

	buildIndices();

	const char x[1][13] = {"bullet"};
	listBulletins(x, 1, 0);
}

omen::~omen()
{
	cleanup();
}

file_header *omen::getFileList()
{
	return mm->workList->existsF("nfile");
}

area_header *omen::getNextArea()
{
	int cMsgNum = areas[ID].nummsgs;
	bool x = (areas[ID].num == -1);

	area_header *tmp = new area_header(mm, ID + 1, areas[ID].numA,
		areas[ID].name, (x ? "Letters addressed to you" :
		areas[ID].name), (x ? "OMEN personal" : "OMEN"),
		areas[ID].attr | (cMsgNum ? ACTIVE : 0), cMsgNum,
		0, 35, 72);
	ID++;
	return tmp;
}

// Build indices from NEWMSGxy.TXT
void omen::buildIndices()
{
	int x;

	body = new bodytype *[maxConf];

	for (x = 0; x < maxConf; x++) {
		body[x] = 0;
		areas[x].nummsgs = 0;
	}

	ndx_fake base, *tmpndx = &base;

	char junk[12];
	const char *p;
	int nlen, personal = 0;
	long counter;
	const char *name = mm->resourceObject->get(UserName);
	nlen = name ? strlen(name) : 0;
	bool checkpers = name && nlen;

	numMsgs = 0;

	while (!feof(infile)) {
		counter = ftell(infile);
		p = nextLine();

		if (!feof(infile) && (*p++ == 1)) {
			tmpndx->next = new ndx_fake;
			tmpndx = tmpndx->next;

			// get area number:
			sscanf(p, "#%s  %d:", junk, &x);
			x = getXNum(x);

			tmpndx->confnum = x;
			tmpndx->pointer = counter;

			tmpndx->pers = false;
			if (checkpers) {
				p = nextLine();
				p = strchr(p, '=') + 3;
				if (!strncasecmp(p, name, nlen)) {
					tmpndx->pers = true;
					personal++;
				}
			}
			numMsgs++;
			areas[x].nummsgs++;

			// skip rest of header:
			while (!feof(infile) && (fgetc(infile) != 2));

			// find length of text:
			long tlen = 0;
			while (!feof(infile) && (fgetc(infile) != 3))
				tlen++;
			tmpndx->length = tlen;
		}
	}

	initBody(base.next, personal);
}

letter_header *omen::getNextLetter()
{
	char date[20], subject[73], c, priflag = 0, *from, *to, *net;
	unsigned long pos;
	int areaID, letterID, x;
	long msgnum, refnum = 0, junk;
	net_address na;

	pos = body[currentArea][currentLetter].pointer;

	fseek(infile, pos + 1, SEEK_SET);
	long len = body[currentArea][currentLetter].msgLength;

	// First header line -- msgnum, date, refnum, priflag:

	from = nextLine() + 1;
	to = strchr(from, ' ');
	*to = '\0';
	msgnum = atol(from);

	from = to + 2;
	sscanf(from, "%d:", &areaID);
	while (!((from[0] == ' ') && (from[1] == ' ')))
		from = strchr(from + 1, ' ');
	from += 2;

	to = strchr(from, '(');
	if (to) {
		to[-2] = '\0';
		sscanf(to, "(%ld/%ld)", &refnum, &junk);
		to = strchr(to + 1, '(');
		priflag = to[1];
	}
	strcpy(date, from);

	// Second line -- to and from:

	na.zone = 0;

	from = nextLine();
	to = strchr(from, '=');
	if (to) {
		to[-1] = '\0';
		to += 3;

		net = strchr(to, '(');
		if (net) {
			net[-7] = '\0';
			na = ++net;
		}
	}

	// Third line -- subject:

	// skip "Subj: "
	for (x = 0; x < 6; x++)
		fgetc(infile);
	x = 0;

	// can't use fgets because line is not LF-terminated
	do {
		c = fgetc(infile);
		subject[x++] = c;
	} while ((c != 2) && (x != 73));
	subject[--x] = '\0';

	if (areas[currentArea].num == -1) {
		areaID = getXNum(areaID);
		letterID = getYNum(areaID, pos);
	} else {
		areaID = currentArea;
		letterID = currentLetter;
	}

	currentLetter++;

	return new letter_header(mm, subject, to, from, date, 0, refnum,
		letterID, msgnum, areaID, (priflag == 'P'), len, this, na,
		!(!(areas[areaID].attr & LATINCHAR)));
}

void omen::prefirstblk()
{
	// Skip header

	while (!feof(infile) && (fgetc(infile) != 2));
}

// Area and packet init: SYSTEMxy.BBS, BNAMESxy.BBS, INFOxy.BBS
void omen::readSystemBBS()
{
	struct ATMP {
		unsigned char BrdNum, BrdStatus, BrdHighNum, BrdNameLen;
		char BrdName[16];
	} *areatmp;

	file_list *wl = mm->workList;

	hasOffConfig = useLatin = 0;

	// The following info is unavailable in OMEN:
	const char *defName = mm->resourceObject->get(UserName);

	mm->resourceObject->set(LoginName, (defName && *defName) ?
		defName: "(set on upload)");
	mm->resourceObject->set(AliasName, "(set on upload)");

	// INFOxy.BBS:

	infile = wl->ftryopen("info");
	if (infile) {
		while (!feof(infile)) {
			const char *line = nextLine();
			if (!strncasecmp(line, "sysop:", 6))
				SysOpName = strdupplus(line + 6);
			else
				if (!strcasecmp(line, "select:on"))
					hasOffConfig = OFFCONFIG;
				else
					if (!strcasecmp(line, "c_set:iso"))
						useLatin = LATINCHAR;
		}
		fclose(infile);
	}

	// SYSTEMxy.BBS, and BNAMESxy.BBS if available:

	infile = wl->ftryopen("system");
	if (infile) {
		const char *s = wl->getName();
		packetBaseName[0] = toupper(s[6]);
		packetBaseName[1] = toupper(s[7]);
		packetBaseName[2] = '\0';

		maxConf = (wl->getSize() - 41) / sizeof(ATMP) + 1;
		struct {
			unsigned char len;
			char name[41];
		} b;

		fread(&b, 1, 41, infile);
		b.name[b.len] = '\0';
		BBSName = strdupplus(b.name);

		areas = new AREAs[maxConf];
		areatmp = new ATMP[maxConf - 1];

		fread(areatmp, sizeof(ATMP), maxConf - 1, infile);
		fclose(infile);

		areas[0].num = -1;
		strcpy(areas[0].numA, "PERS");
		areas[0].name = strdupplus("PERSONAL");
		areas[0].attr = PUBLIC | PRIVATE | COLLECTION;

		infile = wl->ftryopen("bnames");
		for (int x = 1; x < maxConf; x++) {
			areas[x].num = (int) (areatmp[x - 1].BrdHighNum <<
				8) + areatmp[x - 1].BrdNum;
			sprintf(areas[x].numA, "%d", areas[x].num);

			int a = areatmp[x - 1].BrdStatus;
			areas[x].attr = ((a & OM_ACTIVE) ? ACTIVE : 0) |
				((a & OM_PRIVATE) ? PRIVATE : 0) |
				((a & OM_PUBLIC) ? PUBLIC : 0) |
				((a & OM_NETMAIL) ? NETMAIL : 0) |
				((a & OM_ALIAS) ? ALIAS : 0) |
				((a & OM_WRITE) ? 0 : READONLY) |
				hasOffConfig | useLatin;

			if (infile)		// use long area names
				areas[x].name =
				    strdupplus(strchr(nextLine(), ':') + 1);
			else {
				int len = areatmp[x - 1].BrdNameLen;
				areas[x].name = new char[len + 1];
				strncpy(areas[x].name,
				    areatmp[x - 1].BrdName, len);
				areas[x].name[len] = '\0';
			}
		}
		if (infile)
			fclose(infile);
		delete[] areatmp;
	}
}

const char *omen::getExtent()
{
	return extent;
}

bool omen::isLatin()
{
	return !(!(useLatin & LATINCHAR));
}

// -----------------------------------------------------------------
// The OMEN reply methods
// -----------------------------------------------------------------

// Letter attributes in HEADERxy.BBS
enum {OMR_SAVE = 1, OMR_DEL = 2, OMR_TOGGLE = 4, OMR_MOVE = 8,
	OMR_PRIVATE = 0x10, OMR_ALIAS = 0x20};

omenrep::upl_omen::upl_omen(const char *name) : pktreply::upl_base(name)
{
	memset(&omen_rec, 0, sizeof(omen_rec));
}

// convert OMEN reply packet header to C structures
bool omenrep::upl_omen::init(FILE *rep)
{
	if (fread(&omen_rec, sizeof omen_rec, 1, rep) != 1)
		return false;

	refnum = getshort(omen_rec.msghighnumber) << 16 +
		getshort(omen_rec.msgnumber);
	privat = !(!(omen_rec.command & OMR_PRIVATE));

	na.zone = getshort(omen_rec.destzone);
	if (na.zone) {
		na.net = getshort(omen_rec.destnet);
		na.node = getshort(omen_rec.destnode);
		na.point = 0;	// points aren't supported :-(
		na.isSet = true;
	}
	strncpy(subject, omen_rec.subject, omen_rec.sublen);
	subject[omen_rec.sublen] = '\0';
	strncpy(to, omen_rec.to, omen_rec.tolen);
	to[omen_rec.tolen] = '\0';

	origArea = getshort(&omen_rec.curboard);

	return true;
}

// write out OMEN reply packet header
void omenrep::upl_omen::output(FILE *rep)
{
	memset(&omen_rec, 0, sizeof omen_rec);

	omen_rec.command = OMR_SAVE | (privat ? OMR_PRIVATE : 0);

	int refhigh = refnum >> 16;
	int reflow = refnum & 0xffff;
	putshort(omen_rec.msghighnumber, refhigh);
	putshort(omen_rec.msgnumber, reflow);

	if (na.isSet) {
		putshort(omen_rec.destzone, na.zone);
		putshort(omen_rec.destnet, na.net);
		putshort(omen_rec.destnode, na.node);
	}
	omen_rec.sublen = strlen(subject);
	strncpy(omen_rec.subject, subject, omen_rec.sublen);
	omen_rec.tolen = strlen(to);
	strncpy(omen_rec.to, to, omen_rec.tolen);

	putshort(&omen_rec.curboard, origArea);

	fwrite(&omen_rec, sizeof omen_rec, 1, rep);
}

omenrep::omenrep(mmail *mmA, specific_driver *baseClassA)
{
	mm = mmA;
	baseClass = (pktbase *) baseClassA;

	replyText = 0;
	uplListHead = 0;

	replyExists = false;
}

omenrep::~omenrep()
{
	cleanup();
}

// convert one reply to MultiMail's internal format
bool omenrep::getRep1(FILE *rep, upl_omen *l, int recnum)
{
	FILE *orgfile, *destfile;
	char orgname[13];
	int c;
	long count = 0;

	if (!l->init(rep))
		return false;

	sprintf(orgname, "msg%s%02d.txt", baseClass->getBaseName(),
		recnum);

	orgfile = upWorkList->ftryopen(orgname);
	if (orgfile) {
	    destfile = fopen(l->fname, "wt");
	    if (destfile) {
		while ((c = fgetc(orgfile)) != EOF) {
			if (c != '\r') {
				fputc(c, destfile);
				count++;
			}
		}
		fclose(destfile);
	    }
	    fclose(orgfile);
	    upWorkList->kill();
	}

	l->msglen = count;

	return true;
}

// convert all replies
void omenrep::getReplies(FILE *repFile)
{
	noOfLetters = upWorkList->getSize() / sizeof(omenReplyRec);

	upl_omen baseUplList, *currUplList = &baseUplList;

	for (int c = 0; c < noOfLetters; c++) {
		currUplList->nextRecord = new upl_omen;
		currUplList = (upl_omen *) currUplList->nextRecord;
		if (!getRep1(repFile, currUplList, c)) {
			delete currUplList;
			break;
		}
	}
	uplListHead = baseUplList.nextRecord;
}

area_header *omenrep::getNextArea()
{
	return new area_header(mm, 0, "REPLY", "REPLIES",
		"Letters written by you",  "OMEN replies",
		(COLLECTION | REPLYAREA | ACTIVE | PUBLIC | PRIVATE),
		noOfLetters, 0, 35, 72);
}

letter_header *omenrep::getNextLetter()
{
	upl_omen *current = (upl_omen *) uplListCurrent;
	const char *defName = mm->resourceObject->get(UserName);

	letter_header *newLetter = new letter_header(mm,
		current->subject, current->to, (defName && *defName) ?
		defName : "(set on upload)", "(set on upload)", 0,
		current->refnum, currentLetter, currentLetter,
		((omen *) baseClass)->getXNum(current->origArea) + 1,
		current->privat, current->msglen, this, current->na,
		((omen *) baseClass)->isLatin());

	currentLetter++;
	uplListCurrent = uplListCurrent->nextRecord;
	return newLetter;
}

void omenrep::enterLetter(letter_header &newLetter,
			const char *newLetterFileName, long length)
{
	upl_omen *newList = new upl_omen(newLetterFileName);

	strncpy(newList->subject, newLetter.getSubject(), 72);
	strncpy(newList->to, newLetter.getTo(), 35);

	newList->origArea = atoi(mm->areaList->getShortName());
	newList->privat = newLetter.getPrivate();
	newList->refnum = newLetter.getReplyTo();
	newList->na = newLetter.getNetAddr();

	newList->msglen = length;

	addUpl(newList);
}

// write out one reply in OMEN format
void omenrep::addRep1(FILE *rep, upl_base *node, int recnum)
{
	FILE *orgfile, *destfile;
	upl_omen *l = (upl_omen *) node;
	char dest[13];

	l->output(rep);

	sprintf(dest, "MSG%s%02d.TXT", baseClass->getBaseName(),
		recnum);

	orgfile = fopen(l->fname, "rt");
	if (orgfile) {

		destfile = fopen(dest, "wb");
		if (destfile) {

			int c, count = 0, lastsp = 0;
			while ((c = fgetc(orgfile)) != EOF) {
				count++;
				if ((count > 80) && lastsp) {
					fseek(orgfile, lastsp - count,
						SEEK_CUR);
					fseek(destfile, lastsp - count,
						SEEK_CUR);
					c = '\n';
				}
				if ('\n' == c) {
					fprintf(destfile, "\r\n");
					count = lastsp = 0;
				} else {
					fputc(c, destfile);
					if (' ' == c)
						lastsp = count;
				}
			}
			fclose(destfile);
		}
		fclose(orgfile);
	}
}

void omenrep::addHeader(FILE *)
{
}

// set names for reply packet files
void omenrep::repFileName()
{
        const char *basename = baseClass->getBaseName();

	sprintf(replyPacketName, "return%c%c.%s", tolower(basename[0]),
		tolower(basename[1]), ((omen *) baseClass)->getExtent());
	sprintf(replyInnerName, "HEADER%s.BBS", basename);
}

// list files to be archived when creating reply packet
const char *omenrep::repTemplate(bool offres)
{
	static char buff[30];

	sprintf(buff, offres ? "%s *.TXT *.CNF" : "%s *.TXT",
		replyInnerName);

	return buff;
}

// re-read an offline config file
bool omenrep::getOffConfig()
{
	FILE *olc;

	bool status = false;
	upWorkList = new file_list(mm->resourceObject->get(UpWorkDir));

	olc = upWorkList->ftryopen("select");
	if (olc) {
		char line[128];
		int areaOMEN, areaNo, current = -1;

		area_list *al = mm->areaList;
		int maxareas = al->noOfAreas();

		// Like Blue Wave, areas are dropped unless listed in
		// the config file. Unlike Blue Wave, we can't be certain
		// of an area's status (SUBKNOWN), hence the extra checks.

		do {
			myfgets(line, sizeof line, olc);
			sscanf(line, "%d", &areaOMEN);
			areaNo = ((omen *) baseClass)->
				getXNum(areaOMEN) + 1;

			for (int c = current + 1; c < maxareas; c++) {
				al->gotoArea(c);
				if (c == areaNo) {
					if (!(al->getType() & ACTIVE))
						al->Add();
					current = c;
					break;
				} else
					if (al->getType() & ACTIVE)
						al->Drop();
			}
		} while (!feof(olc));

		fclose(olc);
		upWorkList->kill();

		status = true;
	}
	delete upWorkList;
	return status;
}

bool omenrep::makeOffConfig()
{
	FILE *todoor;
	char fname[13];

	sprintf(fname, "SELECT%s.CNF", baseClass->getBaseName());

	todoor = fopen(fname, "wb");
	if (!todoor)
		return false;

	int oldarea = mm->areaList->getAreaNo();

	int maxareas = mm->areaList->noOfAreas();
	for (int areaNo = 0; areaNo < maxareas; areaNo++) {
		mm->areaList->gotoArea(areaNo);
		unsigned long attrib = mm->areaList->getType();

		if (!(attrib & COLLECTION) && (((attrib & ACTIVE)
		    && !(attrib & DROPPED)) || (attrib & ADDED)))
 			fprintf(todoor, "%s\n", mm->areaList->getShortName());
	}
	mm->areaList->gotoArea(oldarea);
	fclose(todoor);

	return true;
}
