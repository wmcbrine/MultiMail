/*
 * MultiMail offline mail reader
 * color handling, and default colors

 Copyright 1998-2015 William McBrine <wmcbrine@gmail.com>,
                     Ingo Brueckl <ib@wupperonline.de>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "interfac.h"

chtype emph(coltype chA)
{
    chtype ch = ColorArray[chA];

    if (has_colors())
        switch (PAIR_NUMBER(ch) & 7) {
        case COLOR_BLACK:
        case COLOR_RED:
        case COLOR_BLUE:
            ch |= A_BOLD;
        }
    else
        ch |= A_BOLD;
    return ch;
}

chtype noemph(coltype chA)
{
    chtype ch = ColorArray[chA];

    if (has_colors())
        switch (PAIR_NUMBER(ch) & 7) {
        case COLOR_WHITE:
        case COLOR_YELLOW:
        case COLOR_GREEN:
        case COLOR_CYAN:
        case COLOR_MAGENTA:
            ch |= A_BOLD;
        }
    return ch;
}

chtype ColorClass::allcolors[numColors] = {
    COL(COLOR_WHITE, COLOR_BLACK),               //Start screen/backgnd
    COL(COLOR_BLUE, COLOR_BLACK) | A_BOLD,       //Start/bdr
    COL(COLOR_MAGENTA, COLOR_BLACK),             //Start screen/bottom

    COL(COLOR_WHITE, COLOR_BLACK) | A_BOLD,      //Help desc.
    COL(COLOR_YELLOW, COLOR_BLACK) | A_BOLD,     //Help keys
    COL(COLOR_WHITE, COLOR_BLUE) | A_BOLD,       //Help 2 bdr
    COL(COLOR_YELLOW, COLOR_BLUE) | A_BOLD,      //Help 2 text

    COL(COLOR_YELLOW, COLOR_BLUE) | A_BOLD,      //Welcome bdr
    COL(COLOR_YELLOW, COLOR_BLUE) | A_BOLD,      //Welcome prog name
    COL(COLOR_CYAN, COLOR_BLUE) | A_BOLD,        //Welcome auth names

    COL(COLOR_YELLOW, COLOR_BLUE) | A_BOLD,      //Add. backgnd
    COL(COLOR_GREEN, COLOR_BLUE) | A_BOLD,       //Add. headers
    COL(COLOR_CYAN, COLOR_BLUE) | A_BOLD,        //Address book/text

    COL(COLOR_WHITE, COLOR_RED) | A_BOLD,        //Warn/text
    COL(COLOR_YELLOW, COLOR_RED) | A_BOLD,       //Warn/hilight

    COL(COLOR_WHITE, COLOR_BLUE) | A_BOLD,       //Letter/text
    COL(COLOR_CYAN, COLOR_BLUE),                 //Letter/quoted text
    COL(COLOR_CYAN, COLOR_BLUE),                 //Letter/tagline
    COL(COLOR_GREEN, COLOR_BLUE) | A_BOLD,       //Letter/tear
    COL(COLOR_GREEN, COLOR_BLUE),                //Letter/hidden
    COL(COLOR_CYAN, COLOR_BLUE),                 //Letter/origin
    COL(COLOR_MAGENTA, COLOR_WHITE) | A_REVERSE, //Letter/bottom statline

    COL(COLOR_BLUE, COLOR_CYAN),                 //Letter/header text
    COL(COLOR_BLACK, COLOR_CYAN),                //msgnum
    COL(COLOR_BLACK, COLOR_CYAN),                //from
    COL(COLOR_BLACK, COLOR_CYAN),                //to
    COL(COLOR_BLACK, COLOR_CYAN),                //subject
    COL(COLOR_BLACK, COLOR_CYAN),                //date
    COL(COLOR_CYAN, COLOR_BLACK) | A_REVERSE,    //flags high
    COL(COLOR_WHITE, COLOR_CYAN),                //flags

    COL(COLOR_YELLOW, COLOR_BLUE) | A_BOLD,      //Packet/header
    COL(COLOR_GREEN, COLOR_BLUE) | A_BOLD,       //line text
    COL(COLOR_CYAN, COLOR_BLUE) | A_BOLD,        //Packet/lines

    COL(COLOR_WHITE, COLOR_BLUE) | A_BOLD,       //Little area
    COL(COLOR_WHITE, COLOR_BLUE) | A_BOLD,       //line text

    COL(COLOR_GREEN, COLOR_BLUE),                //Area list/reply area
    COL(COLOR_CYAN, COLOR_BLUE),                 //Area list/normal
    COL(COLOR_YELLOW, COLOR_BLUE) | A_BOLD,      //info win
    COL(COLOR_WHITE, COLOR_BLUE) | A_BOLD,       //filled text
    COL(COLOR_GREEN, COLOR_BLUE) | A_BOLD,       //border text
    COL(COLOR_YELLOW, COLOR_BLUE) | A_BOLD,      //border
    COL(COLOR_GREEN, COLOR_BLUE) | A_BOLD,       //header text

    COL(COLOR_WHITE, COLOR_BLUE) | A_BOLD,       //Letter text
    COL(COLOR_CYAN, COLOR_BLUE),                 //Letter/enter get1
    COL(COLOR_GREEN, COLOR_BLUE) | A_BOLD,       //get2
    COL(COLOR_WHITE, COLOR_RED) | A_BOLD,        //Letter/save border
    COL(COLOR_WHITE, COLOR_RED) | A_BOLD,        //Letter/save
    COL(COLOR_YELLOW, COLOR_RED) | A_BOLD,       //get

    COL(COLOR_WHITE, COLOR_BLUE),                //Letter list/top text1
    COL(COLOR_GREEN, COLOR_BLUE),                //Letter list/personal
    COL(COLOR_YELLOW, COLOR_BLUE) | A_BOLD,      //Letter list

    COL(COLOR_WHITE, COLOR_BLUE) | A_BOLD,       //top text1
    COL(COLOR_GREEN, COLOR_BLUE) | A_BOLD,       //areaname
    COL(COLOR_YELLOW, COLOR_BLUE) | A_BOLD,      //headers

    COL(COLOR_YELLOW, COLOR_BLUE) | A_BOLD,      //Tagline
    COL(COLOR_GREEN, COLOR_BLUE) | A_BOLD,       //Tagline/text
    COL(COLOR_GREEN, COLOR_BLUE) | A_BOLD,       //key select
    COL(COLOR_GREEN, COLOR_BLUE) | A_BOLD,       //Tagline/enter
    COL(COLOR_CYAN, COLOR_BLUE) | A_BOLD,        //enter get
    COL(COLOR_CYAN, COLOR_BLUE) | A_BOLD,        //lines

    COL(COLOR_WHITE, COLOR_WHITE) | A_BOLD       //All black!
};

const char *ColorClass::col_names[numColors] = {

    "Main_Back", "Main_Border", "Main_BottSeparator",
    "BottHelp_Descrip", "BottHelp_Keys", "Help_Border", "Help_Text",
    "Welcome_Border", "Welcome_Header", "Welcome_Text",
    "Address_Border", "Address_Descrip", "Address_List", "Warn_Text",
    "Warn_Keys", "Letter_Text", "Letter_Quoted", "Letter_Tagline",
    "Letter_Tearline", "Letter_Hidden", "Letter_Origin",
    "Letter_Border", "LH_Text", "LH_Msgnum", "LH_From", "LH_To",
    "LH_Subject", "LH_Date", "LH_FlagsHigh", "LH_Flags",
    "Packet_Border", "Packet_Header", "Packet_List",
    "LittleArea_Header", "LittleArea_List", "Area_Reply", "Area_List",
    "Area_InfoDescrip", "Area_InfoText", "Area_TopText", "Area_Border",
    "Area_Header", "HeadEdit_Text", "HeadEdit_Input1",
    "HeadEdit_Input2", "Save_Border", "Save_Header", "Save_Input",
    "LettList_Text", "LettList_Personal", "LettList_Border",
    "LettList_TopText", "LettList_Area", "LettList_Header",
    "Tag_Border", "Tag_Text", "Tag_Keys", "Tag_Input1", "Tag_Input2",
    "Tag_List", "Shadow"
};

const char *ColorClass::col_intro[] = {
 "---------------",
 "Color selection",
 "---------------",
 "",
 "The format is \"ItemName: <foreground>, <background>, <attribute>\"",
 "Colors are Black, Blue, Green, Cyan, Red, Magenta, Yellow, and White.",
 "Attributes are Bold or Reverse.",
 "",
 "If no color for ItemName is defined, the default will be used. (Defaults",
 "are shown below.) Lines beginning with '#' are commented out.",
 0
};

const char *ColorClass::col_comments[numColors] = {
    "Background colors", 0, 0,
    "Bottom help window", 0,
    "Pop-up help", 0,
    "Welcome window (vanity plate)", 0, 0,
    "Address book", 0, 0,
    "Warning window", 0,
    "Letter window", 0, 0, 0, 0, 0, 0,
    "Letter header", 0, 0, 0, 0, 0, 0, 0,
    "Packet list", 0, 0,
    "Little area list", 0,
    "Area list", 0, 0, 0, 0, 0, 0,
    "Header editor", 0, 0,
    "Save filename input", 0, 0,
    "Letter list", 0, 0, 0, 0, 0,
    "Taglines", 0, 0, 0, 0, 0,
    "Shadows"
};

const chtype ColorClass::mapped[] = {COLOR_BLACK, COLOR_BLUE, COLOR_GREEN,
    COLOR_CYAN, COLOR_RED, COLOR_MAGENTA, COLOR_YELLOW, COLOR_WHITE};

// analyze a <foreground color>,<background color>,<attribute> color string
chtype ColorClass::colorparse(const char *colorstring)
{
    static const char *const cnames[] = {"bla", "blu", "gre", "cya",
        "red", "mag", "yel", "whi", "bol", "rev"};

    static chtype c[] = {COLOR_WHITE, COLOR_BLACK};
    chtype att = A_NORMAL;

    char *pos = (char *) colorstring;

    for (int i = 0; (i < 3) && *pos; i++) {

        while (*pos == ' ' || *pos == '\t')
            pos++;

        for (int j = 0; j < 10; j++)
            if (!strncasecmp(pos, cnames[j], 3)) {
                switch (j) {
                case 8:
                    att = A_BOLD;
                    break;
                case 9:
                    att = A_REVERSE;
                    break;
                default:
                    c[i] = mapped[j];
                }
                break;
            }

        while (*pos && *pos != ',')
            pos++;
        if (*pos == ',')
            pos++;
    }

    // Swap white-on-white for black-on-black:
    if ((c[0] == COLOR_BLACK) && (c[1] == COLOR_BLACK))
        return COL(COLOR_WHITE, COLOR_WHITE) | att;
    else
        return COL(c[0], c[1]) | att;
}

void ColorClass::processOne(int c, const char *resValue)
{
    allcolors[c] = colorparse(resValue);
}

const char *ColorClass::configLineOut(int x)
{
    return decompose(allcolors[x]);
}

const char *ColorClass::findcol(chtype ch)
{
    static const char *const cnames[] = {"Black", "Blue", "Green",
        "Cyan", "Red", "Magenta", "Yellow", "White", ""};
    int x;
    for (x = 0; x < 8; x++)
        if (ch == mapped[x])
            break;
    return cnames[x];
}

const char *ColorClass::decompose(chtype ch)
{
    static char compost[26];
    chtype fg, bg, bold, rev;

    fg = PAIR_NUMBER(ch) >> 3;
    bg = PAIR_NUMBER(ch) & 7;
    bold = ch & A_BOLD;
    rev = ch & A_REVERSE;

    // Swap black-on-black for white-on-white:
    if ((fg == (COLOR_WHITE)) && (bg == (COLOR_WHITE)))
        fg = bg = COLOR_BLACK;

    char *p = compost;
    p += sprintf(p, "%s, %s", findcol(fg), findcol(bg));

    if (bold)
        sprintf(p, ", Bold");
    else if (rev)
        sprintf(p, ", Reverse");

    return compost;
}

void ColorClass::Init()
{
    const char *configname = mm.resourceObject->get(ColorFile);
    bool usecol = mm.resourceObject->getInt(UseColors);

    names = col_names;
    intro = col_intro;
    comments = col_comments;
    configItemNum = numColors;

    if (usecol)
        if (parseConfig(configname))
            newConfig(configname);

    ColorArray = allcolors;
}
