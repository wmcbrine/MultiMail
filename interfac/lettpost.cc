/*
 * MultiMail offline mail reader
 * message editing (continuation of the LetterWindow class)

 Copyright 1996 Kolossvary Tamas <thomas@tvnet.hu>
 Copyright 1997 John Zero <john@graphisoft.hu>
 Copyright 1997-2017 William McBrine <wmcbrine@gmail.com>
 Distributed under the GNU General Public License, version 3 or later. */

#include "interfac.h"

void LetterWindow::set_Letter_Params(net_address &nm, const char *to)
{
    NM = nm;
    delete[] To;
    To = strdupplus(to);
}

void LetterWindow::QuoteText(FILE *reply)
{
    char TMP[81];
    int i;
    bool inet = !(!(mm.areaList->getType() & INTERNET));
    const char *From = mm.letterList->getFrom();

    int width = mm.resourceObject->getInt(QuoteWrapCols);
    if ((width < 20) || (width > 80))
        width = 78;
    width -= inet ? 2 : 6;

    char c;
    const char *s, *quotestr = mm.resourceObject->get(inet ? InetQuote :
                                                      QuoteHead);

    // Format header:

    while ((c = *quotestr++) != 0) {
        if (c != '%')
            fputc(c, reply);
        else {
            s = 0;
            switch (tolower(*quotestr)) {
            case 'f':
                s = From;
                break;
            case 't':
                s = mm.letterList->getTo();
                break;
            case 'd':
                s = mm.letterList->getDate();
                break;
            case 's':
                s = mm.letterList->getSubject();
                break;
            case 'a':
                s = mm.areaList->getDescription();
                break;
            case 'n':
                s = "\n";
                break;
            case '%':
                s = "%";
            }
            if (s) {
                sprintf(TMP, "%.80s", s);
                letterconv_in(TMP);
                fputs(TMP, reply);
                quotestr++;
            }
        }
    }
    fputs("\n\n", reply);

    // Set quote initials if necessary:

    if (!inet) {
        char initials[4];

        // Start off with the first two letters of the name
        strncpy(initials, From, 2);
        initials[2] = '\0';
        initials[3] = '\0';

        // Find the first letters of each of the second and last
        // words in the name, if available
        i = 1;
        for (int j = 1; j < 3; j++) {
            bool end = false;
            while (From[i] && !end) {
                if ((From[i - 1] == ' ') && (From[i] != ' ')) {
                    initials[j] = From[i];
                    if (j == 1)
                        end = true;
                }
                i++;
            }
        }

        letterconv_in(initials);
        sprintf(TMP, " %s> ", initials);
    } else
        strcpy(TMP, "> ");

    // Output quoted text:

    MakeChain(width, true);

    for (i = 0; i < NumOfLines; i++) {
        Line *curr = linelist[i];

        if (!((Sigline == curr->attr) ||
            (!curr->length && (i < (NumOfLines - 1)) &&
            (Sigline == linelist[i + 1]->attr) ))) {

            fputs(curr->length ? ((Quoted == curr->attr) ?
                  (inet ? ">" : ((curr->text[0] == ' ') ? "" : " ")) :
                  TMP) : (inet ? ">" : ""), reply);

            curr->out(reply);
        }
    }

    MakeChain(COLS);
}

int LetterWindow::HeaderLine(ShadowedWin &win, char *buf, int limit,
                             int pos, coltype color)
{
    areaconv_in(buf);
    int getit = win.getstring(pos, 8, buf, limit, color, color);
    areaconv_out(buf);
    return getit;
}

int LetterWindow::EnterHeader(char *FROM, char *TO, char *SUBJ, bool &privat)
{
    static const char *noyes[] = { "No", "Yes" };

    char NETADD[100];
    int result, current, maxitems = 2;
    bool end = false;
    bool hasNet = mm.areaList->isEmail();
    bool hasNews = mm.areaList->isUsenet();
    bool hasTo = hasNews || mm.areaList->hasTo();

    if (hasNet) {
        sprintf(NETADD, "%.99s", (const char *) NM);
        maxitems++;
    } else
        NM.isSet = false;

    if (hasTo)
        maxitems++;

    ShadowedWin rep_header(maxitems + 2, COLS - 4, (LINES / 2) - 3, C_LETEXT);

    rep_header.put(1, 2, "From:");
    if (hasTo)
        rep_header.put(2, 2, "  To:");
    if (hasNet)
        rep_header.put(3, 2, "Addr:");
    rep_header.put(maxitems, 2, "Subj:");

    rep_header.attrib(C_LEGET2);
    areaconv_in(FROM);
    rep_header.put(1, 8, FROM, 69);
    areaconv_out(FROM);

    if (hasNet && !NM.isSet) {
        //NM.isInternet = mm.areaList->isInternet();
        TO[0] = '\0';
        current = 1;
    } else {
        if (hasTo) {
            rep_header.attrib(C_LEGET1);
            areaconv_in(TO);
            rep_header.put(2, 8, TO, 69);
            areaconv_out(TO);
        }
        if (hasNet) {
            rep_header.attrib(C_LEGET2);
            areaconv_in(NETADD);
            rep_header.put(3, 8, NETADD, 69);
            areaconv_out(NETADD);
        }
        current = maxitems - 1;
    }

    rep_header.update();

    do {
        result = HeaderLine(rep_header, (current == (maxitems - 1)) ?
            SUBJ : (((current == 2) && hasNet) ? NETADD : ((current &&
            hasTo) ? TO : FROM)), ((current == 1) && hasNews) ? 512 :
            ((current == (maxitems - 1)) ? mm.areaList->maxSubLen() :
            (((current == 2) && NM.isInternet) ? 72 :
            mm.areaList->maxToLen())), current + 1, (current & 1) ?
            C_LEGET1 : C_LEGET2);

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

    if (result) {
        int pmode = (!hasNet ? mm.areaList->hasPublic() : 0) +
                    ((mm.areaList->hasPrivate() &&
                    !mm.areaList->isUsenet()) ? 2 : 0);

        if (hasNet)
            NM = NETADD;

        switch (pmode) {
        case 1:
            privat = false;
            break;
        case 2:
            privat = true;
            break;
        case 3:
            if (!ui->WarningWindow("Make letter private?", privat ? 0 : noyes))
                privat = !privat;
        }
    }
    return result;
}

long LetterWindow::reconvert(const char *reply_filename)
{
    FILE *src, *dest;

    char *outname = mytmpnam();
    src = fopen(reply_filename, "rt");
    dest = fopen(outname, "wt");

    fseek(src, 0, SEEK_END);
    long replen = ftell(src);
    rewind(src);

    char *body = new char[((replen > MAXBLOCK) ? MAXBLOCK : replen) + 1];

    long totlen = replen;
    replen = 0;
    while (totlen > MAXBLOCK) {
        long blklen = (long) fread(body, 1, MAXBLOCK, src);
        body[blklen] = '\0';
        areaconv_out(body);
        replen += (long) fwrite(body, 1, blklen, dest);
        totlen -= blklen;
    }
    totlen = (long) fread(body, 1, totlen, src);
    fclose(src);

    body[totlen] = '\0';
    areaconv_out(body);
    while (body[totlen - 1] == '\n')
        totlen--;

    replen += (long) fwrite(body, 1, totlen, dest);
    fclose(dest);

    delete[] body;

    remove(reply_filename);
    rename(outname, reply_filename);

    delete[] outname;

    return replen;
}

void LetterWindow::setToFrom(char key, char *TO, char *FROM)
{
    char format[7];
    sprintf(format, "%%.%ds", mm.areaList->maxToLen());

    bool usealias = mm.areaList->getUseAlias();
    if (usealias) {
        const char *name = mm.packet->getAliasName();
        sprintf(FROM, format, name ? name : "");
        if (!FROM[0])
            usealias = false;
    }
    if (!usealias) {
        const char *name = mm.packet->getLoginName();
        sprintf(FROM, format, name ? name : "");
    }

    if (mm.areaList->isUsenet()) {
        const char *newsgrps = 0;

        if (key != 'E') {
            newsgrps = mm.letterList->getFollow();
            if (!newsgrps)
                newsgrps = mm.letterList->getNewsgrps();
        }
        if (!newsgrps)
            newsgrps = mm.areaList->getDescription();

        sprintf(TO, "%.512s", newsgrps);
    } else
        if (key == 'E')
            strcpy(TO, (To ? To : "All"));
        else
            if (mm.areaList->isInternet()) {
                if (key == 'O') {
                    sprintf(TO, format, fromName(mm.letterList->getTo()));
                    NM = fromAddr(mm.letterList->getTo());
                } else {
                    const char *rep = mm.letterList->getReply();

                    sprintf(TO, format, mm.letterList->getFrom());
                    if (rep)
                        NM = fromAddr(rep);
                }
            } else
                sprintf(TO, format, (key == 'O') ? mm.letterList->getTo() :
                        mm.letterList->getFrom());
}

void LetterWindow::EnterLetter(int replyto_area, char key)
{
    FILE *reply;
    char FROM[74], TO[514], SUBJ[514];
    const char *orig_id;

    mystat fileStat;
    time_t oldtime;

    long replen, replyto_num;
    bool privat;

    mm.areaList->gotoArea(replyto_area);

    bool news = mm.areaList->isUsenet();
    bool inet = news || mm.areaList->isInternet();

    // HEADER

    setToFrom(key, TO, FROM);

    if (key == 'E')
        SUBJ[0] = '\0';  //we don't have subject yet
    else {
        const char *s = stripre(mm.letterList->getSubject());
        int len = strlen(s);

        bool useRe = (inet || mm.resourceObject->getInt(ReOnReplies))
                     && ((len + 4) <= mm.areaList->maxSubLen());
        sprintf(SUBJ, useRe ? "Re: %.509s" : "%.512s", s);
    }

    privat = (key == 'E') ? false : mm.letterList->getPrivate();

    if (!EnterHeader(FROM, TO, SUBJ, privat)) {
        NM.isSet = false;
        delete[] To;
        To = 0;
        ui->areas.Select();
        ui->redraw();
        return;
    }

    // Don't use refnum if posting in different area:

    replyto_num = ((key == 'E') || (key == 'N') || (replyto_area !=
        mm.letterList->getAreaID())) ? 0 : mm.letterList->getMsgNum();

    orig_id = replyto_num ? mm.letterList->getMsgID() : 0;

    // BODY

    char *reply_filename = mytmpnam();

    // Quote the old text

    if (key != 'E') {
        reply = fopen(reply_filename, "wt");
        QuoteText(reply);
        fclose(reply);
    }
    fileStat.init(reply_filename);
    oldtime = fileStat.fdate();

    // Edit the reply

    edit(reply_filename);
    ui->areas.Select();
    ui->redraw();

    // Check if modified

    fileStat.init(reply_filename);
    if (fileStat.fdate() == oldtime)
        if (ui->WarningWindow("Cancel this letter?")) {
            remove(reply_filename);
            ui->redraw();
            return;
        }

    // Mark original as replied

    if (key != 'E') {
        int origatt = mm.letterList->getStatus();
        mm.letterList->setStatus(origatt | MS_REPLIED);
        if (!(origatt & MS_REPLIED))
            ui->setAnyRead();
    }

    reply = fopen(reply_filename, "at");

    // Signature

    bool sigset = false;

    const char *sg = mm.resourceObject->get(sigFile);
    if (sg && *sg) {
        FILE *s = fopen(sg, "rt");
        if (s) {
            int c;

            fputs(inet ? "\n-- \n" : "\n", reply);
            while ((c = fgetc(s)) != EOF)
                if (c != '\r')
                    fputc(c, reply);
            fclose(s);

            sigset = true;
        }
    }

    // Tagline

    bool useTag = mm.resourceObject->getInt(UseTaglines) &&
                  ui->taglines.NumOfItems() && ui->Tagwin();
    if (useTag)
        fprintf(reply, inet ? (sigset ? "\n%s\n" : "\n-- \n%s\n") :
                "\n... %s\n", ui->taglines.getCurrent());
    else
        if (!inet)
            fprintf(reply, " \n");

    // Tearline (not for Blue Wave -- it does its own)

    if (!inet) {
        const char *tear = mm.areaList->getTear();

        if (tear)
            fprintf(reply, "%s\n", tear);
    }

    fclose(reply);

    // Reconvert the text

    mm.areaList->gotoArea(replyto_area);
    replen = reconvert(reply_filename);

    mm.areaList->enterLetter(replyto_area, FROM, news ? "All" : TO,
        SUBJ, orig_id, news ? TO : 0, replyto_num, privat, NM,
        reply_filename, replen);

    delete[] reply_filename;

    NM.isSet = false;
    delete[] To;
    To = 0;

    ui->areas.Select();
    ui->setUnsaved();
    ui->redraw();
}

void LetterWindow::forward_header(FILE *fd, const char *FROM,
    const char *TO, const char *SUBJ, int replyto_area, bool is_reply)
{
    enum {area, date, from, to, subj, items};
    static const char *names[items] = {"ly in", "ly on", "ly by",
                                       "ly to", " subj"};
    char Header[512], *p;
    int j;

    int oldarea = mm.letterList->getAreaID();
    mm.areaList->gotoArea(mm.letterList->getAreaID());

    const char *head[items] = {
        mm.areaList->getDescription(), mm.letterList->getDate(),
        mm.letterList->getFrom(), mm.letterList->getTo(),
        mm.letterList->getSubject()
    };

    const char *org[items] = {
        0, 0, FROM, TO, SUBJ
    };

    bool use[items];

    use[area] = (oldarea != replyto_area);
    use[date] = !is_reply;
    for (j = from; j < items; j++)
        use[j] = !(!strcasecmp(org[j], head[j]));

    ui->areas.Select();

    bool anyused = false;

    for (j = 0; j < items; j++)
        if (use[j]) {
            p = Header + sprintf(Header, "%.511s", head[j]);
            if (((j == from) && mm.areaList->isEmail())
                || ((j == to) && mm.areaList->isReplyArea()))

                netAdd(p);
            letterconv_in(Header);
            fprintf(fd, " * Original%s: %s\n", names[j], Header);
            anyused = true;
        }

    if (anyused)
        fprintf(fd, "\n");
}

void LetterWindow::EditLetter(bool forwarding)
{
    FILE *reply;
    char FROM[74], TO[514], SUBJ[514];
    long siz;
    int replyto_num, replyto_area;
    bool privat, newsflag = false;

    bool is_reply_area = (mm.areaList->getAreaNo() == REPLY_AREA);

    NM = mm.letterList->getNetAddr();

    replyto_area = ui->areaMenu();
    if (replyto_area == -1)
        return;

    // The refnum is only good for the original area:

    replyto_num = (replyto_area != mm.letterList->getAreaID()) ?
        0 : mm.letterList->getReplyTo();

    char *msgid = strdupplus(mm.letterList->getMsgID());

    mm.areaList->gotoArea(replyto_area);

    if (forwarding) {
        if (mm.areaList->isEmail()) {
            ui->areas.Select();
            ui->addressbook();
            mm.areaList->gotoArea(replyto_area);
        } else {
            NM.isSet = false;
            newsflag = mm.areaList->isUsenet();
        }
        setToFrom('E', TO, FROM);
        sprintf(SUBJ, "%.513s", mm.letterList->getSubject());
        privat = false;
    } else {
        const char *newsgrps = mm.letterList->getNewsgrps();
        newsflag = !(!newsgrps);

        strcpy(FROM, mm.letterList->getFrom());
        sprintf(TO, "%.512s", newsflag ? newsgrps : mm.letterList->getTo());
        sprintf(SUBJ, "%.512s", mm.letterList->getSubject());
        privat = mm.letterList->getPrivate();
    }

    if (!EnterHeader(FROM, TO, SUBJ, privat)) {
        NM.isSet = false;
        ui->areas.Select();
        ui->redraw();
        return;
    }

    DestroyChain();  // current letter's chain reset

    char *reply_filename = mytmpnam();

    reply = fopen(reply_filename, "wt");
    if (forwarding)
        forward_header(reply, FROM, TO, SUBJ, replyto_area, is_reply_area);

    letter_body *msgBody = mm.letterList->getBody();

    while (msgBody) {
        char *body = msgBody->getText();
        int offset = 0;

        letterconv_in(body);

        // Can't use MakeChain here, or it will wrap the text; so:
        if (!hidden)  // skip hidden lines at start
            while (*body == 1) {
                do {
                    body++;
                    offset++;
                } while (*body && (*body != '\n'));
                while (*body == '\n') {
                    body++;
                    offset++;
                }
            }

            fwrite(body, msgBody->getLength() - offset, 1, reply);

            msgBody = msgBody->next;
    }
    fclose(reply);

    edit(reply_filename);
    siz = reconvert(reply_filename);

    if (!forwarding)
        mm.areaList->killLetter(mm.letterList->getAreaID(),
                                mm.letterList->getMsgNum());

    mm.areaList->enterLetter(replyto_area, FROM, newsflag ? "All" :
        TO, SUBJ, msgid, newsflag ? TO : 0, replyto_num, privat,
        NM, reply_filename, siz);

    delete[] reply_filename;
    delete[] msgid;

    if (is_reply_area) {
        mm.letterList->rrefresh();
        ui->letters.ResetActive();
    }
    ui->areas.Select();

    NM.isSet = false;

    ui->redraw();
    ui->setUnsaved();
}

bool LetterWindow::SplitLetter(int lines)
{
    static int eachmax = mm.resourceObject->getInt(MaxLines);

    if (!lines) {
        char maxlinesA[55];

        sprintf(maxlinesA, "%d", eachmax);
        if (!ui->savePrompt(
            "Max lines per part? (WARNING: Split is not reversible!)",
            maxlinesA) || !sscanf(maxlinesA, "%d", &eachmax))

            return false;
    }
    int maxlines = lines ? lines : eachmax;

    if (maxlines < 20) {
        ui->nonFatalError("Split at less than 20 lines not allowed");
        return false;
    }

    MakeChain(80);
    int orglines = NumOfLines;
    int parts = (orglines - 1) / maxlines + 1;

    if (parts == 1)
        return false;

    NM = mm.letterList->getNetAddr();

    int replyto_area = mm.letterList->getAreaID();
    int replyto_num = mm.letterList->getReplyTo();

    char ORGSUBJ[514], *from, *to, *msgid, *newsgrps;

    from = strdupplus(mm.letterList->getFrom());
    to = strdupplus(mm.letterList->getTo());
    msgid = replyto_num ? strdupplus(mm.letterList->getMsgID()) : 0;
    newsgrps = strdupplus(mm.letterList->getNewsgrps());

    sprintf(ORGSUBJ, "%.510s", mm.letterList->getSubject());

    int x = parts;
    int padsize = 1;
    while (x /= 10)
        padsize++;

    bool privat = mm.letterList->getPrivate();

    int clines = 0;

    mm.areaList->killLetter(replyto_area, mm.letterList->getMsgNum());

    for (int partno = 1; partno <= parts; partno++) {
        FILE *reply;
        char SUBJ[514];

        char *reply_filename = mytmpnam();

        int endline = (orglines > maxlines) ? maxlines: orglines;

        unsigned long replen = 0;
        reply = fopen(reply_filename, "wt");
        for (int i = clines; i < (clines + endline); i++) {
            linelist[i]->out(reply);
            replen += linelist[i]->length + 1;
        }
        fclose(reply);

        sprintf(SUBJ, "%s (%0*d/%d)", ORGSUBJ, padsize, partno, parts);

        mm.areaList->enterLetter(replyto_area, from, to, SUBJ, msgid,
            newsgrps, replyto_num, privat, NM, reply_filename, (long) replen);

        delete[] reply_filename;

        clines += endline;
        orglines -= endline;
    }
    delete[] newsgrps;
    delete[] msgid;
    delete[] to;
    delete[] from;

    NM.isSet = false;

    mm.letterList->rrefresh();

    if (!lines) {
        ui->letters.ResetActive();
        ui->areas.Select();
        ui->setUnsaved();
    }
    return true;
}

void LetterWindow::GetTagline()
{
    ui->taglines.EnterTagline(tagline1);
    ReDraw();
}

bool LetterWindow::EditOriginal()
{
    int old_area = mm.letterList->getAreaID();
    long old_mnum = mm.letterList->getMsgNum();
    int orig_area = mm.areaList->getAreaNo();
    int orig_mnum = mm.letterList->getCurrent();
    int oldPos = position;
    position = 0;

    letter_list *old_list = mm.letterList;
    mm.areaList->gotoArea(REPLY_AREA);
    mm.areaList->getLetterList();
    ui->areas.ResetActive();

    bool found = mm.letterList->findReply(old_area, old_mnum);

    if (found && ui->WarningWindow("A reply exists. Re-edit it?"))
        EditLetter(false);
    else
        found = false;

    position = oldPos;
    delete mm.letterList;
    mm.letterList = old_list;
    mm.areaList->gotoArea(orig_area);
    ui->areas.ResetActive();
    mm.letterList->gotoLetter(orig_mnum);
    ui->letters.ResetActive();

    return found;
}

void LetterWindow::SplitAll(int lines)
{
    letter_list *old_list = 0;
    statetype state = ui->active();
    bool list_is_active = (state == letter) || (state == letterlist);

    ui->areas.Select();
    int orig_area = mm.areaList->getAreaNo();

    bool is_reply_area = (orig_area == REPLY_AREA) && list_is_active;

    if (!is_reply_area) {
        if (list_is_active)
            old_list = mm.letterList;

        mm.areaList->gotoArea(REPLY_AREA);
        mm.areaList->getLetterList();
        ui->areas.ResetActive();
    }

    bool anysplit = false;
    int last = mm.letterList->noOfActive();
    for (int i = 0; i < last; i++) {
        mm.letterList->gotoActive(i);
        if (SplitLetter(lines)) {
            i--;
            last--;
            anysplit = true;
        }
    }

    if (is_reply_area)
        ui->letters.ResetActive();
    else {
        delete mm.letterList;
        if (list_is_active)
            mm.letterList = old_list;
    }
    mm.areaList->gotoArea(orig_area);
    ui->areas.ResetActive();

    if ((state == letter) && !is_reply_area)
        MakeChain(COLS);

    if (anysplit)
        ui->nonFatalError("Some replies were split");
}
