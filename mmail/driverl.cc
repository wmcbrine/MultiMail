/*
 * MultiMail offline mail reader
 * driver_list

 Copyright 1996-1997 Toth Istvan <stoty@vma.bme.hu>
 Copyright 1998-2017 William McBrine <wmcbrine@gmail.com>,
                     Robert Vukovic <vrobert@uns.ns.ac.yu>
 Distributed under the GNU General Public License, version 3 or later. */

#ifdef USE_BW
# include "bw.h"
#endif

#ifdef USE_QWK
# include "qwk.h"
#endif

#ifdef USE_OMEN
# include "omen.h"
#endif

#ifdef USE_SOUP
# include "soup.h"
#endif

#ifdef USE_OPX
# include "opx.h"
#endif

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

driver_list::driver_list(mmail *mm)
{
    pktype mode;
    file_list *wl = mm->workList;

    // This is the new way to set the packet type
#ifdef USE_QWK
    if (wl->exists("control.dat") && wl->exists("messages.dat"))
        mode = PKT_QWK;
    else
#endif
#ifdef USE_BW
        if (wl->exists(".inf"))
            mode = PKT_BW;
        else
#endif
#ifdef USE_OPX
            if (wl->exists("brdinfo.dat"))
                mode = PKT_OPX;
            else
#endif
#ifdef USE_SOUP
                if (wl->exists("areas"))
                    mode = PKT_SOUP;
                else
#endif
#ifdef USE_OMEN
                    if (wl->exists("system"))
                        mode = PKT_OMEN;
                    else
#endif
                        mode = PKT_UNDEF;

    switch (mode) {
#ifdef USE_BW
    case PKT_BW:
        driverList[1].driver = new bluewave(mm);
        driverList[0].driver = new bwreply(mm, driverList[1].driver);
        break;
#endif
#ifdef USE_QWK
    case PKT_QWK:
        driverList[1].driver = new qwkpack(mm);
        driverList[0].driver = new qwkreply(mm, driverList[1].driver);
        break;
#endif
#ifdef USE_OMEN
    case PKT_OMEN:
        driverList[1].driver = new omen(mm);
        driverList[0].driver = new omenrep(mm, driverList[1].driver);
        break;
#endif
#ifdef USE_SOUP
    case PKT_SOUP:
        driverList[1].driver = new soup(mm);
        driverList[0].driver = new souprep(mm, driverList[1].driver);
        break;
#endif
#ifdef USE_OPX
    case PKT_OPX:
        driverList[1].driver = new opxpack(mm);
        driverList[0].driver = new opxreply(mm, driverList[1].driver);
        break;
#endif
    default:
        driverList[1].driver = 0;
        driverList[0].driver = 0;
    }

    noOfDrivers = (mode != PKT_UNDEF) ? 2 : 0;

    if (noOfDrivers) {
        driverList[1].read = new main_read_class(mm, driverList[1].driver);
        driverList[0].read = new reply_read_class(mm, driverList[0].driver);
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
