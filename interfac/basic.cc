/*
 * MultiMail offline mail reader
 * Interface, ShadowedWin, ListWindow

 Copyright (c) 1996 Kolossvary Tamas <thomas@tvnet.hu>
 Copyright (c) 1997 John Zero <john@graphisoft.hu>
 Copyright (c) 2003 William McBrine <wmcbrine@users.sourceforge.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "interfac.h"

Win::Win(int height, int width, int topline, chtype backg)
{
	init(height, width, topline);
	Clear(backg);
}

Win::Win(int height, int width, int topline, coltype backg)
{
	init(height, width, topline);
	Clear(backg);
}

Win::~Win()
{
	delete[] buffer;
	delwin(win);
}

void Win::init(int height, int width, int topline)
{
	// All windows are centered horizontally.

	win = newwin(height, width, topline, (COLS - width) / 2);
	buffer = new chtype[width + 1];
	keypad(win, TRUE);
	cursor_off();
}

void Win::Clear(chtype backg)
{
	wbkgdset(win, backg | ' ');
	werase(win);
	wbkgdset(win, ' ');
	attrib(backg);
}

void Win::Clear(coltype backg)
{
	Clear(ColorArray[backg]);
}

void Win::put(int y, int x, chtype z)
{
	mvwaddch(win, y, x, z);
}

void Win::put(int y, int x, char z)
{
	mvwaddch(win, y, x, (unsigned char) z);
}

void Win::put(int y, int x, const chtype *z, int len)
{
	// The cast is to suppress warnings with certain implementations
	// of curses that omit "const" from the prototype.

	if (!len)
		len = getmaxx(win) - x;

	mvwaddchnstr(win, y, x, (chtype *) z, len);
}

int Win::put(int y, int x, const char *z, int len)
{
	// Formerly just mvwaddstr() -- this ensures printing of (most)
	// characters outside the ASCII range, instead of control codes.

	const int tabwidth = 8;
	chtype z2;
	int counter = 0, limit = getmaxx(win) - x;

	if ((-1 == len) || (len > limit))
		len = limit;

	for (; *z && (counter < len); z++) {
		z2 = ((unsigned char) *z);
		switch (z2) {
#ifndef ALLCHARSOK			// unprintable control codes
		case 14:		// double musical note
			z2 = 19;
			break;
		case 15:		// much like an asterisk
			z2 = '*';
			break;
		case 155:		// ESC + high bit = slash-o,
			z2 = 'o';	// except in CP 437
			break;
		case 8:			// backspace
		case 12:		// form feed
			z2 = '#';
			break;
#endif
		case 27:		// ESC
			z2 = '`';
			break;
		case '\t':		// TAB
			z2 = ' ';
			int i = (tabwidth - 1) - (counter % tabwidth);
			len += i;
			if (len > limit) {
				i = limit - counter - 1;
				len = limit;
			}
			while (i--)
				buffer[counter++] = z2 | curratt;
		}
		if ((z2 < ' ') || ((z2 > 126) && (z2 < 160)))
			if (isoConsole)
				z2 = '?';
			else
				z2 |= A_ALTCHARSET;
		buffer[counter++] = z2 | curratt;
	}
	mvwaddchnstr(win, y, x, buffer, counter);

	return counter;
}

void Win::attrib(chtype z)
{
	curratt = z;
	wattrset(win, z);
}

void Win::attrib(coltype z)
{
	attrib(ColorArray[z]);
}

void Win::horizline(int y, int len)
{
	wmove(win, y, 1);
	whline(win, ACS_HLINE, len);
}

void Win::update()
{
	wrefresh(win);
}

void Win::delay_update()
{
	wnoutrefresh(win);
}

void Win::wtouch()
{
	touchwin(win);
	wnoutrefresh(win);
}

void Win::wscroll(int i)
{
	scrollok(win, TRUE);
	wscrl(win, i);
	scrollok(win, FALSE);
}

void Win::cursor_on()
{
	leaveok(win, FALSE);
#ifdef PDCURSKLUDGE
	PDC_set_cursor_mode(curs_start, curs_end);
#else
	curs_set(1);
#endif
}

void Win::cursor_off()
{
	leaveok(win, TRUE);
	curs_set(0);
}

int Win::keypressed()
{
	// Return key status immediately (non-blocking)

	nocbreak();	// Clear any halfdelay() set
	cbreak();	// Back to normal -- is there a better way?
	nodelay(win, TRUE);
	
	return wgetch(win);
}

int Win::inkey()
{
	// Wait until key pressed, SIGWINCH received, or 5 seconds

#ifdef __PDCURSES__
	nodelay(win, TRUE);
#else
# ifndef NCURSES_VERSION
	nocbreak();	// Solaris curses needs this, otherwise it's
	cbreak();	// stuck in nodelay mode after keypressed()
# endif
#endif
	halfdelay(50);

	return wgetch(win);
}

void Win::boxtitle(coltype backg, const char *title, chtype titleAttrib)
{
	attrib(backg);
	box(win, 0, 0);

	if (title) {
		put(0, 2, ACS_RTEE);
		put(0, 3 + strlen(title), ACS_LTEE);
		attrib(titleAttrib);
		put(0, 3, title);
		attrib(backg);
	}
}

void Win::clreol(int y, int x)
{
	for (int i = x; i < COLS; i++)
		put(y, i, (chtype) ' ' | curratt);
}

#ifdef USE_MOUSE
int Win::xstart()
{
# ifndef NCURSES_MOUSE_VERSION
	return getbegx(win);
# else
	int x, y;
	getbegyx(win, y, x);
	return x;
# endif
}

int Win::ystart()
{
# ifndef NCURSES_MOUSE_VERSION
	return getbegy(win);
# else
	int x, y;
	getbegyx(win, y, x);
	return y;
# endif
}
#endif

ShadowedWin::ShadowedWin(int height, int width, int topline, coltype backg,
	const char *title, coltype titleAttrib) :
	Win(height, width, topline, backg)
{
	// The text to be in "shadow" is taken from different windows,
	// depending on the curses implementation. Note that the default,
	// "stdscr", just makes the shadowed area solid black; only with
	// ncurses and PDCurses does it draw proper shadows.

#ifdef USE_SHADOWS
	int i, j;
	chtype *right, *lower;
# ifndef NCURSES_VERSION
#  ifdef __PDCURSES__
	WINDOW *&newscr = curscr;
#  else
	WINDOW *&newscr = stdscr;
#  endif
# endif
	int xlimit = 2 + (ui->active() != letter);
	int firstcol = (COLS - width) / 2;

	while ((width + firstcol) > (COLS - xlimit))
		width--;
	while ((height + topline) > (LINES - 1))
		height--;

	right = new chtype[(height - 1) << 1];
	lower = new chtype[width + 1];

	// Gather the old text and attribute info:

	for (i = 0; i < (height - 1); i++)
		for (j = 0; j < 2; j++)
			right[(i << 1) + j] = (mvwinch(newscr,
				(topline + i + 1), (firstcol + width + j)) &
					(A_CHARTEXT | A_ALTCHARSET));

	mvwinchnstr(newscr, (topline + height), (firstcol + 2), lower, width);

	// Redraw it in darkened form:

	shadow = newwin(height, width, topline + 1, firstcol + 2);
	leaveok(shadow, TRUE);

	for (i = 0; i < (height - 1); i++)
		for (j = 0; j < 2; j++)
			mvwaddch(shadow, i, width - 2 + j,
				(right[(i << 1) + j] |
				ColorArray[C_SHADOW]));
	for (i = 0; i < width; i++)
		mvwaddch(shadow, height - 1, i, (lower[i] & (A_CHARTEXT |
			A_ALTCHARSET)) | ColorArray[C_SHADOW]);

	wnoutrefresh(shadow);
#endif
	boxtitle(backg, title, ColorArray[titleAttrib]);

#ifdef USE_SHADOWS
	delete[] lower;
	delete[] right;
#endif
}

ShadowedWin::~ShadowedWin()
{
#ifdef USE_SHADOWS
	delwin(shadow);
#endif
}

void ShadowedWin::touch()
{
#ifdef USE_SHADOWS
	touchwin(shadow);
	wnoutrefresh(shadow);
#endif
	Win::wtouch();
}

int ShadowedWin::getstring(int y, int x, char *string, int maxlen,
		coltype bg_color, coltype fg_color)
{
	int i, j, offset, end, c;
	char *tmp = new char[maxlen + 1];

	int width = getmaxx(win) - x - 1;
	int dwidth = (maxlen > width) ? width : maxlen;

	cropesp(string);

	attrib(fg_color);
	for (i = 0; i < maxlen; i++) {
		if (i < dwidth)
			put(y, x + i, ACS_BOARD);
		tmp[i] = '\0';
	}
	tmp[maxlen] = '\0';

	j = strlen(string);
	offset = (j > dwidth) ? j - dwidth : 0;

	for (i = offset; i < j; i++)
		put(y, x + i - offset, string[i]);
	if (!string[0])
		wmove(win, y, x);

	cursor_on();
	update();

	i = end = 0;
	bool first_key = true;
	
	while (!end) {
		do
			c = inkey();
		while (ERR == c);

		// these keys make the string "accepted" and editable:
		if (first_key) {
			first_key = false;

			switch (c) {
			case MM_LEFT:
			case MM_RIGHT:
			case MM_BACKSP:
			case MM_HOME:
			case MM_END:
#ifdef USE_MOUSE
			case MM_MOUSE:
#endif
				strncpy(tmp, string, maxlen);
				i = strlen(tmp);
			}
		}

		switch (c) {
		case MM_DOWN:
			end++;
		case MM_UP:
			end++;
		case '\t':
		case MM_ENTER:
			end++;
		case MM_ESC:
			end++;
			break;
		case MM_LEFT:
			if (i > 0)
				i--;
			break;
		case MM_RIGHT:
			if ((i < maxlen) && tmp[i])
				i++;
			break;
		case 127:
		case MM_DEL:		// Delete key 
			strncpy(&tmp[i], &tmp[i + 1], maxlen - i);
			tmp[maxlen] = '\0';
			break;
		case MM_BACKSP:
			if (i > 0) {
				strncpy(&tmp[i - 1], &tmp[i], maxlen + 1 - i);
				tmp[maxlen] = '\0';
				i--;
			}
			break;
		case MM_HOME:
			i = 0;
			break;
		case MM_END:
			i = strlen(tmp);
			break;
#ifdef USE_MOUSE
		case MM_MOUSE:
			mm_mouse_get();
			if (mouse_event.bstate & BUTTON3_CLICKED)
				end = 1;
			else
				end = 2;
			break;
#endif
		case MM_DISCARD:
			break;
		default:
			for (j = (maxlen - 1); j > i; j--)
				tmp[j] = tmp[j - 1];
			if (i < maxlen) {
				tmp[i] = c;
				i++;
			}
		}
		while (i < offset)
			offset--;
		while ((i - offset) > dwidth)
			offset++;

		for (j = offset; j < dwidth + offset; j++)
			put(y, x + j - offset, (tmp[j] ? (unsigned char)
				tmp[j] : ACS_BOARD));
		wmove(win, y, x + i - offset);
		update();
	}

	if (tmp[0])
		strcpy(string, tmp);

	attrib(bg_color);

	j = strlen(string);
	for (i = 0; i < dwidth; i++)
		put(y, x + i, ((i < j) ? string[i] : ' '));

	update();
	cursor_off();

	delete[] tmp;

	return end - 1;
}

InfoWin::InfoWin(int height, int width, int topline, coltype backg,
	const char *title, coltype titleAttrib, int bottx, int topx) :
	ShadowedWin(height, width, topline, backg, title, titleAttrib)
{
	lineBuf = new char[COLS];
	info = new Win(height - bottx, width - 2, topline + topx, backg);
}

InfoWin::~InfoWin()
{
	delete info;
	delete[] lineBuf;
}

void InfoWin::irefresh()
{
	info->delay_update();
}

void InfoWin::touch()
{
	ShadowedWin::touch();
	info->wtouch();
}

void InfoWin::oneline(int i, chtype ch)
{
	info->attrib(ch);
	info->put(i, 0, lineBuf);
}

void InfoWin::iscrl(int i)
{
	info->wscroll(i);
}

#ifdef USE_MOUSE
int InfoWin::xstartinfo()
{
	return info->xstart();
}

int InfoWin::ystartinfo()
{
	return info->ystart();
}
#endif

ListWindow::ListWindow()
{
	position = active = 0;
	oldPos = oldActive = oldHigh = -100;
	lynxNav = mm.resourceObject->getInt(UseLynxNav);
}

ListWindow::~ListWindow()
{
}

void ListWindow::checkPos(int limit)
{
	if (active >= limit)
		active = limit - 1;

	if (active < 0)
		active = 0;

	if (active < position)
		position = active;
	else
		if (active - position >= list_max_y)
			position = active - list_max_y + 1;

	if (limit > list_max_y) {
		if (position < 0)
			position = 0;
		else
			if (position > limit - list_max_y)
				position = limit - list_max_y;
	} else
		position = 0;
}

chtype ListWindow::setHighlight(chtype ch)
{
	chtype backg = PAIR_NUMBER(ch) & 7;

	switch (backg) {
	case COLOR_BLACK:
	case COLOR_RED:
	case COLOR_BLUE:
		ch = REVERSE(COLOR_WHITE, COLOR_BLACK);
		break;
	default:
		ch = REVERSE(COLOR_BLACK, COLOR_WHITE);
	}
	return ch;
}

void ListWindow::Draw()
{
#ifdef __PDCURSES__
	const chtype current = ACS_DIAMOND | A_ALTCHARSET;
	const chtype old = ACS_BOARD;
#else
	const chtype current = ACS_DIAMOND;
	const chtype old = ACS_CKBOARD;
#endif
	int i, j, limit = NumOfItems();

	checkPos(limit);

	// Highlight bar:

	if (limit > list_max_y) {
		if (oldHigh == -100) {
			list->attrib(borderCol);
			for (i = top_offset; i < (top_offset +
			    list_max_y); i++)
				list->put(i, list_max_x + 1, old);
		}

		i = active * list_max_y / limit;

		if (i != oldHigh) {
			list->attrib(borderCol);
			list->put(top_offset + oldHigh, list_max_x + 1, old);
			list->put(top_offset + i, list_max_x + 1, current);
			list->delay_update();

			oldHigh = i;
		}
	}

	// Scroll or redraw:

	i = position - oldPos;
	switch (i) {
	case -1:
	case 1:
		list->iscrl(i);
	case 0:
		if (active != oldActive) {
			j = oldActive - position;
			if ((j >= 0) && (j < list_max_y))
				oneLine(j);
		}
		oneLine(active - position);
		list->irefresh();
		break;
	default:
		for (j = 0; j < list_max_y; j++) {
			oneLine(j);
			if (position + j == limit - 1)
				j = list_max_y;
		}
		list->touch();
	}
	oldPos = position;
	oldActive = active;
}

void ListWindow::DrawOne(int i, chtype ch)
{
	// Highlight, if drawing the active bar

	if (position + i == active)
		ch = setHighlight(ch);

	list->oneline(i, ch);
}

void ListWindow::DrawOne(int i, coltype ch)
{
	DrawOne(i, ColorArray[ch]);
}

void ListWindow::DrawAll()
{
	oldPos = oldActive = oldHigh = -100;
	Draw();
}

void ListWindow::Move(direction dir)
{
	int limit = NumOfItems();

	switch (dir) {
	case UP:
		active--;
		break;
	case DOWN:
		active++;
		break;
	case PGUP:
		position -= list_max_y;
		active -= list_max_y;
		break;
	case PGDN:
		position += list_max_y;
		active += list_max_y;
		break;
	case HOME:
		active = 0;
		break;
	case END:
		active = limit - 1;
		break;
	}
	checkPos(limit);
}

void ListWindow::setActive(int x)
{
	active = x;
}

int ListWindow::getActive()
{
	return active;
}

searchret ListWindow::search(const char *item, int mode)
{
	int limit = NumOfItems();
	searchret found = False;

	statetype orgstate = ui->active();
	for (int x = active + 1; (x < limit) && (found == False); x++) {
		if (list->keypressed() == 27) {
			found = Abort;
			break;
		}
		found = oneSearch(x, item, mode);
		if (found == True) {
			active = x;
			if (ui->active() == orgstate)
				Draw();
		}
	}
	checkPos(limit);
	return found;
}

void ListWindow::Prev()
{
	Move(UP);
}

void ListWindow::Next()
{
	Move(DOWN);
}


bool ListWindow::KeyHandle(int key)
{
	bool draw = true, end = false;

	switch (key) {
#ifdef USE_MOUSE
	case MM_MOUSE:
		{
		    int begx = list->xstartinfo(), begy = list->ystartinfo();

		    if ( ((mouse_event.x < begx) || (mouse_event.x >
			(begx + list_max_x))) || ((mouse_event.y <
			(begy - 1)) || (mouse_event.y >
			(begy + list_max_y))) ) {
			    draw = false;
			    end = extrakeys(key);
		    } else {
			mouse_event.y -= begy;
			if (-1 == mouse_event.y)
			    Move(UP);
			else
			    if (list_max_y == mouse_event.y)
				Move(DOWN);
			    else
				if ((begx + list_max_x) ==
				    mouse_event.x) {
					if (mouse_event.y > oldHigh)
					    Move(PGDN);
					else
					    if (mouse_event.y < oldHigh)
						Move(PGUP);
				} else {
					bool select = (mouse_event.bstate &
					    BUTTON1_DOUBLE_CLICKED) || (active
					    == (position + mouse_event.y));
					active = position + mouse_event.y;
					if (select) {
						draw = false;
						end = ui->select();
					}
				}
		    }
		}
		break;
#endif
	case MM_LEFT:
		if (lynxNav) {
			draw = false;
			end = ui->back();
			break;
		}
	case MM_MINUS:
		Prev();
		break;
	case MM_RIGHT:
		if (lynxNav) {
			draw = false;
			end = ui->select();
			break;
		}
	case '\t':
	case MM_PLUS:
		Next();
		break;
	case MM_DOWN:
		Move(DOWN);
		break;
	case MM_UP:
		Move(UP);
		break;
	case MM_HOME:
		Move(HOME);
		break;
	case MM_END:
		Move(END);
		break;
	case 'B':
	case MM_PPAGE:
		Move(PGUP);
		break;
	case ' ':
	case 'F':
	case MM_NPAGE:
		Move(PGDN);
		break;
	case '|':
	case '^':
		draw = false;
		{
			char item[80];
			*item = '\0';

			if (ui->savePrompt("Filter on:", item))
				setFilter(item);
		}
		ui->redraw();
		break;
	default:
		draw = false;
		end = extrakeys(key);
	}
	if (draw)
		Draw();

	return end;
}
