/*
 * MultiMail offline mail reader
 * tagline selection, editing

 Copyright 1996-1997 Kolossvary Tamas <thomas@vma.bme.hu>
 Copyright 1997-2017 William McBrine <wmcbrine@gmail.com>
 Distributed under the GNU General Public License, version 3 or later. */

#include "interfac.h"

extern "C" int tnamecmp(const void *a, const void *b)
{
    int d;

    const char *p = (*((tagline **) a))->text;
    const char *q = (*((tagline **) b))->text;

    d = strcasecmp(p, q);
    if (!d)
        d = strcmp(q, p);

    return d;
}

tagline::tagline(const char *tag)
{
    if (tag)
        strncpy(text, tag, TAGLINE_LENGTH);
    killed = false;
    next = 0;
}

TaglineWindow::TaglineWindow()
{
    nodraw = true;
    sorted = false;
    NumOfTaglines = NumOfActive = 0;
    taglist = tagactive = 0;
    filter = 0;
}

TaglineWindow::~TaglineWindow()
{
    DestroyChain();
}

void TaglineWindow::MakeActive()
{
    int expmode = mm.resourceObject->getInt(ExpertMode);
    nodraw = false;

    list_max_y = LINES - (expmode ? 12 : 15);

    int xwidth = COLS - 4;
    if (xwidth > (TAGLINE_LENGTH + 2))
        xwidth = TAGLINE_LENGTH + 2;
    list_max_x = xwidth - 2;

    top_offset = 1;

    borderCol = C_TBBACK;

    char tmp[60];
    char *p = tmp + sprintf(tmp, "Taglines, %s", sorted ? "sorted" :
                            "unsorted");
    if (NumOfActive > list_max_y)
        p += sprintf(p, " (%d)", NumOfActive);
    if (filter)
        sprintf(p, " | %.20s", filter);

    list = new InfoWin(LINES - 10, xwidth, 5, borderCol, tmp, C_TTEXT,
                       expmode ? 2 : 5, top_offset);

    if (!expmode) {
        int x = xwidth / 3 + 1;
        int y = list_max_y + 1;

        list->attrib(C_TKEYSTEXT);
        list->horizline(y);
        list->put(++y, 2, "Q");
        list->put(y, x, "R");
        list->put(y, x * 2, "Enter");
        list->put(++y, 2, "K");
        list->put(y, x, "A");
        //list->put(y, x * 2, " /, .");
        list->put(y, x * 2 + 4, "E");
        list->attrib(C_TTEXT);
        list->put(y, 3, ": Kill current tagline");
        list->put(y, x + 1, ": Add new tagline");
        //list->put(y, x * 2 + 5, ": search / next");
        list->put(y, x * 2 + 5, ": Edit tagline");
        list->put(--y, 3, ": don't apply tagline");
        list->put(y, x + 1, ": Random select tagline");
        list->put(y, x * 2 + 5, ": apply tagline");
    }
    DrawAll();
}

void TaglineWindow::Delete()
{
    delete list;
    nodraw = true;
}

bool TaglineWindow::extrakeys(int key)
{
    switch (key) {
    case 'A':
        EnterTagline();
        break;
    case 'E':
        EditTagline();
        break;
    case 'R':
        RandomTagline();
        break;
    case MM_DEL:
    case 'K':
        if (highlighted)
            kill();
        break;
    case 'S':
    case '$':
        sorted = !sorted;
        MakeChain();
        Delete();
        MakeActive();
    }
    return false;
}

void TaglineWindow::setFilter(const char *item)
{
    delete[] filter;
    filter = strdupplus(item);
    MakeChain();
    if (!NumOfActive) {
        delete[] filter;
        filter = 0;
        MakeChain();
    }
}

void TaglineWindow::RandomTagline()
{
    int i = rand() / (RAND_MAX / NumOfActive);

    Move(KEY_HOME);
    for (int j = 1; j <= i; j++)
        Move(KEY_DOWN);
    DrawAll();
}

void TaglineWindow::EnterTagline(const char *tag)
{
    FILE *fd;
    char newtagline[TAGLINE_LENGTH + 1];
    int y;

    Move(KEY_END);
    if (NumOfActive >= list_max_y) {
        y = list_max_y;
        position++;
    } else
        y = NumOfActive + 1;
    active++;

    if (!nodraw) {
        NumOfActive++;
        Draw();
        NumOfActive--;
    } else {
        int xwidth = COLS - 4;
        if (xwidth > (TAGLINE_LENGTH + 2))
            xwidth = TAGLINE_LENGTH + 2;
        list = new InfoWin(5, xwidth, (LINES - 5) >> 1, C_TBBACK);
        list->attrib(C_TTEXT);
        list->put(1, 1, "Enter new tagline:");
        list->update();
    }

    strcpy(newtagline, tag ? tag : "");

    if (list->getstring(nodraw ? 2 : y, 1, newtagline, TAGLINE_LENGTH,
        C_TENTER, C_TENTERGET)) {

        cropesp(newtagline);

        if (newtagline[0]) {

            //check dupes; also move curr to end of list:
            bool found = false;

            curr = &head;
            while (curr->next && !found) {
                curr = curr->next;
                found = !strcmp(newtagline, curr->text);
            }

            if (!found) {
                curr->next = new tagline(newtagline);
                fd = fopen(tagname, "at");
                if (fd) {
                    fputs(newtagline, fd);
                    fputc('\n', fd);
                    fclose(fd);
                }
                NumOfTaglines++;

                MakeChain();
            } else
                ui->nonFatalError("Already in file");
        }
    }
    Move(KEY_END);

    if (!nodraw) {
        DrawAll();
        doupdate();
    } else
        list->update();
}

void TaglineWindow::EditTagline()
{
    char newtagline[TAGLINE_LENGTH + 1];

    strcpy(newtagline, getCurrent());
    if (list->getstring(active - position + 1, 1, newtagline,
        TAGLINE_LENGTH, C_TENTER, C_TENTERGET)) {

        cropesp(newtagline);
        if (newtagline[0])
            strcpy(tagactive[active]->text, newtagline);
    }
    WriteFile(false);
    Draw();
}

void TaglineWindow::kill()
{
    if (ui->WarningWindow("Remove this tagline?")) {
        if (position)
            position--;

        highlighted->killed = true;
        NumOfTaglines--;

        MakeChain();

        WriteFile(false);
    }
    Delete();
    MakeActive();
}

bool TaglineWindow::ReadFile()
{
    FILE *fd;
    char newtag[TAGLINE_LENGTH + 1];
    bool flag;

    fd = fopen(tagname, "rt");
    flag = !(!fd);

    if (flag) {
        char *end;

        curr = &head;
        do {
            end = myfgets(newtag, sizeof newtag, fd);

            if (end && (newtag[0] != '\n')) {
                if (*end == '\n')
                    *end = '\0';
                curr->next = new tagline(newtag);
                curr = curr->next;
                NumOfTaglines++;
            }
        } while (end);
        fclose(fd);
    }
    return flag;
}

void TaglineWindow::WriteFile(bool message)
{
    FILE *tagx;

    if (message)
        printf("Creating %s...\n", tagname);

    tagx = fopen(tagname, "wt");
    if (tagx) {
        for (int x = 0; x < NumOfTaglines; x++) {
            fputs(taglist[x]->text, tagx);
            fputc('\n', tagx);
        }
        fclose(tagx);
    }
}

void TaglineWindow::MakeChain()
{
    delete[] taglist;
    taglist = new tagline *[NumOfTaglines + 1];

    delete[] tagactive;
    tagactive = new tagline *[NumOfTaglines + 1];

    NumOfActive = 0;

    if (NumOfTaglines) {
        curr = head.next;
        int c = 0;
        while (curr) {
            if (!curr->killed) {
                taglist[c++] = curr;
                if (!filter || searchstr(curr->text, filter))
                    tagactive[NumOfActive++] = curr;
            }
            curr = curr->next;
        }

        if (sorted && (NumOfActive > 1))
            qsort(tagactive, NumOfActive, sizeof(tagline *), tnamecmp);
    }

    tagactive[NumOfTaglines] = 0;    // hack for EnterTagline
}

void TaglineWindow::DestroyChain()
{
    while (NumOfTaglines)
        delete taglist[--NumOfTaglines];
    delete[] taglist;
    delete[] tagactive;
}

void TaglineWindow::oneLine(int i)
{
    int z = position + i;
    curr = (z < NumOfActive) ? tagactive[z] : 0;

    if (z == active)
        highlighted = curr;

    sprintf(list->lineBuf, "%-*.*s", list_max_x, list_max_x,
            curr ? curr->text : " ");

    DrawOne(i, C_TLINES);
}

searchret TaglineWindow::oneSearch(int x, const char *item, int)
{
    return searchstr(tagactive[x]->text, item) ? True : False;
}

int TaglineWindow::NumOfItems()
{
    return NumOfActive;
}

// Create tagline file if it doesn't exist.
void TaglineWindow::Init()
{
    // Default taglines:
#include "tagline.h"

    tagname = mm.resourceObject->get(TaglineFile);

    bool useDefault = !ReadFile();

    if (useDefault) {
        curr = &head;
        for (const char **p = defaultTags; *p; p++) {
            curr->next = new tagline(*p);
            curr = curr->next;
            NumOfTaglines++;
        }
    }

    MakeChain();

    if (useDefault)
        WriteFile(true);
}

const char *TaglineWindow::getCurrent()
{
    return tagactive[active]->text;
}
