/*
 * MultiMail offline mail reader
 * message display

 Copyright (c) 1996 Kolossvary Tamas <thomas@tvnet.hu>
 Copyright (c) 1997 John Zero <john@graphisoft.hu>
 Copyright (c) 2003 William McBrine <wmcbrine@users.sf.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "interfac.h"

LetterWindow::Line::Line()
{
	next = 0;
	text = 0;
	attr = Normal;
}

void LetterWindow::Line::out(FILE *fd)
{
	for (unsigned i = 0; i < length; i++)
		fputc(text[i], fd);
	fputc('\n', fd);
}

LetterWindow::LetterWindow()
{
	linelist = 0;
	NM.isSet = hidden = rot13 = false;
	NumOfLines = 0;
	tagline1[0] = '\0';
	To = 0;

	beepPers = mm.resourceObject->getInt(BeepOnPers);
	lynxNav = mm.resourceObject->getInt(UseLynxNav);
}

LetterWindow::~LetterWindow()
{
	delete[] To;
}

net_address &LetterWindow::PickNetAddr()
{
	Line *line;
	static net_address result;
	int i;

	for (i = 0; i < NumOfLines; i++)
		if (Origin == linelist[i]->attr)
			break;

	if (i != NumOfLines) {
		line = linelist[i];
		i = line->length;

		char *end = (char *) (line->text) + i;

		while (((line->text[i - 1]) != '(') && i > 0)
			i--;
		//we have the opening bracket

		while ((i < (int) line->length) &&
			((line->text[i] < '0') || (line->text[i] > '9')))
				i++;
		//we have the begining of the address

		char c = *end;
		*end = '\0';

		result = &line->text[i];

		*end = c;
	} else
		result = mm.letterList->getNetAddr();

	return result;
}

void LetterWindow::ReDraw()
{
	headbar->wtouch();
	header->wtouch();
	text->wtouch();
	statbar->cursor_on();	// to get it out of the way
	statbar->wtouch();
	statbar->cursor_off();
}

void LetterWindow::DestroyChain()
{
	if (linelist) {
		while (NumOfLines)
			delete linelist[--NumOfLines];
		delete[] linelist;
		linelist = 0;
	}
	letter_in_chain = -1;
}

/* Take a message as returned by getBody() -- the whole thing as one C
   string, with paragraphs separated by '\n' -- and turn it into a linked
   list with an array index, wrapping long paragraphs if needed. Also
   flags hidden and quoted lines.
*/
void LetterWindow::MakeChain(int columns, bool rejoin)
{
	static const char *seenby = "SEEN-BY:";
	const int tabwidth = 8;

	letter_body *msgBody;
	Line head, *curr;

	int x, orgarea = -1;
	if (mm.areaList->isCollection() && !mm.areaList->isReplyArea()) {
		orgarea = mm.areaList->getAreaNo();
		mm.areaList->gotoArea(mm.letterList->getAreaID());
	}
	bool inet = !(!(mm.areaList->getType() & INTERNET));

	DestroyChain();

	letter_in_chain = mm.letterList->getCurrent();
	curr = &head;

	bool qpenc = mm.letterList->isQP();
	if (hidden)
		mm.letterList->setQP(false);

	msgBody = mm.letterList->getBody();

	if (hidden)
		mm.letterList->setQP(qpenc);

	bool insig = false, skipSeenBy = false;

	while (msgBody) {
	    unsigned char each;
#ifdef BOGUS_WARNING
	    char *src = 0,
#else
	    char *src,
#endif
	    	*dest, *begin;
		
	    int len, maxcol;
	    lineattr tmpattr;

	    bool wrapped = false;
	    bool end = false, allHidden = msgBody->isHidden();
//#define XFACE
#ifdef XFACE
	    // Quick X-Face hack
	    if (allHidden) {
		src = msgBody->getText();
		const char *c = searchstr(src, "x-face: ");
		if (c) {
			char tmp[255];
			char *xname = mytmpnam();
			FILE *f = fopen(xname, "w");
			c += 8;
			bool end = false;
			while (!end) {
				fputc(*c++, f);
				if (!(*c) || (('\n' == *c) && !((' ' == c[1])
				    || ('\t' == c[1]))))
					end = true;
			}
			fputc('\n', f);
			fclose(f);
			sprintf(tmp, "(uncompface -X %s | xv -; rm %s) &",
				xname, xname);
			mysystem(tmp);
			delete[] xname;
		}
	    }
#endif
	    if (!hidden && allHidden)
		end = true;
	    else {
	        src = msgBody->getText();
	        letterconv_in(src);
	    }

	    while (!end) {
		if (!hidden)	// skip ^A lines
		    while ((*src == 1) || (skipSeenBy &&
			!strncmp(src, seenby, 8))) {
			    do
				src++;
			    while (*src && (*src != '\n'));
			    while (*src == '\n')
				src++;
		    }

		if (*src) {
		    curr->next = new Line;
		    curr = curr->next;
		    tmpattr = allHidden ? Hidden : Normal;

		    NumOfLines++;

		    if (*src == 1) {	// ^A is hidden line marker
			src++;
			tmpattr = Hidden;
		    } else
			if (skipSeenBy && !strncmp(src, seenby, 8))
				tmpattr = Hidden;

		    begin = 0;
		    curr->text = dest = src;
		    len = 0;

		    maxcol = columns;

		    while (!end && ((len < maxcol) || (rejoin &&
			(Quoted == tmpattr) && (len < 80)))) {

			    each = *src;

			    if (each == '\n') {
				if (wrapped) {
				    wrapped = false;

				    // Strip any trailing spaces (1):

				    while (len && (dest[-1] == ' ')) {
					len--;
					dest--;
				    }

				    // If the next character indicates a new
				    // paragraph, quote or hidden line, this
				    // is the end of the line; otherwise make
				    // it a space:

				    each = src[1];
				    if (!each || (each == '\n') ||
					(each == ' ') || (each == '\t') ||
					(each == '>') || (each == 1) ||
					skipSeenBy) {
					    src++;
					    break;
				    } else
					if (!len)
					    src++;
					else
					    each = ' ';
				} else {	// if wrapped is false, EOL
				    src++;
				    break;
				}
			    }

			    switch (each) {
			    case '\t':
				maxcol -= (tabwidth - (len % tabwidth));
			    case ' ':
				// begin == start of last word (for wrapping):

				begin = 0;
				//if (rejoin && (Quoted != tmpattr))
				//if (Quoted != tmpattr)
				if (Normal == tmpattr)
					if (len >= (maxcol - 1))
						wrapped = true;
				break;
			    case '>':		// Quoted line
				if ((len < 5) && (Normal == tmpattr))
					tmpattr = Quoted;
				break;
			    case ':':
				if ((src[1] == '-') || (src[1] == ')') ||
				    (src[1] == '('))
					break;
			    case '|':
				if ((len == 0) && (Normal == tmpattr))
					tmpattr = Quoted;
				break;
			    default:
				if (rot13) {
				    if (each >= 'A' && each <= 'Z')
					each = (each - 'A' + 13) % 26 + 'A';
				    else if (each >= 'a' && each <= 'z')
					each = (each - 'a' + 13) % 26 + 'a';
				}
			    }
			    if ((each != ' ') && (each != '\t') && !begin)
				begin = src;

			    if (each) {
				*dest++ = each;
				src++;
				len++;
			    }

			    end = !each;	// a 0-terminated string
		    }

		    // Start a new line on a word boundary (if needed):

		    if ((len >= maxcol) && begin && ((src - begin) <
			maxcol) && !(rejoin && (Quoted == tmpattr))) {
			    x = src - begin;
			    len -= x;
			    while (x--)
				*--src = *--dest;
			    //wrapped = rejoin;
			    wrapped = (Hidden != tmpattr);
		    }

		    // Check for sigs:

		    const char *ct = curr->text;

		    if (inet && !insig && (ct[0] == '-') && (ct[1] == '-') &&
		    // (((ct[2] == ' ') && (len == 3)) )) // || (len == 2)))
		    (((ct[2] == ' ') && (len == 3)) || (len == 2)))
			insig = true;

		    if (insig)
			tmpattr = Sigline;

		    // Strip any trailing spaces (2):

		    while (len && ((ct[len - 1] == ' ') ||
			(ct[len - 1] == '\t')))
			    len--;

		    curr->length = len;

		    // Check for taglines, tearlines, and origin lines:

		    if (!inet && (Normal == tmpattr)) {
			each = *ct;
			if ((ct[1] == each) && (ct[2] == each) &&
			   ((ct[3] == ' ') || (len == 3)))
				switch (each) {
				case '.':
					if (len > 4)
						tmpattr = Tagline;
					break;
				case '-':
				case '~':
					tmpattr = Tearline;
				}
			else
				if (!strncmp(ct, " * Origin:", 10)) {
					tmpattr = Origin;

					// SEEN-BY: should only appear
					//after Origin:
					skipSeenBy = true;
				}
		    }

		    curr->attr = tmpattr;

		} else
		    end = true;
	    }

	    msgBody = msgBody->next;
	}

	linelist = new Line *[NumOfLines];
	curr = head.next;
	x = 0;
	while (curr) {
		linelist[x++] = curr;
		curr = curr->next;
	}

	if (orgarea != -1)
		mm.areaList->gotoArea(orgarea);
}

void LetterWindow::StatToggle(int mask)
{
	int stat = mm.letterList->getStatus();
	stat ^= mask;
	mm.letterList->setStatus(stat);
	ui->setAnyRead();
	DrawFlags();
}

void LetterWindow::MakeChainFixPos()
{
	MakeChain(COLS);
	if (position >= NumOfLines)
		position = (NumOfLines > y) ? NumOfLines - y : 0;
}

void LetterWindow::Draw(bool redo)
{
	if (redo) {
		rot13 = false;
		position = 0;
		tagline1[0] = '\0';
		if (!ui->dontRead()) {
			if (!mm.letterList->getRead()) {
				mm.letterList->setRead(); // nem ide kene? de.
				ui->setAnyRead();
			}
			if (beepPers && mm.letterList->isPersonal())
				beep();
		}
	}

	if (letter_in_chain != mm.letterList->getCurrent())
		MakeChainFixPos();

	DrawHeader();
	DrawBody();
	DrawStat();
}

char *LetterWindow::netAdd(char *tmp)
{
	net_address &na = mm.letterList->getNetAddr();
	if (na.isSet) {
		const char *p = na;
		if (*p)
			tmp += sprintf(tmp, (na.isInternet ?
				" <%s>" : " @ %s"), p);
	}
	return tmp;
}

#define flagattr(x) header->attrib((x) ? C_LHFLAGSHI : C_LHFLAGS)

void LetterWindow::DrawFlags()
{
	int stat = mm.letterList->getStatus();

	flagattr(mm.letterList->getPrivate());
	header->put(2, COLS - 27, "Pvt ");
	flagattr(stat & MS_READ);
	header->put(2, COLS - 23, "Read ");
	flagattr(stat & MS_REPLIED);
	header->put(2, COLS - 18, "Repl ");
	flagattr(stat & MS_MARKED);
	header->put(2, COLS - 13, "Mark ");
	flagattr(stat & MS_SAVED);
	header->put(2, COLS - 8, "Save  ");

	header->delay_update();
}

void LetterWindow::lineCount()
{
	char tmp[30];
	int percent = ((position + y) > NumOfLines) ? 100 :
		(((long) position + y) * 100 / NumOfLines);

	header->attrib(C_LHMSGNUM);
	sprintf(tmp, "%9d/%-10d%3d%%    ", position + 1, NumOfLines,
		percent);
	header->put(1, COLS - 28, tmp);
	header->delay_update();
}

void LetterWindow::UpdateHeader()
{
	char tmp[256], *p;

	int orgarea = -1;
	if (mm.areaList->isCollection() && !mm.areaList->isReplyArea()) {
		orgarea = mm.areaList->getAreaNo();
		mm.areaList->gotoArea(mm.letterList->getAreaID());
	}

	int maxToFromWidth = COLS - 42;
	if (maxToFromWidth > 255)
		maxToFromWidth = 255;

	int maxSubjWidth = COLS - 9;
	if (maxSubjWidth > 255)
		maxSubjWidth = 255;

	if (orgarea != -1)
		mm.areaList->gotoArea(orgarea);

	header->attrib(C_LHMSGNUM);
	sprintf(tmp, "%ld (%d of %d)", mm.letterList->getMsgNum(),
		mm.letterList->getCurrent() + 1, mm.areaList->getNoOfLetters());
	header->put(0, 8, tmp);

	if (orgarea != -1)
		mm.areaList->gotoArea(mm.letterList->getAreaID());

	header->attrib(C_LHFROM);
	p = tmp + sprintf(tmp, "%.*s", maxToFromWidth,
		mm.letterList->getFrom());
	if (mm.areaList->isEmail())
		netAdd(p);
	letterconv_in(tmp);
	header->put(1, 8, tmp);

	header->attrib(C_LHTO);
	if (mm.areaList->hasTo()) {
		p = (char *) mm.letterList->getTo();
		if (*p) {
			p = tmp + sprintf(tmp, "%.*s", maxToFromWidth, p);
			if (mm.areaList->isReplyArea())
				if (p != tmp)
					netAdd(p);
		}
	} else
		sprintf(tmp, "%.*s", maxToFromWidth,
			(const char *) mm.letterList->getNetAddr());
		
	letterconv_in(tmp);
	header->put(2, 8, tmp);

	header->attrib(C_LHSUBJ);
	int i = sprintf(tmp, "%.*s", maxSubjWidth,
		mm.letterList->getSubject());
	letterconv_in(tmp);
	header->put(3, 8, tmp);
	header->clreol(3, i + 8);

	header->attrib(C_LHDATE);
	sprintf(tmp, "%.30s", mm.letterList->getDate());

	// Truncate the date to fit, if needed:
	p = tmp;
	while (p && (strlen(tmp) > 26)) {
		p = strrchr(tmp, ' ');
		if (p)
			*p = '\0';
	}

	letterconv_in(tmp);
	header->put(0, COLS - 27, tmp);

	if (orgarea != -1)
		mm.areaList->gotoArea(orgarea);

	DrawFlags();
}

void LetterWindow::DrawHeader()
{
	header->Clear(C_LHEADTEXT);

	header->put(0, 2, "Msg#:");
	header->put(0, COLS - 33, "Date:");

	header->put(1, 2, "From:");
	header->put(1, COLS - 33, "Line:");

	if (mm.areaList->hasTo())
		header->put(2, 2, "  To:");
	else
		header->put(2, 2, "Addr:");
	header->put(2, COLS - 33, "Stat: ");

	header->put(3, 2, "Subj:");

	//header->horizline(4);
	//header->put(4, 0, ACS_LLCORNER);
	//header->put(4, COLS - 1, ACS_LRCORNER);

	UpdateHeader();
}

void LetterWindow::oneLine(int i)
{
	static const coltype useatt[] = {
		C_LHIDDEN, C_LORIGIN, C_LTEAR, C_LTAGLINE, C_LTAGLINE,
		C_LQTEXT, C_LTEXT
	};
	int length, z = position + i;

	if (z < NumOfLines) {
		Line *curr = linelist[z];

		text->attrib(useatt[curr->attr]);

		if (Tagline == curr->attr) {
			length = curr->length - 4;
			if (length > TAGLINE_LENGTH)
				length = TAGLINE_LENGTH;
			strncpy(tagline1, &curr->text[4], TAGLINE_LENGTH);
			tagline1[length] = '\0';
		}
		length = text->put(i, 0, curr->text, curr->length);
	} else {
		text->attrib(C_LTEXT);
		length = 0;
	}
	text->clreol(i, length);
}

void LetterWindow::DrawBody()
{
	lineCount();

	for (int i = 0; i < y; i++)
		oneLine(i);
	text->delay_update();
}

void LetterWindow::DrawStat()
{
	static const char *helpmsg = " F1 or ? - Help ";
	char format[40], *tmp = new char[COLS + 1];
	
	bool expert = mm.resourceObject->getInt(ExpertMode);
	const char *pn = mm.resourceObject->get(PacketName);

	int pnlen = strlen(pn);
	if (pnlen > 20)
		pnlen = 20;
	int maxw = COLS - ((expert ? 4 : 20) + pnlen);

	bool collflag = false;
	if (mm.areaList->isCollection()) {
		if (mm.areaList->isReplyArea()) {
			maxw -= 10;
			sprintf(format, " %%.*s | REPLY in: %%-*.*s%%s");
		} else {
			maxw -= 13;
			sprintf(format, " %%.*s | PERSONAL in: %%-*.*s%%s");
		}
		mm.areaList->gotoArea(mm.letterList->getAreaID());
		collflag = true;
	} else
		sprintf(format, " %%.*s | %%-*.*s%%s");

	const char *s = mm.letterList->getNewsgrps();
	sprintf(tmp, format, pnlen, pn, maxw, maxw,
		s ? s : mm.areaList->getDescription(),
		expert ? "" : helpmsg);
	areaconv_in(tmp);

	if (s && ((int) strlen(s) > maxw))
		strncpy(tmp + maxw + pnlen + 1, "...", 3);

	if (collflag)
		ui->areas.Select();

	statbar->cursor_on();
	statbar->put(0, 0, tmp);
	statbar->delay_update();
	statbar->cursor_off();

	delete[] tmp;
}

void LetterWindow::TimeUpdate()
{
	static int mode = mm.resourceObject->getInt(ClockMode);
	char tmp[6];
	time_t now = time(0);

	if (mode && ((now - lasttime) > 59)) {
		if (1 == mode)
			strftime(tmp, 6, "%H:%M", localtime(&now));
		else {
			long elapsed = (now - starttime) / 60;
			sprintf(tmp, "%02ld:%02ld", (elapsed / 60) % 100,
				elapsed % 60);
		}
		headbar->put(0, COLS - 6, tmp);
		headbar->delay_update();
		lasttime = now - (now % 60) +
			(2 == mode) ? (starttime % 60) : 0;
	}
}

void LetterWindow::MakeActive(bool redo)
{
	DestroyChain();

	y = LINES - 7;

	headbar = new Win(1, COLS, 0, C_LBOTTSTAT);
	header = new Win(5, COLS, 1, C_LHEADTEXT);
	text = new Win(y, COLS, 6, C_LTEXT);
	statbar = new Win(1, COLS, LINES - 1, C_LBOTTSTAT);

	char *tmp = new char[COLS + 1];
	int i = sprintf(tmp, " " MM_TOPHEADER, sysname());
	tmp[i] = '\0';

	headbar->put(0, 0, tmp);
	headbar->clreol(0, i);
	headbar->delay_update();

	delete[] tmp;

	lasttime = 0;
	TimeUpdate();

 	Draw(redo);
}

bool LetterWindow::Next()
{
	if (mm.letterList->getActive() < (mm.letterList->noOfActive() - 1)) {
		ui->letters.Move(KEY_DOWN);
		mm.letterList->gotoActive(mm.letterList->getActive() + 1);
		Draw(true);
		return true;
	} else {
		ui->back();
		doupdate();
		ui->back();
		doupdate();
		ui->areas.KeyHandle('+');
	}
	return false;
}

bool LetterWindow::Previous()
{
	if (mm.letterList->getActive() > 0) {
		ui->letters.Move(KEY_UP);
		mm.letterList->gotoActive(mm.letterList->getActive() - 1);
		Draw(true);
		return true;
	} else {
		ui->back();
		doupdate();
		ui->back();
		doupdate();
		ui->areas.KeyHandle('-');
	}
	return false;
}

void LetterWindow::Move(int dir)
{
	switch (dir) {
	case MM_UP:
		if (position > 0) {
			position--;
			text->wscroll(-1);
			oneLine(0);
			text->delay_update();
			lineCount();
		}
		break;
	case MM_DOWN:
		if (position < NumOfLines - y) {
			position++;
			lineCount();
			text->wscroll(1);
			oneLine(y - 1);
			text->delay_update();
		}
		break;
	case MM_HOME:
		position = 0;
		DrawBody();
		break;
	case MM_END:
		if (NumOfLines > y) {
			position = NumOfLines - y;
			DrawBody();
		}
		break;
	case 'B':
	case MM_PPAGE:
		position -= ((y < position) ? y : position);
		DrawBody();
		break;
	case 'F':
	case MM_NPAGE:
		if (position < NumOfLines - y) {
			position += y;
			if (position > NumOfLines - y)
				position = NumOfLines - y;
			DrawBody();
		}
		break;
	}
}

void LetterWindow::NextDown()
{
	position += y;
	if (position < NumOfLines)
		DrawBody();
	else
		Next();
}

void LetterWindow::Delete()
{
	DestroyChain();
	delete headbar;
	delete header;
	delete text;
	delete statbar;
	headbar = header = text = statbar = 0;
}

bool LetterWindow::Save(int stype)
{
	FILE *fd;

	static const char *ntemplate[] = {
		"%.8s.%.03d", "%.8s.all", "%.8s.mkd"
	};

	static char keepname[3][128];
	char filename[128], oldfname[128];

	stype--;

	if (keepname[stype][0])
		strcpy(filename, keepname[stype]);
	else {
		switch (stype) {
		case 2:					// Marked
		case 1:					// All
			sprintf(filename, ntemplate[stype],
				mm.areaList->getName());
			break;
		case 0:					// This one
			sprintf(filename, ntemplate[0],
				mm.areaList->getName(),
					mm.letterList->getCurrent());
		}

		unspace(filename);
	}

	strcpy(oldfname, filename);

	if (ui->savePrompt("Save to file:", filename)) {
		mychdir(mm.resourceObject->get(SaveDir));
		fd = fopen(homify(filename), "at");
		if (fd) {
			int num = mm.letterList->noOfActive();

			switch (stype) {
			case 2:
			case 1:
				for (int i = 0; i < num; i++) {
					mm.letterList->gotoActive(i);
					if ((stype == 1) ||
					  (mm.letterList->getStatus() &
					    MS_MARKED))
						write_to_file(fd);
				}
				break;
			case 0:
				write_to_file(fd);
			}
			fclose(fd);
			if (!stype)
				MakeChain(COLS);

			ui->setAnyRead();
		} else {
			char tmp[142];

			sprintf(tmp, "%s: Save failed", filename);
			ui->nonFatalError(tmp);

			return false;
		}
		if (strcmp(filename, oldfname))
			strcpy(keepname[stype], filename);

		return true;
	} else {
		ui->nonFatalError("Save aborted");

		return false;
	}
}

void LetterWindow::write_header_to_file(FILE *fd)
{
	enum {system, area, newsg, date, from, to, addr, subj, items};
	static const char *names[items] = {"System", "  Area", "Newsgr",
		"  Date", "  From", "    To", "  Addr", "  Subj"};
	char Header[512], *p;
	int j;

	for (j = 0; j < 72; j++)
		fputc('=', fd);
	fputc('\n', fd);

	mm.areaList->gotoArea(mm.letterList->getAreaID());

	const char *head[items] = {
		mm.packet->getBBSName(),
		mm.areaList->getDescription(),
		mm.letterList->getNewsgrps(), mm.letterList->getDate(),
		mm.letterList->getFrom(), mm.letterList->getTo(),
		mm.letterList->getNetAddr(), mm.letterList->getSubject()
	};

	ui->areas.Select();

	head[to + mm.areaList->hasTo()] = 0;
	if (head[newsg])
		head[area] = 0;

	for (j = 0; j < items; j++)
		if (head[j]) {
			p = Header + sprintf(Header, "%.511s", head[j]);
			if (((j == from) && mm.areaList->isEmail())
			    || ((j == to) && mm.areaList->isReplyArea()))
				netAdd(p);
			letterconv_in(Header);
			fprintf(fd, " %s: %s\n", names[j], Header);
		}

	for (j = 0; j < 72; j++)
		fputc('-', fd);
	fputc('\n', fd);
}

void LetterWindow::write_to_file(FILE *fd)
{
	write_header_to_file(fd);

	// write chain to file

	MakeChain(80);
	for (int i = 0; i < NumOfLines; i++)
		linelist[i]->out(fd);
	fputc('\n', fd);

	// set saved, unmarked -- not part of writing to a file, but anyway

	int stat = mm.letterList->getStatus();
	int oldstat = stat;
	stat |= MS_SAVED;
	stat &= ~MS_MARKED;
	mm.letterList->setStatus(stat);
	if (stat != oldstat)
		ui->setAnyRead();
}

// For searches (may want to start at position == -1):
void LetterWindow::setPos(int x)
{
	position = x;
}

int LetterWindow::getPos()
{
	return position;
}

searchret LetterWindow::search(const char *item)
{
	searchret found = False;

	for (int x = position + 1; (x < NumOfLines) && (found == False);
	    x++) {

		if (text->keypressed() == 27) {
			found = Abort;
			break;
		}

		found = searchstr(linelist[x]->text, item,
			linelist[x]->length) ? True : False;

		if (found == True) {
			position = x;
			if (ui->active() == letter)
				DrawBody();
		}
	}

	return found;
}

void LetterWindow::KeyHandle(int key)
{
	int t_area;

	switch (key) {
	case ERR:			// no key pressed
		if (ui->active() == letter)
			TimeUpdate();
		break;
#ifdef USE_MOUSE
	case MM_MOUSE:
		if (0 == mouse_event.y)
		    Move(KEY_UP);
		else
		    if ((LINES - 1) == mouse_event.y)
			Move(KEY_DOWN);
		    else
			if (mouse_event.y > 5) {
			    if (mouse_event.y > (LINES >> 1))
				NextDown();
			    else
				Move(KEY_PPAGE);
			} else
			    if (3 == mouse_event.y) {
			      if (mouse_event.x >= (COLS - 8))
				KeyHandle('S');
			      else
				if (mouse_event.x >= (COLS - 13))
				  KeyHandle('M');
				else
				  if (mouse_event.x >= (COLS - 18))
				    KeyHandle('R');
				  else
				    if (mouse_event.x >= (COLS - 23))
				      KeyHandle('U');
				    else
				      if (mouse_event.x >= (COLS - 27))
					KeyHandle('N');
			    }
		break;
#endif
	case 'D':
		rot13 = !rot13;
		MakeChainFixPos();
		DrawBody();
		break;
	case 'X':
		hidden = !hidden;
		MakeChainFixPos();
		DrawBody();
		break;
	case 'I':
		{
		    bool stripCR = mm.resourceObject->getInt(StripSoftCR);
		    mm.resourceObject->set(StripSoftCR, !stripCR);
		}
		MakeChainFixPos();
		DrawBody();
		break;
	case 'S':
		Save(1);
		DrawFlags();
		ReDraw();
		break;
	case '?':
	case MM_F1:
		ui->changestate(letter_help);
		break;
	case 'V':
	case 1:				// Ctrl-A
	case 11:			// Ctrl-V
		{
			int nextAns;
			bool cont = false;
			do {
			    Delete();
			    nextAns = ui->ansiLoop(mm.letterList->
				getBody(), mm.letterList->getSubject(),
				mm.letterList->isLatin());
			    if (nextAns == 1)
				cont = Next();
			    else if (nextAns == -1)
				cont = Previous();
			} while (nextAns && cont);
		}
		break;
	case MM_RIGHT:
		if (lynxNav)
			break;
	case MM_PLUS:
		Next();
		break;
	case MM_LEFT:
		if (lynxNav) {
			ui->back();
			break;
		}
	case MM_MINUS:
		Previous();
		break;
	case ' ':
		NextDown();
		break;
	case 6:				// Ctrl-F
		EditLetter(true);
		ui->redraw();
		break;
	default:
	    if (mm.areaList->isReplyArea()) {
		switch(key) {
		case 'R':
		case 'E':
			EditLetter(false);
			ui->redraw();
			break;
		case MM_DEL:
		case 'K':
			ui->kill_letter();
			break;
		case 2:			// Ctrl-B
			SplitLetter();
			ui->redraw();
			break;
		default:
			Move(key);
		}
	    } else {
		switch (key) {
		case '\t':
			ui->letters.Next();
			Draw(true);
			break;
		case 'M':
		case 'U':
			StatToggle((key == 'M') ? MS_MARKED : MS_READ);
			break;
		case 'R':		// Allow re-editing from here:
		case 'O':
			if (mm.letterList->getStatus() & MS_REPLIED)
				if (EditOriginal()) {
					ui->redraw();
					break;
				}
		case 5:
		case 'E':
			t_area = ui->areaMenu();
			if (t_area != -1) {
				mm.areaList->gotoArea(t_area);

				if ((5 == key) || mm.areaList->isEmail())
					if ((5 == key) || ('E' == key))
						ui->addressbook();
					else {
						net_address nm = PickNetAddr();
						set_Letter_Params(nm, 0);
					}
				EnterLetter(t_area, (5 == key) ? 'E' : key);
			}
			break;
		case 'N':
			t_area = (mm.areaList->getType() & INTERNET) ?
				mm.areaList->findInternet() :
				mm.areaList->findNetmail();
			if (t_area != -1) {
			    net_address nm = PickNetAddr();
			    if (nm.isSet) {
				set_Letter_Params(nm, 0);
				EnterLetter(t_area, 'N');
			    } else
				ui->nonFatalError(
					"No reply address");
			} else
				ui->nonFatalError(
					"Netmail is not available");
			break;
		case 'T':
			GetTagline();
			break;
		default:
			Move(key);
		}
	    }
	}
}
