/*
 * MultiMail offline mail reader
 * compress and decompress packets

 Copyright (c) 1997 John Zero <john@graphisoft.hu>
 Copyright (c) 1999 William McBrine <wmcbrine@users.sourceforge.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#ifndef COMPRESS_H
#define COMPRESS_H

#include "mmail.h"

pktstatus uncompressFile(resource *, const char *, const char *,
	bool = false);
int compressAddFile(resource *, const char *, const char *, const char *);
const char *defExtent();

#endif
