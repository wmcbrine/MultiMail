/*
 * MultiMail offline mail reader
 * help windows

 Copyright 1996-1997 Kolossvary Tamas <thomas@vma.bme.hu>
 Copyright 1997-2015 William McBrine <wmcbrine@gmail.com>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "interfac.h"

HelpWindow::HelpWindow()
{
    baseReset();
}

void HelpWindow::newHelpMenu(const char **keys, const char **func, int it)
{
    if (mm.resourceObject->getInt(ExpertMode))
        menu = 0;
    else {
        int x, y, z, end;

        items = it;

        menu = new Win(3, COLS - 2, LINES - 4, C_HELP2);

        midpos = (COLS / 2) - 6;
        endpos = COLS - 21;

        if (items < 10)
            end = items;
        else {
            end = base + 8;
            if (end > items)
                end = items;
        }
        for (z = base; z < end; z++) {
            if (keys[z]) {
                x = (z - base) / 3;
                switch ((z - base) % 3) {
                case 0:
                    y = 2;
                    break;
                case 1:
                    y = midpos;
                    break;
                default:
                    y = endpos;
                }
                menu->attrib(C_HELP1);
                menu->put(x, y, ": ");
                menu->put(x, y + 2, func[z]);

                y -= strlen(keys[z]);

                menu->attrib(C_HELP2);
                menu->put(x, y, keys[z]);
            }
        }
        if (items > 9) {
            menu->put(2, endpos - 1, "O");
            menu->attrib(C_HELP1);
            menu->put(2, endpos, ": Other functions");
        }
        menu->delay_update();
    }
}

void HelpWindow::h_packetlist()
{
    static const char *keys[] = {
        "Q", "Enter", "S, $",
        "K", "/, .", "G",
        "U", "R",

        "A", "^T", "^Z",
        "B", "Space, F", "^X",
        "T", "|, ^"
    }, *func[] = {
        "Quit", "select packet", "change Sort type", 
        "Kill packet", "search / next", "Go to directory",
         "Update list", "Rename packet",

        "Addressbook", "Tagline editor", "command shell",
        "alias for PgUp", "aliases for PgDn", "eXit now",
        "Touch file", "filter list"
    };

    newHelpMenu(keys, func, 16);
}

void HelpWindow::h_arealist()
{
    static const char *keys[] = {
        "Q", "Enter", "F2, !",
        "E", "S, Ins", "U, Del",
        "L", "-", "+"
    }, *func[] = {
        "back to packet list", "select area", "Make reply packet",
        "Enter letter in area", "Subscribe", "Unsubscribe",
       "all/subscribed/active", "prev non-empty", "next non-empty"
    };

    newHelpMenu(keys, func, 9);
}

void HelpWindow::h_letterlist()
{
    static const char *keys[] = {
        "L", "Enter", "$",
        "E",  "^F", "S",
        "U", "M",

        "A", "^T", "F2, !",
        "^E", "-", "+"
    }, *func[] = {
        "List all/unread/marked", "read letter", "change sort type",
        "Enter letter in area", "Forward letter", "Save (all/marked)",
        "Unread/read toggle", "Mark/unmark",

        "Addressbook", "Tagline editor", "make reply packet",
        "Enter from addressbook", "previous unread", "next unread"
    }, *repkeys[] = {
        "K", "Enter", "$",
        "E", "^F", "S",
        0, "^B",

        "A", "^T", "F2, !"
    }, *repfunc[] = {
        "Kill letter", "read letter", "change sort type", 
        "Edit letter", "Forward letter", "Save (all/marked)",
        0, "Break into parts",

        "Addressbook", "Tagline editor", "Make reply packet"
    };

    if (!mm.areaList->isReplyArea())
        newHelpMenu(keys, func, 14);
    else
        newHelpMenu(repkeys, repfunc, 11);
}

void HelpWindow::h_letter(bool isAnsi)
{
    enum {width = 60, citems = 18, regitems = 7, repitems = 3, ansitems = 12};

    static const char *common[citems] = {
        "S - Save letter",
        "A - Addressbook",
        "C - toggle Character set",
        "D - Decrypt (rot13) toggle",
        "X - eXtra (hidden) lines",
        "I - Ignore soft CRs toggle",
        "^T - Tagline editor",
        "^F - Forward letter",
        "F2, ! - Make reply packet",
        "V, ^V, ^A - ANSI viewer",
        "/ - start a search",
        ". - repeat last search",
        "- - previous letter",
        "+, Enter - next letter",
        "Space - page through area",
        "^Z - command shell",
        "Q - back to letter list",
        "^X - eXit " MM_NAME " now"
    }, *regular[] = {
        "E - Enter new letter (post)",
        "R - Reply (followup)",
        "O - reply to Original sender",
        "N - Netmail/Email reply",
        "T - Take tagline",
        "M - Mark/unmark letter",
        "U - Unread/read toggle"
    }, *reply[] = {
        "K - Kill letter",
        "E, R - [Re-]Edit letter",
        "^B - Break reply into parts"
    }, *ansi[] = {
        "S - Save to file",
        "C - toggle Character set",
        "V, A, ^A - Animate",
        "/ - start a search",
        ". - repeat last search",
        "Space - page down/next",
        "- - previous",
        "+ - next",
        "@ - toggle at-code parsing",
        "^V - toggle AVATAR parsing",
        "^B - toggle BSAVE parsing",
        "Q - Quit ANSI viewer"
    };

    int extras = isAnsi ? ansitems : mm.areaList->isReplyArea() ?
                 repitems : regitems;
    int usecommon = isAnsi ? 0 : citems;
    int height = ((extras + usecommon + 1) >> 1) + 4;

    menu = new ShadowedWin(height, width, (LINES - height) >> 1, C_HELP3);
    menu->attrib(C_HELP4);

    const char **extchar = isAnsi ? ansi : ((extras == 7) ? regular : reply);

    int x, line = 0;

    for (x = 0; x < extras; x++) {
        if (!(x & 1))
            line++;
        menu->put(line, (x & 1) ? (width >> 1) + 2 : 2, extchar[x]);
    }

    for (x = extras; x < usecommon + extras; x++) {
        if (!(x & 1))
            line++;
        menu->put(line, (x & 1) ? (width >> 1) + 2 : 2, common[x - extras]);
    }

    menu->put(line + 2, 2, "Plus the standard direction keys");

    menu->wtouch();
}

void HelpWindow::MakeActive()
{
    switch (ui->active()) {
    case ansi_help:
        h_letter(true);
        break;
    case letter_help:
        h_letter(false);
        break;
    case letterlist:
        h_letterlist();
        break;
    case arealist:
        h_arealist();
        break;
    case packetlist:
        h_packetlist();
    default:;
    }
}

void HelpWindow::Delete()
{
    switch (ui->active()) {
    case ansi_help:
    case letter_help:
        delete (ShadowedWin *) menu;
        break;
    case letterlist:
    case arealist:
    case packetlist:
        delete menu;
    default:;
    }
}

void HelpWindow::baseNext()
{
    if (items > 9) {
        base += 8;
        if (base > (items - 1))
            base = 0;
    }
    Delete();
    MakeActive();
}

void HelpWindow::baseReset()
{
    base = 0;
}
