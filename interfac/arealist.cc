/*
 * MultiMail offline mail reader
 * area list

 Copyright (c) 1996 Kolossvary Tamas <thomas@vma.bme.hu>
 Copyright (c) 2003 William McBrine <wmcbrine@users.sourceforge.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "interfac.h"

//**************        LittleAreaListWindow    *****************

void LittleAreaListWindow::MakeActive()
{
	position = 0;
	areanum = -1;

	mm.areaList->setMode(mm.areaList->getMode() - 1);
	mm.areaList->relist();

	list_max_y = (NumOfItems() < LINES - 10) ? NumOfItems() : LINES - 10;
	list_max_x = 52;
	top_offset = 3;

	borderCol = C_LALBTEXT;

	list = new InfoWin(list_max_y + 4, list_max_x + 2, 3, borderCol, 0,
		C_SBACK, 4, top_offset);

	list->put(1, 2, "Reply goes to area:");

	DrawAll();
}

void LittleAreaListWindow::init()
{
	int oldarea = mm.areaList->getAreaNo();

	disp = 0;
	do
		mm.areaList->gotoActive(disp++);
	while (mm.areaList->isCollection());
	disp--;

	mm.areaList->gotoArea(mm.letterList->getAreaID());
	active = mm.areaList->getActive() - disp;

	// restore old area (for collection areas):
	mm.areaList->gotoArea(oldarea);
}

void LittleAreaListWindow::Select()
{
	mm.areaList->gotoActive(active + disp);
}

void LittleAreaListWindow::Delete()
{
	delete list;
}

int LittleAreaListWindow::getArea()
{
	return areanum;
}

int LittleAreaListWindow::NumOfItems()
{
	return mm.areaList->noOfActive() - disp;
}

void LittleAreaListWindow::oneLine(int i)
{
	mm.areaList->gotoActive(position + i + disp);
	sprintf(list->lineBuf, "%c%-50.50s ", (!mm.areaList->isShortlist()
		&& (mm.areaList->getType() & ACTIVE)) ? '*' : ' ',
		mm.areaList->getDescription());
	areaconv_in(list->lineBuf);
	DrawOne(i, C_LALLINES);
}

searchret LittleAreaListWindow::oneSearch(int x, const char *item, int)
{
	mm.areaList->gotoActive(x + disp);
	return mm.areaList->filterCheck(item) ? True : False;
}

bool LittleAreaListWindow::extrakeys(int key)
{
	switch (key) {
	case MM_ENTER:
		Select();
		if (mm.areaList->getType() & (COLLECTION | READONLY)) {
			areanum = -1;
			ui->nonFatalError("Cannot reply there");
		} else
			areanum = mm.areaList->getAreaNo();
		break;
	case 'L':
		Select();
		{
			int x = mm.areaList->getAreaNo();

			ui->areas.Select();
			mm.areaList->relist();
			ui->areas.ResetActive();

			mm.areaList->gotoArea(x);
			active = mm.areaList->getActive() - disp;
		}
		ui->redraw();
	}
	return false;
}

void LittleAreaListWindow::setFilter(const char *item)
{
	mm.areaList->setFilter(item);
	init();
}

//*************         AreaListWindow          ******************

void AreaListWindow::FirstUnread()
{
	int i;

	mm.areaList->updatePers();

	position = active = 0;
	for (i = 0; i < NumOfItems(); i++) {
		mm.areaList->gotoActive(i);
		if (!mm.areaList->getNoOfUnread())
			Move(DOWN);
		else
			break;
	}
	if (i == NumOfItems()) {
		position = active = 0;
		for (i = 0; i < NumOfItems(); i++) {
			mm.areaList->gotoActive(i);
			if (!mm.areaList->getNoOfLetters())
				Move(DOWN);
			else
				break;
		}
	}
}

int AreaListWindow::NumOfItems()
{
	return mm.areaList->noOfActive();
}

void AreaListWindow::oneLine(int i)
{
	char *p = list->lineBuf;

	mm.areaList->gotoActive(position + i);

	unsigned long attrib = mm.areaList->getType();

	if (position + i == active) {
		p += sprintf(p, "%.20s", mm.areaList->getAreaType());

		if (mm.areaList->isNetmail())
			p += sprintf(p, ", Netmail");
		else
			if (mm.areaList->isInternet())
				p += sprintf(p, ", Email");
			else
				if (mm.areaList->isUsenet())
					p += sprintf(p, ", Usenet");
				else
					if (attrib & ECHOAREA)
						p += sprintf(p, ", Echo");

		if (attrib & PERSONLY)
			p += sprintf(p, ", Pers");
		else
			if (attrib & PERSALL)
				p += sprintf(p, ", Pers+All");

		int q = ((list_max_x >> 1) - 8) - (p - list->lineBuf);
		while (--q > 0)
			sprintf(p++, " ");
		p = list->lineBuf;

		list->attrib(C_ALINFOTEXT2);
		list->put(list_max_y + 3 + hasSys, 8, p);

		sprintf(p, format2, mm.areaList->getDescription());
		areaconv_in(p);
		list->put(list_max_y + 4 + hasSys, 20, p);

		list->delay_update();
	}
	p += sprintf(p, format, ((attrib & ADDED) ? '+' :
		((attrib & DROPPED) ? '-' : ((attrib & ACTIVE) &&
		!mm.areaList->isShortlist()) ? '*' :
		((attrib & HASREPLY) ? 'R' : ' '))),
		mm.areaList->getShortName(), mm.areaList->getDescription());

	if (mm.areaList->getNoOfLetters())
		p += sprintf(p, "  %5d  ", mm.areaList->getNoOfLetters());
	else
		p += sprintf(p, "      .  ");

	if (mm.areaList->getNoOfUnread())
		p += sprintf(p, "  %5d   ", mm.areaList->getNoOfUnread());
	else
		p += sprintf(p, "      .   ");

	if (hasPers)
		if (mm.areaList->getNoOfPersonal())
			sprintf(p, "   %5d   ",
				mm.areaList->getNoOfPersonal());
		else
			sprintf(p, "       .   ");

	coltype ch = ((attrib & (REPLYAREA | ADDED | DROPPED)) ||
		((attrib & HASREPLY) && !(attrib & ACTIVE))) ?
		C_ALREPLINE : C_ALPACKETLINE;

	areaconv_in(list->lineBuf);
	DrawOne(i, mm.areaList->getNoOfUnread() ? emph(ch) : noemph(ch));
}

searchret AreaListWindow::oneSearch(int x, const char *item, int mode)
{
	searchret retval;

	mm.areaList->gotoActive(x);
	retval = mm.areaList->filterCheck(item) ? True : False;

	if (!retval && (mode < s_arealist) && mm.areaList->getNoOfLetters()) {
		int oldactive = active;
		ResetActive();
		mm.areaList->getLetterList();
		mm.letterList->setMode(-1);
		mm.letterList->relist();
		ui->changestate(letterlist);
		ui->letters.setActive(-1);
		retval = ui->letters.search(item, mode);
		if (retval != True) {
			active = oldactive;
			ui->changestate(arealist);
			delete mm.letterList;
		}
	}

	return retval;
}

void AreaListWindow::Select()
{
	mm.areaList->gotoActive(active);
}

void AreaListWindow::ResetActive()
{
	active = mm.areaList->getActive();
}

void AreaListWindow::MakeActive()
{
	static const char *almodes[] = {"All", "Subscribed", "Active"};
	int padding, middle;
	char tmp[80], tpad[7];

	hasPers = mm.driverList->hasPersonal();
	mm.areaList->updatePers();

	mm.areaList->setMode(mm.areaList->getMode() - 1);
	mm.areaList->relist();

	const char *bb = mm.resourceObject->get(BBSName);
	const char *sy = mm.resourceObject->get(SysOpName);
	hasSys = ((bb && *bb) || (sy && *sy));

	list_max_y = LINES - (mm.resourceObject->getInt(ExpertMode) ?
			11 : 15) + !hasSys;
	list_max_x = COLS - 6;
	top_offset = 2;

	const char *filter = mm.areaList->getFilter();
	
	char *p = tmp + sprintf(tmp, "%.20s | %s Areas",
		mm.resourceObject->get(PacketName),
		almodes[mm.areaList->getMode()]);

	if (NumOfItems() > list_max_y)
		p += sprintf(p, " (%d)", NumOfItems());

	if (filter)
		sprintf(p, " | %.20s", filter);

	charconv_in(tmp);

	borderCol = C_ALBORDER;

	list = new InfoWin(list_max_y + 6 + hasSys, list_max_x + 2, 2, 
		borderCol, tmp, C_ALBTEXT, 6 + hasSys);

	list->attrib(C_ALHEADTEXT);
	list->put(1, 3, "Area#  Description");
	int newloc = list_max_x - 15;
	if (hasPers)
		newloc -= 11;
	list->put(1, newloc, "Total   Unread");
	if (hasPers)
		list->put(1, list_max_x - 9, "Personal");

	list->horizline(list_max_y + 2, list_max_x);

	padding = list_max_x - 28;
	if (hasPers)
		padding -= 11;
	sprintf(format, "%%c%%6s  %%-%d.%ds", padding, padding);

	middle = (list_max_x - 2) >> 1;

	list->attrib(C_ALINFOTEXT);
	if (hasSys) {
		list->put(list_max_y + 3, 3, "BBS:");
		list->put(list_max_y + 3, middle, " Sysop:");
	}
	list->put(list_max_y + 3 + hasSys, 2, "Type:");
	list->put(list_max_y + 4 + hasSys, 2, "Area description:");

	sprintf(tpad, "%%.%ds", (middle < 87) ? middle - 8 : 79);
	middle += 8;

	list->attrib(C_ALINFOTEXT2);

	if (hasSys) {
		sprintf(tmp, tpad, (bb && *bb) ? bb : "(unknown)");
		charconv_in(tmp);
		list->put(list_max_y + 3, 8, tmp);

		sprintf(tmp, tpad, (sy && *sy) ? sy : "(unknown)");
		charconv_in(tmp);
		list->put(list_max_y + 3, middle, tmp);
	}

	padding = list_max_x - 20;
	sprintf(format2, "%%-%d.%ds", padding, padding);

	DrawAll();
	Select();
}

void AreaListWindow::Delete()
{
	delete list;
}

void AreaListWindow::Prev()
{
	do {
		Move(UP);
		Select();
	} while (!mm.areaList->getNoOfLetters() &&
		 (mm.areaList->getActive() > 0));
}

void AreaListWindow::Next()
{
	do {
		Move(DOWN);
		Select();
	} while (!mm.areaList->getNoOfLetters() &&
		 (mm.areaList->getActive() <
		  mm.areaList->noOfActive() - 1));
}

bool AreaListWindow::extrakeys(int key)
{
	bool end = false;

	Select();
	switch (key) {
	case 5:
	case 'E':
		if (!(mm.areaList->getType() & (COLLECTION | READONLY))) {
			if ((5 == key) || mm.areaList->isEmail()) {
				ui->addressbook();
				Select();
			}
			ui->letterwindow.EnterLetter(
				mm.areaList->getAreaNo(), 'E');
		} else
			ui->nonFatalError("Cannot reply there");
		break;
#ifdef USE_MOUSE
	case MM_MOUSE:
		{
			int begx = list->xstart(), begy = list->ystart();

			if ( (mouse_event.y != begy) ||
			    ((mouse_event.x < (begx + 13)) || (mouse_event.x >
			    (begx + 40))) )
				break;
		}
#endif
	case 'L':
		mm.areaList->relist();
		ResetActive();
		ui->redraw();
		break;
	case 'S':
	case MM_INS:
	case 'U':
	case MM_DEL:
		if (mm.areaList->hasOffConfig()) {
			switch (key) {
			case 'S':
			case MM_INS:
				mm.areaList->Add();
				break;
			default:
				mm.areaList->Drop();
			}
			ui->setUnsavedNoAuto();
			Move(DOWN);
			Draw();
		} else
			ui->nonFatalError(
				"Offline config is unavailable");
	}
	return end;
}

void AreaListWindow::setFilter(const char *item)
{
	mm.areaList->setFilter(item);
}
