/*
 * MultiMail offline mail reader
 * conversion tables ISO 8859-1 <-> IBM codepage 437

 Copyright 1997 Peter Karlsson <peter@softwolves.pp.se>,
                Toth Istvan <stoty@vma.bme.hu>
 Copyright 1998-2015 William McBrine <wmcbrine@gmail.com>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

/* Original tables by Peter Karlsson, modified by William McBrine after 
   DOSEmu's video/terminal.h, by Mark D. Rejhon. */

#include "interfac.h"

enum cdirtype {CC_ISOTO437, CC_437TOISO};

bool isoConsole;

const char *dos2isotab =
  "\307\374\351\342\344\340\345\347\352\353\350\357\356\354\304\305"
  "\311\346\306\364\366\362\373\371\377\326\334\242\243\245\120\146"
  "\341\355\363\372\361\321\252\272\277\055\254\275\274\241\253\273"
  ":%&|{{{..{I.'''.``+}-+}}`.**}=**+*+``..**'.#_][~"
  "a\337\254\266{\363\265t\330\364\326\363o\370En"
  "=\261><()\367=\260\267\267%\140\262= ";

const char *iso2dostab =
  "\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217"
  "\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237 "
  "\255\233\234$\235|\025\"c\246\256\252-r-\370\361\3753\'\346\024\371,1"
  "\370\257\254\253/\250AAAA\216\217\222\200E\220EEIIIID\245OOOO\231x"
  "\231UUU\232Y \341\205\240\203a\204\206\221\207\212\202\210\211\215\241"
  "\214\213 \244\225\242\223o\224\366\224\227\243\226\201y \230";

char *charconv(char *buf, cdirtype cdir)
{
    const char *ct = (cdir == CC_ISOTO437) ? iso2dostab : dos2isotab;

    for (char *p = buf; *p; p++) {
        unsigned char c = *p;
        if (c & 0x80)
            *p = ct[c & 0x7f];
    }
    return buf;
}

char *charconv_in(char *buf)
{
    return (isoConsole ? charconv(buf, CC_437TOISO) : buf);
}

char *charconv_out(char *buf)
{
    return (isoConsole ? charconv(buf, CC_ISOTO437) : buf);
}

char *letterconv_in(char *buf)
{
    return (mm.letterList->isLatin() ^ isoConsole) ?
           charconv(buf, isoConsole ? CC_437TOISO : CC_ISOTO437) : buf;
}

char *letterconv_out(char *buf)
{
    return (mm.letterList->isLatin() ^ isoConsole) ?
           charconv(buf, isoConsole ? CC_ISOTO437 : CC_437TOISO) : buf;
}

char *areaconv_in(char *buf)
{
    return (mm.areaList->isLatin() ^ isoConsole) ?
           charconv(buf, isoConsole ? CC_437TOISO : CC_ISOTO437) : buf;
}

char *areaconv_out(char *buf)
{
    return (mm.areaList->isLatin() ^ isoConsole) ?
           charconv(buf, isoConsole ? CC_ISOTO437 : CC_437TOISO) : buf;
}
