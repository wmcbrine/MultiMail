/*
 * MultiMail offline mail reader
 * Packet base class -- common methods

 Copyright (c) 2003 William McBrine <wmcbrine@users.sf.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "compress.h"
#include "pktbase.h"

/* Not all these methods are used in all derived classes. The formats I
   think of as "QWK-like" -- QWK, OMEN and OPX -- have the most in common:
   message headers and bodies concatenated in a single file, a separate
   file for the area list, and replies matched by conference number.
*/

// -----------------------------------------------------------------
// The packet methods
// -----------------------------------------------------------------

pktbase::pktbase(mmail *mmA)
{
	mm = mmA;
	ID = 0;
	bodyString = 0;
	bulletins = 0;
	infile = 0;
	LoginName = AliasName = BBSName = SysOpName = 0;
}

pktbase::~pktbase()
{
	if (infile)
		fclose(infile);

	delete[] body;
	delete bodyString;
	delete[] bulletins;
	delete[] SysOpName;
	delete[] BBSName;
	delete[] AliasName;
	delete[] LoginName;
}

// Clean up for the QWK-like packets
void pktbase::cleanup()
{
	if (!hasPers) {
		areas--;
		maxConf++;
	}
	while (maxConf--) {
		delete[] body[maxConf];
		delete[] areas[maxConf].name;
	}
	delete[] areas;
}

// Final index build for QWK-like packets
void pktbase::initBody(ndx_fake *tmpndx, int personal)
{
	int x, cMsgNum;

	hasPers = !(!personal);
	if (hasPers) {
		body[0] = new bodytype[personal];
		areas[0].nummsgs = personal;
		personal = 0;
	} else {
		areas++;
		maxConf--;
	}

	for (x = hasPers; x < maxConf; x++) {
		cMsgNum = areas[x].nummsgs;
		if (cMsgNum) {
			body[x] = new bodytype[cMsgNum];
			areas[x].nummsgs = 0;
		}
	}

	for (x = 0; x < numMsgs; x++) {
		int current = tmpndx->confnum - !hasPers;
		body[current][areas[current].nummsgs].pointer =
			tmpndx->pointer;
		body[current][areas[current].nummsgs++].msgLength =
			tmpndx->length;

		if (tmpndx->pers) {
			body[0][personal].pointer = tmpndx->pointer;
			body[0][personal++].msgLength = tmpndx->length;
		}
		ndx_fake *oldndx = tmpndx;
		tmpndx = tmpndx->next;
		delete oldndx;
	}
}

// Match the conference number to the internal number
int pktbase::getXNum(int area)
{
	static int lastres = 0;

	if ((lastres >= maxConf) || (areas[lastres].num > area))
		lastres = 0;

	for (int x = 0; x < maxConf; x++)
		if (areas[lastres].num == area)
			break;
		else {
			lastres++;
			lastres %= maxConf;
		}

	return (areas[lastres].num == area) ? lastres : -1;
}

// Find the original number of a message in a collection area
int pktbase::getYNum(int area, unsigned long rawpos)
{
	static int lastarea = -1;
	static int lastres = 0;

	if (lastarea != area) {
		lastarea = area;
		lastres = 0;
	}

	if (-1 == area)
		return -1;

	int x, limit = areas[area].nummsgs;
	for (x = 0; x < limit; x++)
		if ((unsigned long) body[area][lastres].pointer == rawpos)
			break;
		else {
			lastres++;
			lastres %= limit;
		}

	return (x < limit) ? lastres : -1;
}

int pktbase::getNoOfLetters()
{
	return areas[currentArea].nummsgs;
}

bool pktbase::hasPersArea()
{
	return hasPers;
}

bool pktbase::hasPersonal()
{
	return false;
}

bool pktbase::isLatin()
{
	return false;
}

const char *pktbase::oldFlagsName()
{
	return 0;
}

bool pktbase::readOldFlags()
{
	return false;
}

bool pktbase::saveOldFlags()
{
	return false;
}

int pktbase::getNoOfAreas()
{
	return maxConf;
}

void pktbase::selectArea(int area)
{
	currentArea = area;
	resetLetters();
}

void pktbase::resetLetters()
{
	currentLetter = 0;
}

// returns the body of the requested letter
letter_body *pktbase::getBody(letter_header &mhead)
{
	int AreaID, LetterID;
	long length, offset;
	letter_body head(0, 0), *currblk = &head;

	AreaID = mhead.getAreaID() - 1;
	LetterID = mhead.getLetterID();

	delete bodyString;

	length = limitmem(body[AreaID][LetterID].msgLength);
	offset = body[AreaID][LetterID].pointer;

	bool firstblk = true;

	if (!length)
	    head.next = new letter_body(strdupplus("\n"), 1);
	else
	    while (length) {
		unsigned char *p, *src, *begin;
		long count, blklen, oldoffs;

		fseek(infile, offset, SEEK_SET);
		oldoffs = offset;

		if (firstblk)
			prefirstblk();

		blklen = (length > MAXBLOCK) ? MAXBLOCK : length;
		src = begin = p = new unsigned char[blklen + 1];

		getblk(AreaID, offset, blklen, p, begin);

		if (length > MAXBLOCK) {
			if (begin > src)
				count = begin - src;
			else {
				offset = ftell(infile);
				count = p - src;
			}
			length -= (offset - oldoffs);
		} else {
			// Strip blank lines
			do
				p--;
			while ((*p == ' ') || (*p == '\n'));

			length = 0;
			count = p - src + 1;
		}
		src[count] = '\0';

		p = src;
		if (firstblk)
			postfirstblk(p, mhead);

		currblk->next = new letter_body((char *) src, count,
			p - src);
		currblk = currblk->next;

		firstblk = false;
	    }
	bodyString = head.next;
	head.next = 0;		// Prevent deletion of chain

	endproc(mhead);

	return bodyString;
}

void pktbase::getblk(int, long &offset, long blklen,
	unsigned char *&p, unsigned char *&begin)
{
	for (long count = 0; count < blklen; count++) {
		int kar = fgetc(infile);

		if (!kar)
			kar = ' ';

		if (kar != '\r')
			*p++ = kar;

		if (kar == '\n') {
			begin = p;
			offset = ftell(infile);
		}
	}
}

// The new getBody() framework uses 4 subprocedures, only one of which
// must be defined in all derived classes. Here are dummy procedures for
// the others.

void pktbase::prefirstblk()
{
}

void pktbase::postfirstblk(unsigned char *&, letter_header &)
{
}

void pktbase::endproc(letter_header &)
{
}

// Check the character set kludge lines
void pktbase::checkLatin(letter_header &mhead)
{
	const char *s = strstr(bodyString->getText(), "\001CHRS: L");
	if (!s)
		s = strstr(bodyString->getText(), "\001CHARSET: L");
	if (s)
		mhead.setLatin(true);
}

// Find a hidden line and return its contents
const char *pktbase::getHidden(const char *pattern, char *&end)
{
	char *s = strstr(bodyString->getText(), pattern);

	if (s) {
		s += strlen(pattern);
		end = strchr(s, '\n');
		if (end)
			*end = '\0';
	}
	return s;
}

// Check FMPT, MSGID and character set
void pktbase::fidocheck(letter_header &mhead)
{
	const char *s;
	char *end;

	net_address &na = mhead.getNetAddr();

	// Add point to netmail address, if possible/necessary:
	if (na.isSet)
		if (!na.point) {
			s = strstr(bodyString->getText(),
				"\001FMPT");
			if (s)
				sscanf(s, "\001FMPT%u\n", &na.point);
		}

	// Get MSGID:
	if (!mhead.getMsgID()) {
		s = getHidden("\001MSGID: ", end);
		if (s) {
			mhead.changeMsgID(s);
			if (end)
				*end = '\n';
		}
	}

	// Change to Latin character set, if necessary:
	checkLatin(mhead);
}

// Build a list of bulletin files
void pktbase::listBulletins(const char x[][13], int d, int generic)
{
	file_list *wl = mm->workList;
	int filecount = 0;

	bulletins = new file_header *[wl->getNoOfFiles() + 1];

	for (int c = 0; c < d; c++)
		if (x[c][0])
			wl->addItem(bulletins, x[c], filecount);

	if (generic) {
		wl->addItem(bulletins, "blt", filecount);
		if (generic == 2)
			wl->addItem(bulletins, ".txt", filecount);
	}

	if (filecount)
		bulletins[filecount] = 0;
	else {
		delete[] bulletins;
		bulletins = 0;
	}
}

const char *pktbase::getLoginName()
{
	return LoginName;
}

const char *pktbase::getAliasName()
{
	return AliasName;
}

const char *pktbase::getBBSName()
{
	return BBSName;
}

const char *pktbase::getSysOpName()
{
	return SysOpName;
}

file_header *pktbase::getHello()
{
	return 0;
}

file_header *pktbase::getGoodbye()
{
	return 0;
}

file_header *pktbase::getFileList()
{
	return mm->workList->existsF("newfiles.");
}

file_header **pktbase::getBulletins()
{
	return bulletins;
}

const char *pktbase::getTear(int)
{
	static char tear[80];

	sprintf(tear, "--- " MM_NAME "/%.58s v" MM_VERNUM, sysname());

	return tear;
}

const char *pktbase::getBaseName()
{
	return packetBaseName;
}

char *pktbase::nextLine()
{
	static char line[128];

	char *end = myfgets(line, sizeof line, infile);
	if (end) {
		while ((*end == '\n') || (*end == '\r'))
			*end-- = '\0';
	} else
		line[0] = '\0';

	return line;
}

// -----------------------------------------------------------------
// The reply methods
// -----------------------------------------------------------------

pktreply::upl_base::upl_base(const char *name)
{
	if (name)
		fname = strdupplus(name);
	else 
		fname = mytmpnam();

	nextRecord = 0;
	msglen = 0;
}

pktreply::upl_base::~upl_base()
{
	delete[] fname;
}

pktreply::pktreply(mmail *mmA, specific_driver *baseClassA)
{
	mm = mmA;
	baseClass = (pktbase *) baseClassA;
	replyText = 0;
	uplListHead = 0;
	replyExists = false;
}

pktreply::~pktreply()
{
	if (replyExists) {
		upl_base *next, *curr = uplListHead;

		while (noOfLetters--) {
			remove(curr->fname);
			next = curr->nextRecord;
			delete curr;
			curr = next;
		}
		delete replyText;
	}
}

bool pktreply::checkForReplies()
{
	repFileName();
	mychdir(mm->resourceObject->get(ReplyDir));

	mystat st(replyPacketName);
	replyExists = st.writeable();

	return replyExists;
}

void pktreply::init()
{
	if (replyExists) {
		uncompress();
		readRep();
		currentLetter = 1;
	} else
		noOfLetters = currentLetter = 0;
}

void pktreply::uncompress()
{
	resource *ro = mm->resourceObject;
	char *tmppath = fullpath(ro->get(ReplyDir), replyPacketName);

	uncompressFile(ro, tmppath, ro->get(UpWorkDir));

	delete[] tmppath;
}

int pktreply::getNoOfAreas()
{
	return 1;
}

void pktreply::selectArea(int ID)
{
	if (ID == 0)
		resetLetters();
}

int pktreply::getNoOfLetters()
{
	return noOfLetters;
}

void pktreply::resetLetters()
{
	currentLetter = 1;
	uplListCurrent = uplListHead;
}

letter_body *pktreply::getBody(letter_header &mhead)
{
	FILE *replyFile;
	upl_base *actUplList;
	long length, offset = 0;
	letter_body head(0,0), *currblk = &head;

	int ID = mhead.getLetterID();

	delete replyText;

	actUplList = uplListHead;
	for (int c = 1; c < ID; c++)
		actUplList = actUplList->nextRecord;

	length = limitmem(actUplList->msglen);
	replyFile = fopen(actUplList->fname, "rt");

	while (length) {
		if (replyFile) {
			unsigned char *p, *src, *begin;
			long blklen, count, oldoffs;

			fseek(replyFile, offset, SEEK_SET);
			oldoffs = offset;

			blklen = (length > MAXBLOCK) ? MAXBLOCK : length;
			src = begin = p = new unsigned char[blklen + 1];

			for (count = 0; count < blklen; count++) {
				int kar = fgetc(replyFile);

				if (!kar)
					kar = ' ';

				*p++ = kar;

				if (kar == '\n') {
					begin = p;
					offset = ftell(replyFile);
				}
			}

			if (length > MAXBLOCK) {
				if (begin > src)
					p = begin;
				else
					offset = ftell(replyFile);

				length -= (offset - oldoffs);
			} else
				length = 0;

			*p = '\0';
			count = p - src;

			currblk->next = new letter_body((char *) src, count);
			currblk = currblk->next;
		} else {
			head.next = new letter_body(strdupplus("\n"), 1);
			length = 0;
		}
	}
	if (replyFile)
		fclose(replyFile);

	replyText = head.next;
	head.next = 0;

	return replyText;
}

bool pktreply::hasPersArea()
{
	return false;
}

bool pktreply::hasPersonal()
{
	return false;
}

bool pktreply::isLatin()
{
	return false;
}

const char *pktreply::oldFlagsName()
{
	return 0;
}

bool pktreply::readOldFlags()
{
	return false;
}

bool pktreply::saveOldFlags()
{
	return false;
}

const char *pktreply::getLoginName()
{
	return 0;
}

const char *pktreply::getAliasName()
{
	return 0;
}

const char *pktreply::getBBSName()
{
	return 0;
}

const char *pktreply::getSysOpName()
{
	return 0;
}

file_header *pktreply::getHello()
{
	return 0;
}

file_header *pktreply::getGoodbye()
{
	return 0;
}

file_header *pktreply::getFileList()
{
	return 0;
}

file_header **pktreply::getBulletins()
{
	return 0;
}

const char *pktreply::getTear(int)
{
	return 0;
}

void pktreply::readRep()
{
	upWorkList = new file_list(mm->resourceObject->get(UpWorkDir));

	FILE *repFile = upWorkList->ftryopen(replyInnerName);

	if (repFile) {
		getReplies(repFile);
		fclose(repFile);
		remove(upWorkList->exists(replyInnerName));
	} else
		fatalError("Error opening reply packet");

	delete upWorkList;
}

void pktreply::addUpl(upl_base *newList)
{
        if (!noOfLetters)
                uplListHead = newList;
        else {
                upl_base *workList = uplListHead;
                for (int c = 1; c < noOfLetters; c++)   //go to last elem
                        workList = workList->nextRecord;
                workList->nextRecord = newList;
        }
        noOfLetters++;
        replyExists = true;
}

void pktreply::killLetter(int letterNo)
{
	upl_base *actUplList, *tmpUplList;

	if (!noOfLetters || (letterNo < 1) || (letterNo > noOfLetters))
		fatalError("Internal error in pktreply::killLetter");

	if (letterNo == 1) {
		tmpUplList = uplListHead;
		uplListHead = uplListHead->nextRecord;
	} else {
		actUplList = uplListHead;
		for (int c = 1; c < letterNo - 1; c++)
			actUplList = actUplList->nextRecord;
		tmpUplList = actUplList->nextRecord;
		actUplList->nextRecord = (letterNo == noOfLetters) ? 0 :
			actUplList->nextRecord->nextRecord;
	}
	noOfLetters--;
	remove(tmpUplList->fname);
	delete tmpUplList;
	resetLetters();
}

area_header *pktreply::refreshArea()
{
	return getNextArea();
}

bool pktreply::makeReply()
{
	if (mychdir(mm->resourceObject->get(UpWorkDir)))
		fatalError("Could not cd to upworkdir in pktreply::makeReply");

	bool offres = mm->areaList->anyChanged();
	if (offres)
		offres = makeOffConfig();
	if (!noOfLetters && !offres) {
		deleteReplies();
		return true;
	}

	FILE *repFile;

	repFile = fopen(replyInnerName, "wb");	//!! no check yet

	addHeader(repFile);

	upl_base *actUplList = uplListHead;
	for (int c = 0; c < noOfLetters; c++) {
		addRep1(repFile, actUplList, c);
		actUplList = actUplList->nextRecord;
	}

	fclose(repFile);

	// delete old packet
	deleteReplies();

	// pack the files
	int result = compressAddFile(mm->resourceObject,
		mm->resourceObject->get(ReplyDir), replyPacketName,
		repTemplate(offres));

	// clean up the work area
	clearDirectory(mm->resourceObject->get(UpWorkDir));

	return !result && checkForReplies();
}

void pktreply::deleteReplies()
{
	char *tmppath = fullpath(mm->resourceObject->get(ReplyDir),
		replyPacketName);

	remove(tmppath);

	delete[] tmppath;
}
