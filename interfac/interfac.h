/*
 * MultiMail offline mail reader
 * most class definitions for the interface

 Copyright (c) 1996 Kolossvary Tamas <thomas@tvnet.hu>
 Copyright (c) 1997 John Zero <john@graphisoft.hu>
 Copyright (c) 2003 William McBrine <wmcbrine@users.sourceforge.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#ifndef INTERFACE_H
#define INTERFACE_H

#include "../mmail/mmail.h"

extern "C" {
#include <signal.h>

#include CURS_INC
}

#if defined(NCURSES_MOUSE_VERSION) && (NCURSES_MOUSE_VERSION == 1)
# define USE_MOUSE
#else
# if (defined(__PDCURSES__) && defined(__WIN32__)) || defined(XCURSES)
#  define USE_MOUSE
# endif
#endif

/* The following assumes that Ncurses' internal SIGWINCH handler is enabled
   if, and only if, the Ncurses version is 5.0 or higher. That's the default;
   if your setup is different, you should manually define or undefine
   NCURSES_SIGWINCH as appropriate.
*/

#ifdef SIGWINCH
# if defined(NCURSES_VERSION_MAJOR) && (NCURSES_VERSION_MAJOR > 4)
#  define NCURSES_SIGWINCH
# endif
# ifndef KEY_RESIZE
#  define KEY_RESIZE 0777
# endif
#endif

#ifdef XCURSES
# define NOREVERSE
#endif

#include "mmcolor.h"
#include "isoconv.h"

#define MINWIDTH 60
#ifdef VANITY_PLATE
# define MINHINORM 19
# define MINHIEXPERT 17
#else
# define MINHINORM 17
# define MINHIEXPERT 17
#endif

enum direction {UP, DOWN, PGUP, PGDN, HOME, END};
enum statetype {nostate, packetlist, arealist, letterlist, letter,
		letter_help, littlearealist, address, tagwin, ansiwin,
		ansi_help};
enum searchret {False, True, Abort};
enum lineattr {Hidden, Origin, Tearline, Tagline, Sigline, Quoted,
		Normal};

enum {s_fulltext = 1, s_headers, s_arealist, s_pktlist};

#if defined (SIGWINCH) && !defined (XCURSES) && !defined(NCURSES_SIGWINCH)
void sigwinchHandler(int);
#endif

#define TAGLINE_LENGTH 76

/* Include Keypad keys for PDCurses */

#ifdef __PDCURSES__
# define MM_PLUS	'+': case PADPLUS
# define MM_MINUS	'-': case PADMINUS
# define MM_ENTER	'\r': case '\n': case PADENTER
# define MM_SLASH	'/': case PADSLASH
# define MM_UP		KEY_UP: case KEY_A2
# define MM_DOWN	KEY_DOWN: case KEY_C2
# define MM_LEFT	KEY_LEFT: case KEY_B1
# define MM_RIGHT	KEY_RIGHT: case KEY_B3
# define MM_HOME	KEY_HOME: case KEY_A1
# define MM_END		KEY_END: case KEY_LL: case KEY_C1
# define MM_PPAGE	KEY_PPAGE: case KEY_A3
# define MM_NPAGE	KEY_NPAGE: case KEY_C3
# define MM_INS		KEY_IC: case PAD0
# define MM_DEL		KEY_DC: case PADSTOP
#else
# define MM_PLUS	'+'
# define MM_MINUS	'-'
# define MM_ENTER	'\r': case '\n'
# define MM_SLASH	'/'
# define MM_UP		KEY_UP
# define MM_DOWN	KEY_DOWN
# define MM_LEFT	KEY_LEFT
# define MM_RIGHT	KEY_RIGHT
# define MM_HOME	KEY_HOME
# define MM_END		KEY_END: case KEY_LL
# define MM_PPAGE	KEY_PPAGE
# define MM_NPAGE	KEY_NPAGE
# define MM_INS		KEY_IC
# define MM_DEL		KEY_DC
#endif

/* Deal with the latest PDCurses misfeature -- version 2.4, for Windows
   or X, reports shift, control and alt as keys:
*/
#ifdef KEY_SHIFT_R
# define MM_DISCARD	KEY_SHIFT_R: case KEY_SHIFT_L: case KEY_CONTROL_R: \
			case KEY_CONTROL_L: case KEY_ALT_R: case KEY_ALT_L: \
			case ERR
#else
# define MM_DISCARD	ERR
#endif

#define MM_BACKSP	KEY_BACKSPACE: case 8
#define MM_ESC		27
#define MM_F1		KEY_F(1)
#define MM_F2		KEY_F(2)

#ifdef USE_MOUSE
# define MM_MOUSE	KEY_MOUSE
#endif

class ColorClass : public baseconfig
{
	static chtype allcolors[];
	static const char *col_names[], *col_intro[], *col_comments[];
	static const chtype mapped[];

	chtype colorparse(const char *);
	void processOne(int, const char *);
	const char *configLineOut(int);
	const char *findcol(chtype);
	const char *decompose(chtype);
 public:
	void Init();
};

class Win
{
 protected:
	WINDOW *win;
	chtype *buffer, curratt;
 public:
	Win(int, int, int, chtype);
	Win(int, int, int, coltype);
	~Win();
	void init(int, int, int);
	void Clear(chtype);
	void Clear(coltype);
	void put(int, int, chtype);
	void put(int, int, char);
	void put(int, int, const chtype *, int = 0);
	int put(int, int, const char *, int = -1);
	void attrib(chtype);
	void attrib(coltype);
	void horizline(int, int);
	void update();
	void delay_update();
	void wtouch();
	void wscroll(int);
	void cursor_on();
	void cursor_off();
	int keypressed();
	int inkey();
	void boxtitle(coltype, const char *, chtype);
	void clreol(int, int);
#ifdef USE_MOUSE
	int xstart();
	int ystart();
#endif
};

class ShadowedWin : public Win
{
#ifdef USE_SHADOWS
	WINDOW *shadow;
#endif
 public:
	ShadowedWin(int, int, int, coltype, const char * = 0,
		coltype = C_SBACK);
	~ShadowedWin();
	void touch();
	int getstring(int, int, char *, int, coltype, coltype);
};

class InfoWin : public ShadowedWin
{
	Win *info;
 public:
	char *lineBuf;

	InfoWin(int, int, int, coltype, const char * = 0,
		coltype = C_SBACK, int = 3, int = 2);
	~InfoWin();
	void irefresh();
	void touch();
	void oneline(int, chtype);
	void iscrl(int);

#ifdef USE_MOUSE
	int xstartinfo();
	int ystartinfo();
#endif
};

class ListWindow
{
 private:
	bool lynxNav;	//use Lynx-like navigation?
	int oldPos;	//position at last Draw()
	int oldActive;	//active at last Draw()
	int oldHigh;	//location of highlight bar at last Draw()

	void checkPos(int);
	chtype setHighlight(chtype);
 protected:
	InfoWin *list;
  	int list_max_y, list_max_x, top_offset;
	int position;	//the first element in the window
	int active;	//this is the highlited	

	coltype borderCol;

	void Draw();		//refreshes the window
	void DrawOne(int, chtype);
	void DrawOne(int, coltype);
	void DrawAll();
	virtual int NumOfItems() = 0;
	virtual void oneLine(int) = 0;
	virtual searchret oneSearch(int, const char *, int) = 0;
	virtual bool extrakeys(int) = 0;
	virtual void setFilter(const char *) = 0;
 public:
 	ListWindow();
	virtual ~ListWindow();
	void Move(direction);		//scrolloz
	void setActive(int);
	int getActive();
	searchret search(const char *, int);
	bool KeyHandle(int);
	virtual void Delete() = 0;
	virtual void Prev();
	virtual void Next();
};

#ifdef VANITY_PLATE

class Welcome
{
 	ShadowedWin *window;
 public:
 	void MakeActive();
 	void Delete();
};

#endif

class AddressBook : public ListWindow
{
	class Person
	{
	 public:
	 	Person *next;
 		char *name;
 		net_address netmail_addr;
		bool killed;

		Person(const char * = 0, const char * = 0);
		Person(const char *, net_address &);
		~Person();

		void setname(const char *);
		void dump(FILE *);
	};

	Person head, *curr, *highlighted, **people, **living;
	const char *addfname;
	char *filter;
	int NumOfPersons, NumOfActive;
	bool NoEnter, inletter;

	friend int perscomp(const void *, const void *);

  	int NumOfItems();
	void oneLine(int);
	searchret oneSearch(int, const char *, int);
	bool extrakeys(int);
	void setFilter(const char *);

	void Add(const char *, net_address &);
	void GetAddress();
	int HeaderLine(ShadowedWin &, char *, int, int, coltype);
	int Edit(Person &);
	void NewAddress();
	void ChangeAddress();
 	void ReadFile();
 	void DestroyChain();
	void MakeChain();
	void ReChain();
 	void SetLetterThings();
	void WriteFile();
	void kill();
public:
	AddressBook();
	~AddressBook();
	void MakeActive(bool);
	void Delete();
	void Init();
};


class TaglineWindow : public ListWindow
{
	class tagline
	{
	 public:
		tagline(const char * = 0);
		tagline *next;
		char text[TAGLINE_LENGTH + 1];
		bool killed;
	};

	char format[20];

	tagline head, *curr, *highlighted, **taglist, **tagactive;
	const char *tagname;
	char *filter;
  	int NumOfTaglines, NumOfActive;
	bool nodraw, sorted;

	friend int tnamecmp(const void *, const void *);

	void oneLine(int);
	searchret oneSearch(int, const char *, int);
	bool extrakeys(int);
	void setFilter(const char *);

	void kill();
	bool ReadFile();
	void WriteFile(bool);
	void DestroyChain();
	void MakeChain();
	void RandomTagline();
 public:
 	TaglineWindow();
	~TaglineWindow();
	void MakeActive();
	void Delete();
 	void EnterTagline(const char * = 0);
	void EditTagline();
	void Init();
	int NumOfItems();
	const char *getCurrent();
};

class LittleAreaListWindow : public ListWindow
{
	int disp, areanum;

	int NumOfItems();
	void oneLine(int);
	searchret oneSearch(int, const char *, int);
	bool extrakeys(int);
	void setFilter(const char *);
	void Select();
 public:
	void init();
 	void MakeActive();
 	void Delete();
	int getArea();
};

class PacketListWindow : public ListWindow
{
	class oneDir
	{
	 public:
		oneDir *parent;
		char *name;
		int position, active;

		oneDir(const char *, oneDir *);
		~oneDir();
	};

#ifdef VANITY_PLATE
	Welcome welcome;
#endif
	file_list *packetList;
	oneDir *currDir, *origDir;
#ifdef HAS_HOME
	char *home;
#endif
	time_t currTime;
	int noDirs, noFiles;
	bool sorttype;

	int NumOfItems();
	void oneLine(int);
	searchret oneSearch(int, const char *, int);
	bool extrakeys(int);
	void setFilter(const char *);

	void newList();
	bool newDir(const char *);
	void gotoDir();
	void renamePacket();
	void killPacket();
	void MakeActiveCore();
 public:
	PacketListWindow();
	~PacketListWindow();
	void init();
 	void MakeActive();
 	void Delete();
	void Select();
	pktstatus OpenPacket();
	bool back();
};

class AreaListWindow : public ListWindow
{
	char format[40], format2[40];
	bool hasPers, hasSys;

 	int NumOfItems();
	void oneLine(int);
	searchret oneSearch(int, const char *, int);
	bool extrakeys(int);
	void setFilter(const char *);

 public:
	void ResetActive();
 	void MakeActive();
 	void Delete();
	void Select();
	void ReDraw();
	void FirstUnread();
	void Prev();
	void Next();
};

class LetterListWindow : public ListWindow
{
	char format[50], *topline;

	int NumOfItems();
	void oneLine(int);
	searchret oneSearch(int, const char *, int);
	bool extrakeys(int);
	void setFilter(const char *);

	void listSave();
	void setFormat();
	void MakeActiveCore();
 public:
	LetterListWindow();
	void ResetActive();
	void MakeActive();
	void Delete();
	void Select();
	void FirstUnread();
	void Prev();
	void Next();
};

class LetterWindow
{
	class Line
	{
	 public:
		Line *next;
		const char *text;
		unsigned length;
		lineattr attr;

		Line();
		void out(FILE *);
	};

	Win *headbar, *header, *text, *statbar;
	Line **linelist;
	char tagline1[TAGLINE_LENGTH + 1], *To;
	int letter_in_chain;	//0 = no letter in chain
	int position;		//which row is the first in the text window
	int NumOfLines;
	int y;			//height of the window, set by MakeActive
	int beepPers;
	bool rot13, hidden, lynxNav;
	net_address NM;
	time_t lasttime;

	void lineCount();
	void oneLine(int);
	void Move(int);
	char *netAdd(char *);
	int HeaderLine(ShadowedWin &, char *, int, int, coltype);
	int EnterHeader(char *, char *, char *, bool &);
	void QuoteText(FILE *);
	void DestroyChain();
	void setToFrom(char, char *, char *);
	void forward_header(FILE *, const char *, const char *,
		const char *, int, bool);
	void EditLetter(bool);
	bool SplitLetter(int = 0);
	long reconvert(const char *);
	void write_header_to_file(FILE *);
	void write_to_file(FILE *);
	void GetTagline();
	bool Previous();
	void NextDown();
	void MakeChain(int, bool = false);
	void MakeChainFixPos();
	void DrawFlags();
	void UpdateHeader();
 	void DrawHeader();
 	void DrawBody();
	void DrawStat();
	bool EditOriginal();
 public:
	LetterWindow();
	~LetterWindow();

 	void MakeActive(bool);
	void TimeUpdate();
 	void Delete();
	bool Next();
 	void Draw(bool = false);
	void ReDraw();
	bool Save(int);
	void EnterLetter(int, char);
	void StatToggle(int);
	net_address &PickNetAddr();
	void set_Letter_Params(net_address &, const char *);
	void setPos(int);
	int getPos();
	searchret search(const char *);
	void SplitAll(int);
	void KeyHandle(int);
};

class HelpWindow
{
 	Win *menu;
	int midpos, endpos, base, items;

	void newHelpMenu(const char **, const char **, int);
	void h_packetlist();
	void h_arealist();
	void h_letterlist();
	void h_letter(bool);
 public:
	HelpWindow();
	void MakeActive();
	void Delete();
	void baseNext();
	void baseReset();
};

class StringFile
{
	FILE *afile;
	file_header *fromFile;
	letter_body *msgBody, *msgBodyFirst;
	const unsigned char *srcStr, *curpos;
 public:
	//void init(const char *);
	void init(file_header *);
	void init(letter_body *);
	void close();
	unsigned char nextchar();
	void backup(int);
	void reset();
	bool anyleft();
};

class AnsiWindow
{
	class AnsiLine
	{
	 private:
		AnsiLine *prev, *next;
		union {
			chtype *text;
			char *atext;
		};
		unsigned length;
		chtype att;
		bool isasc;
	 public:
		AnsiLine *getprev();
		AnsiLine *getnext(bool = false);

	 	AnsiLine(AnsiLine * = 0);
		~AnsiLine();
		int unpack(chtype *);
		void pack(chtype *, int);
		void show(Win *, int);
		void unpacktext(char *);
		void remapzero(chtype newatt);
	};

	bool colorsused[64];
	char escparm[256];	//temp copy of ESC sequence parameters
	const char *title;
	StringFile source;
	Win *header, *text, *statbar, *animtext;
	AnsiLine *head, *curr, **linelist;
	int position;		//which row is the first in the window
	int NumOfLines;
	int x, y;		//dimensions of the window
	int cpx, cpy, lpy;	//ANSI positions
	int spx, spy;		//stored ANSI positions
	int tlen;		//maximum X reached
	int ccf, ccb, cfl, cbr, crv;	//colors and attributes
	int oldcolorx, oldcolory;
	int baseline;		//base for positions in non-anim mode
	bool anim;		//animate mode?
	bool ansiAbort;
	chtype *chtmp, attrib;	//current attribute
	bool isLatin;
	int atparse;
	
	void oneLine(int);
	void lineCount();
	void DrawBody();
	int getparm();
	void cls();
	void colreset();
	void colorcore();
	void colorset();
	void athandle();
	void escfig();
	void posreset();
	void checkpos();
	void update(unsigned char);
	void ResetChain();
	void MakeChain();
	void DestroyChain();
	void animate();
	void statupdate(const char * = 0);
	void Save();
 public:
	void set(letter_body *, const char *, bool);
	void set(file_header *, const char *, bool);
	void MakeActive();
	void Delete();
	void setPos(int);
	int getPos();
	searchret search(const char *);
	void KeyHandle(int);
};

class Interface
{
#ifdef USE_SHELL
	Shell shell;
#endif
	PacketListWindow packets;
	AddressBook addresses;
	HelpWindow helpwindow;
	LittleAreaListWindow littleareas;

	Win *screen;
	ListWindow *currList;
 	statetype state, prevstate, searchstate;
	const char *searchItem, *cmdpktname;
	file_header *newFiles, **bulletins;
	int Key, searchmode, s_oldpos, width_min, height_min;
	bool unsaved_reply, any_read, addrparm, commandline, abortNow,
		dontSetAsRead, lynxNav;
#ifdef SIGWINCH
	bool resized;

	void sigwinch();
#endif
	void init_colors();
	void oldstate(statetype);
	void helpreset(statetype);
	void newstate(statetype);
	void alive();
	void screen_init();
	const char *pkterrmsg(pktstatus);
	void newpacket();
	void create_reply_packet();
	void save_read();
	int ansiCommon();
	void searchNext();
	void searchSet();
	void KeyHandle();
 public:
	ColorClass colorlist;
	AreaListWindow areas;
	LetterListWindow letters;
	LetterWindow letterwindow;
	TaglineWindow taglines;
	AnsiWindow ansiwindow;

 	Interface();
	~Interface();
	void init();
	void main();
 	int WarningWindow(const char *, const char ** = 0, int = 2);
	int savePrompt(const char *, char *);
	void nonFatalError(const char *);
	void ReportWindow(const char *);
	void changestate(statetype);
	void redraw();
 	bool select();
 	bool back();		//returns true if we have to quit
	statetype active();
	statetype prevactive();
	void addressbook(bool = false);
	bool Tagwin();
	int ansiLoop(letter_body *, const char *, bool);
	int ansiFile(file_header *, const char *, bool);
	int areaMenu();
	void kill_letter();
	void setUnsaved();
	void setUnsavedNoAuto();
	void setAnyRead();
	void setKey(int);
	bool dontRead();
	bool fromCommandLine(const char *);
};

extern mmail mm;
extern Interface *ui;
extern time_t starttime;

#ifdef USE_MOUSE
# ifndef NCURSES_MOUSE_VERSION

typedef struct {
	int x, y;
	unsigned long bstate;
} MEVENT;

# endif

extern MEVENT mouse_event;

void mm_mouse_get();
#endif

#ifdef PDCURSKLUDGE
extern "C" {
int PDC_get_cursor_mode();
int PDC_set_cursor_mode(int, int);
}
extern int curs_start, curs_end;
#endif

#endif
