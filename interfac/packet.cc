/*
 * MultiMail offline mail reader
 * packet list window, vanity plate

 Copyright (c) 1996 Kolossvary Tamas <thomas@vma.bme.hu>
 Copyright (c) 1997 John Zero <john@graphisoft.hu>
 Copyright (c) 2004 William McBrine <wmcbrine@users.sf.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "interfac.h"

#ifdef VANITY_PLATE

void Welcome::MakeActive()
{
	window = new ShadowedWin(6, 50, 2, C_WBORDER);
	window->attrib(C_WELCOME1);
	window->put(1, 7,
		"Welcome to " MM_NAME " Offline Reader!");
	//window->put(2, 13, "http://multimail.sf.net/");
	window->attrib(C_WELCOME2);
	window->put(3, 2,
		"Copyright (c) 2004 William McBrine, Kolossvary"); 
	window->put(4, 7,
		"Tamas, Toth Istvan, John Zero, et al.");
	window->touch();
}

void Welcome::Delete()
{
	delete window;
}

#endif

PacketListWindow::oneDir::oneDir(const char *nameA, oneDir *parentA) :
	parent(parentA)
{
	name = fixPath(nameA);
	position = active = 0;
}

PacketListWindow::oneDir::~oneDir()
{
	delete[] name;
}

PacketListWindow::PacketListWindow()
{
#ifdef HAS_HOME
	home = getenv("HOME");
#endif
	origDir = 0;
	packetList = 0;
}

void PacketListWindow::init()
{
	mychdir(mm.resourceObject->get(PacketDir));
	char *tmp = mygetcwd();
	origDir = new oneDir(tmp, 0);
	delete[] tmp;

	currDir = origDir;

	sorttype = mm.resourceObject->getInt(PacketSort);
	newList();
	if (noFiles)			// If there are any files,
		active = noDirs;	// set active to the first one.
}

PacketListWindow::~PacketListWindow()
{
	delete origDir;
	delete packetList;
}

void PacketListWindow::newList()
{
	delete packetList;

	mm.resourceObject->set(oldPacketName, (char *) 0);

	const char *target = currDir->name;
	mm.resourceObject->set(PacketDir, target);

	time(&currTime);

	packetList = new file_list(target, sorttype, true);

	noDirs = packetList->getNoOfDirs();
	noFiles = packetList->getNoOfFiles();
}

void PacketListWindow::MakeActiveCore()
{
	const int stline =
#ifdef VANITY_PLATE
		9;
#else
		2;
#endif
	list_max_y = LINES - (stline +
		(mm.resourceObject->getInt(ExpertMode) ? 5 : 9));

	bool usenum = false;
	int items = NumOfItems();

	if (list_max_y > items)
		list_max_y = items ? items : 1;
	else
		usenum = true;

	char tmp[46], *dest = tmp, *src = currDir->name;
	int end, maxlen = 36, len = strlen(src);

	if (usenum)
		maxlen -= sprintf(tmp, " (%d)", noFiles);

#ifdef HAS_HOME
	int hlen = home ? strlen(home) : 0;
	bool inhome = hlen && (hlen <= len) && !strncmp(home,
		currDir->name, hlen);

	if (inhome && ((len - hlen) < maxlen)) {
		tmp[0] = '~';
		dest++;
		src += hlen;
		end = len - hlen + 1;
	} else
#endif
		if (len > maxlen) {
			strcpy(tmp, "...");
			dest += 3;
			src += (len - maxlen) + 3;
			end = maxlen;
		} else
			end = len;

	strcpy(dest, src);
	canonize(dest);

	int newend = end + sprintf(tmp + end, ", by %s", sorttype ?
		"time" : "name");
	if (usenum)
		newend += sprintf(tmp + newend, " (%d)", noFiles);

	const char *filter = packetList->getFilter();
	if (filter) {
		int flen = strlen(filter);
		int fmax = list_max_x - 5 - newend;
		if (flen > fmax)
			flen = fmax;

		sprintf(tmp + newend, " | %.*s", flen, filter);
	}

	list = new InfoWin(list_max_y + 3, list_max_x + 2, stline, borderCol,
		tmp, C_PHEADTEXT);

	list->attrib(C_PLINES);
	tmp[end] = '\0';
	list->put(0, 3, tmp);

	list->attrib(C_PHEADTEXT);
	list->put(1, 3, "Packet                  Size    Date");
	list->touch();

	DrawAll();
}

void PacketListWindow::MakeActive()
{
	list_max_x = 48;
	top_offset = 2;

	borderCol = C_PBBACK;

#ifdef VANITY_PLATE
	welcome.MakeActive();
#endif
	MakeActiveCore();
}

int PacketListWindow::NumOfItems()
{
	return noDirs + noFiles;
}

void PacketListWindow::Delete()
{
	delete list;	
#ifdef VANITY_PLATE
	welcome.Delete();
#endif
}

void PacketListWindow::oneLine(int i)
{
	char *tmp = list->lineBuf;
	int absPos = position + i;
	time_t tmpt;

	packetList->gotoFile(absPos);

	if (absPos < noDirs) {
		absPos = sprintf(tmp, "  <%.28s",
			packetList->getName());
		char *tmp2 = tmp + absPos;
		*tmp2++ = '>';

		absPos = 32 - absPos;
		while (--absPos > 0)
			*tmp2++ = ' ';
	} else {
		const char *tmp2 = packetList->getName();

		strcpy(tmp, "          ");

		if (*tmp2 == '.')
			sprintf(&tmp[2], "%-20.20s", tmp2);
		else {
			for (int j = 2; *tmp2 && (*tmp2 != '.') &&
				(j < 10); j++)
					tmp[j] = *tmp2++;

			sprintf(&tmp[10], "%-10.10s", tmp2);
		}

		sprintf(&tmp[20], "%12lu",
			(unsigned long) packetList->getSize());
	}

	tmpt = packetList->getDate();

#ifdef TIMEKLUDGE
	if (!tmpt)
		tmpt = currTime;
#endif
	long dtime = currTime - tmpt;

	// 15000000 secs = approx six months (use year if older):
	strftime(&tmp[32], 17, ((dtime < 0 || dtime > 15000000L) ?
		"  %b %d  %Y  " : "  %b %d %H:%M  "), localtime(&tmpt));

	DrawOne(i, C_PLINES);
}

searchret PacketListWindow::oneSearch(int x, const char *item, int mode)
{
	const char *s;
	searchret retval;

	packetList->gotoFile(x);

	s = packetList->getName();
	retval = searchstr(s, item) ? True : False;

	if ((retval == False) && (x >= noDirs) &&
	    (mode < s_pktlist)) {
		int oldactive = active;
		active = x;
		if (OpenPacket() == PKT_OK) {
			mm.checkForReplies();
			mm.openReply();

			ui->redraw();
			ui->ReportWindow("Searching (ESC to abort)...");

			mm.areaList = new area_list(&mm);
			mm.areaList->getRepList();
			mm.driverList->initRead();
			mm.areaList->setMode(-1);
			mm.areaList->relist();
			ui->changestate(arealist);
			ui->areas.setActive(-1);
			retval = ui->areas.search(item, mode);
			if (retval != True) {
				active = oldactive;
				mm.Delete();
				ui->changestate(packetlist);
			}
		} else
			active = oldactive;
	}

	return retval;
}

void PacketListWindow::Select()
{
	packetList->gotoFile(active);
}

bool PacketListWindow::back()
{
	bool end = false;

	if (currDir != origDir) {
		oneDir *oldDir = currDir->parent;
		delete currDir;
		currDir = oldDir;

		newList();
		position = currDir->position;
		active = currDir->active;

		ui->redraw();
	} else
		end = true;
	return end;
}

bool PacketListWindow::extrakeys(int key)
{
	bool end = false;

	switch (key) {
#ifdef USE_MOUSE
	case MM_MOUSE:
		{
			int begx = list->xstart(), begy = list->ystart();

			if ( (mouse_event.y != begy) ||
			    ((mouse_event.x < (begx + 3)) || (mouse_event.x >
			    (begx + list_max_x))) )
				break;
		}
#endif
	case 'S':
	case '$':
		packetList->resort();
		sorttype = !sorttype;
		delete list;
		MakeActiveCore();
		break;
	case 'G':
		gotoDir();
		break;
	case 'R':
		renamePacket();
		break;
	case MM_DEL:
	case 'K':
		killPacket();
		break;
	case 'T':
		Select();
		packetList->setDate();
		time(&currTime);
		delete list;
		MakeActiveCore();
		break;
	case 'U':
		newList();
		ui->redraw();
	}
	return end;
}

void PacketListWindow::setFilter(const char *item)
{
	packetList->setFilter(item);

	noDirs = packetList->getNoOfDirs();
	noFiles = packetList->getNoOfFiles();
}

bool PacketListWindow::newDir(const char *dname)
{
	char *result = packetList->changeDir(homify(dname));

	if (result) {
		currDir->position = position;
		currDir->active = active;

		oneDir *nd = new oneDir(result, currDir);
		currDir = nd;

		newList();
		position = 0;
		active = noFiles ? noDirs : 0;

		delete[] result;
		return true;
	}
	return false;
}

void PacketListWindow::gotoDir()
{
	char pathname[70];
	pathname[0] = '\0';

	if (ui->savePrompt("New directory:", pathname) &&
	    pathname[0]) {

		if (newDir(pathname))
			ui->redraw();
		else
			ui->nonFatalError("Could not change to directory");

	} else
		ui->nonFatalError("Change cancelled");
}

void PacketListWindow::renamePacket()
{
	if (active >= noDirs) {
		Select();

		const char *fname = packetList->getName();

		char question[60], answer[60];
		sprintf(question, "New filename for %.39s:", fname);

		if (getNumExt(fname) != -1)
			sprintf(answer, "%.59s", fname);
		else {
			const char *base = findBaseName(fname);
			int ext = packetList->nextNumExt(base);

			sprintf(answer, "%.55s.%03d", base, ext);
		}

		if (ui->savePrompt(question, answer) &&
		    answer[0] && strcmp(fname, answer)) {
			const char *expanswer = homify(answer);

			bool changeit =
				!(packetList->exists(expanswer));

			if (changeit) {
				Select();
				changeit = !(packetList->
					changeName(expanswer));

				if (changeit) {
					newList();
					ui->redraw();
				} else
					ui->nonFatalError("Rename failed");
			} else
				ui->nonFatalError("Name already used");
		} else
			ui->nonFatalError("Rename cancelled");
	}
}

void PacketListWindow::killPacket()
{
	if (active >= noDirs) {
		Select();

		char tmp[128];
		sprintf(tmp, "Do you really want to delete %.90s?",
			packetList->getName());

		if (ui->WarningWindow(tmp)) {
			packetList->kill();
			noFiles = packetList->getNoOfFiles();
		}
		ui->redraw();
	}
}

pktstatus PacketListWindow::OpenPacket()
{
	Select();
	if (active < noDirs) {

		if (newDir(0)) 
			ui->redraw();
		else
			ui->nonFatalError("Could not change to directory");

		return NEW_DIR;
	} else
		return (active < NumOfItems()) ?
			mm.selectPacket(packetList->getName()) : PKT_UNFOUND;
}
