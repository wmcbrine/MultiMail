/*
 * MultiMail offline mail reader
 * ANSI image/text viewer

 Copyright (c) 2003 William McBrine <wmcbrine@users.sf.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "interfac.h"

#ifdef LIMIT_MEM
extern "C" {
#include <alloc.h>
}
#endif

//------------------
// AnsiLine methods
//------------------

AnsiWindow::AnsiLine::AnsiLine(AnsiLine *parent) : prev(parent)
{
	next = 0;
	text = 0;
	length = 0;
	isasc = false;
}

AnsiWindow::AnsiLine::~AnsiLine()
{
	if (isasc)
		delete[] atext;
	else
		delete[] text;
}

AnsiWindow::AnsiLine *AnsiWindow::AnsiLine::getnext(bool isnew)
{
	if (!next)
		if (isnew)
			next = new AnsiLine(this);
	return next;
}

AnsiWindow::AnsiLine *AnsiWindow::AnsiLine::getprev()
{
	return prev;
}

int AnsiWindow::AnsiLine::unpack(chtype *tmp)
{
	int i;

	if (isasc)
		for (i = 0; i < (int) length; i++)
			tmp[i] = att | atext[i];
	else
		memcpy(tmp, text, length * sizeof(chtype));

	for (i = length; i < COLS; i++)
		tmp[i] = ' ' | C_ANSIBACK;

	return length;
}

void AnsiWindow::AnsiLine::pack(chtype *tmp, int newlen)
{
	int i;

	if (isasc)
		delete[] atext;
	else
		delete[] text;

	isasc = true;
	att = *tmp & ~(A_CHARTEXT);

	for (i = 0; (i < newlen) && isasc; i++)
		if ((tmp[i] & ~(A_CHARTEXT)) != att)
			isasc = false;

	if (isasc) {
		atext = new char[newlen];
		for (i = 0; i < newlen; i++)
			atext[i] = tmp[i] & (A_CHARTEXT);
	} else {
		text = new chtype[newlen];
		memcpy(text, tmp, newlen * sizeof(chtype));
	}

	length = newlen;
}

void AnsiWindow::AnsiLine::show(Win *win, int i)
{
	if (length)
		if (isasc) {
			win->attrib(att);
			win->put(i, 0, atext, length);
		} else {
			win->attrib(0);
			win->put(i, 0, text, length);
		}

	win->attrib(C_ANSIBACK);
	win->clreol(i, length);
}

void AnsiWindow::AnsiLine::unpacktext(char *tmp)
{
	if (isasc) {
		memcpy(tmp, atext, length);
		tmp += length;
	} else
		for (unsigned i = 0; i < length; i++)
			*tmp++ = (text[i] & (A_CHARTEXT));
	*tmp = '\0';
}

void AnsiWindow::AnsiLine::remapzero(chtype newatt)
{
	if (isasc) {
		if (!PAIR_NUMBER(att & (A_COLOR))) {
			att &= ~(A_COLOR);
			att |= newatt;
		}
	} else
		for (unsigned i = 0; i < length; i++)
			if (!PAIR_NUMBER(text[i] & (A_COLOR))) {
				text[i] &= ~(A_COLOR);
				text[i] |= newatt;
			}
}

//--------------------
// StringFile methods
//--------------------

/*
void StringFile::init(const char *srcStrA)
{
	fromFile = 0;
	msgBody = 0;
	srcStr = (const unsigned char *) srcStrA;

	reset();
}
*/

void StringFile::init(file_header *fromFileA)
{
	fromFile = fromFileA;
	srcStr = 0;
	afile = 0;

	reset();
}

void StringFile::init(letter_body *msgBodyA)
{
	fromFile = 0;
	msgBody = msgBodyFirst = msgBodyA;

	reset();
}

void StringFile::close()
{
	if (fromFile) {
		fclose(afile);
		afile = 0;
	}
}

unsigned char StringFile::nextchar()
{
	int result;

	if (fromFile) {
		result = fgetc(afile);
		if (EOF == result)
			result = 0;
	} else {
		result = *curpos++;

		if (!result && msgBody) {
			msgBody = msgBody->next;
			if (msgBody) {
				srcStr = (const unsigned char *)
					msgBody->getText();
				curpos = srcStr;
				result = *curpos++;
			}
		}
	}

	return result;
}

void StringFile::backup(int c)
{
	if (fromFile)
		ungetc(c, afile);
	else
		if (curpos > srcStr)
			curpos--;
}

void StringFile::reset()
{
	if (fromFile) {
		if (!afile) {
			afile = mm.workList->ftryopen(fromFile->getName());
			if (!afile)
				fatalError("Error opening ANSI file");
		}
		fseek(afile, 0, SEEK_SET);
	} else {
		if (msgBodyFirst) {
			msgBody = msgBodyFirst;
			srcStr = (const unsigned char *)
				msgBody->getText();
		}
		curpos = srcStr;
	}
}

bool StringFile::anyleft()
{
	return !(fromFile ? feof(afile) :
		(!*curpos && (!msgBody || !msgBody->next)));
}

//--------------------
// AnsiWindow methods
//--------------------

void AnsiWindow::DestroyChain()
{
	while (NumOfLines--)
		delete linelist[NumOfLines];
	delete[] linelist;
}

int AnsiWindow::getparm()
{
	char *parm;
	int value;

	if (escparm[0]) { 
		for (parm = escparm; (*parm != ';') && *parm; parm++);

		if (*parm == ';') {			// more params after
			*parm++ = '\0';
			value = (parm == escparm + 1) ? 1 : atoi(escparm);
			strcpy(escparm, parm);
		} else {				// last parameter
			value = atoi(escparm);
			escparm[0] = '\0';
		}
	} else						// empty last param
		value = 1;

	return value;
}

void AnsiWindow::cls()
{
	if (anim)
		animtext->Clear(C_ANSIBACK);
	else {
		cpy = NumOfLines - baseline - 1;
		checkpos();

		if (baseline < (NumOfLines - 1))
			baseline = NumOfLines - 1;
	}
	posreset();	
}

void AnsiWindow::colreset()
{
	cfl = crv = cbr = 0;
	ccf = COLOR_WHITE;
	ccb = COLOR_BLACK;
}

void AnsiWindow::colorcore()
{
	// Attribute set

	// Check bold and blinking:

	attrib = (cbr ? A_BOLD : 0) | (cfl ? A_BLINK : 0) |

	// If animating, check for color pair 0 (assumes COLOR_BLACK == 0),
	// and for remapped black-on-black:

		((!anim || ((ccb | ccf) || !(oldcolorx | oldcolory))) ?

		(crv ? REVERSE(ccf, ccb) : COL(ccf, ccb)) :

		(crv ? REVERSE(oldcolorx, oldcolory) : COL(oldcolorx,
		oldcolory)));

	// If not animating, mark color pair as used:

	if (!anim)
#ifdef NOREVERSE
		if (crv)
			colorsused[(ccb << 3) + ccf] = true;
		else
#endif
			colorsused[(ccf << 3) + ccb] = true;
}

void AnsiWindow::colorset()
{
	static const int colortable[8] = {COLOR_BLACK, COLOR_RED,
		COLOR_GREEN, COLOR_YELLOW, COLOR_BLUE, COLOR_MAGENTA,
		COLOR_CYAN, COLOR_WHITE};
	int tmp;

	while(escparm[0]) {
		tmp = getparm();
		switch (tmp) {
		case 0:			// reset colors
			colreset();
			break;
		case 1:			// bright
			cbr = 1;
			break;
		case 5:			// flashing
			cfl = 1;
			break;
		case 7:			// reverse
			crv = 1;
			break;
		default:
			if ((tmp > 29) && (tmp < 38))		// foreground
				ccf = colortable[tmp - 30];
			else if ((tmp > 39) && (tmp < 48))	// background
				ccb = colortable[tmp - 40];
		}
	}

	colorcore();
}

void AnsiWindow::athandle()
{
	static const int colortable[8] = {COLOR_BLACK, COLOR_BLUE,
		COLOR_GREEN, COLOR_CYAN, COLOR_RED, COLOR_MAGENTA,
		COLOR_YELLOW, COLOR_WHITE};
	static int oldccf = -1, oldccb, oldcbr, oldcfl;
	unsigned fg = 0, bg = 0;
	char c[2];

	bool xmode = false;

	c[0] = source.nextchar();
	c[1] = '\0';

	switch (c[0]) {
	case 'X':
		xmode = true;
		c[0] = source.nextchar();
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		sscanf(c, "%x", &bg);

		c[0] = source.nextchar();
		sscanf(c, "%x", &fg);

		if (1 == atparse)
			if (!fg && !bg) {
				oldccf = ccf;
				oldccb = ccb;
				oldcbr = cbr;
				oldcfl = cfl;
			} else {
				if ((15 == fg) && (15 == bg)) {
					if (-1 != oldccf) {
						ccf = oldccf;
						ccb = oldccb;
						cbr = oldcbr;
						cfl = oldcfl;

						colorcore();
					}
				} else {
					cbr = (fg > 7);
					cfl = (bg > 7);

					ccf = colortable[cbr ? (fg - 8) : fg];
					ccb = colortable[cfl ? (bg - 8) : bg];

					colorcore();
				}
			}

		if (!xmode) {
			c[0] = source.nextchar();
			if ('@' != c[0])
				source.backup(c[0]);
		}
		break;
	case 'C':
		c[0] = source.nextchar();
		c[1] = source.nextchar();

		if (('L' == c[0]) && ('S' == c[1])) {
			cls();
			c[0] = source.nextchar();
		} else {
			update('@');
			update('C');
			update(c[0]);
			update(c[1]);
		}
		break;
	default:
		source.backup(c[0]);
		update('@');
	}
}

void AnsiWindow::escfig()
{
	char a[2];

	a[0] = source.nextchar();
	a[1] = '\0';

	switch (a[0]) {
	case 'A':			// cursor up
		cpy -= getparm();
		if (cpy < 0)
			cpy = 0;
		break;
	case 'B':			// cursor down
		cpy += getparm();
		if (anim && (cpy > (LINES - 2)))
			cpy = LINES - 2;
		break;
	case 'C':			// cursor right
		cpx += getparm();
		if (cpx > (COLS - 1))
			cpx = COLS - 1;
		break;
	case 'D':			// cursor left
		cpx -= getparm();
		if (cpx < 0)
			cpx = 0;
		break;
	case 'J':			// clear screen
		if (getparm() == 2)
			cls();
		break;
	case 'H':			// set cursor position
	case 'f':
		cpy = getparm() - 1;
		cpx = getparm() - 1;
		break;
	case 's':			// save position
		spx = cpx;
		spy = cpy;
		break;
	case 'u':			// restore position
		cpx = spx;
		cpy = spy;
		break;
	case 'm':			// attribute set (colors, etc.)
		colorset();
		break;
	case 'M':			// eat the music (Kate Bush)
		do
			*a = source.nextchar();
		while ((14 != *a) && ('\n' != *a));
		break;

	// I know this is strange, but I think it beats the alternatives:

	default:
		if ((a[0] >= '0') && (a[0] <= '9'))

	case '=':			// for some weird mutant codes
	case '?':

	case ';':			// parameter separator
		{
			strcat(escparm, a);
			escfig();
		}
	}
}

void AnsiWindow::posreset()
{
	cpx = cpy = lpy = spx = spy = 0;
}

void AnsiWindow::checkpos()
{
	if (cpy != lpy) {
		curr->pack(chtmp, tlen);

		if (cpy > lpy) {
		    for (; lpy != cpy; lpy++)		// move down and
			curr = curr->getnext(true);	// allocate space
		} else
		    if (cpy < lpy) {
			for (; lpy != cpy; lpy--)	// move up
				curr = curr->getprev();
		    }

		if ((cpy + 1) > (NumOfLines - baseline))
			NumOfLines = cpy + baseline + 1;

		tlen = curr->unpack(chtmp);
	}
}

void AnsiWindow::update(unsigned char c)
{
	static const chtype acstrans[] = {
		ACS_BOARD, ACS_BOARD, ACS_CKBOARD, ACS_VLINE, ACS_RTEE,
		ACS_RTEE, ACS_RTEE, ACS_URCORNER, ACS_URCORNER, ACS_RTEE,
		ACS_VLINE, ACS_URCORNER, ACS_LRCORNER, ACS_LRCORNER,
		ACS_LRCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_BTEE,
		ACS_TTEE, ACS_LTEE, ACS_HLINE, ACS_PLUS, ACS_LTEE,
		ACS_LTEE, ACS_LLCORNER, ACS_ULCORNER, ACS_BTEE, ACS_TTEE,
		ACS_LTEE, ACS_HLINE, ACS_PLUS, ACS_BTEE, ACS_BTEE,
		ACS_TTEE, ACS_TTEE, ACS_LLCORNER, ACS_LLCORNER,
		ACS_ULCORNER, ACS_ULCORNER, ACS_PLUS, ACS_PLUS,
		ACS_LRCORNER, ACS_ULCORNER, ACS_BLOCK,

		// There are no good equivalents for these four:

		'_', '[', ']', '~'

		//ACS_BULLET, ACS_VLINE, ACS_VLINE, ACS_BULLET
	};


	if (!ansiAbort) {
		chtype ouch;

		if (isoConsole && !isLatin) {
			if ((c > 175) && (c < 224)) {
				ouch = acstrans[c - 176];

				// suppress or'ing of A_ALTCHARSET:
				c = ' ';

			} else {
				if (c & 0x80)
					c = (unsigned char)
						dos2isotab[c & 0x7f];
				ouch = c;
			}
		} else {
			if (!isoConsole && isLatin)
				if (c & 0x80)
					c = (unsigned char)
						iso2dostab[c & 0x7f];
			ouch = c;
		}

		if ((c < ' ') || ((c > 126) && (c < 160)))
			ouch |= A_ALTCHARSET;

		ouch |= attrib;

		int limit = LINES - 2;

		if (anim) {
			if (cpy > limit) {
				animtext->wscroll(cpy - limit);
				for (int i = cpy - limit; i; i--)
					animtext->clreol(limit - i + 1, 0);
				cpy = limit;
			}
			animtext->attrib(0);
			animtext->put(cpy, cpx++, ouch);
			animtext->attrib(C_ANSIBACK);
			animtext->update();
			ansiAbort |= (animtext->keypressed() != ERR);
		} else {
			checkpos();
			chtmp[cpx++] = ouch;
			if (cpx > tlen)
				tlen = cpx;
		}
		if (cpx == COLS) {
			cpx = 0;
			cpy++;
		}
	}
}

void AnsiWindow::ResetChain()
{
	head = new AnsiLine();
	curr = head;
	posreset();
	curr = curr->getnext(true);
	tlen = 0;
	NumOfLines = 1;
	baseline = 0;
}

void AnsiWindow::MakeChain()
{
	unsigned char c;

	ansiAbort = false;
	if (!anim) {
		ResetChain();
		chtmp = new chtype[COLS];
		curr->unpack(chtmp);
	}
	attrib = C_ANSIBACK;
	colreset();

	source.reset();

	do {
		c = source.nextchar();
#ifdef LIMIT_MEM
		if (coreleft() < ((unsigned long) NumOfLines *
		    sizeof(AnsiLine *) + 0x200))
			c = 0;
#endif

		switch (c) {
		case 1:			// hidden lines (only in pos 0)
			if (!cpx) {
				do
					c = source.nextchar();
				while (source.anyleft() && (c != '\n'));
			}
		case 0:
			break;
		case 8:			// backspace
			if (cpx)
				cpx--;
			break;
		case 10:
			cpy++;
		case 13:
			cpx = 0;
		case 7:			// ^G: beep
		case 26:		// ^Z: EOF for DOS
			break;
		case 12:		// form feed
			if (anim) {
				animtext->Clear(C_ANSIBACK);
				posreset();
			}
			break;
#ifndef ALLCHARSOK			// unprintable control codes
		case 14:		// double musical note
			update(19);
			break;
		case 15:		// much like an asterisk
			update('*');
			break;
		case 155:		// ESC + high bit = slash-o,
			update('o');	// except in CP 437
			break;
#endif
		case '`':
		case 27:		// ESC
			c = source.nextchar();
			if ('[' == c) {
				escparm[0] = '\0';
				escfig();
			} else {
				source.backup(c);
				update('`');
			}
			break;
		case '\t':		// TAB
			cpx = ((cpx / 8) + 1) * 8;
			while (cpx >= COLS) {
				cpx -= COLS;
				cpy++;
			}
			break;
		case '@':
			if (atparse) {
				athandle();
				break;
			}
		default:
			update(c);
		}
	} while (c && !ansiAbort);

	if (!anim) {
		curr->pack(chtmp, tlen);
		delete[] chtmp;

		if (!ansiAbort) {
			linelist = new AnsiLine *[NumOfLines];
			curr = head->getnext();
			int i = 0;
			while (curr) {
				linelist[i++] = curr;
				curr = curr->getnext();
			}
		}

		delete head;
	}
}

void AnsiWindow::statupdate(const char *intro)
{
	static const char *helpmsg = "F1 or ? - Help ",
		*mainstat = " ANSI View";
	char *tmp = new char[COLS + 1];
	const char *pn = mm.resourceObject->get(PacketName);
	bool expert = mm.resourceObject->getInt(ExpertMode);
	
	int pnlen = strlen(pn);
	if (pnlen > 20)
		pnlen = 20;

	int maxw = COLS - pnlen - (expert ? 16 : 31);
	sprintf(tmp, " %.*s |%s: %-*.*s %s", pnlen, pn,
		(intro ? intro : mainstat),
		maxw, maxw, title, expert ? "" : helpmsg);

	statbar->cursor_on();
	statbar->put(0, 0, tmp);
	statbar->update();
	statbar->cursor_off();

	delete[] tmp;
}

void AnsiWindow::animate()
{
	animtext = new Win(LINES - 1, COLS, 0, C_ANSIBACK);
	animtext->cursor_on();

	posreset();
	colreset();
	anim = true;

	statupdate(" Animating");

	MakeChain();

	if (!ansiAbort)
		statupdate("      Done");

	anim = false;
	while (!ansiAbort && (animtext->inkey() == ERR));

	animtext->cursor_off();
	delete animtext;

	header->wtouch();
	text->wtouch();
	statupdate();
}

void AnsiWindow::oneLine(int i)
{
	linelist[position + i]->show(text, i);
}

void AnsiWindow::lineCount()
{
	char tmp[26];
	int percent = ((long) position + y) * 100 / NumOfLines;
	if (percent > 100)
		percent = 100;

	sprintf(tmp, "Lines: %6d/%-6d %3d%%", position + 1, NumOfLines,
		percent);
	header->put(0, COLS - 26, tmp);
	header->delay_update();
}

void AnsiWindow::DrawBody()
{
	lineCount();

	for (int i = 0; i < y; i++)
		if ((position + i) < NumOfLines)
			oneLine(i);
		else
			text->clreol(i, 0);
}

void AnsiWindow::set(letter_body *ansiSource, const char *winTitle,
	bool latin)
{
	source.init(ansiSource);

	position = 0;
	isLatin = latin;
	title = winTitle;
	atparse = 1;
}

void AnsiWindow::set(file_header *f, const char *winTitle, bool latin)
{
	source.init(f);

	position = 0;
	isLatin = latin;
	title = winTitle;
	atparse = 1;
}

void AnsiWindow::MakeActive()
{
	int i;
	bool end, oldAbort;

	header = new Win(1, COLS, 0, C_LBOTTSTAT);
	text = new Win(LINES - 2, COLS, 1, C_ANSIBACK);
	statbar = new Win(1, COLS, LINES - 1, C_LBOTTSTAT);

	anim = false;
	oldAbort = ansiAbort;

	char *tmp = new char[COLS + 1];
	i = sprintf(tmp, " " MM_TOPHEADER, MM_NAME, MM_MAJOR, MM_MINOR);
	for (; i < COLS; i++)
		tmp[i] = ' ';
	tmp[i] = '\0';

	header->put(0, 0, tmp);
	header->delay_update();
	delete[] tmp;

	y = LINES - 2;
	x = COLS;

	for (i = 0; i < 64; i++)
		colorsused[i] = false;

	colorsused[PAIR_NUMBER(C_LBOTTSTAT)] = 1;  //don't remap stat bars

	oldcolorx = oldcolory = 0;

	init_pair(((COLOR_WHITE) << 3) + (COLOR_WHITE),
		COLOR_WHITE, COLOR_WHITE);

	MakeChain();

	// This deals with the unavailability of color pair 0:

	if (colorsused[0]) {	// assumes COLOR_BLACK == 0

		// Find an unused color pair for black-on-black:

		for (end = false, i = 1; i < 64 && !end; i++)
			if (!colorsused[i]) {
				end = true;
				oldcolorx = i >> 3;
				oldcolory = i & 7;
				init_pair(i, COLOR_BLACK, COLOR_BLACK);
			}

		// Remap all instances of color pair 0 to the new pair:

		if (end)
			for (i = 0; i < NumOfLines; i++)
				linelist[i]->remapzero(COL(oldcolorx,
					oldcolory));
	}

	DrawBody();

	text->delay_update();

	statupdate();

	ansiAbort = oldAbort;
}

void AnsiWindow::Delete()
{
	ansiAbort = true;

	// Restore remapped color pairs:

	if (oldcolorx + oldcolory)
		init_pair((oldcolorx << 3) + oldcolory,
			oldcolorx, oldcolory);
	init_pair(((COLOR_WHITE) << 3) + (COLOR_WHITE),
		COLOR_BLACK, COLOR_BLACK);

	DestroyChain();
	delete header;
	delete text;
	delete statbar;
	source.close();
}

void AnsiWindow::setPos(int n)
{
	position = n;
}

int AnsiWindow::getPos()
{
	return position;
}

searchret AnsiWindow::search(const char *item)
{
	searchret found = False;

	char *buffer = new char[COLS + 1];

	for (int n = position + 1; (n < NumOfLines) && (found == False);
	    n++) {

		if (text->keypressed() == 27) {
			found = Abort;
			break;
		}

		linelist[n]->unpacktext(buffer);
		found = searchstr(buffer, item) ? True : False;

		if (found == True) {
			position = n;
			DrawBody();
			text->delay_update();
		}
	}

	delete[] buffer;
	return found;
}

void AnsiWindow::Save()
{
	FILE *fd;
	static char keepname[128];
	char filename[128], oldfname[128];

	if (keepname[0])
		strcpy(filename, keepname);
	else {
		sprintf(filename, "%.8s.ans", title);
		unspace(filename);
	}

	strcpy(oldfname, filename);

	if (ui->savePrompt("Save to file:", filename)) {
		mychdir(mm.resourceObject->get(SaveDir));
		fd = fopen(homify(filename), "at");
		if (fd) {
			unsigned char c;
			source.reset();
			do {
				c = source.nextchar();
				if (c)
					fputc(c, fd);
			} while (c);
			fclose(fd);
		} else {
			char tmp[142];

			sprintf(tmp, "%s: Save failed", filename);
			ui->nonFatalError(tmp);
		}
		if (strcmp(filename, oldfname))
			strcpy(keepname, filename);
	} else
		ui->nonFatalError("Save aborted");
}

void AnsiWindow::KeyHandle(int key)
{
	switch (key) {
#ifdef USE_MOUSE
	case MM_MOUSE:
		if (0 == mouse_event.y)
			KeyHandle(KEY_UP);
		else
			if ((LINES - 1) == mouse_event.y)
				KeyHandle(KEY_DOWN);
			else
				if (mouse_event.y > (LINES >> 1))
					KeyHandle(KEY_NPAGE);
				else
					KeyHandle(KEY_PPAGE);
		break;
#endif
	case MM_UP:
		if (position > 0) {
			position--;
			text->wscroll(-1);
			oneLine(0);
			lineCount();
		}
		break;
	case MM_DOWN:
		if (position < NumOfLines - y) {
			position++;
			text->wscroll(1);
			oneLine(y - 1);
			lineCount();
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
	case ' ':
		position += y;
		if (position < NumOfLines)
			DrawBody();
		else
			ui->setKey('\n');
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
	case 'V':
	case 1:
	case 22:
		animate();
		break;
	case 'S':
		Save();
		DrawBody();
		break;
	case '@':
		atparse++;
		if (3 == atparse)
			atparse = 0;
		ui->redraw();
		break;
	case MM_F1:
	case '?':
		ui->changestate(ansi_help);
	}
	text->delay_update();
}
