/*
 * MultiMail offline mail reader
 * address book

 Copyright (c) 1996 Kolossvary Tamas <thomas@vma.bme.hu>
 Copyright (c) 2003 William McBrine <wmcbrine@users.sf.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "interfac.h"

extern "C" int perscomp(const void *a, const void *b)
{
	return strcasecmp((*((Person **) a))->name,
		(*((Person **) b))->name);
}

Person::Person(const char *sname, const char *saddr)
{
	if (saddr && (*saddr == 'I'))
		saddr++;
	netmail_addr = saddr;
	setname(sname);
	killed = false;
	next = 0;
}

Person::Person(const char *sname, net_address &naddr)
{
	netmail_addr = naddr;
	setname(sname);
	killed = false;
	next = 0;
}

Person::~Person()
{
	delete[] name;
}

void Person::setname(const char *sname)
{
	name = strdupplus(sname);
}

void Person::dump(FILE *fd)
{
	fprintf(fd, (netmail_addr.isInternet ? "%s\nI%s\n\n" :
		"%s\n%s\n\n"), name, (const char *) netmail_addr);
}

AddressBook::AddressBook()
{
	NumOfPersons = NumOfActive = 0;
	people = living = 0;
	filter = 0;
}

AddressBook::~AddressBook()
{
	DestroyChain();
}

void AddressBook::MakeActive(bool NoEnterA)
{
	int expmode = mm.resourceObject->getInt(ExpertMode);
	statetype s = ui->prevactive();
	if (s != address)
		inletter = ((s == letter) || (s == littlearealist)) &&
			!mm.areaList->isReplyArea();

	NoEnter = NoEnterA;

	list_max_y = LINES - (expmode ? 9 : 12);
	list_max_x = COLS - 6;
	top_offset = 2;

	borderCol = C_ADDR1;

	char tmp[60];
	char *p = tmp + sprintf(tmp, "Addresses");
	if (NumOfActive > list_max_y)
		p += sprintf(p, " (%d)", NumOfActive);
	if (filter)
		sprintf(p, " | %.20s", filter);

	int xwidth = list_max_x + 2;
	list = new InfoWin(LINES - 6, xwidth, 2, borderCol, tmp, C_ADDR2,
		expmode ? 3 : 6);

	list->attrib(C_ADDR2);
	list->put(1, 2,
		"Name                            Netmail address");

	if (!expmode) {
		int x = xwidth / 3 + 1;
		int y = list_max_y + 2;

		list->horizline(y, list_max_x);

		list->put(++y, 3, ": Quit addressbook");
		list->put(y, x + 3, ": Add new address");
		list->put(y, x * 2 + 1, ": search / next");
		list->put(++y, 3, ": Kill current address");
		list->put(y, x + 3, ": Edit address");

		if (inletter)
			list->put(y, x * 2 + 1,
				  ": address from Letter");

		list->attrib(C_ADDR1);
		list->put(y, 2, "K");
		list->put(y, x + 2, "E");

		if (inletter)
			list->put(y, x * 2, "L");

		list->put(--y, 2, "Q");
		list->put(y, x + 2, "A");
		list->put(y, x * 2 - 3, "/, .");
	}
	DrawAll();
}

void AddressBook::Delete()
{
	delete list;
}

bool AddressBook::extrakeys(int key)
{
	switch (key) {
	case MM_ENTER:
		SetLetterThings();
		break;
	case 'A':
		NewAddress();
		break;
	case 'E':
		ChangeAddress();
		break;
	case MM_DEL:
	case 'K':
		kill();
		break;
	case 'L':
		GetAddress();
	}
	return false;
}

void AddressBook::setFilter(const char *item)
{
	delete[] filter;
	filter = strdupplus(item);
	MakeChain();
	if (!NumOfActive) {
		delete[] filter;
		filter = 0;
		MakeChain();
	}
}

void AddressBook::WriteFile()
{
	FILE *fd = fopen(addfname, "wt");
	for (int x = 0; x < NumOfPersons; x++)
		people[x]->dump(fd);
	fclose(fd);
}

void AddressBook::kill()
{
	if (highlighted) {
		if (ui->WarningWindow("Remove this address?")) {
			highlighted->killed = true;

			if (position)
				position--;

			NumOfPersons--;

			MakeChain();
			WriteFile();
		}
		Delete();
		MakeActive(NoEnter);
	}
}

void AddressBook::SetLetterThings()
{
	if (!NoEnter && highlighted)
		ui->letterwindow.set_Letter_Params(
			highlighted->netmail_addr, highlighted->name);
}

void AddressBook::Add(const char *Name, net_address &Address)
{
	if (Address.isSet) {
		bool found = false;

		// Dupe check; also positions curr at end of list:
		curr = &head;
		while (curr->next && !found) {
			curr = curr->next;
			found = (curr->netmail_addr == Address) &&
				!strcasecmp(curr->name, Name);
		}

		if (!found) {
			curr->next = new Person(Name, Address);

			FILE *fd = fopen(addfname, "at");
			curr->next->dump(fd);
			fclose(fd);

			NumOfPersons++;
			MakeChain();

			active = NumOfPersons;
			Draw();
		} else
			ui->nonFatalError("Already in addressbook");
	} else
		ui->nonFatalError("No address found");
}

void AddressBook::GetAddress()
{
	if (inletter)
		Add(mm.letterList->getFrom(),
			ui->letterwindow.PickNetAddr());
}

int AddressBook::HeaderLine(ShadowedWin &win, char *buf, int limit,
				int pos, coltype color)
{
	int getit = win.getstring(pos, 8, buf, limit, color, color);
	return getit;
}

int AddressBook::Edit(Person &p)
{
	char NAME[100], NETADD[100];

	const int maxitems = 2;
	int result, current = 0;
	bool end = false;

	if (p.netmail_addr.isSet) {
		sprintf(NAME, "%.99s", p.name);
		sprintf(NETADD, "%.99s", (const char *) p.netmail_addr);
	} else
		NAME[0] = NETADD[0] = '\0';

	ShadowedWin add_edit(4, COLS - 4, (LINES >> 1) - 2, C_LETEXT);

	add_edit.put(1, 2, "Name:");
	add_edit.put(2, 2, "Addr:");

	add_edit.attrib(C_LEGET1);
	add_edit.put(1, 8, NAME);

	add_edit.attrib(C_LEGET2);
	add_edit.put(2, 8, NETADD);

	add_edit.update();

	do {
		result = HeaderLine(add_edit, current ? NETADD : NAME,
			99, current + 1, current ? C_LEGET2 : C_LEGET1);

		switch (result) {
		case 0:
			end = true;
			break;
		case 1:
			current++;
			if (current == maxitems)
				end = true;
			break;
		case 2:
			if (current > 0)
				current--;
			break;
		case 3:
			if (current < maxitems - 1)
				current++;
		}
	} while (!end);

	if (result && NAME[0] && NETADD[0]) {
		p.setname(NAME);
		p.netmail_addr = NETADD;
		if (!p.netmail_addr.isSet)
			result = 0;
	} else
		result = 0;
	return result;
}

void AddressBook::NewAddress()
{
	Person p;

	p.netmail_addr.isSet = false;
	if (Edit(p))
		Add(p.name, p.netmail_addr);
	ui->redraw();
}

void AddressBook::ChangeAddress()
{
	if (highlighted)
		if (Edit(*highlighted))
			WriteFile();
	ui->redraw();
}

void AddressBook::oneLine(int i)
{
	int z = position + i;
	curr = (z < NumOfActive) ? living[z] : 0;

	if (z == active)
		highlighted = curr;

	char *tmp = list->lineBuf;
	int x = curr ? sprintf(tmp, " %-31s %s", curr->name,
		(const char *) curr->netmail_addr) : 0;
	for (; x < list_max_x; x++)
		tmp[x] = ' ';
	tmp[x] = '\0';

	DrawOne(i, C_ADDR3);
}

searchret AddressBook::oneSearch(int x, const char *item, int)
{
	const char *s;

	s = searchstr(living[x]->name, item);
	if (!s)
		s = searchstr(living[x]->netmail_addr, item);
	return s ? True : False;
}

int AddressBook::NumOfItems()
{
	return NumOfActive;
}

void AddressBook::ReadFile()
{
	FILE *fd;
	char name[256], nmaddr[256], other[256];
	bool end = false;

	fd = fopen(addfname, "rt");
	if (fd) {
		curr = &head;
		while (!end) {
			do
				end = !myfgets(name, sizeof name, fd);
			while (name[0] == '\n' && !end);

			end = !myfgets(nmaddr, sizeof nmaddr, fd);

			if (!end) {
				strtok(name, "\n");
				strtok(nmaddr, "\n");
				curr->next = new Person(name, nmaddr);
				curr = curr->next;
				NumOfPersons++;
			}

			do
				end = !myfgets(other, sizeof other, fd);
			while (other[0] != '\n' && !end);
		}
		fclose(fd);
	}

	MakeChain();

	if (NumOfPersons > 1) {
		qsort(people, NumOfPersons, sizeof(Person *), perscomp);
		if (NumOfActive > 1)
			qsort(living, NumOfActive, sizeof(Person *), perscomp);
		ReChain();
	}
}


void AddressBook::MakeChain()
{
	delete[] people;
	delete[] living;

	NumOfActive = 0;

	if (NumOfPersons) {
		people = new Person *[NumOfPersons];
		living = new Person *[NumOfPersons];

		curr = head.next;
		int c = 0;
		while (curr) {
			if (!curr->killed) {
				people[c++] = curr;
				if (!filter || (searchstr(curr->name, filter)
				    || searchstr(curr->netmail_addr, filter)))
					living[NumOfActive++] = curr;
			}
			curr = curr->next;
		}
	} else
		people = living = 0;
}

void AddressBook::ReChain()
{
	head.next = people[0];
	for (int c = 0; c < (NumOfPersons - 1); c++)
		people[c]->next = people[c + 1];

	people[NumOfPersons - 1]->next = 0;
}

void AddressBook::DestroyChain()
{
	while (NumOfPersons)
		delete people[--NumOfPersons];
	delete[] people;
}

void AddressBook::Init()
{
	addfname = mm.resourceObject->get(AddressFile);
	ReadFile();
}
