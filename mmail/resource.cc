/*
 * MultiMail offline mail reader
 * resource class

 Copyright (c) 1996 Toth Istvan <stoty@vma.bme.hu>
 Copyright (c) 2007 William McBrine <wmcbrine@users.sf.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "mmail.h"
#include "../interfac/error.h"

/* Default filenames. */

#ifdef __MSDOS__
# define DEFEDIT "edit"
# define DEFZIP "pkzip -#"
# define DEFUNZIP "pkunzip -# -o"
# define DEFLHA "lha a /m"
# define DEFUNLHA "lha e"
#else
# ifdef __WIN32__
#  define DEFEDIT "start /w notepad"
# else
#  ifdef __OS2__
#   define DEFEDIT "tedit"
#  else
#   define DEFEDIT "vi"
#  endif
# endif
# define DEFZIP "zip -jkq"
# define DEFUNZIP "unzip -joLq"
# define DEFLHA "lha af"
# define DEFUNLHA "lha efi"
#endif

#define DEFARJ "arj a -e"
#define DEFUNARJ "arj e"
#define DEFRAR "rar u -ep -inul"
#define DEFUNRAR "rar e -cl -o+ -inul"
#define DEFTAR "tar zcf"
#define DEFUNTAR "tar zxf"

#define DEFNONE "xxcompress"
#define DEFUNNONE "xxuncompress"

#ifdef DOSNAMES
# define RCNAME "mmail.rc"
# define ADDRBOOK "address.bk"
#else
# define RCNAME ".mmailrc"
# define ADDRBOOK "addressbook"
#endif

// ================
// baseconfig class
// ================

baseconfig::~baseconfig()
{
}

bool baseconfig::parseConfig(const char *configFileName)
{
	FILE *configFile;
	char buffer[256], *pos, *resName, *resValue;
	int vermajor = 0, verminor = 0;

	configFile = fopen(configFileName, "rt");
	if (configFile) {
	    while (myfgets(buffer, sizeof buffer, configFile)) {
		if ((buffer[0] != '#') && (buffer[0] != '\n')) {
			pos = buffer;

			//leading spaces
			while (*pos == ' ' || *pos == '\t')
				pos++;

			//skip "bw" -- for backwards compatiblity
			if (*pos == 'b' && pos[1] == 'w')
				pos += 2;

			//resName
			resName = pos;
			while (*pos != ':' && *pos != '=' &&
				*pos != ' ' && *pos != '\t' && *pos)
					pos++;
			if (*pos)
				*pos++ = '\0';

			//chars between strings
			while (*pos == ' ' || *pos == '\t' ||
				*pos == ':' || *pos == '=')
					pos++;

			//resValue
			resValue = pos;
			while (*pos != '\n' && *pos)
				pos++;
			*pos = '\0';

			if (!strncasecmp("ver", resName, 3))
				sscanf(resValue, "%d.%d", &vermajor,
					&verminor);
			else
				processOneByName(resName, resValue);

		}
	    }
	    fclose(configFile);
	}

	// Does the config file need updating?
	return (vermajor < MM_MAJOR) || ((vermajor == MM_MAJOR) &&
		(verminor < MM_MINOR));
}

void baseconfig::newConfig(const char *configname)
{
	FILE *fd;
	const char **p;

	printf("Updating %s...\n", configname);

	fd = fopen(configname, "wt");
	if (fd) {
		for (p = intro; *p; p++)
			fprintf(fd, "# %s\n", *p);

		fprintf(fd, "\nVersion: " MM_VERNUM "\n");

		for (int x = 0; x < configItemNum; x++) {
			if (comments[x])
				fprintf(fd, "\n# %s\n", comments[x]);
			fprintf(fd, "%s: %s\n", names[x],
				configLineOut(x));
		}
		fclose(fd);
	} else
		pauseError("Error writing config file");
}

void baseconfig::processOneByName(const char *resName, const char *resValue)
{
	int c;
	for (c = 0; c < configItemNum; c++)
		if (!strcasecmp(names[c], resName)) {
			processOne(c, resValue);
			break;
		}
	if (c == configItemNum)
		printf("Unrecognized keyword: %s\n",
			resName);
}

// ==============
// resource class
// ==============

const int startUpLen =
 50
#ifdef USE_SPAWNO
 + 1
#endif
#ifdef HAS_TRANS
 + 1
#endif
 ;

const char *resource::rc_names[startUpLen] =
{
	"UserName", "InetAddr", "QuoteHead", "InetQuote",
	"mmHomeDir", "TempDir", "signature", "editor",
	"PacketDir", "ReplyDir", "SaveDir", "AddressBook", "TaglineFile",
	"ColorFile", "UseColors",
#ifdef HAS_TRANS
	"Transparency",
#endif
	"BackFill",
	"arjUncompressCommand", "zipUncompressCommand",
	"lhaUncompressCommand", "rarUncompressCommand",
	"tarUncompressCommand", "unknownUncompressCommand",
	"arjCompressCommand", "zipCompressCommand", "lhaCompressCommand",
	"rarCompressCommand", "tarCompressCommand",
	"unknownCompressCommand", "PacketSort", "AreaMode", "LetterSort",
	"LetterMode", "ClockMode", "Charset", "UseTaglines",
	"AutoSaveReplies", "StripSoftCR", "BeepOnPers", "UseLynxNav",
	"ReOnReplies", "QuoteWrapCols", "MaxLines", "outCharset",
	"UseQPMailHead", "UseQPNewsHead", "UseQPMail", "UseQPNews",
	"ExpertMode", "IgnoreNDX", "Mouse"
#ifdef USE_SPAWNO
	, "swapOut"
#endif
};

const char *resource::rc_intro[] = {
 "-----------------------",
 MM_NAME " configuration",
 "-----------------------",
 "",
 "Any of these keywords may be omitted, in which case the default values",
 "(shown here) will be used.",
 "",
 "If you change either of the base directories, all the subsequent paths",
 "will be changed, unless they're overriden in the individual settings.",
 "",
 "Please see the man page for a more thorough explanation of these options.",
 0
};

const char *resource::rc_comments[startUpLen] = {
 "Your name, as you want it to appear on replies (used mainly in SOUP)",
 "Your Internet email address (used only in SOUP replies)",
 "Quote header for replies (non-Internet)",
 "Quote header for Internet email and Usenet replies",
 "Base directories (derived from $HOME or $MMAIL)", 0,
 "Signature (file) that should be appended to each message. (Not used\n"
 "# unless specified here.)",
 "Editor for replies = $EDITOR; or if not defined, " DEFEDIT,
 MM_NAME " will look for packets here",
 "Reply packets go here",
 "Saved messages go in this directory, by default",
 "Full paths to the address book, tagline and color specification files",
	0, 0,
 "Color or monochrome? (Mono mode uses the default colors)",
#ifdef HAS_TRANS
 "Make black backgrounds transparent? (Only works with ncurses)",
#endif
 "Fill background with checkerboard pattern (ACS_BOARD)?",
 "Decompression commands (must include an option to junk/discard paths!)",
	0, 0, 0, 0, 0,
 "Compression commands (must include an option to junk/discard paths!)",
	0, 0, 0, 0, 0,
 "Default sort for packet list: by Name or Time (most recent first)",
 "Default mode for area list: All, Subscribed, or Active",
 "Default sort for letter list: by Subject, Number, From or To",
 "Default mode for letter list: All or Unread",
 "Clock in letter window: Off, Time (of day), or Elapsed (since startup)",
 "Console character set: CP437 (IBM PC) or Latin-1 (ISO-8859-1)",
 "Prompt to add taglines to replies?",
 "Save replies after editing without prompting?",
 "Strip \"soft carriage returns\" (char 141) from messages?",
 "Beep when a personal message is opened in the letter window?",
 "Use Lynx-like navigation (right arrow selects, left backs out)?",
 "Add \"Re: \" prefix on Subject of replies? (Note that it will be added\n"
 "# in Internet email and Usenet areas regardless of this setting.)",
 "Wrap quoted text at this column width (including quote marks)",
 "Maximum lines per part for reply split (see docs)",
 "8-bit character set for SOUP packets (see docs)",
 "Quoted-printable options for outgoing messages (see docs)",
	0, 0, 0,
 "Supress help messages (use more of the screen for content)",
 "For QWK only: Generate indexes from MESSAGES.DAT instead of *.NDX",
 "Allow use of the mouse?"
#ifdef USE_SPAWNO
 , "Attempt to swap MultiMail out of conventional memory when shelling"
#endif
};

const int resource::startUp[startUpLen] =
{
	UserName, InetAddr, QuoteHead, InetQuote, mmHomeDir, TempDir,
	sigFile, editor, PacketDir, ReplyDir, SaveDir, AddressFile,
	TaglineFile, ColorFile, UseColors,
#ifdef HAS_TRANS
	Transparency,
#endif
	BackFill,
	arjUncompressCommand,
	zipUncompressCommand, lhaUncompressCommand, rarUncompressCommand,
	tarUncompressCommand, unknownUncompressCommand,
	arjCompressCommand, zipCompressCommand, lhaCompressCommand,
	rarCompressCommand, tarCompressCommand, unknownCompressCommand,
	PacketSort, AreaMode, LetterSort, LetterMode, ClockMode, Charset,
	UseTaglines, AutoSaveReplies, StripSoftCR, BeepOnPers, UseLynxNav,
	ReOnReplies, QuoteWrapCols, MaxLines, outCharset, UseQPMailHead,
	UseQPNewsHead, UseQPMail, UseQPNews, ExpertMode, IgnoreNDX, Mouse
#ifdef USE_SPAWNO
	, swapOut
#endif
};

const int resource::defInt[] =
{
	1,	// PacketSort == by time
	1, 	// AreaMode == subscribed
	0,	// LetterSort == by subject
	1,	// LetterMode == unread
#ifdef DOSCHARS
	0,	// Charset == CP437
#else
	1,	// Charset == Latin-1
#endif
	1,	// UseTaglines == Yes
	1,	// AutoSaveReplies == Yes
	0,	// StripSoftCR == No
	0,	// BeepOnPers == No
	1,	// UseLynxNav == Yes
	1,	// ReOnReplies == Yes
	78,	// QuoteWrapCols
	0,	// MaxLines == disabled
	1,	// UseQPMailHead == Yes
	1,	// UseQPNewsHead == Yes
	1,	// UseQPMail == Yes
	0,	// UseQPNews == No
	0,	// ExpertMode == No
	0,	// IgnoreNDX = No
	1,	// Mouse = Yes
#ifdef USE_SPAWNO
	1,	// swapOut == Yes
#endif
	1,	// UseColors == Yes
#ifdef HAS_TRANS
	0,	// Transparency == No
#endif
	1,	// BackFill == Yes
	1	// ClockMode == Time
};

resource::resource()
{
	const char *greeting =
		"\nWelcome to " MM_NAME " v" MM_VERNUM "!\n\n"
		"A new or updated " RCNAME " has been written. "
		"If you continue now, " MM_NAME " will\nuse the default "
		"values for any new keywords. (Existing keywords have been "
		"\npreserved.) If you wish to edit your " RCNAME " first, "
		"say 'Y' at the prompt.\n\nEdit " RCNAME " now? (y/n) ";

	names = rc_names;
	intro = rc_intro;
	comments = rc_comments;
	configItemNum = startUpLen;

	int c;
	for (c = 0; c < noOfStrings; c++)
		resourceData[c] = 0;
	for (c = noOfStrings; c < noOfResources; c++) {
		int d = c - noOfStrings;
		resourceInt[d] = defInt[d];
	}
	set(outCharset, "iso-8859-1");

	initinit();
	homeInit();
	mmHomeInit();

	char *configFileName = fullpath(resourceData[homeDir], RCNAME);

	if (parseConfig(configFileName)) {
		newConfig(configFileName);
		printf(greeting);
		char inp = fgetc(stdin);

		if (toupper(inp) == 'Y') {
			mysystem2(resourceData[editor], configFileName);
			parseConfig(configFileName);
		}
	}

	delete[] configFileName;

	if (!verifyPaths())
		fatalError("Unable to access data directories");

	resourceData[BaseDir] = mytmpdir(resourceData[TempDir]);
	bool tmpok = checkPath(resourceData[BaseDir], false);
	if (!tmpok)
		fatalError("Unable to create temp directory");
	subPath(WorkDir, "work");
	subPath(UpWorkDir, "upwork");
}

resource::~resource()
{
	clearDirectory(resourceData[WorkDir]);
	clearDirectory(resourceData[UpWorkDir]);
	mychdir(resourceData[BaseDir]);
	myrmdir(resourceData[WorkDir]);
	myrmdir(resourceData[UpWorkDir]);
	clearDirectory(resourceData[BaseDir]);
	mychdir(resourceData[TempDir]);
	myrmdir(resourceData[BaseDir]);
	for (int c = 0; c < noOfStrings; c++)
		delete[] resourceData[c];
}

bool resource::checkPath(const char *onepath, bool show)
{
	if (mychdir(onepath)) {
		if (show)
			printf("Creating %s...\n", onepath);
		if (mymkdir(onepath))
			return false;
	}
	return true;
}

bool resource::verifyPaths()
{
	if (checkPath(resourceData[mmHomeDir], true))
	    if (checkPath(resourceData[PacketDir], true))
		if (checkPath(resourceData[ReplyDir], true))
		    if (checkPath(resourceData[SaveDir], true))
			return true;
	return false;
}

void resource::processOne(int c, const char *resValue)
{
	if (*resValue) {
		c = startUp[c];
		if (c < noOfStrings) {
			// Canonized for the benefit of the Win32 version:
			set_noalloc(c, (c >= noOfRaw) ?
				canonize(fixPath(resValue)) :
				strdupplus(resValue));
			if (mmHomeDir == c)
				mmHomeInit();
		} else {
			int x = 0;
			char r = toupper(*resValue);

			switch (c) {
			case PacketSort:
				x = (r == 'T');
				break;
			case AreaMode:
				x = (r == 'S');
				if (!x) {
					r = toupper(resValue[1]);
					if (r == 'C')
						x = 2;
				}
				break;
			case LetterSort:
				switch (r) {
				case 'N':
					x = 1;
					break;
				case 'F':
					x = 2;
					break;
				case 'T':
					x = 3;
				}
				break;
			case LetterMode:
				x = (r == 'U');
				break;
			case ClockMode:
				switch (r) {
				case 'O':
					x = 0;
					break;
				case 'T':
					x = 1;
					break;
				case 'E':
					x = 2;
				}
				break;
			case Charset:
				x = (r == 'L');
				break;
			case QuoteWrapCols:
			case MaxLines:
				sscanf(resValue, "%d", &x);
				break;
			default:
				x = (r == 'Y');
			}

			set(c, x);
		}
	}
}

const char *resource::configLineOut(int x)
{
	static const char *pktopt[] = {"Name", "Time"},
		*areaopt[] = {"All", "Subscribed", "Active"},
		*lttopt1[] = {"Subject", "Number", "From", "To"},
		*lttopt2[] = {"All", "Unread"},
		*clockopt[] = {"Off", "Time", "Elapsed"},
		*charopt[] = {"CP437", "Latin-1"},
		*stdopt[] = {"No", "Yes"};

	x = startUp[x];

	if ((x == MaxLines) || (x == QuoteWrapCols)) {
		static char value[8];
		sprintf(value, "%d", getInt(x));
		return value;
	} else
		return (x < noOfStrings) ? get(x) :
			((x == PacketSort) ? pktopt :
			((x == AreaMode) ? areaopt :
			((x == LetterSort) ? lttopt1 :
			((x == LetterMode) ? lttopt2 :
			((x == ClockMode) ? clockopt :
			((x == Charset) ? charopt :
			stdopt))))))[getInt(x)];
}

const char *resource::get(int ID) const
{
	if (ID >= noOfStrings)
		fatalError("String resource out of range");
	return resourceData[ID];
}

int resource::getInt(int ID) const
{
	if (ID < noOfStrings)
		fatalError("Integer resource out of range");
	ID -= noOfStrings;
	return resourceInt[ID];
}

void resource::set(int ID, const char *newValue)
{
	if (ID >= noOfStrings)
		fatalError("String resource out of range");
	delete[] resourceData[ID];
	resourceData[ID] = strdupplus(newValue);
}

void resource::set_noalloc(int ID, char *newValue)
{
	if (ID >= noOfStrings)
		fatalError("String resource out of range");
	delete[] resourceData[ID];
	resourceData[ID] = newValue;
}

void resource::set(int ID, int newValue)
{
	if (ID < noOfStrings)
		fatalError("Integer resource out of range");
	ID -= noOfStrings;
	resourceInt[ID] = newValue;
}

// --------------------------------------------------------------------
// The resource initializer functions
// --------------------------------------------------------------------

void resource::homeInit()
{
	bool usingHOME = false;

	const char *envhome = getenv("MMAIL");
	if (!envhome) {
		envhome = getenv("HOME");
		if (envhome)
			usingHOME = true;
		else
			envhome = error.getOrigDir();
	}

	set_noalloc(homeDir, canonize(fixPath(envhome)));

	if (usingHOME)
		set_noalloc(mmHomeDir,
			canonize(fullpath(resourceData[homeDir], "mmail")));
	else
		set(mmHomeDir, resourceData[homeDir]);
}

void resource::mmEachInit(int index, const char *dirname)
{
	set_noalloc(index, canonize(fullpath(resourceData[mmHomeDir],
		dirname)));
}

void resource::subPath(int index, const char *dirname)
{
	char *tmp = fullpath(resourceData[BaseDir], dirname);
	set_noalloc(index, tmp);
	if (!checkPath(tmp, 0))
		fatalError("tmp Dir could not be created");
}

void resource::initinit()
{
	set(arjUncompressCommand, DEFUNARJ);
	set(zipUncompressCommand, DEFUNZIP);
	set(lhaUncompressCommand, DEFUNLHA);
	set(rarUncompressCommand, DEFUNRAR);
	set(tarUncompressCommand, DEFUNTAR);
	set(unknownUncompressCommand, DEFUNNONE);
	set(arjCompressCommand, DEFARJ);
	set(zipCompressCommand, DEFZIP);
	set(lhaCompressCommand, DEFLHA);
	set(rarCompressCommand, DEFRAR);
	set(tarCompressCommand, DEFTAR);
	set(unknownCompressCommand, DEFNONE);

	set(UncompressCommand, DEFUNZIP);
	set(CompressCommand, DEFZIP);

	set(sigFile, "");
	set(UserName, "");
	set(InetAddr, "");
	set(QuoteHead, "-=> %f wrote to %t <=-");
	set(InetQuote, "On %d, %f wrote:");

	char *p = getenv("EDITOR");
	set(editor, (p ? p : DEFEDIT));
}

void resource::mmHomeInit()
{
	set(TempDir, resourceData[mmHomeDir]);

	mmEachInit(PacketDir, "down");
	mmEachInit(ReplyDir, "up");
	mmEachInit(SaveDir, "save");
	mmEachInit(AddressFile, ADDRBOOK);
	mmEachInit(TaglineFile, "taglines");
	mmEachInit(ColorFile, "colors");
}
