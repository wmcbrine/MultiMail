/*
 * MultiMail offline mail reader
 * SOUP

 Copyright 1999-2015 William McBrine <wmcbrine@gmail.com>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#ifndef SOUP_H
#define SOUP_H

#include "pktbase.h"

class sheader {
    enum {date, from, to, reply, subject, newsgrps, follow, refs,
          msgid, items};
    static const char *compare[items];
    char *values[items];
 public:
    long msglen;
    bool has8bit, qpenc;

    sheader();
    ~sheader();
    bool init(FILE *);
    bool init(const char *, const char *, const char *, const char *,
              const char *, const char *, long);
    void output(FILE *, const char *, bool, bool);
    const char *From();
    const char *Subject();
    const char *Date();
    const char *ReplyTo();
    const char *To();
    const char *Newsgrps();
    const char *Follow();
    const char *Msgid();
    const char *Refs();
};

class soup : public pktbase
{
    struct AREAs {
        char *name;
        int nummsgs;
        unsigned long attr;
        char mode;
        char numA[10], msgfile[10];
        AREAs *next;
    } **areas;

    bool msgopen(int);
    bool parseFrom(const char *);
    void buildIndices();
    void readAreas();
 public:
    soup(mmail *);
    ~soup();
    area_header *getNextArea();
    int getNoOfLetters();
    letter_header *getNextLetter();
    letter_body *getBody(letter_header &);
    const char *getTear(int);
    bool isLatin();
};

class souprep : public pktreply
{
    class upl_soup : public upl_base
    {
     public:
        sheader sHead;
        net_address na;
        int origArea;
        long refnum;
        bool privat;

        upl_soup(const char * = 0);
    };
    bool getRep1(FILE *, upl_soup *);
    void getReplies(FILE *);
    void addRep1(FILE *, upl_base *, int);
    void addHeader(FILE *);
    void repFileName();
    const char *repTemplate(bool);
 public:
    souprep(mmail *, specific_driver *);
    ~souprep();
    area_header *getNextArea();
    letter_header *getNextLetter();
    void enterLetter(letter_header &, const char *, long);
    bool getOffConfig();
    bool makeOffConfig();
};

#endif
