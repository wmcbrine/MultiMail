/*
 * MultiMail offline mail reader
 * QWK

 Copyright (c) 1997 John Zero <john@graphisoft.hu>
 Copyright (c) 2003 William McBrine <wmcbrine@users.sf.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "qwk.h"
#include "compress.h"

unsigned char *onecomp(unsigned char *p, char *dest, const char *comp)
{
	int len = strlen(comp);

	if (!strncasecmp((char *) p, comp, len)) {
		p += len;
		while (*p == ' ')
			p++;
		int x;
		for (x = 0; *p && (*p != '\n') && (x < 71); x++)
			dest[x] = *p++;
		dest[x] = '\0';

		while (*p == '\n')
			p++;
		return p;
	}
	return 0;
}

// -----------------------------------------------------------------
// The qheader methods
// -----------------------------------------------------------------

bool qheader::init(FILE *datFile)
{
	qwkmsg_header qh;
	char buf[9], *err;

	if (!fread(&qh, 1, sizeof qh, datFile))
		return false;

	getQfield(from, qh.from, 25);
	getQfield(to, qh.to, 25);
	getQfield(subject, qh.subject, 25);

	cropesp(from);
	cropesp(to);
	cropesp(subject);

	getQfield(date, qh.date, 8);
	date[2] = '-';
	date[5] = '-';		// To deal with some broken messages
	strcat(date, " ");

	getQfield(buf, qh.time, 5);
	strcat(date, buf);

	getQfield(buf, qh.refnum, 8);
	refnum = atol(buf);

	getQfield(buf, qh.msgnum, 7);
	cropesp(buf);
	msgnum = strtol(buf, &err, 10);
	if (*err)
		return false;	// bogus message

	getQfield(buf, qh.chunks, 6);
	msglen = (atol(buf) - 1) << 7;

	privat = (qh.status == '*') || (qh.status == '+');

	// Is this a block of net-status flags?
	netblock = !qh.status || (qh.status == '\xff');

	origArea = getshort(&qh.confLSB);

	return true;
}

// Just the fields needed for building the index:
bool qheader::init_short(FILE *datFile)
{
	qwkmsg_header qh;
	char buf[9];

	if (!fread(&qh, 1, sizeof qh, datFile))
		return false;

	getQfield(to, qh.to, 25);

	cropesp(to);

	getQfield(buf, qh.chunks, 6);
	msglen = (atol(buf) - 1) << 7;

	// Is this a block of net-status flags?
	netblock = !qh.status || (qh.status == '\xff');

	origArea = getshort(&qh.confLSB);

	return true;
}

// Write the header to a file, except for the length:
void qheader::output(FILE *repFile)
{
	qwkmsg_header qh;
	char buf[10];
	int sublen;

	sublen = strlen(subject);
	if (sublen > 25)
		sublen = 25;

	memset(&qh, ' ', sizeof qh);

	sprintf(buf, " %-6ld", msgnum);
	strncpy(qh.msgnum, buf, 7);

	putshort(&qh.confLSB, msgnum);

	if (refnum) {
		sprintf(buf, " %-7ld", refnum);
		strncpy(qh.refnum, buf, 8);
	}
	strncpy(qh.to, to, strlen(to));
	strncpy(qh.from, from, strlen(from));
	strncpy(qh.subject, subject, sublen);

	qh.alive = (char) 0xE1;

	strncpy(qh.date, date, 8);
	strncpy(qh.time, &date[9], 5);

	if (privat)
		qh.status = '*';

	fwrite(&qh, 1, sizeof qh, repFile);
}

// Pad out with spaces, as necessary, and write the length to the header:
void qheader::set_length(FILE *repFile, long headerpos, long curpos)
{
	long length;

	for (length = curpos - headerpos; (length & 0x7f); length++)
		fputc(' ', repFile);

	fseek(repFile, headerpos + CHUNK_OFFSET, SEEK_SET);
	fprintf(repFile, "%-6ld", (length >> 7));
	fseek(repFile, headerpos + length, SEEK_SET);
}

// -----------------------------------------------------------------
// The QWK methods
// -----------------------------------------------------------------

qwkpack::qwkpack(mmail *mmA) : pktbase(mmA)
{
	//mm = mmA;
	//ID = 0;
	//bodyString = 0;

	qwke = !(!mm->workList->exists("toreader.ext"));

	readControlDat();
	if (qwke)
		readToReader();
	else
		readDoorId();

	infile = mm->workList->ftryopen("messages.dat");
	if (!infile)
		fatalError("Could not open MESSAGES.DAT");

	readIndices();

	listBulletins(&textfiles[1], 1);
}

qwkpack::~qwkpack()
{
	cleanup();
}

file_header *qwkpack::getHello()
{
	return (textfiles[0] && textfiles[0][0]) ?
		mm->workList->existsF(textfiles[0]) : 0;
}

file_header *qwkpack::getGoodbye()
{
	return (textfiles[2] && textfiles[2][0]) ?
		mm->workList->existsF(textfiles[2]) : 0;
}

unsigned long qwkpack::MSBINtolong(unsigned const char *ms)
{
	return ((((unsigned long) ms[0] + ((unsigned long) ms[1] << 8) +
		  ((unsigned long) ms[2] << 16)) | 0x800000L) >>
			(24 + 0x80 - ms[3]));
}

area_header *qwkpack::getNextArea()
{
	int cMsgNum = areas[ID].nummsgs;
	bool x = (areas[ID].num == -1);

	area_header *tmp = new area_header(mm,
		ID + 1, areas[ID].numA, areas[ID].name,
		(x ? "Letters addressed to you" : areas[ID].name),
		(qwke ? (x ? "QWKE personal" : "QWKE") :
		(x ? "QWK personal" : "QWK")),
		areas[ID].attr | hasOffConfig | (cMsgNum ? ACTIVE : 0),
		cMsgNum, 0, 25, qwke ? 72 : 25);
	ID++;
	return tmp;
}

bool qwkpack::isQWKE()
{
	return qwke;
}

bool qwkpack::externalIndex()
{
	const char *p;
	bool hasNdx;

	hasPers = !(!mm->workList->exists("personal.ndx"));

	p = mm->workList->exists(".ndx");
	hasNdx = !(!p);

	if (hasNdx) {
		struct {
        		unsigned char MSB[4];
        		unsigned char confnum;
		} ndx_rec;

		FILE *idxFile;
		char fname[13];

		if (!hasPers) {
			areas++;
			maxConf--;
		}

		// Store the size of the .DAT for future comparison as
		// a check against invalid .NDX entries.

		fseek(infile, 0, SEEK_END);
		unsigned long endpoint = (unsigned long) ftell(infile);
		if (endpoint > 128)
			endpoint -= 128;

		while (p) {
			int x, cMsgNum = 0;

			strncpy(fname, p, strlen(p) - 4);
			fname[strlen(p) - 4] = '\0';
			x = atoi(fname);

			if (!x)
				if (!strcasecmp(fname, "personal"))
					x = -1;
				else
					if (strcmp(fname, "000"))
						x = -2;	// fname is not a num

			if (x != -2) {
			    x = getXNum(x);
			    if (-1 == x)	// fname is a num but not a
				hasNdx = false;	// valid conference
			}

			if (x >= 0) {

			    idxFile = mm->workList->ftryopen(p);
			    if (idxFile) {
				cMsgNum = mm->workList->getSize() / ndxRecLen;
				body[x] = new bodytype[cMsgNum];
				for (int y = 0; y < cMsgNum; y++) {
				    fread(&ndx_rec, ndxRecLen, 1, idxFile);
				    unsigned long temp =
					MSBINtolong(ndx_rec.MSB) << 7;

				    // If any .NDX entries appear corrupted,
				    // we're better off aborting this and
				    // using the new (purely .DAT-based)
				    // indexing method (in readIndices()).
				    
				    if ((temp < 256) || (temp > endpoint)) {
					hasNdx = false;	// use other method
					break;
				    } else
					body[x][y].pointer = temp;
				}
				fclose(idxFile);
			    }

			    if (hasNdx) {
				areas[x].nummsgs = cMsgNum;
				numMsgs += cMsgNum;
			    }
			}
			p = hasNdx ? mm->workList->getNext(".ndx") : 0;
		}

		// Clean up after aborting

		if (!hasNdx) {
			if (!hasPers) {
				areas--;
				maxConf++;
			}
			for (int x = 0; x < maxConf; x++) {
				delete[] body[x];
				body[x] = 0;
				areas[x].nummsgs = 0;
			}
			numMsgs = 0;
		}
	}
	return hasNdx;
}

void qwkpack::readIndices()
{
	int x;

	body = new bodytype *[maxConf];

	for (x = 0; x < maxConf; x++) {
		body[x] = 0;
		areas[x].nummsgs = 0;
	}

	numMsgs = 0;

	if (mm->resourceObject->getInt(IgnoreNDX) || !externalIndex()) {
		ndx_fake base, *tmpndx = &base;

		long counter;
		int personal = 0;
		const char *name = mm->resourceObject->get(LoginName);
		const char *alias = mm->resourceObject->get(AliasName);

		qheader qHead;

		fseek(infile, 128, SEEK_SET);
		while (qHead.init_short(infile))
		    if (!qHead.netblock) {	// skip net-status flags
			counter = ftell(infile);
			x = getXNum(qHead.origArea);

			if (-1 != x) {
				tmpndx->next = new ndx_fake;
				tmpndx = tmpndx->next;

				tmpndx->confnum = x;

				if (!strcasecmp(qHead.to, name) ||
				    (qwke && !strcasecmp(qHead.to, alias))) {
					tmpndx->pers = true;
					personal++;
				} else
					tmpndx->pers = false;

				tmpndx->pointer = counter;
				tmpndx->length = 0;	// temp

				areas[x].nummsgs++;
				numMsgs++;
			}

			fseek(infile, qHead.msglen, SEEK_CUR);
		    }

		initBody(base.next, personal);
	}
}

letter_header *qwkpack::getNextLetter()
{
	static net_address nullNet;
	qheader q;
	unsigned long pos, rawpos;
	int areaID, letterID;

	rawpos = body[currentArea][currentLetter].pointer;
	pos = rawpos - 128;

	fseek(infile, pos, SEEK_SET);
	if (q.init(infile)) {
		if (areas[currentArea].num == -1) {
			areaID = getXNum(q.origArea);
			letterID = getYNum(areaID, rawpos);
		} else {
			areaID = currentArea;
			letterID = currentLetter;
		}
	} else
		areaID = letterID = -1;

	if ((-1 == areaID) || (-1 == letterID))
		return new letter_header(mm, "MESSAGES.DAT", "READING",
			"ERROR", "ERROR", 0, 0, 0, 0, 0, false, 0,
			this, nullNet, false);
	
	body[areaID][letterID].msgLength = q.msglen;

	currentLetter++;

	return new letter_header(mm, q.subject, q.to, q.from,
		q.date, 0, q.refnum, letterID, q.msgnum, areaID,
		q.privat, q.msglen, this, nullNet,
		!(!(areas[areaID].attr & LATINCHAR)));
}

void qwkpack::getblk(int, long &offset, long blklen,
	unsigned char *&p, unsigned char *&begin)
{
	for (long count = 0; count < blklen; count++) {
		int kar = fgetc(infile);

		if (!kar)
			kar = ' ';

		*p++ = (kar == 227) ? '\n' : kar;	

		if (kar == 227) {
			begin = p;
			offset = ftell(infile);
		}
	}
}

void qwkpack::postfirstblk(unsigned char *&p, letter_header &mhead)
{
	// Get extended (QWKE-type) info, if available:

	char extsubj[72]; // extended subject or other line
	bool anyfound;
	unsigned char *q;

	do {
		anyfound = false;

		q = onecomp(p, extsubj, "subject:");

		if (!q) {
			q = onecomp(p + 1, extsubj,
				"@subject:");
			if (q) {
				extsubj[strlen(extsubj) - 1] = '\0';
				cropesp(extsubj);
			}
		}

		// For WWIV QWK door:
		if (!q)
			q = onecomp(p, extsubj, "title:");
		if (q) {
			p = q;
			mhead.changeSubject(extsubj);
			anyfound = true;
		}

		// To and From are now checked only
		// in QWKE packets:

		if (qwke) {
			q = onecomp(p, extsubj, "to:");
			if (q) {
				p = q;
				mhead.changeTo(extsubj);
				anyfound = true;
			}

			q = onecomp(p, extsubj, "from:");
			if (q) {
				p = q;
				mhead.changeFrom(extsubj);
				anyfound = true;
			}
		}

	} while (anyfound);
}

void qwkpack::endproc(letter_header &mhead)
{
	// Change to Latin character set, if necessary:
	checkLatin(mhead);
}

void qwkpack::readControlDat()
{
	char *p, *q;

	infile = mm->workList->ftryopen("control.dat");
	if (!infile)
		fatalError("Could not open CONTROL.DAT");

	BBSName = strdupplus(nextLine());		// 1: BBS name
	nextLine();					// 2: city/state
	nextLine();					// 3: phone#

	q = nextLine();					// 4: sysop's name
	int slen = strlen(q);
	if (slen > 6) {
		p = q + slen - 5;
		if (!strcasecmp(p, "Sysop")) {
			if (*--p == ' ')
				p--;
			if (*p == ',')
				*p = '\0';
		}
	}
	SysOpName = strdupplus(q);

	q = nextLine();					// 5: doorserno,BBSid
	strtok(q, ",");
	p = strtok(0, " ");
	strncpy(packetBaseName, p, 8);
	packetBaseName[8] = '\0';

	nextLine();					// 6: time&date

	p = nextLine();					// 7: user's name
	cropesp(p);
	mm->resourceObject->set(LoginName, p);
	mm->resourceObject->set(AliasName, p);

	nextLine();					// 8: blank/any
	nextLine();					// 9: anyth.
	nextLine();					// 10: # messages
							// (not reliable)
	maxConf = atoi(nextLine()) + 2;			// 11: Max conf #

	areas = new AREAs[maxConf];

	areas[0].num = -1;
	strcpy(areas[0].numA, "PERS");
	areas[0].name = strdupplus("PERSONAL");
	areas[0].attr = PUBLIC | PRIVATE | COLLECTION;

	int c;
	for (c = 1; c < maxConf; c++) {
		areas[c].num = atoi(nextLine());		// conf #
		sprintf(areas[c].numA, "%d", areas[c].num);
		areas[c].name = strdupplus(nextLine());		// conf name
		areas[c].attr = PUBLIC | PRIVATE;
	}

	for (c = 0; c < 3; c++)
		strncpy(textfiles[c], nextLine(), 12);

	fclose(infile);
}

void qwkpack::readDoorId()
{
	const char *s;
	bool hasAdd = false, hasDrop = false;

	strcpy(controlname, "QMAIL");

	infile = mm->workList->ftryopen("door.id");
	if (infile) {
		while (!feof(infile)) {
			s = nextLine();
			if (!strcasecmp(s, "CONTROLTYPE = ADD"))
			    hasAdd = true;
			else
			    if (!strcasecmp(s, "CONTROLTYPE = DROP"))
				hasDrop = true;
			    else
				if (!strncasecmp(s, "CONTROLNAME", 11))
				    sprintf(controlname, "%.25s", s + 14);
		}
		fclose(infile);
	}
	hasOffConfig = (hasAdd && hasDrop) ? OFFCONFIG : 0;
}

// Read the QWKE file TOREADER.EXT
void qwkpack::readToReader()
{
	char *s;
	int cnum;
	unsigned long attr;

	hasOffConfig = OFFCONFIG;

	infile = mm->workList->ftryopen("toreader.ext");
	if (infile) {
		while (!feof(infile)) {
			s = nextLine();

			if (!strncasecmp(s, "area ", 5)) {
			    if (sscanf(s + 5, "%d %s", &cnum, s) == 2) {

				attr = SUBKNOWN;

				// If a group is marked subscribed:
				if (strchr(s, 'a'))
					attr |= ACTIVE;
				else
				    if (strchr(s, 'p'))
					attr |= (ACTIVE | PERSONLY);
				    else
					if (strchr(s, 'g'))
					    attr |= (ACTIVE | PERSALL);

				// "Handles" or "Anonymous":
				if (strchr(s, 'H') || strchr(s, 'A'))
					attr |= ALIAS;

				// Public-only/Private-only:
				if (strchr(s, 'P'))
					attr |= PRIVATE;
				else
					if (strchr(s, 'O'))
						attr |= PUBLIC;
					else
						attr |= (PUBLIC | PRIVATE);

				// Read-only:
				if (strchr(s, 'R'))
					attr |= READONLY;

				/* Set character set to Latin-1 for
				   Internet or Usenet areas -- but is this
				   the right thing here?
				*/
				if (strchr(s, 'U') || strchr(s, 'I'))
					attr |= LATINCHAR;

				if (strchr(s, 'E'))
					attr |= ECHOAREA;

				areas[getXNum(cnum)].attr = attr;
			    }
			} else
				if (!strncasecmp(s, "alias ", 6)) {
					cropesp(s);
					mm->resourceObject->set(AliasName,
						s + 6);
				}
		}
		fclose(infile);
	}
}

const char *qwkpack::ctrlName()
{
	return controlname;
}

// -----------------------------------------------------------------
// The QWK reply methods
// -----------------------------------------------------------------

qwkreply::upl_qwk::upl_qwk(const char *name) : pktreply::upl_base(name)
{
	memset(&qHead, 0, sizeof(qHead));
}

qwkreply::qwkreply(mmail *mmA, specific_driver *baseClassA)
{
	mm = mmA;
	baseClass = (pktbase *) baseClassA;

	replyText = 0;
	qwke = ((qwkpack *) baseClass)->isQWKE();

	uplListHead = 0;

	replyExists = false;
}

qwkreply::~qwkreply()
{
	cleanup();
}

bool qwkreply::getRep1(FILE *rep, upl_qwk *l)
{
	FILE *replyFile;
	char *p, *q, blk[128];

	if (!l->qHead.init(rep))
		return false;

	replyFile = fopen(l->fname, "wt");
	if (!replyFile)
		return false;

	long count, length = 0, chunks = l->qHead.msglen >> 7;

	for (count = 0; count < chunks; count++) {
		fread(blk, 1, 128, rep);

		for (p = blk; p < (blk + 128); p++)
			if (*p == (char) 227)
				*p = '\n';	// PI-softcr

		// Get extended (QWKE-type) subject line, if available:

		p = blk;
		if (!count) {
			q = (char *) onecomp((unsigned char *) p,
				l->qHead.subject, "subject:");
			if (q)
				p = q;
		}

		q = blk + 127;
		if (count == (chunks - 1))
			for (; ((*q == ' ') || (*q == '\n')) && (q > blk);
				q--);

		length += (long) fwrite(p, 1, q - p + 1, replyFile);
	}
	fclose(replyFile);

	l->qHead.msglen = l->msglen = length;

	return true;
}

void qwkreply::getReplies(FILE *repFile)
{
	fseek(repFile, 128, SEEK_SET);

	noOfLetters = 0;

	upl_qwk baseUplList, *currUplList = &baseUplList;

	while (!feof(repFile)) {
		currUplList->nextRecord = new upl_qwk;
		currUplList = (upl_qwk *) currUplList->nextRecord;
		if (!getRep1(repFile, currUplList)) {
			delete currUplList;
			break;
		}
		noOfLetters++;
	}
	uplListHead = baseUplList.nextRecord;
}

area_header *qwkreply::getNextArea()
{
	return new area_header(mm, 0, "REPLY", "REPLIES",
		"Letters written by you", (qwke ? "QWKE replies" :
		"QWK replies"), (COLLECTION | REPLYAREA | ACTIVE |
		PUBLIC | PRIVATE), noOfLetters, 0, 25, qwke ? 72 : 25);
}

letter_header *qwkreply::getNextLetter()
{
	static net_address nullNet;
	upl_qwk *current = (upl_qwk *) uplListCurrent;

	int area = ((qwkpack *) baseClass)->
		getXNum((int) current->qHead.msgnum) + 1;

	letter_header *newLetter = new letter_header(mm,
		current->qHead.subject, current->qHead.to,
		current->qHead.from, current->qHead.date, 0,
		current->qHead.refnum, currentLetter, currentLetter,
		area, current->qHead.privat, current->qHead.msglen, this,
		nullNet, mm->areaList->isLatin(area));

	currentLetter++;
	uplListCurrent = uplListCurrent->nextRecord;
	return newLetter;
}

void qwkreply::enterLetter(letter_header &newLetter,
			const char *newLetterFileName, long length)
{
	// Specify the format separately from strftime() to supress
	// GCC's Y2K warning:
	const char *datefmt_qwk = "%m-%d-%y %H:%M";

	upl_qwk *newList = new upl_qwk(newLetterFileName);

	strncpy(newList->qHead.subject, newLetter.getSubject(), 71);
	strncpy(newList->qHead.from, newLetter.getFrom(), 25);
	strncpy(newList->qHead.to, newLetter.getTo(), 25);

	newList->qHead.msgnum = atol(mm->areaList->getShortName());
	newList->qHead.privat = newLetter.getPrivate();
	newList->qHead.refnum = newLetter.getReplyTo();

	time_t now = time(0);
	strftime(newList->qHead.date, 15, datefmt_qwk, localtime(&now));

	newList->qHead.msglen = newList->msglen = length;

	addUpl(newList);
}

void qwkreply::addRep1(FILE *rep, upl_base *node, int)
{
	FILE *replyFile;
	upl_qwk *l = (upl_qwk *) node;
	long count = 0;

	long headerpos = ftell(rep);
	l->qHead.output(rep);

	if (strlen(l->qHead.subject) > 25)
		fprintf(rep, "Subject: %s%c%c", l->qHead.subject,
			(char) 227, (char) 227);

	replyFile = fopen(l->fname, "rt");
	if (replyFile) {

		int c, lastsp = 0;
		while ((c = fgetc(replyFile)) != EOF) {
			count++;
			if ((count > 80) && lastsp) {
				fseek(replyFile, lastsp - count,
					SEEK_CUR);
				fseek(rep, lastsp - count,
					SEEK_CUR);
				c = '\n';
			}
			if ('\n' == c) {
				fputc((char) 227, rep);
				count = lastsp = 0;
			} else {
				fputc(c, rep);
				if (' ' == c)
					lastsp = count;
			}
		}
		fclose(replyFile);
	}

	fputc((char) 227, rep);

	long curpos = ftell(rep);
	l->qHead.set_length(rep, headerpos, curpos);
}

void qwkreply::addHeader(FILE *repFile)
{
	char tmp[129];
	sprintf(tmp, "%-128s", baseClass->getBaseName());
	fwrite(tmp, 128, 1, repFile);
}

void qwkreply::repFileName()
{
	int x;
	const char *basename = baseClass->getBaseName();

	for (x = 0; basename[x]; x++) {
		replyPacketName[x] = tolower(basename[x]);
		replyInnerName[x] = toupper(basename[x]);
	}
	strcpy(replyPacketName + x, ".rep");
	strcpy(replyInnerName + x, ".MSG");
}

const char *qwkreply::repTemplate(bool offres)
{
	static char buff[30];

	sprintf(buff, (offres && qwke) ? "%s TODOOR.EXT" : "%s",
		replyInnerName);

	return buff;
}

bool qwkreply::getOffConfig()
{
	bool status = false;

	if (qwke) {
		FILE *olc;

		upWorkList = new file_list(mm->resourceObject->get(UpWorkDir));

		olc = upWorkList->ftryopen("todoor.ext");
		if (olc) {
			char line[128], mode;
			int areaQWK, areaNo;

			while (!feof(olc)) {
				myfgets(line, sizeof line, olc);
				if (sscanf(line, "AREA %d %c", &areaQWK,
				    &mode) == 2) {
					areaNo = ((qwkpack *) baseClass)->
						getXNum(areaQWK) + 1;
					mm->areaList->gotoArea(areaNo);
					if (mode == 'D')
						mm->areaList->Drop();
					else
						mm->areaList->Add();
				}
			}
			fclose(olc);
			upWorkList->kill();

			status = true;
		}
		delete upWorkList;
	}
	return status;
}

bool qwkreply::makeOffConfig()
{
	FILE *todoor = 0;	// warning suppression
	net_address bogus;
	letter_header *ctrlMsg;
	const char *myname = 0, *ctrlName = 0;

	if (qwke) {
		todoor = fopen("TODOOR.EXT", "wb");
		if (!todoor)
			return false;
	} else {
		myname = mm->resourceObject->get(LoginName);
		ctrlName = ((qwkpack *) baseClass)->ctrlName();
	}

	int oldarea = mm->areaList->getAreaNo();

	int maxareas = mm->areaList->noOfAreas();
	for (int areaNo = 0; areaNo < maxareas; areaNo++) {
		mm->areaList->gotoArea(areaNo);
		unsigned long attrib = mm->areaList->getType();

		if (attrib & (ADDED | DROPPED)) {
			if (qwke)
				fprintf(todoor, "AREA %s %c\r\n",
					mm->areaList->getShortName(),
					(attrib & ADDED) ?
					((attrib & PERSONLY) ? 'p' :
					((attrib & PERSALL) ? 'g' : 'a'))
					: 'D');
			else {
				ctrlMsg = new letter_header(mm, (attrib &
				ADDED) ? "ADD" : "DROP", ctrlName, myname,
				"", 0, 0, 0, 0, areaNo, false, 0, this,
				bogus, false);

				enterLetter(*ctrlMsg, "", 0);

				delete ctrlMsg;

				if (attrib & ADDED)
					mm->areaList->Drop();
				else
					mm->areaList->Add();
			}
		}
	}
	mm->areaList->gotoArea(oldarea);

	if (qwke)
		fclose(todoor);
	else
		mm->areaList->refreshArea();

	return true;
}
