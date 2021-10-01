/*
 * MultiMail offline mail reader
 * net_address class

 Copyright 2021 William McBrine <wmcbrine@gmail.com>
 Distributed under the GNU General Public License, version 3 or later. */

#include "mmail.h"

net_address::net_address()
{
    isSet = isInternet = false;
    zone = 0;
    inetAddr = 0;
}

net_address::net_address(net_address &x)
{
    isSet = isInternet = false;
    zone = 0;
    inetAddr = 0;
    copy(x);
}

net_address::~net_address()
{
    if (isSet && isInternet)
        delete[] inetAddr;
}

bool net_address::operator==(net_address &x)
{
    if (isInternet != x.isInternet)
        return false;

    if (isInternet)
        return !strcmp(inetAddr, x.inetAddr);
    else
        return (zone == x.zone) && (net == x.net) &&
               (node == x.node) && (point == x.point);
}

net_address &net_address::operator=(const char *source)
{
    isInternet = source ? !(!strchr(source, '@')) : false;

    if (isInternet) {
        delete[] inetAddr;
        inetAddr = strdupplus(source);
        isSet = true;
    } else {
        if (sscanf(source ? source : "", "%u:%u/%u.%u",
                   &zone, &net, &node, &point) == 3)
            point = 0;

        isSet = !(!zone);
    }

    return *this;
}

void net_address::copy(net_address &x)
{
    isSet = x.isSet;
    if (isSet) {
        isInternet = x.isInternet;
        if (isInternet)
            inetAddr = strdupplus(x.inetAddr);
        else {
            zone = x.zone;
            net = x.net;
            node = x.node;
            point = x.point;
        }
    }
}

net_address &net_address::operator=(net_address &x)
{
    copy(x);
    return *this;
}

net_address::operator const char *()
{
    static char netText[25];

    if (isSet)
        if (isInternet)
            return inetAddr;
        else
            if (point)
                sprintf(netText, "%u:%u/%u.%u", zone, net, node, point);
            else
                sprintf(netText, "%u:%u/%u", zone, net, node);
    else
        netText[0] = '\0';

    return netText;
}
