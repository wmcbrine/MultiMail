/*
 * MultiMail offline mail reader
 * compress and decompress packets

 Copyright (c) 1997 John Zero <john@graphisoft.hu>
 Copyright (c) 2001 William McBrine <wmcbrine@users.sourceforge.net>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "compress.h"

enum atype {A_ARJ, A_ZIP, A_LHA, A_RAR, A_TAR, A_UNKNOWN, A_UNEXIST};

atype lastAType = A_UNKNOWN;	// saves last atype for compress routine

atype getArchiveType(const char *fname)
{
	FILE *f;
	unsigned magic;
	atype tip = A_UNKNOWN;

	f = fopen(fname, "rb");
	if (!f)
		return A_UNEXIST;
	magic = fgetc(f) << 8;
	magic += fgetc(f);

	switch (magic) {
	case 0x60EA:
		tip = A_ARJ;
		break;
	case 0x1F8B:
		tip = A_TAR;	// actually the GZIP signature
		break;
	case 0x504B:		// PK - check for ZIP
		if (3 == fgetc(f))
			if (4 == fgetc(f))
				tip = A_ZIP;
		break;
	case 0x5261:		// Ra - chech for RAR
		if ('r' == fgetc(f))
			if ('!' == fgetc(f))
				tip = A_RAR;
		break;
	default:		// can be LHA - check 3. and 4. bytes
		if ('-' == fgetc(f))
			if ('l' == fgetc(f))
				tip = A_LHA;
	// we should check another byte (l/z) but i'm lazy
	}
	fclose(f);

	return tip;
}

// clears the working directory and uncompresses the packet into it.
pktstatus uncompressFile(resource *ro, const char *fname,
	const char *todir, bool setAType)
{
	static const int uncstr[] = {
		arjUncompressCommand, zipUncompressCommand,
		lhaUncompressCommand, rarUncompressCommand,
		tarUncompressCommand, unknownUncompressCommand
	};
	clearDirectory(todir);

	atype at = getArchiveType(fname);
	if (at == A_UNEXIST)
		return PKT_UNFOUND;
	if (setAType)
		lastAType = at;

	return mysystem2(ro->get(uncstr[at]), fname) ? UNCOMP_FAIL : PKT_OK;
}

int compressAddFile(resource *ro, const char *arcdir, const char *arcfile, 
			const char *addfname)
{
	static const int cmpstr[] = {
		arjCompressCommand, zipCompressCommand,
		lhaCompressCommand, rarCompressCommand,
		tarCompressCommand, unknownCompressCommand
	};
	int result;

	char *filepath = fullpath(arcdir, arcfile);
	
	mystat st(filepath);

#ifdef TAR_KLUDGE
	// For tar files, forget the parameter passed -- just archive
	// everything. (Adding a single file would be a multi-step
	// process. You could do it via a script.)

	if (lastAType == A_TAR)
		addfname = "*";
#endif
	if (!st.readable() || st.writeable()) {
		const char *cm = ro->get(cmpstr[lastAType]);
		char *qname = canonize(quotespace(filepath));
		char *cmdline = new char[strlen(qname) + strlen(cm) +
			strlen(addfname) + 6];
		sprintf(cmdline, "%s %s %s", cm, qname, addfname);

		result = mysystem(cmdline);

		st.reset_date(filepath);
		
		if (lastAType == A_LHA) {	// then the fixup
			strcpy(filepath, arcfile);
			strtok(filepath, ".");
			sprintf(cmdline, "%s/%s.bak", arcdir, filepath);
			remove(cmdline);
		}
		
		delete[] cmdline;
		delete[] qname;
	} else
		result = -1;

	delete[] filepath;

	return result;
}

// mainly for use with OMEN replies
const char *defExtent()
{
	static const char *ext[] = {"arj", "zip", "lzh", "rar", "tgz", ""};

	return ext[lastAType];
}
