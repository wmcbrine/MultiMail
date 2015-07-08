/*
 * MultiMail offline mail reader
 * compress and decompress packets

 Copyright 1997 John Zero <john@graphisoft.hu>
 Copyright 1998-2015 William McBrine <wmcbrine@gmail.com>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#ifndef COMPRESS_H
#define COMPRESS_H

#include "mmail.h"

pktstatus uncompressFile(resource *, const char *, const char *, bool = false);
int compressAddFile(resource *, const char *, const char *, const char *);
const char *defExtent();

#endif
