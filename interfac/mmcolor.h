/*
 * MultiMail offline mail reader
 * color pairs #define'd here

 Copyright 1996-1997 John Zero <john@graphisoft.hu>
 Copyright 1997-2017 William McBrine <wmcbrine@gmail.com>
 Distributed under the GNU General Public License, version 3 or later. */

#ifndef MMCOLOR_H
#define MMCOLOR_H

#define COL(f, b) COLOR_PAIR(((f) << 3) + (b))
#define REVERSE(f, b) ((COL((f), (b))) | (A_REVERSE))

#define C_ANSIBACK COL(COLOR_WHITE, COLOR_BLACK)

enum coltype {
    C_SBACK,        //Start screen/backgnd
    C_SBORDER,      //Start/bdr
    C_SSEPBOTT,     //Start screen/bottom
    C_HELP1,        //Help desc.
    C_HELP2,        //Help keys
    C_HELP3,        //Help 2 bdr
    C_HELP4,        //Help 2 text
    C_WBORDER,      //Welcome border
    C_WELCOME1,     //Welcome prog name
    C_WELCOME2,     //Welcome auth names
    C_ADDR1,        //Add. backgnd
    C_ADDR2,        //Add. headers
    C_ADDR3,        //Address book/text
    C_WTEXT,        //Warn/text
    C_WTEXTHI,      //Warn/hilight
    C_LTEXT,        //Letter/text
    C_LQTEXT,       //Letter/quoted text
    C_LTAGLINE,     //Letter/tagline
    C_LTEAR,        //Letter/tear
    C_LHIDDEN,      //Letter/hidden
    C_LORIGIN,      //Letter/origin
    C_LBOTTSTAT,    //Letter/bottom statline
    C_LHEADTEXT,    //Letter/header text
    C_LHMSGNUM,     //msgnum
    C_LHFROM,       //from
    C_LHTO,         //to
    C_LHSUBJ,       //subject
    C_LHDATE,       //date
    C_LHFLAGSHI,    //flags high
    C_LHFLAGS,      //flags
    C_PBBACK,       //Packet/header
    C_PHEADTEXT,    //line text
    C_PLINES,       //Packet/lines
    C_LALBTEXT,     //Little area
    C_LALLINES,     //line text
    C_ALREPLINE,    //Area list/reply area
    C_ALPACKETLINE, //Area list/normal
    C_ALINFOTEXT,   //info win
    C_ALINFOTEXT2,  //filled text
    C_ALBTEXT,      //border text
    C_ALBORDER,     //border
    C_ALHEADTEXT,   //header text
    C_LETEXT,       //Letter text
    C_LEGET1,       //Letter/enter get1
    C_LEGET2,       //get2
    C_LLSAVEBORD,   //Letter/save border
    C_LLSAVETEXT,   //Letter/save
    C_LLSAVEGET,    //get
    C_LISTWIN,      //Letter list/top text1
    C_LLPERSONAL,   //Letter list/personal
    C_LLBBORD,      //Letter list
    C_LLTOPTEXT1,   //top text1
    C_LLTOPTEXT2,   //areaname
    C_LLHEAD,       //headers
    C_TBBACK,       //Tagline
    C_TTEXT,        //Tagline/text
    C_TKEYSTEXT,    //key select
    C_TENTER,       //Tagline/enter
    C_TENTERGET,    //enter get
    C_TLINES,       //lines
    C_SHADOW,       //All black!
    numColors
};

chtype emph(coltype);
chtype noemph(coltype);

extern const chtype *ColorArray;

#endif
