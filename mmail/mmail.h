/*
 * MultiMail offline mail reader
 * mmail class

 Copyright (c) 1996 Toth Istvan <stoty@vma.bme.hu>
 Copyright (c) 2003 William McBrine <wmcbrine@users.sf.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#ifndef MMAIL_H 
#define MMAIL_H

#include "../config.h"
#include "../mmail/misc.h"
#include "../mmail/resource.h"
#include "../interfac/mysystem.h"

extern "C" {
#include <ctype.h>
#include <string.h>

/* QNX needs strings.h for strcasecmp(); seems harmless on others */

#ifndef USE_STRICMP
# include <strings.h>
#endif
}
#include <stdlib.h>

// Number of the Reply area
#define REPLY_AREA 0

// Area types -- bit f.
#define COLLECTION	1L
#define REPLYAREA	2L
#define ACTIVE		4L
#define ALIAS		8L
#define NETMAIL		0x10L
#define INTERNET	0x20L
#define PUBLIC		0x40L
#define PRIVATE		0x80L
#define LATINCHAR	0x100L
#define ECHOAREA	0x200L
#define PERSONLY	0x400L
#define PERSALL		0x800L
#define SUBKNOWN	0x1000L
#define ADDED		0x2000L
#define DROPPED		0x4000L
#define OFFCONFIG	0x8000L
#define READONLY	0x10000L
#define HASREPLY	0x20000L

// Mail statuses -- bit f.
enum {MS_READ = 1, MS_REPLIED = 2, MS_MARKED = 4, MS_PERSTO = 8,
	MS_PERSFROM = 0x10, MS_SAVED = 0x20};

// For letter_list::sort
enum {LS_SUBJ, LS_MSGNUM, LS_FROM, LS_TO};

enum pktstatus {PKT_OK, UNCOMP_FAIL, PTYPE_UNK, NEW_DIR, PKT_NOFILES,
	PKT_UNFOUND};

class mmail;
class resource;
class file_header;
class file_list;
class area_header;
class area_list;
class letter_body;
class letter_header;
class letter_list;
class specific_driver;
class reply_driver;
class driver_list;
class read_class;

class net_address
{
	char *inetAddr;

	void copy(net_address &);
 public:
	unsigned zone, net, node, point;
	bool isInternet, isSet;

	net_address();
	net_address(net_address &);
	~net_address();
	bool operator==(net_address &);
	net_address &operator=(const char *);
	net_address &operator=(net_address &);
	operator const char *();
};

class mmail
{
 public:
	resource *resourceObject;
	file_list *workList;
	driver_list *driverList;
	area_list *areaList;
	letter_list *letterList;

	mmail();
	~mmail();
	pktstatus selectPacket(const char *);
	void Delete();
	bool saveRead();
	file_header *getFileList();
	file_header **getBulletins();
	bool isLatin();
	bool checkForReplies();
	bool makeReply();
	void deleteReplies();
	void openReply();
	bool getOffConfig();
};	

class file_header
{
	char *name;
	time_t date;
	off_t size;
 public:
	file_header *next;

	file_header(const char *, time_t, off_t);
	~file_header();

	const char *getName() const;
	time_t getDate() const;
	void setDate();
	off_t getSize() const;
};

class file_list
{
	file_header **files, **dirs;
	char *DirName, *filter;
	int noOfFiles, noOfDirs, activeFile;
	bool sorttype, dirlist;

	void cleanup();
	void relist();

	void sort();
	file_header *base() const;
	file_header *base(int) const;
 public:
	file_list(const char *, bool = false, bool = false);
	~file_list();

	void resort();

	int getNoOfDirs() const;
	int getNoOfFiles() const;
	const char *getDirName() const;
	void gotoFile(int);
	char *changeDir(const char * = 0);
	int changeName(const char *);

	const char *getName() const;
	time_t getDate() const;
	void setDate();
	off_t getSize() const;

	const char *getNext(const char *);
	file_header *getNextF(const char *);
	const char *exists(const char *);
	file_header *existsF(const char *);

	void addItem(file_header **, const char *, int &);

	char *expandName(const char *);
	FILE *ftryopen(const char *);
	void kill();
	int nextNumExt(const char *);
	const char *getFilter() const;
	void setFilter(const char *);
};

class area_header
{
	mmail *mm;
	specific_driver *driver;
	const char *shortName, *name, *description, *areaType;
	unsigned long type;
	int noOfLetters, noOfPersonal, noOfReplies, num, maxtolen, maxsublen;
 public:
	area_header(mmail *, int, const char *, const char *, const char *,
		const char *, unsigned long, int, int, int, int);
	inline const char *getShortName() const;
	inline const char *getName() const;
	inline const char *getDescription() const;
	inline const char *getAreaType() const;
	inline const char *getTear();
	inline unsigned long getType() const;
 	inline int getNoOfLetters() const;
	inline int getNoOfUnread();
	inline int getNoOfMarked();
	inline int getNoOfPersonal() const;
	inline bool getUseAlias() const;
	inline bool isCollection() const;
	inline bool isReplyArea() const;
	inline bool isNetmail() const;
	inline bool isInternet() const;
	inline bool isEmail() const;
	inline bool isUsenet() const;
	inline bool isLatin() const;
	inline bool isReadOnly() const;
	inline bool hasTo() const;
	inline bool hasPublic() const;
	inline bool hasPrivate() const;
	inline int maxToLen() const;
	inline int maxSubLen() const;
	inline bool hasOffConfig() const;
	inline void Add();
	inline void Drop();
	inline void addReply();
	inline void killReply();
	bool isActive() const;
};

class area_list
{
	mmail *mm;
	area_header **areaHeader;
	int no, noActive, current, *activeHeader;
	int almode;
	char *filter;
 public:
	area_list(mmail *);
	~area_list();
	bool relist();
	int getRepList();
	void updatePers();

	const char *getShortName() const;
	const char *getName() const;
	const char *getName(int);
	const char *getDescription() const;
	const char *getDescription(int);
	const char *getAreaType() const;
	const char *getTear();
	unsigned long getType() const;
	int getNoOfLetters() const;
	int getNoOfUnread() const;
	int getNoOfMarked() const;
	int getNoOfPersonal() const;
	bool getUseAlias() const;
	bool isCollection() const;
	bool isReplyArea() const;
	bool isEmail() const;
	bool isNetmail() const;
	bool isInternet() const;
	bool isUsenet() const;
	bool isLatin() const;
	bool isLatin(int);
	bool isReadOnly() const;
	bool hasTo() const;
	bool hasPublic() const;
	bool hasPrivate() const;
	int maxToLen() const;
	int maxSubLen() const;
	bool hasOffConfig() const;
	void Add();
	void Drop();

	bool isShortlist() const;
	int getMode() const;
	void setMode(int);
	void getLetterList();
	void enterLetter(int, const char *, const char *, const char *,
			const char *, const char *, int, bool,
			net_address &, const char *, long);
	void killLetter(int, long);
	void refreshArea();
	void gotoArea(int);
	void gotoActive(int);
	int getAreaNo() const;
	int getActive();
	int noOfAreas() const;
	int noOfActive() const;
	int findNetmail() const;
	int findInternet() const;
	bool anyChanged() const;
	const char *getFilter() const;
	void setFilter(const char *);
	const char *filterCheck(const char *);
};

class letter_body
{
	char *text;
	long length, offset;
	bool hidden;
 public:
	letter_body *next;

	letter_body(char *, long, long = 0, bool = false);
	~letter_body();
	char *getText();
	long getLength();
	bool isHidden();
};

class letter_header
{
	driver_list *dl;
	read_class *readO;
	specific_driver *driver;
	char *subject, *to, *from, *date, *msgid, *newsgrps, *follow, *reply;
	long replyTo;
	int LetterID, AreaID;
	bool privat, persfrom, persto;
	int length;
	long msgNum;
	net_address netAddr;
	bool charset, qpenc;
 public:
	letter_header(mmail *, const char *, const char *, const char *,
		const char *, const char *, long, int, long, int, bool,
		int, specific_driver *, net_address &, bool = false,
		const char * = 0, const char * = 0, const char * = 0,
		bool = false);
	~letter_header();

	void changeSubject(const char *);
	void changeTo(const char *);
	void changeFrom(const char *);
	void changeDate(const char *);
	void changeMsgID(const char *);
	void changeNewsgrps(const char *);
	void changeFollow(const char *);
	void changeReplyTo(const char *);

	const char *getSubject() const;
	const char *getTo() const;
	const char *getFrom() const;
	const char *getDate() const;
	const char *getMsgID() const;
	const char *getNewsgrps() const;
	const char *getFollow() const;
	const char *getReply() const;
	net_address &getNetAddr();
	long getReplyTo() const;
	bool getPrivate() const;
	inline specific_driver *getDriver() const;
	inline letter_body *getBody();
	int getLetterID() const;
	int getAreaID() const;
	inline long getMsgNum() const;
	inline int getLength() const;
	inline bool isPersonal() const;
	inline bool isLatin() const;
	bool isQP() const;

	void setLatin(bool);
	void setQP(bool);

	inline bool getRead();
	inline void setRead();
	inline int getStatus();
	inline void setStatus(int);
};

class letter_list
{ 
	mmail *mm;
	driver_list *dl;
	specific_driver *driver;
	read_class *readO;
	letter_header **letterHeader;
	int noOfLetters, noActive, areaNumber, currentLetter;
	int *activeHeader;
	int llmode;
	unsigned long type;
	bool isColl;
	char *filter;

	void init();
	void cleanup();
	void sort();
 public:
	letter_list(mmail *, int, unsigned long);
	~letter_list();
	void relist();
	void resort();
	int getMode() const;
	void setMode(int);

	const char *getSubject() const;
	const char *getTo() const;
	const char *getFrom() const;
	const char *getDate() const;
	const char *getMsgID() const;
	const char *getNewsgrps() const;
	const char *getFollow() const;
	const char *getReply() const;
	letter_body *getBody();
	net_address &getNetAddr();
	long getReplyTo() const;
	long getMsgNum() const;
	int getAreaID() const;
	bool getPrivate() const;
	int getLength() const;
	bool isPersonal() const;
	bool isLatin() const;
	bool isQP() const;

	void setQP(bool);

	int getStatus();
	void setStatus(int);
	bool getRead();
	void setRead();
	
	void rrefresh();
	bool findMsgNum(long);
	bool findReply(int, long);
	int noOfLetter() const;
	int noOfActive() const;
	void gotoLetter(int);
	void gotoActive(int);
	int getCurrent() const;
	int getActive() const;
	const char *getFilter() const;
	void setFilter(const char *);
	const char *filterCheck(const char *);
};

class driver_list
{
	struct driver_struct {
		specific_driver *driver;
		read_class *read;
		int offset;
	} driverList[2];

	int noOfDrivers, attributes;
 public:
	driver_list(mmail *);
	~driver_list();
	void initRead();
	int getNoOfDrivers() const;
	specific_driver *getDriver(int);
	reply_driver *getReplyDriver();
	read_class *getReadObject(specific_driver *);
	int getOffset(specific_driver *);
	bool hasPersonal() const;
};

class read_class
{
 public:
	virtual ~read_class();
	virtual void init() = 0;
	virtual void setRead(int, int, bool) = 0;
	virtual bool getRead(int, int) = 0;
	virtual void setStatus(int, int, int) = 0;
	virtual int getStatus(int, int) = 0;
	virtual int getNoOfUnread(int) = 0;
	virtual int getNoOfMarked(int) = 0;
	virtual bool saveAll() = 0;
};

class main_read_class : public read_class
{
	mmail *mm;
	resource *ro;
	specific_driver *driver;
	int noOfAreas, **readStore, *noOfLetters;
	bool hasPersArea, hasPersNdx;
 public:
	main_read_class(mmail *, specific_driver *);
	~main_read_class();
	void init();
	void setRead(int, int, bool);
	bool getRead(int, int);
	void setStatus(int, int, int);
	int getStatus(int, int);
	int getNoOfUnread(int);
	int getNoOfMarked(int);
	bool saveAll();
	const char *readFilePath(const char *);
};

class reply_read_class: public read_class
{
 public:
	reply_read_class(mmail *, specific_driver *);
	~reply_read_class();
	void init();
	void setRead(int, int, bool);
	bool getRead(int, int);
	void setStatus(int, int, int);
	int getStatus(int, int);
	int getNoOfUnread(int);
	int getNoOfMarked(int);
	bool saveAll();
};	

class specific_driver
{
 public:
	virtual ~specific_driver();
	virtual bool hasPersArea();
	virtual bool isLatin();
	virtual const char *oldFlagsName();
	virtual bool readOldFlags();
	virtual bool saveOldFlags();
	virtual int getNoOfAreas() = 0;
	virtual area_header *getNextArea() = 0;
	virtual void selectArea(int) = 0;
	virtual int getNoOfLetters() = 0;
	virtual void resetLetters() = 0;
	virtual letter_header *getNextLetter() = 0;
	virtual letter_body *getBody(letter_header &) = 0;
	virtual file_header *getFileList() = 0;
	virtual file_header **getBulletins() = 0;
	virtual const char *getTear(int) = 0;
};

class reply_driver : public specific_driver
{
 public:	
	virtual ~reply_driver();
	virtual bool checkForReplies() = 0;
	virtual void init() = 0;
	virtual void enterLetter(letter_header &, const char *, long) = 0;
	virtual void killLetter(int) = 0;
	virtual area_header *refreshArea() = 0;
	virtual bool makeReply() = 0;
	virtual void deleteReplies() = 0;
	virtual bool getOffConfig() = 0;
	virtual bool makeOffConfig() = 0;
};

// Letter sort type flag
extern int lsorttype;

#endif
