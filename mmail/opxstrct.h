/* OPX packet structures, in C
   Reverse engineered by William McBrine <wmcbrine@users.sourceforge.net>
   Placed in the Public Domain

   Version 1.2 of this document, Dec. 28, 2000
    -- Separated Fido structure from MAIL.DAT header
    -- Updated email address, revised commentary
   Version 1.1 of this document (unreleased), Feb. 14, 2000
    -- Modified description of line endings
    -- DUSRCFG.DAT and EXTAREAS.DAT, contributed by Armando Ramos
    -- MAIL.FDX info
    -- A few additional flags; realignment of brdRec
    -- More details on <BBSID>.ID
    -- Changed string definition
   Version 1.0 of this document, Oct. 27, 1999
    -- First public documentation of the OPX format

   This is still not a complete specification. Although adequate for a
   reader, it should not be used as the basis for a door.

   BRDINFO.DAT contains the list of areas, along with things like sysop
   and BBS names; MAIL.DAT holds the actual messages. (These correspond
   roughly to CONTROL.DAT and MESSAGES.DAT, respectively, in QWK.) Each
   packet also has two index files, MAIL.FDX and MAIL.IDX. The .IDX file
   appears redundant, and is not documented here. The .FDX file is needed
   to ensure correct handling of long messages (> 64K), and is also used
   to store read markers. The new files list is NEWFILES.TXT, and a
   variety of bulletin files may be present. Offline config is handled
   through the optional files DUSRCFG.DAT (from the door) and RUSRCFG.DAT
   (from the reader).

   The structures were originally designed for Borland Pascal. Here's a
   macro to help define the strings in C terms:
*/

#define pstring(y,x) unsigned char y[x + 1]

/* Note that in version 1.0, this was instead defined as:

    #define pstring(y,x) struct {unsigned char len; char data[x];} y

   But I found that on some systems, the structs were being padded for
   alignment.

   Here are macros for little-endian shorts (16-bit) and longs (32-bit) --
   similar to the tWORD and tDWORD defintions in the Blue Wave specs,
   except that I don't draw a distinction between signed and unsigned, and
   I use only the portable (byte-by-byte) definition:
*/

typedef unsigned char pshort[2];
typedef unsigned char plong[4];
typedef unsigned char pbyte;

/* ### Fido Message Header ### */

/* This is used both in MAIL.DAT, and in replies. It replaces the
   "repHead" struct used in earlier versions of this document, and changes
   the "msgHead" struct.

   Based on Fido packet specifications, this struct uses null-terminated 
   (C-style) strings instead of the BP strings used elsewhere.
*/

#define FIDO_HEAD_SIZE 190

typedef struct {
	char from[36];		/* From: (null-terminated string) */
	char to[36];		/* To: */
	char subject[72];	/* Subject: */
	char date[20];		/* Date in ASCII (not authoritative) */
	pshort dest_zone;	/* Fido zone number of destination -- in
				   replies, set to 0 for non-netmail */
	pshort dest_node;	/* Node of dest. */
	pshort orig_node;	/* Node number of originating system */
	pshort orig_zone;	/* Zone of orig. */
	pshort orig_net;	/* Net of orig. */
	pshort dest_net;	/* Net of dest. */
	plong date_written;	/* Date in packed MS-DOS format */
	plong date_arrived;	/* Date the message arrived on the BBS, in
				   packed MS-DOS format (meaningless in
				   replies) */
	pshort reply;		/* Number of message that this replies to,
				   if applicable */
	pshort attr;		/* Attributes */
	pshort up;		/* Number of message that replies to this
				   one, if applicable (meaningless in
				   replies) */
} fidoHead;

/* "attr" is a bitfield with the following values: */

#define OPX_PRIVATE	1	/* Private message */
#define OPX_CRASH	2	/* Fido crashmail */
#define OPX_RECEIVED	4	/* Read by addressee */
#define OPX_SENT	8
#define OPX_FATTACH	16
#define OPX_ORPHAN	64
#define OPX_KILL	128
#define OPX_LOCAL	256	/* Some readers set this on every reply */
#define OPX_HOLD	512
#define OPX_FREQ	2048
#define OPX_RREQ	4096
#define OPX_RECEIPT	8192
#define OPX_FUREQ	32768

/* "Packed MS-DOS format" dates are the format used by MS-DOS in some of
   its time/date routines, and in the FAT file system. They can cover
   dates from 1980 through 2107 -- better than a signed 32-bit time_t, but
   still limited. The ASCII date field is deprecated.

   bits 00-04 = day of month
   bits 05-08 = month
   bits 09-15 = year - 1980

   bits 16-20 = second / 2
   bits 21-26 = minute
   bits 27-31 = hour
*/

/* ### BRDINFO.DAT structures ### */

/* The Header */

/* To ensure correct operation where alignment padding is used, when 
   reading from or writing to disk, use the _SIZE defines given here
   rather than "sizeof":
*/
#define BRD_HEAD_SIZE 743

typedef struct {
	char unknown1[15];
	pstring(doorid,20);	/* ID of the door that made this */
	pstring(bbsid,8);	/* Like BBSID in QWK; used for replies */
	pstring(bbsname,60);	/* BBS name */
	pstring(sysopname,50);	/* Sysop's name */
	char unknown2[81];
	pstring(zone,6);	/* Fidonet zone number of BBS, in ASCII */
	pstring(net,6);		/* Net number */
	pstring(node,6);	/* Node number */
	char unknown3[252];
	pstring(doorver,10);	/* Version number of door */
	char unknown4[2];
	pstring(phoneno,26);	/* Phone number of BBS */
	char unknown5[4];
	pstring(bbstype,123);	/* BBS software name and version -- not
				   the right length, I'm sure */
	pstring(username,35);	/* User's name */
	char unknown6[21];
	pshort numofareas;	/* Number of conferences */
	char unknown7[4];
	pbyte readerfiles;	/* Number of readerfiles */
} brdHeader;

/* This is followed by a 13-byte record (a Pascal string[12]) for each
   readerfile, as specified in brdHeader.readerfiles. Then there's a
   single byte before the board records begin, which is an obsolete area
   counter. Note that in version 1.0 of this document, I chose to ignore
   this and assume that the board record started one byte earlier.
*/

/* Each Area */

#define BRD_REC_SIZE 86

typedef struct {
	pshort acclevel;	/* Access level */
	pbyte conflow;		/* Low byte of conf. number (obsolete) */
	pstring(name,70);	/* Name of conference on BBS */
	pshort confnum;		/* Number of conference on BBS */
	char unknown2[3];
	pshort attrib;		/* Area attributes (bitflags) */
	char unknown3[2];
	pbyte attrib2;		/* More area attributes */
	pbyte scanned;		/* Subscribed flag -- not reliable */
	pbyte oldattrib;	/* Low byte of attrib (obsolete) */
} brdRec;

/* Some of the flags in attrib appear to be: */

#define OPX_NETMAIL	1
#define OPX_PRIVONLY	4
#define OPX_PUBONLY	8

/* And in attrib2: */

#define OPX_INTERNET	64
#define OPX_USENET	128

/* After all the areas comes some extra data which I'm ignoring for now.
   It appears to be a list of the message number, conference number, and
   attribute (?) for each message in the packet.
*/

/* ### EXTAREAS.DAT ### */

/* Some packets have extra area data in the file EXTAREAS.DAT. This file 
   consists entirely of brdRec records, as in BRDINFO.DAT. The procedure 
   for handing area data when this file is present should be as follows:

     X = brdHeader.numofareas
     Y = Length of file "EXTAREAS.DAT" / Structure length of brdRec (86)
     Z = X - Y
     Read Z amount of area info from "BRDINFO.DAT"
     Read Y amount of area info from "EXTAREAS.DAT"
*/

/* ### MAIL.DAT structures ### */

/* Each message consists of the header, in a fixed format shown below, 
   followed by the message text, whose length is specified in the header. 
   Messages for all areas are concatenated.

   The first 14 bytes of the header are specific to OPX; the remainder is
   based on Fidonet packet structures. The length field specifies the
   length of the entire message, including the classic Fido header, but
   NOT including the OPX-specific part of the header (those first 14 bytes).

   Since the header size is fixed (AFAIK), a more useful interpretation of 
   the length field might be the length of the text plus 0xBE bytes. Also, 
   because the field is only a 16-bit integer, it will be invalid if a 
   message longer than 64k is packed.
*/

/* OPX Message Header */

#define MSG_HEAD_SIZE 204

typedef struct {
	pshort msgnum;		/* Message number on BBS */
	pshort confnum;		/* Conference number */
	pshort length;		/* Length of text + Fido header (0xBE) */
	char unknown1;
        char msgtype;           /* 'D' = Direct (personal),
                                   'K' = Keyword, or ' ' */
	char unknown2[6];

	fidoHead f;		/* Classic Fido header */
} msgHead;

/* The message text consists of lines delimited by LF, CRLF, CR, or even a
   misused "soft CR" character (0x8D). There's no consistency, and I'm not
   sure whether this covers all the possible forms. Various Fido-style
   "hidden lines" may be present in the text, including "INETORIG
   <address>" on Internet email.
*/

/* ### MAIL.FDX structures ### */

/* Basically one header, and then one record per message; but it's a
   little trickier than that.
*/

/* The Header */

#define FDX_HEAD_SIZE 25

typedef struct {
	pshort RowsInPage;	/* Normally the total number of messages */
	pshort ColsInPage;	/* Always 1, in MAIL.FDX */
	pshort PagesDown;
	pshort PagesAcross; 
	pshort ElSize;		/* Size of element; i.e., sizeof(fdxRec) */
	pshort PageSize;	/* RowsInPage * ColsInPage * ElSize */
	pshort PageCount;	/* Normally 1, but see below */
	plong NextAvail;	/* Next "page" = Total file size, here */
	char ID[7];		/* Always "\006VARRAY" */
} fdxHeader;

/* This is followed by a table of pointers to pages; each is a plong.
   You're supposed to use this by looping from 1 to PageCount, reading
   each entry from the pointer table, then seeking to that position and
   reading RowsInPage records for each one.

   In practice, PageCount seems always to be 1, while RowsInPage is
   equivalent to the total number of messages in the packet. But I can't
   guarantee this. If more than one page were used, the RowsInPage value
   would become tricky, since only one value is used for all pages; if
   fdxRec records were split over multiple pages, the same number would
   have to be assigned to each page, as there seems to be no provision for
   having one page shorter than another. On the other hand, a second page
   would seem to be required for any packet with more than 5957 messages.
   (Perhaps such a large packet is simply not allowed?)

   PagesDown and PagesAcross also seem always to be 1.
*/

/* Each Message */

#define FDX_REC_SIZE 11

typedef struct {
	pshort confnum;		/* Area number */
	pshort msgnum;		/* Message number */
	char msgtype;		/* 'D' = Direct (personal),
				   'K' = Keyword, or ' ' */
	pbyte flags;		/* Read, replied, etc. */
	pbyte marks;		/* Marked, etc. */
	plong offset;		/* Start of message in MAIL.DAT */
} fdxRec;

/* Although these records contain the conference and message numbers
   themselves, they should be stored in the same order as the messages in
   MAIL.DAT. Message lengths can be calculated by subtracting the offset 
   field of an fdxRec from that of the next one.
*/

/* "flags" is a bitfield with the following values: */

#define FDX_READ	0x01  /* Set when read by reader */
#define FDX_REPLIED	0x02  /* Set if a reply exists for this message */
#define FDX_SEARCH	0x04  /* Set if message is a search hit */

/* "marks" is a bitfield with the following values: */

#define FDX_KILL	0x01
#define FDX_FILE	0x02
#define FDX_PRINT	0x04
#define FDX_READ2	0x08
#define FDX_URGENT	0x10
#define FDX_TAGGED	0x20
#define FDX_DOS		0x40

/* ### DUSRCFG.DAT structures ### */

/* Only some packets have this. It indicates which areas are subscribed
   to, and enables offline config. There's one header record, followed by 
   one record for each area.
*/

/* The Header */

#define OCFG_HEAD_SIZE 120

typedef struct {
	plong unknown1;
	pbyte flags1, flags2, flags3, flags4;	/* Bit-mapped options */
	pshort numofareas;	/* Total number of areas */
	pbyte helplevel;	/* Door Menu Help Level:
				   0 = NOVICE, 1 = EXPERT, 2 = GXPRESS */
	pbyte flags5;
	char unknown2[108];
} ocfgHeader;

/* Values for the flags are as follows: */

/* Bit-masks for flags1 */
#define OCFG_GRAPHICS		1	/* Use Door Ansi Graphics */
#define OCFG_HOTKEYS		2	/* Use Door Menu Hot Keys */
#define OCFG_GROUPMAIL		4	/* Accept Group Mail */
#define OCFG_MY_MAIL		8	/* Scan Your Own Mail */

/* Bit-masks for flags2 */
#define OCFG_NEWFILES		4	/* Scan for New Files */
#define OCFG_MAIL_ONLY		64	/* Show Areas with Mail Only */

/* Bit-masks for flags3 */
#define OCFG_VACATION		4	/* Use Vacation Saver Mail */
#define OCFG_BULLETIN		8	/* Scan For News Bulletins */
#define OCFG_IBM_CHAR		16	/* Use IBM Characters */
#define OCFG_REP_RECPT		32	/* Send REPLY Receipt */
#define OCFG_SEND_QWK_NDX	64	/* QWK: Send NDX index files */
#define OCFG_STRIP_QWK_KLUDGE	128	/* QWK: Strip Kludges Lines */

/* Bit-masks for flags4 */
#define OCFG_AUTO_XPRESS	1	/* Use Auto Xpress Starter */
#define OCFG_SEL_AREAS_ONLY	8	/* Send Selected Areas Only */
#define OCFG_PACKET_EXT		64	/* Use Packet Extension */
#define OCFG_USE_FLEX		128	/* Use Flex Assistant */

/* Bit-masks for flags5 */
#define OCFG_QWK_WRAP		1	/* QWK: Perform Word Wrapping */
#define OCFG_SKIP_RIP		2	/* Skip RIP Graphics */
#define OCFG_SEARCH_MSG		4	/* Search Message Body */
#define OCFG_COLORED_NEW_FILES	8	/* Colorized New Files List */

/* Each Area */

#define OCFG_REC_SIZE 3

typedef struct {
	pshort confnum;
	pbyte scanned;		/* 1 = Scan, 0 = Don't scan */
} ocfgRec;

/* After all the specified area records, some extra records sometimes
   appear at the end of the file; I don't know their purpose, if any.
*/

/* ### REPLIES ### */

/* Reply packets are named in the form <BBSID>.REP. Inside each packet is
   one file per message, in a form very close to that used in MAIL.DAT,
   with a header followed by the text; along with a text file named
   <BBSID>.ID. Where offline config is supported, RUSRCFG.DAT may also be
   included.

   The header is just the classic Fido header ("fidoHead"). This omits one
   crucial piece of information: the conference number! Instead, it's
   encoded in the filename of the message. The names come in two forms.
   Messages which are not replies to existing messages are named as:

   !N<x>.<yyy>

   where <x> is the serial number of the message (this is ignored, AFAIK),
   in decimal, with no leading zeroes; and <yyy> is the destination area,
   in _base 36_, with leading zeroes if needed to pad it out to three
   characters. Messages which are replies are named:

   !R<x>.<yyy>

   In this case, <x> is the number of the message to which this is a
   reply. (This is redundant with the reply field in the header.) <x> is
   again a decimal number with no leading zeroes, and <yyy> is the same in
   both forms. Note that this system implies there can only be one reply
   to a given message in a single reply packet; some readers enforce this.
   (The limit can be circumvented by not using the R form for subsequent
   replies.)

   After the header comes the message text. The length is determined by
   the file length, since there's no length field in the header. The text
   is CRLF-delimited paragraphs. I'm not sure whether the lines should be
   forced to wrap (like QWK) or not (like Blue Wave); when I tested this,
   I got inconsistent results.

   Several Fido-style "hidden lines" (beginning with ctrl-A) may be
   present; one reader typically includes a PID and MSGID, and adds an
   INTL for netmail. Internet email is indicated by an "INETDEST" kludge
   line (ctrl-A, "INETDEST", a space (no colon), and then the address).

   Some readers end the text with a zero byte, but this doesn't appear to
   be necessary.
*/

/* <BBSID>.ID */

/* A CRLF-delimited plain ASCII text file with seven lines. It seems to be
   intended for security/validation purposes, mainly indicating the
   registration status of the reader.

   Line 1: A textual representation of a boolean, either "TRUE" or 
           "FALSE". It's "TRUE" only if the packet comes from a registered
           reader AND the user name matches the registered user.
   Line 2: Blank, in replies where line 1 is "FALSE"; otherwise, a
           hexadecimal number of unknown significance.
   Line 3: Version number of the reader, in decimal.
   Line 4: Another boolean; always "TRUE" in my limited experience. I
           don't yet know its purpose.
   Line 5: The date the packet was created, in packed MS-DOS format,
           treated as a signed number and written out as a decimal number.
   Line 6: The "reader code" in decimal; 0 for unregistered readers. This
           shows up even when the names don't match.
   Line 7: Another number; it seems to be always "-598939720" when line 1
           is "FALSE", but varies otherwise.

   Also, the file is given a weird time stamp, although that doesn't seem
   to be required by the doors I've tested it with.
*/

/* RUSRCFG.DAT */

/* This file is the same format as DUSRCFG.DAT, but is generated by the
   reader. It is only present when the configuration is changed, and may
   only be generated if a DUSRCFG.DAT file exists in the original packet.
   Unchanged information from DUSRCFG.DAT is carried over to RUSRCFG.DAT.
*/
