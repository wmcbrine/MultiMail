/*
 * MultiMail offline mail reader
 * OPX

 Copyright (c) 2003 William McBrine <wmcbrine@users.sf.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "opx.h"
#include "compress.h"

// -----------------------------------------------------------------
// The OPX methods
// -----------------------------------------------------------------

opxpack::opxpack(mmail *mmA) : pktbase(mmA)
{
	readBrdinfoDat();

	infile = mm->workList->ftryopen("mail.dat");
	if (!infile)
		fatalError("Could not open MAIL.DAT");

	buildIndices();
}

opxpack::~opxpack()
{
	cleanup();
}

file_header *opxpack::getHello()
{
	return (bulletins && *bulletins) ?
		mm->workList->existsF(bulletins) : 0;
}

area_header *opxpack::getNextArea()
{
	int cMsgNum = areas[ID].nummsgs;
	bool x = (areas[ID].num == - 1);

	area_header *tmp = new area_header(mm, ID + 1,
		areas[ID].numA, areas[ID].name,
		(x ? "Letters addressed to you" : areas[ID].name),
		(x ? "OPX personal" : "OPX"),
		areas[ID].attr | (cMsgNum ? ACTIVE : 0),
		cMsgNum, 0, 35, 71);
	ID++;
	return tmp;
}

void opxpack::buildIndices()
{
	FILE *fdxFile = 0;	// warning supression
	msgHead mhead;
	fdxHeader fhead;
	fdxRec frec;
	int x, totMsgs = 0;

	body = new bodytype *[maxConf];

	for (x = 0; x < maxConf; x++) {
		body[x] = 0;
		areas[x].nummsgs = 0;
	}

	ndx_fake base, *tmpndx = &base;

	long counter, length, personal = 0;
	long mdatlen = mm->workList->getSize();

	bool hasFdx = (mm->workList->exists("mail.fdx") != 0);

	if (hasFdx) {
		fdxFile = mm->workList->ftryopen("mail.fdx");
		if (fdxFile) {
			fread(&fhead, FDX_HEAD_SIZE, 1, fdxFile);
			if (getshort(fhead.PageCount) != 1) {
				// Skip it, we don't understand it
				hasFdx = false;
				fclose(fdxFile);
			} else {
				totMsgs = getshort(fhead.RowsInPage);
				fseek(fdxFile, 4, SEEK_CUR);
				fread(&frec, FDX_REC_SIZE, 1, fdxFile);
			}
		} else
			hasFdx = false;
	}

	numMsgs = 0;

	while ((hasFdx ? (numMsgs < totMsgs) :
	    fread(&mhead, MSG_HEAD_SIZE, 1, infile))) {

#ifdef BOGUS_WARNING
		bool pers = false;
#else
		bool pers;
#endif
		if (hasFdx) {
			counter = getlong(frec.offset) + MSG_HEAD_SIZE;
			x = getXNum(getshort(frec.confnum));
			pers = ('D' == frec.msgtype);

			bool fdxAvail = (numMsgs < (totMsgs - 1));
			if (fdxAvail)
				fread(&frec, FDX_REC_SIZE, 1, fdxFile);
			length = (fdxAvail ? getlong(frec.offset) :
				mdatlen) - counter;
		} else {
			counter = ftell(infile);
			x = getXNum(getshort(mhead.confnum));
			pers = ('D' == mhead.msgtype);

			length = getshort(mhead.length) - 0xbe;

			fseek(infile, length, SEEK_CUR);
		}

		tmpndx->next = new ndx_fake;
		tmpndx = tmpndx->next;

		tmpndx->confnum = x;

		tmpndx->pers = pers;
		if (pers)
			personal++;

		tmpndx->pointer = counter;
		tmpndx->length = length;

		numMsgs++;
		areas[x].nummsgs++;
	}

	if (hasFdx)
		fclose(fdxFile);

	initBody(base.next, personal);
}

letter_header *opxpack::getNextLetter()
{
	msgHead mhead;
	unsigned long pos, rawpos;
	int areaID, letterID;

	rawpos = body[currentArea][currentLetter].pointer;
	pos = rawpos - MSG_HEAD_SIZE;

	fseek(infile, pos, SEEK_SET);
	if (!fread(&mhead, MSG_HEAD_SIZE, 1, infile))
		fatalError("Error reading MAIL.DAT");

	if (areas[currentArea].num == -1) {
		areaID = getXNum(getshort(mhead.confnum));
		letterID = getYNum(areaID, rawpos);
	} else {
		areaID = currentArea;
		letterID = currentLetter;
	}

        net_address na;
	if (areas[areaID].attr & INTERNET)
		na = mhead.f.from;
	else {
		na.zone = getshort(mhead.f.orig_zone);
		if (na.zone) {
			na.net = getshort(mhead.f.orig_net);
			na.node = getshort(mhead.f.orig_node);
			na.point = 0;   // set from getBody()
			na.isSet = true;
		}
	}

	bool privat = getshort(mhead.f.attr) & OPX_PRIVATE;

	char date[30];
	strftime(date, 30, "%b %d %Y  %H:%M",
		getdostime(getlong(mhead.f.date_written)));

	currentLetter++;

	return new letter_header(mm, mhead.f.subject, mhead.f.to, 
		mhead.f.from, date, 0, getshort(mhead.f.reply), letterID,
		getshort(mhead.msgnum), areaID, privat,
		getshort(mhead.length), this, na,
		!(!(areas[areaID].attr & LATINCHAR)));
}

void opxpack::getblk(int, long &offset, long blklen,
	unsigned char *&p, unsigned char *&begin)
{
	int lastkar = -1;

	for (long count = 0; count < blklen; count++) {
		int kar = fgetc(infile);

		if (!kar)
			kar = ' ';

		// "Soft CRs" are misused in some OPX packets:
		if (kar == 0x8d)
			kar = '\n';

		// Line endings are a mess in OPX -- can be
		// CR, LF, _or_ CR/LF!

		if ((lastkar == '\r') && (kar != '\n')) {
			*p++ = '\n';
			begin = p;
			offset = ftell(infile) - 1;
		}

		if (kar != '\r')
			*p++ = kar;

		if (kar == '\n') {
			begin = p;
			offset = ftell(infile);
		}

		lastkar = kar;
	}
}

void opxpack::endproc(letter_header &mhead)
{
	// Extra header info embedded in the text:

	char *end;

	const char *s = getHidden("\001INETORIG ", end);
	if (s) {
		net_address &na = mhead.getNetAddr();

		na = s;
		if (end)
			*end = '\n';
	} else
		fidocheck(mhead);
}

// Read a Borland Pascal pstring, return a C string
char *opxpack::pstrget(void *src)
{
	unsigned len = (unsigned) *((unsigned char *) src);
	char *dest = new char[len + 1];

	strncpy(dest, ((const char *) src) + 1, len);
	dest[len] = '\0';

	return dest;
}

void opxpack::readBrdinfoDat()
{
	FILE *brdinfoFile, *ocfgFile;
	brdHeader header;
	brdRec boardrec;
	ocfgRec offrec;
	char *p;
	int brdCount, extCount;
	bool hasExtra;

	ocfgFile = mm->workList->ftryopen("dusrcfg.dat");
	if (ocfgFile) {
		fread(&confhead, OCFG_HEAD_SIZE, 1, ocfgFile);
		hasOffConfig = OFFCONFIG;
	} else
		hasOffConfig = 0;

	brdinfoFile = mm->workList->ftryopen("brdinfo.dat");
	if (!brdinfoFile)
		fatalError("Could not open BRDINFO.DAT");

	if (!fread(&header, BRD_HEAD_SIZE, 1, brdinfoFile))
		fatalError("Error reading BRDINFO.DAT");

	p = pstrget(&header.bbsid);
	strcpy(packetBaseName, p);
	delete[] p;

	BBSName = pstrget(&header.bbsname);
	SysOpName = pstrget(&header.sysopname);

	p = pstrget(&header.username);
	mm->resourceObject->set(LoginName, p);
	mm->resourceObject->set_noalloc(AliasName, p);

	bulletins = header.readerfiles ?
		new char[header.readerfiles * 13] : 0;

	int c;
	for (c = 0; c < header.readerfiles; c++) {
		pstring(readerf,12);

		fread(&readerf, 13, 1, brdinfoFile);
		strncpy(bulletins + c * 13, (char *) readerf + 1, *readerf);
		bulletins[c * 13 + *readerf] = '\0';
	}

	if (header.readerfiles > 1)
		listBulletins((const char (*)[13]) (bulletins + 13),
			header.readerfiles - 1, 1);
	else
		listBulletins(0, 0, 1);

	// Skip old numofareas byte:
	fseek(brdinfoFile, 1, SEEK_CUR);

	maxConf = getshort(header.numofareas) + 1;
	areas = new AREAs[maxConf];

	hasExtra = (mm->workList->exists("extareas.dat") != 0);
	extCount = hasExtra ? (mm->workList->getSize() / BRD_REC_SIZE) : 0;
	brdCount = maxConf - extCount;

	areas[0].num = -1;
	strcpy(areas[0].numA, "PERS");
	areas[0].name = strdupplus("PERSONAL");
	areas[0].attr = PUBLIC | PRIVATE | COLLECTION;

	for (c = 1; c < maxConf; c++) {
		if (hasExtra && (c == brdCount)) {
			fclose(brdinfoFile);
			brdinfoFile =
				mm->workList->ftryopen("extareas.dat");
			if (!brdinfoFile)
				fatalError("Could not open EXTAREAS.DAT");
		}
		fread(&boardrec, BRD_REC_SIZE, 1, brdinfoFile);

		areas[c].num = getshort(boardrec.confnum);
		sprintf(areas[c].numA, "%d", areas[c].num);

		areas[c].name = pstrget(&boardrec.name);

		bool selected = !(!boardrec.scanned);
		if (hasOffConfig) {
			fread(&offrec, OCFG_REC_SIZE, 1, ocfgFile);
			if (getshort(offrec.confnum) == (unsigned)
			    areas[c].num)
				selected = !(!offrec.scanned);
		}

		unsigned rawattr = getshort(boardrec.attrib);
		areas[c].attr = (((rawattr & OPX_NETMAIL) |
			(boardrec.attrib2 & OPX_INTERNET)) ? NETMAIL : 0) |
			((rawattr & OPX_PRIVONLY) ? 0 : PUBLIC) |
			((rawattr & OPX_PUBONLY) ? 0 : PRIVATE) |
			(((boardrec.attrib2 & OPX_USENET) |
			(boardrec.attrib2 & OPX_INTERNET)) ? (INTERNET |
			LATINCHAR) : 0) | (selected ? ACTIVE : 0) |
			hasOffConfig | (hasOffConfig ? SUBKNOWN : 0);
	}

	fclose(brdinfoFile);

	if (hasOffConfig)
		fclose(ocfgFile);
}

ocfgHeader *opxpack::offhead()
{
	return &confhead;
}

const char *opxpack::oldFlagsName()
{
	return "mail.fdx";
}

// Read in an .FDX file
bool opxpack::readOldFlags()
{
	FILE *fdxFile;
	fdxHeader fhead;
	fdxRec frec;
	int totmsgs, area = -1, lastarea, msgnum;
	letter_list *ll = 0;

	fdxFile = mm->workList->ftryopen(oldFlagsName());
	if (!fdxFile)
		return false;

	fread(&fhead, FDX_HEAD_SIZE, 1, fdxFile);
	if (getshort(fhead.PageCount) != 1) {
		fclose(fdxFile);
		return false;
	}

	fseek(fdxFile, 4, SEEK_CUR);
	totmsgs = getshort(fhead.RowsInPage);

	area_list *al = mm->areaList;

	for (int x = 0; x < totmsgs; x++) {
		fread(&frec, FDX_REC_SIZE, 1, fdxFile);

		lastarea = area;
		area = getshort(frec.confnum);
		msgnum = getshort(frec.msgnum);

		if (lastarea != area) {
			delete ll;
			al->gotoArea(getXNum(area) + 1);
			al->getLetterList();
			ll = mm->letterList;
			ll->gotoLetter(-1);
		}

		ll->findMsgNum(msgnum);

		int stat = ((frec.flags & FDX_READ) ? MS_READ : 0) |
			((frec.flags & FDX_REPLIED) ? MS_REPLIED : 0) |
			((frec.marks & FDX_TAGGED) ? MS_MARKED : 0) |
			((frec.msgtype == 'D') ? MS_PERSTO : 0);

		ll->setStatus(stat);
	}
	delete ll;

	fclose(fdxFile);

	return true;
}

// Write out an .FDX file
bool opxpack::saveOldFlags()
{
	FILE *fdxFile;

	// If there are more messages than will fit in a 64K block,
	// it would need multiple pages, so forget it:

	if (((long) numMsgs * FDX_REC_SIZE) > 0x10000)
		return false;

	fdxFile = fopen(oldFlagsName(), "wb");
	if (!fdxFile)
		return false;

	// Fill the header and write it out:

	fdxHeader fhead;
	putshort(fhead.RowsInPage, numMsgs);
	putshort(fhead.ColsInPage, 1);
	putshort(fhead.PagesDown, 1);
	putshort(fhead.PagesAcross, 1);
	putshort(fhead.ElSize, FDX_REC_SIZE);
	putshort(fhead.PageSize, numMsgs * FDX_REC_SIZE);
	putshort(fhead.PageCount, 1);
	putlong(fhead.NextAvail, (long) numMsgs * FDX_REC_SIZE +
		FDX_HEAD_SIZE + 4);
	strncpy(fhead.ID, "\006VARRAY", 7);

	fwrite(&fhead, FDX_HEAD_SIZE, 1, fdxFile);

	plong addr;
	putlong(addr, FDX_HEAD_SIZE + 4);

	fwrite(addr, 4, 1, fdxFile);

	// And now, the body:

	area_list *al = mm->areaList;
	int maxareas = al->noOfAreas();
	for (int c = 0; c < maxareas; c++) {
		al->gotoArea(c);
		if (!al->isCollection()) {
			al->getLetterList();
			letter_list *ll = mm->letterList;

			for (int d = 0; d < ll->noOfLetter(); d++) {
			    ll->gotoLetter(d);
			    int stat = ll->getStatus();

			    fdxRec frec;

			    int anum = atoi(al->getShortName());
			    putshort(frec.confnum, anum);
			    putshort(frec.msgnum, ll->getMsgNum());

			    long offset = body[c - 1][d].pointer -
				MSG_HEAD_SIZE;
			    putlong(frec.offset, offset);

			    frec.flags =
				((stat & MS_READ) ? FDX_READ : 0) |
				((stat & MS_REPLIED) ? FDX_REPLIED : 0);

			    frec.marks =
				(stat & MS_MARKED) ? FDX_TAGGED : 0;

			    frec.msgtype =
				((stat & MS_PERSTO) ? 'D' : ' ');

			    fwrite(&frec, FDX_REC_SIZE, 1, fdxFile);
			}
			delete ll;
		}
	}
	fclose(fdxFile);

	return true;
}

// -----------------------------------------------------------------
// The OPX reply methods
// -----------------------------------------------------------------

opxreply::upl_opx::upl_opx(const char *name) : pktreply::upl_base(name)
{
	memset(&rhead, 0, sizeof(rhead));
	msgid = 0;
}

opxreply::upl_opx::~upl_opx()
{
	delete[] msgid;
}

opxreply::opxreply(mmail *mmA, specific_driver *baseClassA) :
	pktreply(mmA, baseClassA)
{
}

opxreply::~opxreply()
{
}

int opxreply::getArea(const char *fname)
{
	int sum = 0;

	fname = strchr(fname, '.') + 1;
	for (int x = 0; x < 3; x++) {
		sum *= 36;

		unsigned char c = toupper(fname[x]);
		if ((c >= '0') && (c <= '9'))
			sum += c - '0';
		else
			if ((c >= 'A') && (c <= 'Z'))
				sum += c - 'A' + 10;
	}
	return sum;
}

bool opxreply::getRep1(const char *orgname, upl_opx *l)
{
	FILE *orgfile, *destfile;
	int c;
	long count = 0;

	orgfile = fopen(orgname, "rb");
	if (orgfile) {

	    fread(&l->rhead, FIDO_HEAD_SIZE, 1, orgfile);
	    l->area = getArea(orgname);

	    net_address na;
	    na.zone = getshort(l->rhead.dest_zone);
	    if (na.zone) {
		na.net = getshort(l->rhead.dest_net);
		na.node = getshort(l->rhead.dest_node);
		na.point = 0;
		na.isSet = true;
	    }

	    destfile = fopen(l->fname, "wt");
	    if (destfile) {
		while ((c = fgetc(orgfile)) != EOF) {
			if (c == '\001') {
				c = fgetc(orgfile);
				int x;
				bool isReply = (c == 'R'),
					isPoint = (c == 'T'),
					isInet = (c == 'I');

				if (isReply || isPoint || isInet) {
					for (x = 0; x < (isInet ? 8 :
					    (isPoint ? 4 : 6)); x++)
						fgetc(orgfile);
				}
				x = 0;
				while ((c != EOF) && (c != '\n')) {
					c = fgetc(orgfile);
					x++;
				}
				if (isReply || isPoint || isInet) {
					char *tmp = new char[x];
					fseek(orgfile, x * -1, SEEK_CUR);
					fread(tmp, 1, x, orgfile);
					strtok(tmp, "\r");
				
					if (isReply)
						l->msgid = tmp;
					else {
						if (isInet)
							na = tmp;
						else
							sscanf(tmp, "%u",
								&na.point);
						delete[] tmp;
					}
				}
				c = '\r';
			}
			if (c && (c != '\r')) {
				fputc(c, destfile);
				count++;
			}
		}
		fclose(destfile);
	    }

	    l->na = na;

	    fclose(orgfile);
	}

	l->msglen = count;

	remove(orgname);

	return true;
}

void opxreply::getReplies(FILE *)
{
	noOfLetters = 0;

	upl_opx baseUplList, *currUplList = &baseUplList;
	const char *p;

	upWorkList->gotoFile(-1);
	while ((p = upWorkList->getNext("!")) != 0) {
		currUplList->nextRecord = new upl_opx;
		currUplList = (upl_opx *) currUplList->nextRecord;
		if (!getRep1(p, currUplList)) {
			delete currUplList;
			break;
		}
		noOfLetters++;
	}
	uplListHead = baseUplList.nextRecord;
}

area_header *opxreply::getNextArea()
{
	return new area_header(mm, 0, "REPLY", "REPLIES",
		"Letters written by you", "OPX replies",
		(COLLECTION | REPLYAREA | ACTIVE | PUBLIC | PRIVATE),
		noOfLetters, 0, 35, 71);
}

letter_header *opxreply::getNextLetter()
{
	upl_opx *current = (upl_opx *) uplListCurrent;

	char date[30];
	strftime(date, 30, "%b %d %Y  %H:%M",
		getdostime(getlong(current->rhead.date_written)));

	int area = ((opxpack *) baseClass)->getXNum(current->area) + 1;

	letter_header *newLetter = new letter_header(mm,
		current->rhead.subject, current->rhead.to,
		current->rhead.from, date, current->msgid,
		getshort(current->rhead.reply), currentLetter, currentLetter,
		area, getshort(current->rhead.attr) & OPX_PRIVATE,
		current->msglen, this, current->na,
		mm->areaList->isLatin(area));

	currentLetter++;
	uplListCurrent = uplListCurrent->nextRecord;
	return newLetter;
}

void opxreply::enterLetter(letter_header &newLetter,
			const char *newLetterFileName, long length)
{
	// Specify the format separately from strftime() to supress
	// GGC's Y2K warning:
	const char *datefmt_opx = "%d %b %y  %H:%M:%S";

	upl_opx *newList = new upl_opx(newLetterFileName);

	int attrib = newLetter.getPrivate() ? OPX_PRIVATE : 0;

	strncpy(newList->rhead.subject, newLetter.getSubject(), 71);
	strncpy(newList->rhead.from, newLetter.getFrom(), 35);
	strncpy(newList->rhead.to, newLetter.getTo(), 35);

	const char *msgid = newLetter.getMsgID();
	if (msgid)
		newList->msgid = strdupplus(msgid);

	newList->area = atoi(mm->areaList->getShortName());
	newList->na = newLetter.getNetAddr();

	putshort(newList->rhead.attr, attrib);
	putshort(newList->rhead.reply, newLetter.getReplyTo());

	time_t now = time(0);
	strftime(newList->rhead.date, 20, datefmt_opx, localtime(&now));
	unsigned long dostime = mkdostime(localtime(&now));
	putlong(newList->rhead.date_written, dostime);
	putlong(newList->rhead.date_arrived, dostime);

	newList->msglen = length;

	addUpl(newList);
}

const char *opxreply::freeFileName(upl_opx *l)
{
	static char fname[13];
	char ext[4];
	unsigned area[3], x;

	area[2] = l->area;
	area[0] = area[2] / (36 * 36);
	area[2] %= (36 * 36);
        area[1] = area[2] / 36;
	area[2] %= 36;

	for (x = 0; x < 3; x++)
		ext[x] = area[x] + ((area[x] < 10) ? '0' : ('A' - 10));
	ext[3] = '\0';

	mystat st;
	int reply = getshort(l->rhead.reply);
	if (reply) {
		sprintf(fname, "!R%d.%s", reply, ext);
		if (!st.init(fname))
			return fname;
	}
	x = 1;
	do
		sprintf(fname, "!N%d.%s", x++, ext);
	while (st.init(fname));

	return fname;
}

void opxreply::addRep1(FILE *, upl_base *node, int)
{
	FILE *orgfile, *destfile;
	upl_opx *l = (upl_opx *) node;
	const char *dest;

	dest = freeFileName(l);

	orgfile = fopen(l->fname, "rt");
	if (orgfile) {

		destfile = fopen(dest, "wb");
		if (destfile) {

			fwrite(&l->rhead, FIDO_HEAD_SIZE, 1, destfile);

			bool skipPID = false;

			if (l->na.isSet) {
			    if (l->na.isInternet) {
				fprintf(destfile, "\001INETDEST %s\r\n",
					(const char *) l->na);
				skipPID = true;
			    } else {
				putshort(l->rhead.dest_zone, l->na.zone);
				putshort(l->rhead.dest_net, l->na.net);
				putshort(l->rhead.dest_node, l->na.node);

				// I should probably add the originating
				// address here, but it's frequently
				// bogus. Let's try it this way.

				if (l->na.point)
					fprintf(destfile, "\001TOPT %d\r\n",
						l->na.point);
			    }
			}

			if (l->msgid)
				fprintf(destfile, "\001REPLY: %s\r\n",
					l->msgid);

			if (!skipPID)
				fprintf(destfile, "\001PID: " MM_NAME
					"/%s v" MM_VERNUM "\r\n", sysname());

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

			fprintf(destfile, "\r\n");
			fputc(0, destfile);

			fclose(destfile);
		}
		fclose(orgfile);
	}
}

void opxreply::addHeader(FILE *repFile)
{
	// <BBSID>.ID -- fake it.

	long magic = -598939720L;
	time_t now = time(0);

	fprintf(repFile, "FALSE\r\n\r\n4.4\r\nTRUE\r\n%ld\r\n0\r\n%ld\r\n",
		(signed long) mkdostime(localtime(&now)), magic);
}

void opxreply::repFileName()
{
	int x;
	const char *basename = baseClass->getBaseName();

	for (x = 0; basename[x]; x++) {
		replyPacketName[x] = tolower(basename[x]);
		replyInnerName[x] = toupper(basename[x]);
	}
	strcpy(replyPacketName + x, ".rep");
	strcpy(replyInnerName + x, ".ID");
}

const char *opxreply::repTemplate(bool)
{
	return "*.*";
}

bool opxreply::getOffConfig()
{
	FILE *olc;
	bool status = false;

	upWorkList = new file_list(mm->resourceObject->get(UpWorkDir));

	olc = upWorkList->ftryopen("rusrcfg.dat");
	if (olc) {
		ocfgHeader offhead;
		ocfgRec offrec;
		int areaOPX, areaNo;

		fread(&offhead, OCFG_HEAD_SIZE, 1, olc);
		int totareas = getshort(offhead.numofareas);

		for (int i = 0; i < totareas; i++) {
			fread(&offrec, OCFG_REC_SIZE, 1, olc);
			areaOPX = getshort(offrec.confnum);

			areaNo = ((opxpack *) baseClass)->
				getXNum(areaOPX) + 1;
			mm->areaList->gotoArea(areaNo);

			if (offrec.scanned)
				mm->areaList->Add();
			else
				mm->areaList->Drop();
		}
		fclose(olc);
		upWorkList->kill();

		status = true;
	}
	delete upWorkList;

	return status;
}

bool opxreply::makeOffConfig()
{
	FILE *olc;
	ocfgRec offrec;

	olc = fopen("RUSRCFG.DAT", "wb");
	if (!olc)
		return false;

	fwrite(((opxpack *) baseClass)->offhead(),
		OCFG_HEAD_SIZE, 1, olc);

	int oldarea = mm->areaList->getAreaNo();

	int maxareas = mm->areaList->noOfAreas();
	for (int areaNo = 0; areaNo < maxareas; areaNo++) {
		mm->areaList->gotoArea(areaNo);

		unsigned long attrib = mm->areaList->getType();
		int anum = atoi(mm->areaList->getShortName());

		if (!(attrib & COLLECTION)) {
			putshort(offrec.confnum, anum);
			offrec.scanned = ((((attrib & ACTIVE) && !(attrib
			    & DROPPED)) || (attrib & ADDED)));
			fwrite(&offrec, OCFG_REC_SIZE, 1, olc);
		}
	}
	mm->areaList->gotoArea(oldarea);
	fclose(olc);

	return true;
}
