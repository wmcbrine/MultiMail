/*
 * MultiMail offline mail reader
 * message list

 Copyright (c) 1996 Kolossvary Tamas <thomas@tvnet.hu>
 Copyright (c) 1997 John Zero <john@graphisoft.hu>
 Copyright (c) 2003 William McBrine <wmcbrine@users.sf.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "interfac.h"

LetterListWindow::LetterListWindow()
{
	lsorttype = mm.resourceObject->getInt(LetterSort);
}

void LetterListWindow::listSave()
{
	static const char *saveopts[] = { "Marked", "All", "This one",
		"Quit" };

	int marked = !(!mm.areaList->getNoOfMarked());

	int status = ui->WarningWindow("Save which?", saveopts +
		!marked, 3 + marked);
	if (status) {
		bool saveok = ui->letterwindow.Save(status);
		if ((status == 1) && saveok)
			Move(KEY_DOWN);
	}
}

void LetterListWindow::Next()
{
	do {
		Move(KEY_DOWN);
		mm.letterList->gotoActive(active);
	} while (mm.letterList->getRead() && ((active + 1) < NumOfItems()));
}

void LetterListWindow::FirstUnread()
{
	position = 0;
	active = 0;
}

void LetterListWindow::Prev()
{
	do {
		Move(KEY_UP);
		mm.letterList->gotoActive(active);
	} while (mm.letterList->getRead() && (active > 0));
}

int LetterListWindow::NumOfItems()
{
	return mm.letterList->noOfActive();
}

void LetterListWindow::oneLine(int i)
{
	mm.letterList->gotoActive(position + i);

	int st = mm.letterList->getStatus();

	char *p = list->lineBuf;
	p += sprintf(p, format, (st & MS_MARKED) ? 'M' :
	    (st & MS_SAVED) ? 's' : ' ', (st & MS_REPLIED) ? '~' : ' ',
		(st & MS_READ) ? '*' : ' ', mm.letterList->getMsgNum(),
		    mm.letterList->getFrom(), mm.letterList->getTo(),
			stripre(mm.letterList->getSubject()));

	if (mm.areaList->isCollection()) {
		const char *origArea = mm.letterList->getNewsgrps();
		if (!origArea)
			origArea = mm.areaList->
			    getDescription(mm.letterList->getAreaID());
		if (origArea)
			sprintf(p - 15, " %-13.13s ", origArea);
	}

	coltype linecol = mm.letterList->isPersonal() ? C_LLPERSONAL :
		C_LISTWIN;

	letterconv_in(list->lineBuf);
	DrawOne(i, (st & MS_READ) ? noemph(linecol) : emph(linecol));
}

searchret LetterListWindow::oneSearch(int x, const char *item, int mode)
{
	searchret retval;

	mm.letterList->gotoActive(x);
	retval = mm.letterList->filterCheck(item) ? True : False;

	if (!retval && (mode == s_fulltext)) {
		ui->changestate(letter);
		ui->letterwindow.setPos(-1);
		retval = ui->letterwindow.search(item);
		if (retval != True)
			ui->changestate(letterlist);
	}

	return retval;
}

void LetterListWindow::setFormat()
{
	char topformat[50];
	int tot, maxFromLen, maxToLen, maxSubjLen;

	tot = COLS - 19;
	maxSubjLen = tot / 2;
	tot -= maxSubjLen;
	maxToLen = tot / 2;
	maxFromLen = tot - maxToLen;

	if (!mm.areaList->hasTo() || (mm.areaList->isCollection() &&
	    !mm.areaList->isReplyArea())) {
		maxSubjLen += maxToLen;
		maxToLen = 0;
	}

	if (mm.areaList->isReplyArea()) {
		maxSubjLen += maxFromLen;
		maxFromLen = 0;
	}

	sprintf(format, "%%c%%c%%c%%6ld  %%-%d.%ds %%-%d.%ds %%-%d.%ds",
		maxFromLen, maxFromLen, maxToLen, maxToLen,
			maxSubjLen, maxSubjLen);

	sprintf(topformat, "   Msg#%s", format + 10);
	sprintf(topline, topformat, "From", "To", "Subject");
}

void LetterListWindow::MakeActiveCore()
{
	static const char *llmodes[] = {"All", "Unread", "Marked"},
		*llsorts[] = {"subject", "number", "from", "to"};

	int maxbott = LINES -
		(mm.resourceObject->getInt(ExpertMode) ? 7 : 11);
	list_max_y = (NumOfItems() < maxbott) ? NumOfItems() : maxbott;

	bool too_many = (NumOfItems() > list_max_y);

	const char *modestr = llmodes[mm.letterList->getMode()];
	const char *sortstr = llsorts[lsorttype];
	const char *pn = mm.resourceObject->get(PacketName);
	const char *filter = mm.letterList->getFilter();

	int pnlen = strlen(pn);
	if (pnlen > 20)
		pnlen = 20;
	int offset = strlen(modestr) + pnlen + 3;
	int flen = filter ? strlen(filter) + 3 : 0;
	if (flen > 20)
		flen = 20;
	int nwidth = COLS - (too_many ? 27 : 19) - offset - flen -
		strlen(sortstr);

	char *title = new char[COLS + 1];

	char *end = title + sprintf(title, "%.*s | %s in %.*s",
		pnlen, pn, modestr, nwidth, mm.areaList->getDescription());

	char *newend = end + sprintf(end, ", by %s", sortstr);
	if (too_many)
		newend += sprintf(newend, " (%d)", NumOfItems());
	if (flen)
		sprintf(newend, " | %.*s", flen - 3, filter);

	areaconv_in(title);

	borderCol = C_LLBBORD;

	list = new InfoWin(list_max_y + 3, list_max_x + 2, 2, borderCol,
		title, C_LLTOPTEXT1);

	list->attrib(C_LLTOPTEXT2);

	*end = '\0';
	offset += 3;
	list->put(0, offset + 3, title + offset);

	delete[] title;

	list->attrib(C_LLHEAD);
	list->put(1, 3, topline);

	if (mm.areaList->isCollection())
		list->put(1, COLS - 19, "Area");
	list->touch();

	DrawAll();
	Select();
}

void LetterListWindow::MakeActive()
{
	top_offset = 2;
	list_max_x = COLS - 6;

	topline = new char[COLS + 1];

	setFormat();
	ui->areas.Select();
	MakeActiveCore();
}

void LetterListWindow::Select()
{
	mm.letterList->gotoActive(active);
}

void LetterListWindow::ResetActive()
{
        active = mm.letterList->getActive();
}

void LetterListWindow::Delete()
{
	delete list;
	delete[] topline;
}

bool LetterListWindow::extrakeys(int key)
{
	Select();
	switch (key) {
#ifdef USE_MOUSE
	case MM_MOUSE:
		{
			int begx = list->xstart(), begy = list->ystart();

			if (mouse_event.y == begy) {
				if ((mouse_event.x > (begx + 12)) &&
				    (mouse_event.x < (begx + 27)))
					extrakeys('L');
				else
				    if ((mouse_event.x > (begx + 27)) &&
					(mouse_event.x < (begx + list_max_x)))
					    extrakeys('$');
			}
		}
		break;
#endif
	case 'U':
	case 'M':	// Toggle read/unread and marked from letterlist
		mm.letterList->setStatus(mm.letterList->getStatus() ^
			((key == 'U') ? MS_READ : MS_MARKED));
		ui->setAnyRead();
		Move(KEY_DOWN);
		Draw();
		break;
	case 5:
	case 'E':
		if (mm.areaList->isReplyArea())
			ui->letterwindow.KeyHandle('E');
		else
		    if (!(mm.areaList->getType() & (COLLECTION | READONLY))) {
			    if ((5 == key) || mm.areaList->isEmail())
				ui->addressbook();
			    ui->letterwindow.EnterLetter(
				mm.areaList->getAreaNo(), 'E');
		    } else
			    ui->nonFatalError("Cannot reply there");
		break;
	case 2:
	case 6:
	case MM_DEL:
	case 'K':
		if (mm.areaList->isReplyArea())
			ui->letterwindow.KeyHandle(key);
		break;
	case 'L':
		mm.letterList->relist();
		ResetActive();
		ui->redraw();
		break;
	case '$':
		mm.letterList->resort();
		ui->redraw();
		break;
	case 'S':
		listSave();
		ui->redraw();
	}
	return false;
}

void LetterListWindow::setFilter(const char *item)
{
	mm.letterList->setFilter(item);
}
