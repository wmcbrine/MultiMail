/*
 * MultiMail offline mail reader
 * letter_header and letter_list

 Copyright (c) 1996 Toth Istvan <stoty@vma.bme.hu>
 Copyright (c) 2003 William McBrine <wmcbrine@users.sf.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "mmail.h"

int lsorttype;	// Outside the classes because it needs to be accessible
		// by lettercomp(), which is outside because it needs to
		// be for qsort(). :-/

extern "C" int lmsgncomp(const void *a, const void *b)
{
	return (*((letter_header **) a))->getLetterID() -
		(*((letter_header **) b))->getLetterID();
}

extern "C" int lettercomp(const void *A, const void *B)
{
	const char *x, *y;
	int d, l1, l2;
	letter_header **a = (letter_header **) A;
	letter_header **b = (letter_header **) B;

	switch(lsorttype) {
	case LS_SUBJ:
		x = stripre((*a)->getSubject());
		y = stripre((*b)->getSubject());
		break;
	case LS_FROM:
		x = (*a)->getFrom();
		y = (*b)->getFrom();
		break;
	default:
		x = (*a)->getTo();
		y = (*b)->getTo();
	}

	l1 = strlen(x);
	l2 = strlen(y);

	if (!l1 || !l2)		// For idiots who don't add a Subject
		d = l2 - l1;
	else {
		if (l2 < l1)
			l1 = l2;

		d = strncasecmp(x, y, l1);
		if (!d)
			d = lmsgncomp(A, B);
	}
	return d;
}

// -----------------------------------------------------------------
// Letter body methods
// -----------------------------------------------------------------

letter_body::letter_body(char *textA, long lengthA, long offsetA,
	bool hiddenA) :
	text(textA), length(lengthA), offset(offsetA), hidden(hiddenA)
{
	next = 0;
}

letter_body::~letter_body()
{
	delete[] text;
	delete next;
}

char *letter_body::getText()
{
	return text + offset;
}

long letter_body::getLength()
{
	return length - offset;
}

bool letter_body::isHidden()
{
	return hidden;
}

// -----------------------------------------------------------------
// Letter header methods
// -----------------------------------------------------------------

letter_header::letter_header(mmail *mmA, const char *subjectA,
		const char *toA, const char *fromA, const char *dateA,
		const char *msgidA, long replyToA, int LetterIDA,
		long msgNumA, int AreaIDA, bool privatA, int lengthA,
		specific_driver *driverA, net_address &netAddrA,
		bool charsetA, const char *newsgrpsA, const char *followA,
		const char *replyA, bool qpencA) :
		driver(driverA), replyTo(replyToA), LetterID(LetterIDA),
		AreaID(AreaIDA), privat(privatA), length(lengthA),
		msgNum(msgNumA), netAddr(netAddrA), charset(charsetA),
		qpenc(qpencA)
{
	dl = mmA->driverList;
	readO = dl->getReadObject(driver);

	const char *cset = mmA->resourceObject->get(outCharset);

	subject = strdupblank(subjectA);
	headdec(subjectA, cset, subject);

	to = strdupblank(toA);
	headdec(toA, cset, to);

	from = strdupblank(fromA);
	headdec(fromA, cset, from);

	date = strdupplus(dateA);
	msgid = strdupplus(msgidA);
	newsgrps = strdupplus(newsgrpsA);
	follow = strdupplus(followA);
	reply = strdupplus(replyA);

	const char *login = mmA->resourceObject->get(LoginName);
	const char *alias = mmA->resourceObject->get(AliasName);

	persto = (!strcasecmp(to, login) || (alias && *alias &&
		!strcasecmp(to, alias)));
	persfrom = (!strcasecmp(from, login) || (alias && *alias &&
		!strcasecmp(from, alias)));
}

letter_header::~letter_header()
{
	delete[] subject;
	delete[] to;
	delete[] from;
	delete[] date;
	delete[] msgid;
	delete[] newsgrps;
	delete[] follow;
	delete[] reply;
}

void letter_header::changeSubject(const char *newsubj)
{
	delete[] subject;
	subject = strdupplus(newsubj);
}

void letter_header::changeTo(const char *newto)
{
	delete[] to;
	to = strdupplus(newto);
}

void letter_header::changeFrom(const char *newfrom)
{
	delete[] from;
	from = strdupplus(newfrom);
}

void letter_header::changeDate(const char *newdate)
{
	delete[] date;
	date = strdupplus(newdate);
}

void letter_header::changeMsgID(const char *newmsgid)
{
	delete[] msgid;
	msgid = strdupplus(newmsgid);
}

void letter_header::changeNewsgrps(const char *news)
{
	delete[] newsgrps;
	newsgrps = strdupplus(news);
}

void letter_header::changeFollow(const char *newfollow)
{
	delete[] follow;
	follow = strdupplus(newfollow);
}

void letter_header::changeReplyTo(const char *newreply)
{
	delete[] reply;
	reply = strdupplus(newreply);
}

const char *letter_header::getSubject() const
{
	return subject;
}

const char *letter_header::getTo() const
{
	return to;
}

const char *letter_header::getFrom() const
{
	return from;
}

const char *letter_header::getDate() const
{
	return date;
}

const char *letter_header::getMsgID() const
{
	return msgid;
}

const char *letter_header::getNewsgrps() const
{
	return newsgrps;
}

const char *letter_header::getFollow() const
{
	return follow;
}

const char *letter_header::getReply() const
{
	return reply;
}

long letter_header::getReplyTo() const
{
	return replyTo;
}

int letter_header::getLetterID() const
{
	return LetterID;
}

int letter_header::getAreaID() const
{
	return AreaID + dl->getOffset(driver);
}

bool letter_header::getPrivate() const
{
	return privat;
}

letter_body *letter_header::getBody()
{
	return driver->getBody(*this);
}

int letter_header::getLength() const
{
	return length;
}

bool letter_header::getRead()
{
	return readO->getRead(AreaID, LetterID);
}

void letter_header::setRead()
{
	readO->setRead(AreaID, LetterID, true);
}

int letter_header::getStatus()
{
	return readO->getStatus(AreaID, LetterID) |
		(persto ? MS_PERSTO : 0) | (persfrom ? MS_PERSFROM : 0);
}

void letter_header::setStatus(int stat)
{
	readO->setStatus(AreaID, LetterID, stat);
}

long letter_header::getMsgNum() const
{
	return msgNum;
}

net_address &letter_header::getNetAddr()
{
	return netAddr;
}

bool letter_header::isPersonal() const
{
	return persfrom || persto;
}

bool letter_header::isLatin() const
{
	return charset;
}

void letter_header::setLatin(bool charsetA)
{
	charset = charsetA;
}

bool letter_header::isQP() const
{
	return qpenc;
}

void letter_header::setQP(bool qpencA)
{
	qpenc = qpencA;
}

// -----------------------------------------------------------------
// Letterlist methods
// -----------------------------------------------------------------

letter_list::letter_list(mmail *mmA, int areaNumberA, unsigned long typeA) :
	mm(mmA), areaNumber(areaNumberA), type(typeA)
{
	dl = mm->driverList;
	driver = dl->getDriver(areaNumber);
	areaNumber -= dl->getOffset(driver);
	readO = dl->getReadObject(driver);
	isColl = (type & COLLECTION) && !(type & REPLYAREA);
	init();
}

letter_list::~letter_list()
{
	cleanup();
}

void letter_list::init()
{
	driver->selectArea(areaNumber);
	noOfLetters = driver->getNoOfLetters();

	filter = 0;

	activeHeader = new int[noOfLetters];
	letterHeader = new letter_header *[noOfLetters];

	driver->resetLetters();
	for (int c = 0; c < noOfLetters; c++)
		letterHeader[c] = driver->getNextLetter();

	currentLetter = 0;
	llmode = mm->resourceObject->getInt(LetterMode) - 1;

	sort();
	relist();
}

void letter_list::relist()
{
	int stat, c = currentLetter;
	noActive = 0;

	while (noOfLetters && !noActive) {
		switch (llmode) {
		case 0:
			llmode++;
			break;
		case 1:
			if (readO->getNoOfMarked(areaNumber)) {
				llmode++;
				break;
			}
		default:
			llmode = 0;
		}
		for (currentLetter = 0; currentLetter < noOfLetters;
		     currentLetter++)
			if (!filter || filterCheck(filter)) {
			    stat = getStatus();

			    if ((llmode == 0) ||
				((llmode == 1) && !(stat & MS_READ)) ||
				((llmode == 2) && (stat & MS_MARKED)))
				    activeHeader[noActive++] = currentLetter;
			}

		// If filter caught nothing, clear it and try again
		if (filter && !noActive) {
			llmode--;
			delete[] filter;
			filter = 0;
		}
	}

	currentLetter = c;
}

void letter_list::cleanup()
{
	while (noOfLetters)
		delete letterHeader[--noOfLetters];
	delete[] letterHeader;
	delete[] activeHeader;
	delete[] filter;
}

void letter_list::sort()
{
	if ((noOfLetters > 1) && !(isColl || ((areaNumber +
	    dl->getOffset(driver)) == REPLY_AREA)))
		qsort(letterHeader, noOfLetters, sizeof(letter_header *),
			(lsorttype == LS_MSGNUM) ? lmsgncomp : lettercomp);
}

void letter_list::resort()
{
	if (lsorttype == LS_TO)
		lsorttype = LS_SUBJ;
	else {
		lsorttype++;

		if ((lsorttype == LS_TO) && ((type & INTERNET) &&
		    !(type & NETMAIL)))
			lsorttype = LS_SUBJ;
	}
	sort();

	llmode--;
	relist();
}

int letter_list::getMode() const
{
	return llmode;
}

void letter_list::setMode(int newmode)
{
	llmode = newmode;
}

int letter_list::noOfLetter() const
{
	return noOfLetters;
}

int letter_list::noOfActive() const
{
	return noActive;
}

const char *letter_list::getSubject() const
{
	return letterHeader[currentLetter]->getSubject();
}

const char *letter_list::getFrom() const
{
	return letterHeader[currentLetter]->getFrom();
}

long letter_list::getMsgNum() const
{
	return letterHeader[currentLetter]->getMsgNum();
}

const char *letter_list::getTo() const
{
	return letterHeader[currentLetter]->getTo();
}

const char *letter_list::getDate() const
{
	return letterHeader[currentLetter]->getDate();
}

const char *letter_list::getMsgID() const
{
	return letterHeader[currentLetter]->getMsgID();
}

const char *letter_list::getNewsgrps() const
{
	return letterHeader[currentLetter]->getNewsgrps();
}

const char *letter_list::getFollow() const
{
	return letterHeader[currentLetter]->getFollow();
}

const char *letter_list::getReply() const
{
	return letterHeader[currentLetter]->getReply();
}

long letter_list::getReplyTo() const
{
	return letterHeader[currentLetter]->getReplyTo();
}

int letter_list::getAreaID() const
{
	return letterHeader[currentLetter]->getAreaID();
}

bool letter_list::getPrivate() const
{
	return letterHeader[currentLetter]->getPrivate();
}

letter_body *letter_list::getBody()
{
	return letterHeader[currentLetter]->getBody();
}

int letter_list::getLength() const
{
	return letterHeader[currentLetter]->getLength();
}

net_address &letter_list::getNetAddr()
{
	return letterHeader[currentLetter]->getNetAddr();
}

bool letter_list::isPersonal() const
{
	return letterHeader[currentLetter]->isPersonal();
}

bool letter_list::isLatin() const
{
	return letterHeader[currentLetter]->isLatin();
}

bool letter_list::isQP() const
{
	return letterHeader[currentLetter]->isQP();
}

void letter_list::setQP(bool qpencA)
{
	letterHeader[currentLetter]->setQP(qpencA);
}

bool letter_list::getRead()
{
	bool read = letterHeader[currentLetter]->getRead();
	if (isColl)
		readO->setRead(areaNumber, currentLetter, read);
	return read;
}

void letter_list::setRead()
{
	if (isColl)
		readO->setRead(areaNumber, currentLetter, true);
	letterHeader[currentLetter]->setRead();
}

int letter_list::getStatus()
{
	int stat = letterHeader[currentLetter]->getStatus();
	if (isColl)
		readO->setStatus(areaNumber, currentLetter, stat);
	return stat;
}

void letter_list::setStatus(int stat)
{
	if (isColl)
		readO->setStatus(areaNumber, currentLetter, stat);
	letterHeader[currentLetter]->setStatus(stat);
}

int letter_list::getCurrent() const
{
	return currentLetter;
}

int letter_list::getActive() const
{
	int c;

	for (c = 0; c < noActive; c++)
		if (activeHeader[c] >= currentLetter)
			break;
	return c;
}

void letter_list::gotoLetter(int newLetter)
{
	if (newLetter < noOfLetters)
		currentLetter = newLetter;
}

void letter_list::gotoActive(int activeA)
{
	if ((activeA >= 0) && (activeA < noActive))
		currentLetter = activeHeader[activeA];
}

void letter_list::rrefresh()
{
	cleanup();
	init();
	gotoActive(noActive - 1);
}

bool letter_list::findMsgNum(long msgnum)
{
	bool found = false;

	for (int x = 1; !found && (x <= noOfLetters); x++) {
		int y = (currentLetter + x) % noOfLetters;
		found = (letterHeader[y]->getMsgNum() == msgnum);
		if (found)
			currentLetter = y;
	}

	return found;
}

bool letter_list::findReply(int area, long msgnum)
{
	bool found = false;

	for (int x = 0; !found && (x < noOfLetters); x++) {
		found = (letterHeader[x]->getReplyTo() == msgnum) &&
			(letterHeader[x]->getAreaID() == area);
		if (found)
			currentLetter = x;
	}

	return found;
}

const char *letter_list::getFilter() const
{
        return filter;
}

void letter_list::setFilter(const char *newfilter)
{
        delete[] filter;
        filter = (newfilter && *newfilter) ? strdupplus(newfilter) : 0;
	llmode--;
	relist();
}

const char *letter_list::filterCheck(const char *item)
{
	const char *s = searchstr(getFrom(), item);
	if (!s) {
		s = searchstr(getTo(), item);
		if (!s)
			s = searchstr(getSubject(), item);
	}

	return s;
}
