/*
 * MultiMail offline mail reader
 * OPX

 Copyright 1999-2015 William McBrine <wmcbrine@gmail.com>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#ifndef OPX_H
#define OPX_H

#include "pktbase.h"
#include "opxstrct.h"

class opxpack : public pktbase
{
    ocfgHeader confhead;
    char *bulletins;

    char *pstrget(void *);
    void readBrdinfoDat();
    void buildIndices();

    void getblk(int, long &, long, unsigned char *&, unsigned char *&);
    void endproc(letter_header &);
 public:
    opxpack(mmail *);
    ~opxpack();
    area_header *getNextArea();
    letter_header *getNextLetter();
    ocfgHeader *offhead();
    const char *oldFlagsName();
    bool readOldFlags();
    bool saveOldFlags();
};

class opxreply : public pktreply
{
    class upl_opx : public upl_base {
     public:
        fidoHead rhead;
        net_address na;
        char *msgid;
        int area;

        upl_opx(const char * = 0);
        ~upl_opx();
    };

    int getArea(const char *);
    bool getRep1(const char *, upl_opx *);
    void getReplies(FILE *);
    const char *freeFileName(upl_opx *);
    void addRep1(FILE *, upl_base *, int);
    void addHeader(FILE *);
    void repFileName();
    const char *repTemplate(bool);
 public:
    opxreply(mmail *, specific_driver *);
    ~opxreply();
    area_header *getNextArea();
    letter_header *getNextLetter();
    void enterLetter(letter_header &, const char *, long);
    bool getOffConfig();
    bool makeOffConfig();
};

#endif
