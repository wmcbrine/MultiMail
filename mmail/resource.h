/*
 * MultiMail offline mail reader
 * resource class

 Copyright 1996-1997 Toth Istvan <stoty@vma.bme.hu>
 Copyright 1997-2017 William McBrine <wmcbrine@gmail.com>
 Distributed under the GNU General Public License, version 3 or later. */

#ifndef RESOURCE_H
#define RESOURCE_H

enum {
    UserName, InetAddr, QuoteHead, InetQuote, outCharset, noOfRaw
};

enum {
    homeDir = noOfRaw, mmHomeDir, PacketDir, TempDir, BaseDir, WorkDir,
    UncompressCommand, PacketName, ReplyDir, CompressCommand, UpWorkDir,
    editor, SaveDir, AddressFile, TaglineFile, arjUncompressCommand,
    zipUncompressCommand, lhaUncompressCommand, rarUncompressCommand,
    tarUncompressCommand, unknownUncompressCommand, arjCompressCommand,
    zipCompressCommand, lhaCompressCommand, rarCompressCommand,
    tarCompressCommand, unknownCompressCommand, sigFile, ColorFile,
    oldPacketName, noOfStrings
};

enum {
    PacketSort = noOfStrings, AreaMode, LetterSort, LetterMode,
    Charset, UseTaglines, AutoSaveReplies, StripSoftCR, BeepOnPers,
    UseLynxNav, ReOnReplies, QuoteWrapCols, MaxLines, UseQPMailHead,
    UseQPNewsHead, UseQPMail, UseQPNews, ExpertMode, IgnoreNDX, Mouse,
#ifdef USE_SPAWNO
    swapOut,
#endif
    UseColors, Transparency, BackFill, ClockMode, noOfResources
};

class baseconfig
{
 protected:
    const char **names, **comments, **intro;
    int configItemNum;

    bool parseConfig(const char *);
    void newConfig(const char *);
    virtual void processOne(int, const char *) = 0;
    virtual const char *configLineOut(int) = 0;
 public:
    void processOneByName(const char *, const char *);
    virtual ~baseconfig();
};

class resource : public baseconfig
{
    static const char *rc_names[], *rc_intro[], *rc_comments[];
    static const int startUp[], defInt[];

    char *resourceData[noOfStrings];
    int resourceInt[noOfResources - noOfStrings];

    void homeInit();
    void mmEachInit(int, const char *);
    void subPath(int, const char *);
    void initinit();
    void mmHomeInit();
    void processOne(int, const char *);
    const char *configLineOut(int);
    bool checkPath(const char *, bool);
    bool verifyPaths();
 public:
    resource();
    ~resource();
    const char *get(int) const;
    int getInt(int) const;
    void set(int, const char *);
    void set_noalloc(int, char *);
    void set(int, int);
};

#endif
