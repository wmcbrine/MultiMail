/*
 * MultiMail offline mail reader
 * QWK

 Copyright 1997 John Zero <john@graphisoft.hu>
 Copyright 1997-2021 William McBrine <wmcbrine@gmail.com>
 Distributed under the GNU General Public License, version 3 or later. */

#include "qwk.h"
#include "compress.h"

unsigned char *onecomp(unsigned char *p, char *dest, const char *comp)
{
    size_t len = strlen(comp);

    if (!strncasecmp((char *) p, comp, len)) {
        p += len;
        while (*p == ' ')
            p++;
        int x;
        for (x = 0; *p && (*p != '\n') && (x < 71); x++)
            dest[x] = *p++;
        dest[x] = '\0';

        while (*p == '\n')
            p++;
        return p;
    }
    return 0;
}

// -----------------------------------------------------------------
// The qheader methods
// -----------------------------------------------------------------

bool qheader::init(FILE *datFile)
{
    qwkmsg_header qh;
    char buf[9], *err;

    if (!fread(&qh, 1, sizeof qh, datFile))
        return false;

    get_field(from, qh.from, 25);
    get_field(to, qh.to, 25);
    get_field(subject, qh.subject, 25);

    memcpy(date, qh.date, 8);
    date[2] = '-';
    date[5] = '-';        // To deal with some broken messages
    date[8] = ' ';

    strnzcpy(date + 9, qh.time, 5);

    strnzcpy(buf, qh.refnum, 8);
    refnum = atol(buf);

    get_field(buf, qh.msgnum, 7);
    msgnum = strtol(buf, &err, 10);
    if (*err)
        return false;    // bogus message

    strnzcpy(buf, qh.chunks, 6);
    msglen = (atol(buf) - 1) << 7;

    privat = (qh.status == '*') || (qh.status == '+');

    // Is this a block of net-status flags?
    netblock = !qh.status || (qh.status == '\xff');

    origArea = getshort(&qh.confLSB);

    return true;
}

// Just the fields needed for building the index:
bool qheader::init_short(FILE *datFile)
{
    qwkmsg_header qh;
    char buf[9], *err;
    long rawlen;

    if (!fread(&qh, 1, sizeof qh, datFile))
        return false;

    get_field(to, qh.to, 25);

    strnzcpy(buf, qh.chunks, 6);
    rawlen = strtol(buf, &err, 10);
    if ((*err && *err != ' ') || (rawlen < 2)) {
        netblock = true;    // bad header, keep scanning
        return true;
    }

    msglen = (rawlen - 1) << 7;

    // Is this a block of net-status flags?
    netblock = !qh.status || (qh.status == '\xff');

    origArea = getshort(&qh.confLSB);

    return true;
}

// Write the header to a file, except for the length:
void qheader::output(FILE *repFile)
{
    qwkmsg_header qh;
    char buf[10];
    size_t sublen, tolen, fromlen;

    sublen = strlen(subject);
    if (sublen > 25)
        sublen = 25;

    tolen = strlen(to);
    if (tolen > 25)
        tolen = 25;

    fromlen = strlen(from);
    if (fromlen > 25)
        fromlen = 25;

    memset(&qh, ' ', sizeof qh);

    sprintf(buf, " %-6ld", msgnum);
    memcpy(qh.msgnum, buf, 7);

    putshort(&qh.confLSB, msgnum);

    if (refnum) {
        sprintf(buf, " %-7ld", refnum);
        memcpy(qh.refnum, buf, 8);
    }
    memcpy(qh.to, to, tolen);
    memcpy(qh.from, from, fromlen);
    memcpy(qh.subject, subject, sublen);

    qh.alive = (char) 0xE1;

    memcpy(qh.date, date, 8);
    memcpy(qh.time, &date[9], 5);

    if (privat)
        qh.status = '*';

    fwrite(&qh, 1, sizeof qh, repFile);
}

// Pad out with spaces, as necessary, and write the length to the header:
void qheader::set_length(FILE *repFile, long headerpos, long curpos)
{
    long length;

    for (length = curpos - headerpos; (length & 0x7f); length++)
        fputc(' ', repFile);

    fseek(repFile, headerpos + CHUNK_OFFSET, SEEK_SET);
    fprintf(repFile, "%-6ld", (length >> 7));
    fseek(repFile, headerpos + length, SEEK_SET);
}

// Return space-padded field as string, stripped and null-terminated
void qheader::get_field(char *dest, const char *source, size_t len)
{
    while (len && (' ' == source[len - 1]))
        len--;
    memcpy(dest, source, len);
    dest[len] = '\0';
}


// -----------------------------------------------------------------
// The QWK methods
// -----------------------------------------------------------------

qwkpack::qwkpack() : pktbase()
{
    qwke = !(!mm.workList->exists("toreader.ext"));

    readControlDat();
    readDoorId();
    if (qwke)
        readToReader();

    infile = mm.workList->ftryopen("messages.dat");
    if (!infile)
        fatalError("Could not open MESSAGES.DAT");

    readIndices();

    listBulletins((const char (*)[13]) newsfile, 1);
}

qwkpack::~qwkpack()
{
    cleanup();
}

unsigned long qwkpack::MSBINtolong(unsigned const char *ms)
{
    return ((((unsigned long) ms[0] + ((unsigned long) ms[1] << 8) +
        ((unsigned long) ms[2] << 16)) | 0x800000L) >> (24 + 0x80 - ms[3]));
}

area_header *qwkpack::getNextArea()
{
    int cMsgNum = areas[ID].nummsgs;
    bool x = (areas[ID].num == -1);

    area_header *tmp = new area_header(ID + 1, areas[ID].numA,
        areas[ID].name, (x ? "Letters addressed to you" : areas[ID].name),
        (greekqwk ? (x ? "GreekQWK personal" : "GreekQWK") : (qwke ? (x
        ? "QWKE personal" : "QWKE") : (x ? "QWK personal" : "QWK"))),
        areas[ID].attr | hasOffConfig | (cMsgNum ? ACTIVE : 0), cMsgNum,
        0, qwke ? 71 : 25, qwke ? 71 : 25);

    ID++;
    return tmp;
}

bool qwkpack::isQWKE()
{
    return qwke;
}

bool qwkpack::isGreekQWK()
{
    return greekqwk;
}

bool qwkpack::externalIndex()
{
    const char *p;
    bool hasNdx;

    hasPers = !(!mm.workList->exists("personal.ndx"));

    p = mm.workList->exists(".ndx");
    hasNdx = !(!p);

    if (hasNdx) {
        struct {
            unsigned char MSB[4];
            unsigned char confnum;
        } ndx_rec;

        FILE *idxFile;
        char fname[13];

        if (!hasPers) {
            areas++;
            maxConf--;
        }

        // Store the size of the .DAT for future comparison as
        // a check against invalid .NDX entries.

        fseek(infile, 0, SEEK_END);
        unsigned long endpoint = (unsigned long) ftell(infile);
        if (endpoint > 128)
            endpoint -= 128;

        while (p) {
            int x, cMsgNum = 0;

            memcpy(fname, p, strlen(p) - 4);
            fname[strlen(p) - 4] = '\0';
            x = atoi(fname);

            if (!x) {
                if (!strcasecmp(fname, "personal"))
                    x = -1;
                else
                    if (strcmp(fname, "000"))
                        x = -2;      // fname is not a num
            }
            if (x != -2) {
                x = getXNum(x);
                if (-1 == x)         // fname is a num but not a
                    hasNdx = false;  // valid conference
            }

            if (x >= 0) {

                idxFile = mm.workList->ftryopen(p);
                if (idxFile) {
                    cMsgNum = mm.workList->getSize() / ndxRecLen;
                    body[x] = new bodytype[cMsgNum];
                    for (int y = 0; y < cMsgNum; y++) {
                        // If any .NDX entries appear corrupted, we're
                        // better off aborting this and using the new
                        // (purely .DAT-based) indexing method (in
                        // readIndices()).

                        if (!fread(&ndx_rec, ndxRecLen, 1, idxFile)) {
                            hasNdx = false;
                            break;
                        }
                        unsigned long temp = MSBINtolong(ndx_rec.MSB) << 7;

                        if ((temp < 256) || (temp > endpoint)) {
                            hasNdx = false;    // use other method
                            break;
                        } else
                            body[x][y].pointer = temp;
                    }
                    fclose(idxFile);
                }

                if (hasNdx) {
                    areas[x].nummsgs = cMsgNum;
                    numMsgs += cMsgNum;
                }
            }
            p = hasNdx ? mm.workList->getNext(".ndx") : 0;
        }

        // Clean up after aborting

        if (!hasNdx) {
            if (!hasPers) {
                areas--;
                maxConf++;
            }
            for (int x = 0; x < maxConf; x++) {
                delete[] body[x];
                body[x] = 0;
                areas[x].nummsgs = 0;
            }
            numMsgs = 0;
        }
    }
    return hasNdx;
}

void qwkpack::readIndices()
{
    int x;

    body = new bodytype *[maxConf];

    for (x = 0; x < maxConf; x++) {
        body[x] = 0;
        areas[x].nummsgs = 0;
    }

    numMsgs = 0;

    if (mm.res.getInt(IgnoreNDX) || !externalIndex()) {
        ndx_fake base, *tmpndx = &base;

        long counter;
        int personal = 0;

        qheader qHead;

        fseek(infile, 128, SEEK_SET);
        while (qHead.init_short(infile))
            if (!qHead.netblock) {      // skip net-status flags
                counter = ftell(infile);
                x = getXNum(qHead.origArea);

                if (-1 != x) {
                    tmpndx->next = new ndx_fake;
                    tmpndx = tmpndx->next;

                    tmpndx->confnum = x;

                    if (!strcasecmp(qHead.to, LoginName) ||
                        (qwke && !strcasecmp(qHead.to, AliasName))) {

                        tmpndx->pers = true;
                        personal++;
                    } else
                        tmpndx->pers = false;

                    tmpndx->pointer = counter;
                    tmpndx->length = 0;    // temp

                    areas[x].nummsgs++;
                    numMsgs++;
                }

                fseek(infile, qHead.msglen, SEEK_CUR);
            }

        initBody(base.next, personal);
    }
}

letter_header *qwkpack::getNextLetter()
{
    static net_address nullNet;
    qheader q;
    unsigned long pos, rawpos;
    int areaID, letterID;

    rawpos = body[currentArea][currentLetter].pointer;
    pos = rawpos - 128;

    fseek(infile, pos, SEEK_SET);
    if (q.init(infile)) {
        if (areas[currentArea].num == -1) {
            areaID = getXNum(q.origArea);
            letterID = getYNum(areaID, rawpos);
        } else {
            areaID = currentArea;
            letterID = currentLetter;
        }
    } else
        areaID = letterID = -1;

    if ((-1 == areaID) || (-1 == letterID))
        return new letter_header("MESSAGES.DAT", "READING", "ERROR",
                                 "ERROR", 0, 0, 0, 0, 0, false, 0, this,
                                 nullNet, false);

    body[areaID][letterID].msgLength = q.msglen;

    currentLetter++;

    return new letter_header(q.subject, q.to, q.from, q.date, 0,
        q.refnum, letterID, q.msgnum, areaID, q.privat, q.msglen, this,
        nullNet, !(!(areas[areaID].attr & LATINCHAR)));
}

void qwkpack::getblk(int, long &offset, long blklen,
                     unsigned char *&p, unsigned char *&begin)
{
    int linebreak = greekqwk ? 12 : 227;

    for (long count = 0; count < blklen; count++) {
        int kar = fgetc(infile);

        if (!kar)
            kar = ' ';

        *p++ = (kar == linebreak) ? '\n' : kar;

        if (kar == linebreak) {
            begin = p;
            offset = ftell(infile);
        }
    }
}

void qwkpack::postfirstblk(unsigned char *&p, letter_header &mhead)
{
    // Get extended (QWKE-type) info, if available:

    char extsubj[72]; // extended subject or other line
    bool anyfound;
    unsigned char *q;

    do {
        anyfound = false;

        q = onecomp(p, extsubj, "subject:");

        if (!q) {
            q = onecomp(p + 1, extsubj, "@subject:");
            if (q) {
                extsubj[strlen(extsubj) - 1] = '\0';
                cropesp(extsubj);
            }
        }

        // For WWIV QWK door:
        if (!q)
            q = onecomp(p, extsubj, "title:");
        if (q) {
            p = q;
            mhead.changeSubject(extsubj);
            anyfound = true;
        }

        // To and From are now checked only in QWKE packets:

        if (qwke) {
            q = onecomp(p, extsubj, "to:");
            if (q) {
                p = q;
                mhead.changeTo(extsubj);
                anyfound = true;
            }

            q = onecomp(p, extsubj, "from:");
            if (q) {
                p = q;
                mhead.changeFrom(extsubj);
                anyfound = true;
            }
        }

        // Skip Synchronet kludge lines:

        static const char *skippers[] = {"@VIA:", "@TZ:", "@MSGID:",
            "@REPLY:", "@REPLYTO:"};

        for (int x = 0; x < 5; x++) {
            q = onecomp(p, extsubj, skippers[x]);
            if (q) {
                p = q;
                anyfound = true;
                break;
            }
        }

    } while (anyfound);
}

void qwkpack::endproc(letter_header &mhead)
{
    // Change to Latin character set, if necessary:
    checkLatin(mhead);
}

void qwkpack::readControlDat()
{
    char *p, *q;

    infile = mm.workList->ftryopen("control.dat");
    if (!infile)
        fatalError("Could not open CONTROL.DAT");

    BBSName = strdupplus(nextLine());           // 1: BBS name
    nextLine();                                 // 2: city/state
    nextLine();                                 // 3: phone#

    q = nextLine();                             // 4: sysop's name
    size_t slen = strlen(q);
    if (slen > 6) {
        p = q + slen - 5;
        if (!strcasecmp(p, "Sysop")) {
            if (*--p == ' ')
                p--;
            if (*p == ',')
                *p = '\0';
        }
    }
    SysOpName = strdupplus(q);

    q = nextLine();                             // 5: doorserno, BBSid
    strtok(q, ",");
    p = strtok(0, " ");
    strnzcpy(packetBaseName, p, 8);

    nextLine();                                 // 6: time & date

    p = nextLine();                             // 7: user's name
    cropesp(p);
    LoginName = strdupplus(p);
    AliasName = strdupplus(p);

    nextLine();                                 // 8: blank/any
    nextLine();                                 // 9: anything
    nextLine();                                 // 10: # messages (unreliable)
    maxConf = atoi(nextLine()) + 2;             // 11: Max conf #

    areas = new AREAs[maxConf];

    areas[0].num = -1;
    strcpy(areas[0].numA, "PERS");
    areas[0].name = strdupplus("PERSONAL");
    areas[0].attr = PUBLIC | PRIVATE | COLLECTION;

    int c;
    for (c = 1; c < maxConf; c++) {
        areas[c].num = atoi(nextLine());              // conf #
        sprintf(areas[c].numA, "%d", areas[c].num);
        areas[c].name = strdupplus(nextLine());       // conf name
        areas[c].attr = PUBLIC | PRIVATE;
    }

    hello = strdupplus(nextLine());
    strncpy(newsfile[0], nextLine(), 12);
    goodbye = strdupplus(nextLine());

    fclose(infile);
}

void qwkpack::readDoorId()
{
    bool hasAdd = false, hasDrop = false;

    if (!qwke)
        strcpy(controlname, "QMAIL");

    greekqwk = false;

    infile = mm.workList->ftryopen("door.id");
    if (infile) {
        const char *s, *door;
        char tmp[80], *t;

        t = strdupplus(nextLine());     // DOOR =
        s = nextLine();                 // VERSION =
        door = strchr(t, '=') + 2;
        mm.synchro = !strcmp(door, "Synchronet");
        sprintf(tmp, "%s %s", door, strchr(s, '=') + 2);
        delete[] t;
        DoorProg = strdupplus(tmp);

        s = nextLine();                 // SYSTEM =
        BBSProg = strdupplus(strchr(s, '=') + 2);

        while (!feof(infile)) {
            s = nextLine();
            if (!strcasecmp(s, "CONTROLTYPE = ADD"))
                hasAdd = true;
            else
                if (!strcasecmp(s, "CONTROLTYPE = DROP"))
                    hasDrop = true;
                else
                    if (!strncasecmp(s, "CONTROLNAME", 11))
                        strnzcpy(controlname, s + 14, 25);
                    else
                        if (!strcasecmp(s, "GREEKQWK"))
                            greekqwk = true;
        }
        fclose(infile);
    }
    if (!qwke)
        hasOffConfig = (hasAdd && hasDrop) ? OFFCONFIG : 0;
}

// Read the QWKE file TOREADER.EXT
void qwkpack::readToReader()
{
    char *s;
    int cnum;
    unsigned long attr;

    hasOffConfig = OFFCONFIG;

    infile = mm.workList->ftryopen("toreader.ext");
    if (infile) {
        while (!feof(infile)) {
            s = nextLine();

            if (!strncasecmp(s, "area ", 5)) {
                if (sscanf(s + 5, "%d %s", &cnum, s) == 2) {

                    attr = SUBKNOWN;

                    // If a group is marked subscribed:
                    if (strchr(s, 'a'))
                        attr |= ACTIVE;
                    else
                        if (strchr(s, 'p'))
                            attr |= (ACTIVE | PERSONLY);
                        else
                            if (strchr(s, 'g'))
                                attr |= (ACTIVE | PERSALL);

                    // "Handles" or "Anonymous":
                    if (strchr(s, 'H') || strchr(s, 'A'))
                        attr |= ALIAS;

                    // Public-only/Private-only:
                    if (strchr(s, 'P'))
                        attr |= PRIVATE;
                    else
                        if (strchr(s, 'O'))
                            attr |= PUBLIC;
                        else
                            attr |= (PUBLIC | PRIVATE);

                    // Read-only:
                    if (strchr(s, 'R'))
                        attr |= READONLY;

                    /* Set character set to Latin-1 for Internet or
                       Usenet areas -- but is this the right thing here? */

                    if (strchr(s, 'U') || strchr(s, 'I'))
                        attr |= LATINCHAR;

                    if (strchr(s, 'E'))
                        attr |= ECHOAREA;

                    areas[getXNum(cnum)].attr = attr;
                }
            } else
                if (!strncasecmp(s, "alias ", 6)) {
                    cropesp(s);
                    delete[] AliasName;
                    AliasName = strdupplus(s + 6);
                }
        }
        fclose(infile);
    }
}

const char *qwkpack::ctrlName()
{
    return controlname;
}

// -----------------------------------------------------------------
// The QWK reply methods
// -----------------------------------------------------------------

qwkreply::upl_qwk::upl_qwk(const char *name) : pktreply::upl_base(name)
{
    memset(&qHead, 0, sizeof(qHead));
}

qwkreply::qwkreply() : pktreply()
{
    qwke = ((qwkpack *) mm.packet)->isQWKE();
    greekqwk = ((qwkpack *) mm.packet)->isGreekQWK();
}

qwkreply::~qwkreply()
{
}

bool qwkreply::getRep1(FILE *rep, upl_qwk *l)
{
    FILE *replyFile;
    char *p, *q, blk[1280];

    if (!l->qHead.init(rep))
        return false;

    replyFile = fopen(l->fname, "wt");
    if (!replyFile)
        return false;

    long count, length = 0, chunks = l->qHead.msglen >> 7;
    char linebreak = greekqwk ? ((char) 12) : ((char) 227);

    bool firstblk = true;
    while (chunks) {
        count = (chunks > 10) ? 10 : chunks;
        chunks -= count;
        count <<= 7;

        if (!fread(blk, 1, count, rep))
            fatalError("Error reading reply file");

        for (p = blk; p < (blk + count); p++)
            if (*p == linebreak)
                *p = '\n';         // PI-softcr

        p = blk;

        // Get extended (QWKE-type) info, if available:

        if (firstblk) {
            firstblk = false;

            bool anyfound;

            do {
                anyfound = false;

                q = (char *) onecomp((unsigned char *) p,
                                     l->qHead.subject, "subject:");
                if (q) {
                    p = q;
                    anyfound = true;
                }

                if (qwke) {
                    q = (char *) onecomp((unsigned char *) p,
                                         l->qHead.to, "to:");
                    if (q) {
                        p = q;
                        anyfound = true;
                    }

                    q = (char *) onecomp((unsigned char *) p,
                                         l->qHead.from, "from:");
                    if (q) {
                        p = q;
                        anyfound = true;
                    }
                }

            } while (anyfound);
        }

        q = blk + count - 1;
        if (!chunks)
            for (; ((*q == ' ') || (*q == '\n')) && (q > blk); q--);

        length += (long) fwrite(p, 1, q - p + 1, replyFile);
    }
    fclose(replyFile);

    l->qHead.msglen = l->msglen = length;

    return true;
}

void qwkreply::getReplies(FILE *repFile)
{
    fseek(repFile, 128, SEEK_SET);

    noOfLetters = 0;

    upl_qwk baseUplList, *currUplList = &baseUplList;

    while (!feof(repFile)) {
        currUplList->nextRecord = new upl_qwk;
        currUplList = (upl_qwk *) currUplList->nextRecord;
        if (!getRep1(repFile, currUplList)) {
            delete currUplList;
            break;
        }
        noOfLetters++;
    }
    uplListHead = baseUplList.nextRecord;
}

area_header *qwkreply::getNextArea()
{
    return new area_header(0, "REPLY", "REPLIES",
        "Letters written by you", (greekqwk ? "GreekQWK replies" :
        (qwke ? "QWKE replies" : "QWK replies")),
        (COLLECTION | REPLYAREA | ACTIVE | PUBLIC | PRIVATE),
        noOfLetters, 0, qwke ? 71 : 25, qwke ? 71 : 25);
}

letter_header *qwkreply::getNextLetter()
{
    static net_address nullNet;
    upl_qwk *current = (upl_qwk *) uplListCurrent;

    int area = ((qwkpack *) mm.packet)->
        getXNum((int) current->qHead.msgnum) + 1;

    letter_header *newLetter = new letter_header(
        current->qHead.subject, current->qHead.to, current->qHead.from,
        current->qHead.date, 0, current->qHead.refnum, currentLetter,
        currentLetter, area, current->qHead.privat,
        current->qHead.msglen, this, nullNet, mm.areaList->isLatin(area));

    currentLetter++;
    uplListCurrent = uplListCurrent->nextRecord;
    return newLetter;
}

void qwkreply::enterLetter(letter_header &newLetter,
                           const char *newLetterFileName, long length)
{
    // Specify the format separately from strftime() to suppress
    // GCC's Y2K warning:
    const char *datefmt_qwk = "%m-%d-%y %H:%M";

    upl_qwk *newList = new upl_qwk(newLetterFileName);

    strncpy(newList->qHead.subject, newLetter.getSubject(),
            sizeof(newList->qHead.subject) - 1);
    strncpy(newList->qHead.from, newLetter.getFrom(),
            sizeof(newList->qHead.from) - 1);
    strncpy(newList->qHead.to, newLetter.getTo(),
            sizeof(newList->qHead.to) - 1);

    newList->qHead.msgnum = atol(mm.areaList->getShortName());
    newList->qHead.privat = newLetter.getPrivate();
    newList->qHead.refnum = newLetter.getReplyTo();

    time_t now = time(0);
    strftime(newList->qHead.date, 15, datefmt_qwk, localtime(&now));

    newList->qHead.msglen = newList->msglen = length;

    addUpl(newList);
}

void qwkreply::addRep1(FILE *rep, upl_base *node, int)
{
    FILE *replyFile;
    upl_qwk *l = (upl_qwk *) node;
    long count = 0;
    char linebreak = greekqwk ? ((char) 12) : ((char) 227);

    long headerpos = ftell(rep);
    l->qHead.output(rep);

    bool longfrom = strlen(l->qHead.from) > 25;
    bool longto = strlen(l->qHead.to) > 25;
    bool longsubj = strlen(l->qHead.subject) > 25;

    if (longfrom)
        fprintf(rep, "From: %s%c", l->qHead.from, linebreak);
    if (longto)
        fprintf(rep, "To: %s%c", l->qHead.to, linebreak);
    if (longsubj)
        fprintf(rep, "Subject: %s%c", l->qHead.subject, linebreak);
    if (longfrom || longto || longsubj)
        fprintf(rep, "%c", linebreak);

    replyFile = fopen(l->fname, "rt");
    if (replyFile) {

        int c, lastsp = 0;
        while ((c = fgetc(replyFile)) != EOF) {
            count++;
            if ((count > 80) && lastsp) {
                fseek(replyFile, lastsp - count, SEEK_CUR);
                fseek(rep, lastsp - count, SEEK_CUR);
                c = '\n';
            }
            if ('\n' == c) {
                fputc(linebreak, rep);
                count = lastsp = 0;
            } else {
                fputc(c, rep);
                if (' ' == c)
                    lastsp = count;
            }
        }
        fclose(replyFile);
    }

    fputc(linebreak, rep);

    long curpos = ftell(rep);
    l->qHead.set_length(rep, headerpos, curpos);
}

void qwkreply::addHeader(FILE *repFile)
{
    char tmp[129];
    sprintf(tmp, "%-128s", getBaseName());
    fwrite(tmp, 128, 1, repFile);
}

void qwkreply::repFileName()
{
    int x;
    const char *basename = getBaseName();

    for (x = 0; basename[x]; x++) {
        replyPacketName[x] = tolower(basename[x]);
        replyInnerName[x] = toupper(basename[x]);
    }
    strcpy(replyPacketName + x, ".rep");
    strcpy(replyInnerName + x, ".MSG");
}

const char *qwkreply::repTemplate(bool offres)
{
    static char buff[30];

    sprintf(buff, (offres && qwke) ? "%s TODOOR.EXT" : "%s", replyInnerName);

    return buff;
}

bool qwkreply::getOffConfig()
{
    bool status = false;

    if (qwke) {
        FILE *olc;

        upWorkList = new file_list(mm.res.get(UpWorkDir));

        olc = upWorkList->ftryopen("todoor.ext");
        if (olc) {
            char line[128], mode;
            int areaQWK, areaNo;

            while (!feof(olc)) {
                myfgets(line, sizeof line, olc);
                if (sscanf(line, "AREA %d %c", &areaQWK, &mode) == 2) {
                    areaNo = ((qwkpack *) mm.packet)->getXNum(areaQWK) + 1;
                    mm.areaList->gotoArea(areaNo);
                    if (mode == 'D')
                        mm.areaList->Drop();
                    else
                        mm.areaList->Add();
                }
            }
            fclose(olc);
            upWorkList->kill();

            status = true;
        }
        delete upWorkList;
    }
    return status;
}

bool qwkreply::makeOffConfig()
{
    FILE *todoor = 0;    // warning suppression
    net_address bogus;
    letter_header *ctrlMsg;
    const char *myname = 0, *ctrlName = 0;

    if (qwke) {
        todoor = fopen("TODOOR.EXT", "wb");
        if (!todoor)
            return false;
    } else {
        myname = mm.packet->getLoginName();
        ctrlName = ((qwkpack *) mm.packet)->ctrlName();
    }

    int oldarea = mm.areaList->getAreaNo();

    int maxareas = mm.areaList->noOfAreas();
    for (int areaNo = 0; areaNo < maxareas; areaNo++) {
        mm.areaList->gotoArea(areaNo);
        unsigned long attrib = mm.areaList->getType();

        if (attrib & (ADDED | DROPPED)) {
            if (qwke)
                fprintf(todoor, "AREA %s %c\r\n",
                        mm.areaList->getShortName(), (attrib & ADDED) ?
                        ((attrib & PERSONLY) ? 'p' : ((attrib & PERSALL)
                        ? 'g' : 'a')) : 'D');
            else {
                ctrlMsg = new letter_header((attrib & ADDED) ?
                          "ADD" : "DROP", ctrlName, myname, "", 0, 0, 0,
                          0, areaNo, false, 0, this, bogus, false);

                enterLetter(*ctrlMsg, "", 0);

                delete ctrlMsg;

                if (attrib & ADDED)
                    mm.areaList->Drop();
                else
                    mm.areaList->Add();
            }
        }
    }
    mm.areaList->gotoArea(oldarea);

    if (qwke)
        fclose(todoor);
    else
        mm.areaList->refreshArea();

    return true;
}
