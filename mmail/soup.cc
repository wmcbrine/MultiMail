/*
 * MultiMail offline mail reader
 * SOUP

 Copyright 1999-2017 William McBrine <wmcbrine@gmail.com>
 Distributed under the GNU General Public License, version 3 or later. */

#include "soup.h"
#include "compress.h"

// -----------------------------------------------------------------
// The sheader methods
// -----------------------------------------------------------------

const char *sheader::compare[] =
{
    "Date:", "From:", "To:", "Reply-To:", "Subject:", "Newsgroups:",
    "Followup-To:", "References:", "Message-ID:"
};

sheader::sheader()
{
    for (int c = 0; c < items; c++)
        values[c] = 0;

    has8bit = qpenc = false;
}

sheader::~sheader()
{
    for (int c = 0; c < items; c++)
        delete[] values[c];
}

// Read and parse header; assumes msg is already at start of text
bool sheader::init(FILE *msg)
{
    char buffer[1000], *end;
    int lastc = -1;

    do {
        if (feof(msg))
            return false;

        // Get first 999 bytes of header line
        if (!fgets(buffer, sizeof buffer, msg))
            return false;
        end = buffer + strlen(buffer) - 1;

        // If there's more, rewind to last space; remainder
        // will be handled as a continuation
        if (*end != '\n') {
            long skip = -1;
            while ((*end != ' ') && (end > (buffer + 1))) {
                end--;
                skip--;
            }
            if (end > (buffer + 1)) {
                fseek(msg, skip, SEEK_CUR);
                *end = '\0';
            } else            // give up on getting the rest
                while (fgetc(msg) != '\n');
        }

        // If the line isn't blank...
        if (buffer != end) {

            if (*end == '\n')
                *end = '\0';

            char *sp = strpbrk(buffer, " \t");
            // continuation
            if ((sp == buffer) && (lastc != -1)) {
                while ((*sp == ' ') || (*sp == '\t'))
                    sp++;
                char *newval = new char[strlen(values[lastc]) +
                                        strlen(sp) + 2];
                sprintf(newval, "%s %s", values[lastc], sp);
                delete[] values[lastc];
                values[lastc] = newval;
            } else
            // new header
                if (sp) {
                    *sp = '\0';
                    int c;

                    for (c = 0; c < items; c++)
                        if (!values[c] && !strcasecmp(buffer, compare[c])) {
                            do
                                sp++;
                            while ((*sp == ' ') || (*sp == '\t'));
                            values[c] = strdupplus(sp);
                            lastc = c;
                            break;
                        }
                        if (c == items) {
                            lastc = -1;
                            *sp = ' ';
                            if (!strcasecmp(buffer,
                                "content-transfer-encoding: quoted-printable"))

                                qpenc = true;
                        }
                }
        }
    } while (buffer != end);    // End of header == blank line

    // If the last msgid is truncated, drop it

    if (values[refs]) {
        end = strrchr(values[refs], '<');
        if (!end) {    // no Message-ID's!
            delete values[refs];
            values[refs] = 0;
        } else
            if (!strchr(end, '>')) {
                if (end[-1] == ' ')
                    end--;
                *end = '\0';
            }
    }

    return true;
}

// Set header values from strings
bool sheader::init(const char *fromA, const char *toName, const char *toAddr,
    const char *subjectA, const char *newsA, const char *refsA, long length)
{
    values[from] = strdupplus(fromA);
    values[subject] = strdupplus(subjectA);
    values[newsgrps] = strdupplus(newsA);
    values[refs] = strdupplus(refsA);

    if ((toName && toAddr) && strcmp(toName, "All")) {
        if (*toName && strcmp(toName, toAddr)) {
            values[to] = new char[strlen(toName) + strlen(toAddr) + 6];
            sprintf(values[to], quoteIt(toName) ? "\"%s\" <%s>" : "%s <%s>",
                    toName, toAddr);
        } else
            values[to] = strdupplus(toAddr);
    }

    char dateA[40];
    time_t t;

    time(&t);
    strftime(dateA, sizeof dateA, "%a, %d %b %Y %X GMT", gmtime(&t));

    values[date] = strdupplus(dateA);

    msglen = length;

    return true;
}

// Write out the header in 'b' or 'B' form
void sheader::output(FILE *msg, const char *cset, bool useQPHead,
    bool useQPBody)
{
    static const char *MIMEhead =
        "MIME-Version: 1.0\nContent-Type: text/plain; "
        "charset=%.14s\nContent-Transfer-Encoding: %s\n";

    static const char *eightbit = "8bit", *QP = "quoted-printable";

    for (int c = 0; c <= refs; c++)
        if (values[c] && values[c][0]) {
            fprintf(msg, "%s ", compare[c]);
            if (((c == from) || (c == to) || (c == subject)) && useQPHead)
                headenc((unsigned char *) values[c], cset, msg);
            else
                fprintf(msg, "%s", values[c]);
            fprintf(msg, "\n");
        }

    if (has8bit)
        fprintf(msg, MIMEhead, cset, useQPBody ? QP : eightbit);

    fprintf(msg, "User-Agent: " MM_NAME "/" MM_VERNUM " (SOUP; %s)\n\n",
            sysname());
}

const char *sheader::From()
{
    return values[from] ? values[from] : "";
}

const char *sheader::Subject()
{
    return values[subject] ? values[subject] : "";
}

const char *sheader::Date()
{
    return values[date] ? values[date] : "";
}

const char *sheader::ReplyTo()
{
    return values[reply];
}

const char *sheader::To()
{
    return values[to];
}

const char *sheader::Newsgrps()
{
    return values[newsgrps];
}

const char *sheader::Follow()
{
    return values[follow];
}

const char *sheader::Msgid()
{
    return values[msgid];
}

const char *sheader::Refs()
{
    return values[refs];
}

// -----------------------------------------------------------------
// The SOUP methods
// -----------------------------------------------------------------

soup::soup(mmail *mmA) : pktbase(mmA)
{
    strncpy(packetBaseName,
            findBaseName(mm->resourceObject->get(PacketName)), 8);
    packetBaseName[8] = '\0';

    hasOffConfig = false;  // :-( For now, at least.

    readAreas();
    buildIndices();

    const char x[1][13] = {"info"};
    listBulletins(x, 1, 0);

    hasPers = false;
}

soup::~soup()
{
    while (maxConf--) {
        delete[] body[maxConf];
        delete[] areas[maxConf]->name;
        delete areas[maxConf];
    }
    delete[] areas;
}

bool soup::msgopen(int area)
{
    static int oldArea;

    if (!infile)
        oldArea = -1;    // Reset (new packet)

    if (area != oldArea) {
        if (oldArea != -1)
            fclose(infile);

        oldArea = area;

        char tmp[13];
        const char *fname = areas[area]->msgfile;
        if (!(*fname))
            return false;

        sprintf(tmp, "%.8s.MSG", fname);

        infile = mm->workList->ftryopen(tmp);
        if (!infile)
            fatalError("Could not open .MSG file");
    }
    return true;
}

area_header *soup::getNextArea()
{
    int cMsgNum = areas[ID]->nummsgs;

    area_header *tmp = new area_header(mm, ID + 1, areas[ID]->numA,
                       areas[ID]->msgfile, areas[ID]->name, "SOUP",
                       areas[ID]->attr | (cMsgNum ? ACTIVE : 0), cMsgNum,
                       0, 99, 511);
    ID++;
    return tmp;
}

int soup::getNoOfLetters()
{
    return areas[currentArea]->nummsgs;
}

// Check for valid "From "-line message separator
bool soup::parseFrom(const char *s)
{
    // Based on the VALID macro from the IMAP toolkit
    // by Mark Crispin <mrc@cac.washington.edu>
    // Copyright 1989-2000 University of Washington
    // http://www.washington.edu/imap/
    // Modified by William McBrine

    int ti = 0;            // Time Index (location of date string)
    const char *x = s + strlen(s);

    while (('\n' == x[-1]) || ('\r' == x[-1]))
        x--;

    if ((x - s) > 40) {
        int c;

        for (c = -1; x[c] != ' '; c--);

        if (!strncmp(x + c - 12, " remote from", 12))
            x += c - 12;
    }
    if ((x - s) > 26) {
        if (' ' == x[-5]) {                 // Year is last field?
            if (':' == x[-8])
                ti = -5;
            else
                if (' ' == x[-9])
                    ti = -9;
                else
                    if ((' ' == x[-11]) &&
                        (('+' == x[-10]) || ('-' == x[-10])))

                       ti = -11;
        } else
            if (' ' == x[-4]) {             // Or three-letter time zone?
                if (' ' == x[-9])
                    ti = -9;
            } else
                if (' ' == x[-6]) {         // Or numeric time zone?
                    if ((' ' == x[-11]) && (('+' == x[-5]) || ('-' == x[-5])))
                        ti = -11;
                }

        if (ti && !((':' == x[ti - 3]) && (' ' == x[ti -= ((':' == x[ti
            - 6]) ? 9 : 6)]) && (' ' == x[ti - 3]) && (' ' == x[ti - 7])
            && (' ' == x[ti - 11])))

            ti = 0;
    }

    return (ti != 0);
}

// Build indices from *.MSG -- does not handle type 'M', nor *.IDX
void soup::buildIndices()
{
    int x, cMsgNum;

    numMsgs = 0;

    body = new bodytype *[maxConf];

    ndx_fake base, *oldndx, *tmpndx;

    for (x = 0; x < maxConf; x++) {

        body[x] = 0;
        cMsgNum = 0;

        if (msgopen(x)) {

            tmpndx = &base;

            switch (areas[x]->mode) {
            case 'B':
            case 'b':
            case 'u':
            case 'n':   // bogus identifer used by GNUS, same as u
                long offset, counter;

                while (!feof(infile)) {
                    if (toupper(areas[x]->mode) == 'B') {
                        unsigned char offsetA[4];

                        if (fread(offsetA, 1, 4, infile) == 4)
                            offset = getblong(offsetA);
                        else
                            offset = -1;
                    } else {
                        char buffer[128];

                        if (myfgets(buffer, sizeof buffer, infile))
                            sscanf(buffer, "#! rnews %ld", &offset);
                        else
                            offset = -1;
                    }
                    if (offset != -1) {
                        counter = ftell(infile);
                        fseek(infile, offset, SEEK_CUR);

                        tmpndx->next = new ndx_fake;
                        tmpndx = tmpndx->next;

                        tmpndx->pointer = counter;
                        tmpndx->length = offset;

                        numMsgs++;
                        cMsgNum++;
                    }
                }
                break;
            case 'm':
                char buffer[128];
                long c, lastc = -1;

                while (!feof(infile)) {
                    c = ftell(infile);
                    if (myfgets(buffer, sizeof buffer, infile))
                        if (!strncmp(buffer, "From ", 5))
                            if (parseFrom(buffer)) {
                                if (lastc != -1) {
                                    tmpndx->next = new ndx_fake;
                                    tmpndx = tmpndx->next;

                                    tmpndx->pointer = lastc;
                                    tmpndx->length = c - lastc;

                                    numMsgs++;
                                    cMsgNum++;
                                }
                                lastc = c;
                            }
                }
                if (lastc != -1) {
                    tmpndx->next = new ndx_fake;
                    tmpndx = tmpndx->next;

                    tmpndx->pointer = lastc;
                    tmpndx->length = ftell(infile) - lastc;

                    numMsgs++;
                    cMsgNum++;
                }
            }
        }
        areas[x]->nummsgs = cMsgNum;

        if (cMsgNum) {
            body[x] = new bodytype[cMsgNum];

            tmpndx = base.next;
            for (int y = 0; y < cMsgNum; y++) {
                body[x][y].pointer = tmpndx->pointer;
                body[x][y].msgLength = tmpndx->length;
                oldndx = tmpndx;
                tmpndx = tmpndx->next;
                delete oldndx;
            }
        }
    }
}

letter_header *soup::getNextLetter()
{
    sheader sHead;

    long len = body[currentArea][currentLetter].msgLength;

    // Read in header:

    if (msgopen(currentArea)) {
        fseek(infile, body[currentArea][currentLetter].pointer, SEEK_SET);
        sHead.init(infile);
    }

    // Get address from "From:" line:

    net_address na;
    na = fromAddr(sHead.From());

    // Get name from "From:" line:

    const char *fr = fromName(sHead.From());

    // Join Message-ID and References lines:

    const char *refs = sHead.Refs(), *msgid = sHead.Msgid();
    char *fullref = 0;

    if (refs) {
        if (msgid) {
            fullref = new char[strlen(msgid) + strlen(refs) + 2];
            sprintf(fullref, "%s %s", refs, msgid);
            msgid = fullref;
        } else
            msgid = refs;
    }

    letter_header *tmp = new letter_header(mm, sHead.Subject(),
        sHead.To() ? sHead.To() : "All", *fr ? fr : (const char *) na,
        sHead.Date(), msgid, 0, currentLetter, currentLetter + 1,
        currentArea, false, len, this, na, true, sHead.Newsgrps(),
        sHead.Follow(), sHead.ReplyTo(), sHead.qpenc);

    currentLetter++;

    delete[] fullref;

    return tmp;
}

// returns the body of the requested letter
letter_body *soup::getBody(letter_header &mhead)
{
    int AreaID, LetterID;
    long length, offset;
    letter_body head(0, 0), *currblk = &head;

    AreaID = mhead.getAreaID() - 1;
    LetterID = mhead.getLetterID();

    delete bodyString;

    length = limitmem(body[AreaID][LetterID].msgLength);
    offset = body[AreaID][LetterID].pointer;

    msgopen(AreaID);

    bool firstblk = true;

    if (!length)
        head.next = new letter_body(strdupplus("\n"), 1);
    else
        while (length) {
            unsigned char *p, *src, *begin;
            long count, blklen, oldoffs;

            fseek(infile, offset, SEEK_SET);
            oldoffs = offset;

            if (firstblk) {
                int lastkar = -1, kar = -1;

                do {
                    if ('\r' != kar)
                        lastkar = kar;
                    kar = fgetc(infile);
                } while (!(('\n' == kar) && ('\n' == lastkar)));

                long count = ftell(infile) - offset;

                fseek(infile, offset, SEEK_SET);
                src = new unsigned char[count + 1];
                p = src;

                getblk(0, offset, count, p, begin);
                *p = '\0';

                currblk->next = new letter_body((char *) src, count, 0, true);
                currblk = currblk->next;

                oldoffs = offset = ftell(infile);
                length -= count;
            }

            blklen = (length > MAXBLOCK) ? MAXBLOCK : length;
            src = begin = p = new unsigned char[blklen + 1];

            getblk(0, offset, blklen, p, begin);

            if (length > MAXBLOCK) {
                if (begin > src)
                    count = begin - src;
                else {
                    offset = ftell(infile);
                    count = p - src;
                }
                length -= (offset - oldoffs);
            } else {
                // Strip blank lines
                do
                    p--;
                while ((*p == ' ') || (*p == '\n'));

                length = 0;
                count = p - src + 1;
            }
            src[count] = '\0';

            if (mhead.isQP()) {
                unsigned char *p = qpdecode(src);
                *p = '\0';
                count = p - src;
            }

            p = src;

            currblk->next = new letter_body((char *) src, count, p - src);
            currblk = currblk->next;

            firstblk = false;
        }
    bodyString = head.next;
    head.next = 0;             // Prevent deletion of chain

    return bodyString;
}

// Area and packet init
void soup::readAreas()
{
    file_list *wl = mm->workList;

    // Info not available in SOUP:

    const char *defName = mm->resourceObject->get(UserName);
    const char *defAddr = mm->resourceObject->get(InetAddr);

    if (defAddr) {
        if (defName && *defName && strcmp(defName, defAddr)) {
            LoginName = new char[strlen(defName) + strlen(defAddr) + 6];
            sprintf(LoginName, quoteIt(defName) ? "\"%s\" <%s>" : "%s <%s>",
                    defName, defAddr);
        } else
            LoginName = strdupplus(defAddr);
    }

    // AREAS:

    maxConf = 1;

    AREAs base, *tmparea;

    // Email area is always present:

    base.next = new AREAs;
    tmparea = base.next;
    memset(tmparea, 0, sizeof(AREAs));
    tmparea->name = strdupplus("Email");
    tmparea->attr = ACTIVE | LATINCHAR | INTERNET | NETMAIL | PRIVATE;
    strcpy(tmparea->numA, "0");

    FILE *afile = wl->ftryopen("areas");
    if (afile) {
        char buffer[128], *msgfile, *name, *rawattr;
        do
            if (myfgets(buffer, sizeof buffer, afile)) {

                msgfile = strtok(buffer, "\t");
                name = strtok(0, "\t");
                rawattr = strtok(0, "\t");

                // If the Email area has not been set yet, the
                // first email area in the packet (in any) becomes it.

                if (!base.next->msgfile[0] && ((rawattr[2] == 'm') ||
                    ((rawattr[2] != 'n') && !((*rawattr == 'u') ||
                    (*rawattr == 'B'))))) {

                    strncpy(base.next->msgfile, msgfile, 9);
                    delete[] base.next->name;
                    base.next->name = strdupplus(name);
                    base.next->mode = *rawattr;
                } else {
                    tmparea->next = new AREAs;
                    tmparea = tmparea->next;
                    memset(tmparea, 0, sizeof(AREAs));

                    strncpy(tmparea->msgfile, msgfile, 9);
                    tmparea->name = strdupplus(name);

                    tmparea->mode = *rawattr;

                    tmparea->attr = ACTIVE | LATINCHAR | INTERNET |
                        (((rawattr[2] == 'n') || ((rawattr[2] != 'm') &&
                        ((*rawattr == 'B') || (*rawattr == 'u')))) ?
                        PUBLIC : (NETMAIL | PRIVATE));

                    sprintf(tmparea->numA, "%5d", maxConf++);
                }
            }
        while (!feof(afile));
        fclose(afile);

        areas = new AREAs *[maxConf];
        tmparea = base.next;
        for (int i = 0; i < maxConf; i++) {
            areas[i] = tmparea;
            tmparea = tmparea->next;
        }
    } else
        areas = 0;
}

const char *soup::getTear(int)
{
    return 0;
}

bool soup::isLatin()
{
    return true;
}

// -----------------------------------------------------------------
// The SOUP reply methods
// -----------------------------------------------------------------

souprep::upl_soup::upl_soup(const char *name) : pktreply::upl_base(name)
{
}

souprep::souprep(mmail *mmA, specific_driver *baseClassA) :
    pktreply(mmA, baseClassA)
{
}

souprep::~souprep()
{
}

// convert one reply to MultiMail's internal format
bool souprep::getRep1(FILE *rep, upl_soup *l)
{
    FILE *orgfile, *destfile;
    char buffer[128], *msgfile, *mnflag; //, *rawattr;
    long count = 0;
    bool has8bit = false;

    if (!myfgets(buffer, sizeof buffer, rep))
        return false;

    msgfile = strtok(buffer, "\t");
    mnflag = strtok(0, "\t");
    //rawattr = strtok(0, "\t");

    l->privat = (*mnflag == 'm');

    orgfile = upWorkList->ftryopen(msgfile);
    if (orgfile) {
        fseek(orgfile, 4, SEEK_SET);
        if (!l->sHead.init(orgfile))
            return false;

        const char *to = l->sHead.To();
        if (to)
            l->na = fromAddr(to);

        destfile = fopen(l->fname, "wt");
        if (destfile) {
            if (l->sHead.qpenc) {
                count = qpdecode(orgfile, destfile);
                has8bit = true;
            } else {
                int c;
                while ((c = fgetc(orgfile)) != EOF) {
                    fputc(c, destfile);
                    if (c & 0x80)
                        has8bit = true;
                    count++;
                }
            }
            fclose(destfile);
        }
        fclose(orgfile);
    }

    l->msglen = l->sHead.msglen = count;
    l->sHead.has8bit = has8bit;

    l->refnum = 0;
    l->origArea = l->privat ? 1 : -1;

    remove(msgfile);

    return true;
}

// convert all replies
void souprep::getReplies(FILE *repFile)
{
    noOfLetters = 0;

    upl_soup baseUplList, *currUplList = &baseUplList;

    while (!feof(repFile)) {
        currUplList->nextRecord = new upl_soup;
        currUplList = (upl_soup *) currUplList->nextRecord;
        if (!getRep1(repFile, currUplList)) {
            delete currUplList;
            break;
        }
        noOfLetters++;
    }
    uplListHead = baseUplList.nextRecord;
}

area_header *souprep::getNextArea()
{
    return new area_header(mm, 0, "REPLY", "REPLIES",
        "Letters written by you", "SOUP replies",
        (COLLECTION | REPLYAREA | ACTIVE | PUBLIC | PRIVATE),
        noOfLetters, 0, 99, 511);
}

letter_header *souprep::getNextLetter()
{
    upl_soup *current = (upl_soup *) uplListCurrent;
    const char *to = current->sHead.To();
    const char *ng = current->sHead.Newsgrps();
    int cn = current->origArea;

    if (ng && (cn == -1)) {
        for (int c = 2; c < mm->areaList->noOfAreas(); c++)
            if (strstr(ng, mm->areaList->getDescription(c))) {
                cn = c;
                break;
            }
        if (cn == -1)
            cn = 2;

        current->origArea = cn;
    }

    letter_header *newLetter = new letter_header(mm,
        current->sHead.Subject(), to ? fromName(to) : "All",
        current->sHead.From(), current->sHead.Date(),
        current->sHead.Refs(), current->refnum, currentLetter,
        currentLetter, cn, current->privat, current->msglen, this,
        current->na, true, ng);

    currentLetter++;
    uplListCurrent = uplListCurrent->nextRecord;
    return newLetter;
}

void souprep::enterLetter(letter_header &newLetter,
                          const char *newLetterFileName, long length)
{
    upl_soup *newList = new upl_soup(newLetterFileName);

    newList->origArea = mm->areaList->getAreaNo();
    newList->privat = newLetter.getPrivate();
    newList->refnum = newLetter.getReplyTo();
    newList->na = newLetter.getNetAddr();

    newList->sHead.init(newLetter.getFrom(), newLetter.getTo(),
        (const char *) newLetter.getNetAddr(), newLetter.getSubject(),
        newLetter.getNewsgrps(), newLetter.getMsgID(), length);

    newList->msglen = length;

    // Check for 8-bit characters -- there should be a better way to do this:

    FILE *tmp;
    bool has8bit = false;

    tmp = fopen(newLetterFileName, "rt");
    if (tmp) {
        int c;
        while ((c = fgetc(tmp)) != EOF)
            if (c & 0x80)
                has8bit = true;

        fclose(tmp);
    }

    newList->sHead.has8bit = has8bit;

    addUpl(newList);
}

// write out one reply in SOUP format
void souprep::addRep1(FILE *rep, upl_base *node, int recnum)
{
    FILE *orgfile, *destfile;
    upl_soup *l = (upl_soup *) node;
    char dest[13];

    sprintf(dest, "R%07d.MSG", recnum);
    fprintf(rep, "R%07d\t%sn\n", recnum, l->privat ? "mail\tb" : "news\tB");

    orgfile = fopen(l->fname, "rt");
    if (orgfile) {

        destfile = fopen(dest, "wb");
        if (destfile) {
            unsigned char outlen[4];
            putblong(outlen, 0L);
            fwrite(outlen, 4, 1, destfile);

            bool useQPbody = l->privat ?
                mm->resourceObject->getInt(UseQPMail) :
                mm->resourceObject->getInt(UseQPNews);

            bool useQPhead = l->privat ?
                mm->resourceObject->getInt(UseQPMailHead) :
                mm->resourceObject->getInt(UseQPNewsHead);

            l->sHead.output(destfile, mm->resourceObject->get(outCharset),
                            useQPhead, useQPbody);

            if (useQPbody && l->sHead.has8bit)
                qpencode(orgfile, destfile);
            else {
                int c, count = 0, lastsp = 0;
                while ((c = fgetc(orgfile)) != EOF) {
                    count++;
                    if ((count > 80) && lastsp) {
                        fseek(orgfile, lastsp - count, SEEK_CUR);
                        fseek(destfile, lastsp - count, SEEK_CUR);
                        c = '\n';
                    }

                    if ('\n' == c)
                        count = lastsp = 0;
                    else
                        if (' ' == c)
                            lastsp = count;

                    fputc(c, destfile);
                }
            }

            putblong(outlen, ftell(destfile) - 4L);

            fseek(destfile, 0, SEEK_SET);
            fwrite(outlen, 4, 1, destfile);
            fseek(destfile, 0, SEEK_END);

            fclose(destfile);
        }
        fclose(orgfile);
    }
}

void souprep::addHeader(FILE *)
{
}

// set names for reply packet files
void souprep::repFileName()
{
    const char *basename = baseClass->getBaseName();

    sprintf(replyPacketName, "%s.rep", basename);
    sprintf(replyInnerName, "REPLIES");
}

// list files to be archived when creating reply packet
const char *souprep::repTemplate(bool)
{
    return "REPLIES *.MSG";
}

// re-read an offline config file -- not implemented yet
bool souprep::getOffConfig()
{
    return false;
}

bool souprep::makeOffConfig()
{
    return false;
}
