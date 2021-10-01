/*
 * MultiMail offline mail reader
 * mmail class

 Copyright 1996 Toth Istvan <stoty@vma.bme.hu>
 Copyright 1998-2021 William McBrine <wmcbrine@gmail.com>,
                     Robert Vukovic <vrobert@uns.ns.ac.yu>
 Distributed under the GNU General Public License, version 3 or later. */

#include "mmail.h"
#include "compress.h"
#include "../interfac/error.h"

#include "bw.h"
#include "qwk.h"
#include "omen.h"
#include "soup.h"
#include "opx.h"

enum pktype {PKT_QWK, PKT_BW, PKT_OMEN, PKT_SOUP, PKT_OPX, PKT_UNDEF};

void mmail::detect_and_open()
{
    pktype mode;

    if (workList->exists("control.dat") && workList->exists("messages.dat"))
        mode = PKT_QWK;
    else
        if (workList->exists(".inf"))
            mode = PKT_BW;
        else
            if (workList->exists("brdinfo.dat"))
                mode = PKT_OPX;
            else
                if (workList->exists("areas"))
                    mode = PKT_SOUP;
                else
                    if (workList->exists("system"))
                        mode = PKT_OMEN;
                    else
                        mode = PKT_UNDEF;

    switch (mode) {
    case PKT_BW:
        packet = new bluewave();
        reply = new bwreply();
        break;
    case PKT_QWK:
        packet = new qwkpack();
        reply = new qwkreply();
        break;
    case PKT_OMEN:
        packet = new omen();
        reply = new omenrep();
        break;
    case PKT_SOUP:
        packet = new soup();
        reply = new souprep();
        break;
    case PKT_OPX:
        packet = new opxpack();
        reply = new opxreply();
        break;
    default:
        packet = 0;
        reply = 0;
    }

    if (mode != PKT_UNDEF) {
        packet_read = new main_read_class(packet);
        reply_read = new reply_read_class(reply);
    } else {
        packet_read = 0;
        reply_read = 0;
    }
}

void mmail::Delete()
{
    if (packet) {
        delete reply_read;
        delete packet_read;
        delete reply;
        delete packet;
    }

    delete areaList;
    delete workList;
}

// Open a packet
pktstatus mmail::selectPacket(const char *packetName)
{
    pktstatus result;

    const char *x = strrchr(packetName, '/');
    if (!x)
        x = strrchr(packetName, '\\');
    if (x) {
        size_t len = x - packetName;
        char *fname = new char[len + 1];
        strnzcpy(fname, packetName, len);

        mychdir(error.getOrigDir());
        mychdir(fname);
        delete[] fname;

        fname = mygetcwd();
        res.set_noalloc(PacketDir, fname);
        packetName = x + 1;
    }
    res.set(PacketName, packetName);

    // Uncompression is done here
    char *fpath = fullpath(res.get(PacketDir), packetName);

    if (!res.get(oldPacketName) ||
        strcmp(packetName, res.get(oldPacketName))) {

        res.set(oldPacketName, packetName);
        result = uncompressFile(fpath, res.get(WorkDir), true);
        if (result != PKT_OK)
            return result;
    }

    delete[] fpath;

    workList = new file_list(res.get(WorkDir));

    if (!workList->getNoOfFiles()) {
        delete workList;
        return PKT_NOFILES;
    }

    detect_and_open();

    if (!packet) {
        delete workList;
        return PTYPE_UNK;
    }
    return PKT_OK;
}

// Save last read pointers
bool mmail::saveRead()
{
    return mm.getReadObject(packet)->saveAll();
}

// Is there a reply packet?
bool mmail::checkForReplies()
{
    return reply->checkForReplies();
}

// Create a reply packet
bool mmail::makeReply()
{
    return reply->makeReply();
}

void mmail::deleteReplies()
{
    reply->deleteReplies();

    // to reset the "replyExists" flag (inelegant, I know):
    checkForReplies();
}

void mmail::openReply()
{
    reply->init();
}

bool mmail::getOffConfig()
{
    return reply->getOffConfig();
}

void mmail::initRead()
{
    if (reply_read)
        reply_read->init();
    if (packet_read)
        packet_read->init();
}

specific_driver *mmail::getDriver(int areaNo)
{
    return (areaNo != REPLY_AREA) ? packet : reply;
}

read_class *mmail::getReadObject(specific_driver *driver)
{
    return (driver == packet) ? packet_read : reply_read;
}

int mmail::getOffset(specific_driver *driver)
{
    return (driver == packet);
}
