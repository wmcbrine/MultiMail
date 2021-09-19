/*
 * MultiMail offline mail reader
 * driver_list

 Copyright 1996-1997 Toth Istvan <stoty@vma.bme.hu>
 Copyright 1998-2021 William McBrine <wmcbrine@gmail.com>,
                     Robert Vukovic <vrobert@uns.ns.ac.yu>
 Distributed under the GNU General Public License, version 3 or later. */

#include "bw.h"
#include "qwk.h"
#include "omen.h"
#include "soup.h"
#include "opx.h"

enum pktype {PKT_QWK, PKT_BW, PKT_OMEN, PKT_SOUP, PKT_OPX, PKT_UNDEF};

// ------------------------------------------------
// Virtual specific_driver and reply_driver methods
// ------------------------------------------------

specific_driver::~specific_driver()
{
}

reply_driver::~reply_driver()
{
}

// ------------------
// DriverList methods
// ------------------

driver_list::driver_list()
{
    pktype mode;
    file_list *wl = mm.workList;

    // This is the new way to set the packet type
    if (wl->exists("control.dat") && wl->exists("messages.dat"))
        mode = PKT_QWK;
    else
        if (wl->exists(".inf"))
            mode = PKT_BW;
        else
            if (wl->exists("brdinfo.dat"))
                mode = PKT_OPX;
            else
                if (wl->exists("areas"))
                    mode = PKT_SOUP;
                else
                    if (wl->exists("system"))
                        mode = PKT_OMEN;
                    else
                        mode = PKT_UNDEF;

    switch (mode) {
    case PKT_BW:
        driverList[1].driver = new bluewave();
        driverList[0].driver = new bwreply(driverList[1].driver);
        break;
    case PKT_QWK:
        driverList[1].driver = new qwkpack();
        driverList[0].driver = new qwkreply(driverList[1].driver);
        break;
    case PKT_OMEN:
        driverList[1].driver = new omen();
        driverList[0].driver = new omenrep(driverList[1].driver);
        break;
    case PKT_SOUP:
        driverList[1].driver = new soup();
        driverList[0].driver = new souprep(driverList[1].driver);
        break;
    case PKT_OPX:
        driverList[1].driver = new opxpack();
        driverList[0].driver = new opxreply(driverList[1].driver);
        break;
    default:
        driverList[1].driver = 0;
        driverList[0].driver = 0;
    }

    noOfDrivers = (mode != PKT_UNDEF) ? 2 : 0;

    if (noOfDrivers) {
        driverList[1].read = new main_read_class(driverList[1].driver);
        driverList[0].read = new reply_read_class(driverList[0].driver);
    } else {
        driverList[1].read = 0;
        driverList[0].read = 0;
    }
}

driver_list::~driver_list()
{
    while (noOfDrivers--) {
        delete driverList[noOfDrivers].read;
        delete driverList[noOfDrivers].driver;
    }
}

void driver_list::initRead()
{
    if (driverList[0].read)
        driverList[0].read->init();
    if (driverList[1].read)
        driverList[1].read->init();
}

int driver_list::getNoOfDrivers() const
{
    return noOfDrivers;
}

specific_driver *driver_list::getDriver(int areaNo)
{
    int c = (areaNo != REPLY_AREA);
    return driverList[c].driver;
}

reply_driver *driver_list::getReplyDriver()
{
    return (reply_driver *) driverList[0].driver;
}

read_class *driver_list::getReadObject(specific_driver *driver)
{
    int c = (driver == driverList[1].driver);
    return driverList[c].read;
}

int driver_list::getOffset(specific_driver *driver)
{
    return (driver == driverList[1].driver);
}
