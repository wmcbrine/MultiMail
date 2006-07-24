/*
 * MultiMail offline mail reader
 * miscellaneous routines (global)

 Copyright (c) 1996 Toth Istvan <stoty@vma.bme.hu>
 Copyright (c) 2006 William McBrine <wmcbrine@users.sourceforge.net>,
                    Peter Karlsson <peter@softwolves.pp.se>

 Distributed under the GNU General Public License.
 For details, see the file COPYING in the parent directory. */

#include "mmail.h"

// get a little-endian short, return an int
unsigned getshort(const unsigned char *x)
{
	return ((unsigned) x[1] << 8) + (unsigned) x[0];
}

// get a little-endian long
unsigned long getlong(const unsigned char *x)
{
	return ((unsigned long) x[3] << 24) + ((unsigned long) x[2] << 16) +
		((unsigned long) x[1] << 8) + (unsigned long) x[0];
}

// get a big-endian long
unsigned long getblong(const unsigned char *x)
{
	return ((unsigned long) x[0] << 24) + ((unsigned long) x[1] << 16) +
		((unsigned long) x[2] << 8) + (unsigned long) x[3];
}

// put an int into a little-endian short
void putshort(unsigned char *dest, unsigned source)
{
	dest[0] = source & 0xff;
	dest[1] = (source & 0xff00) >> 8;
}

// put a long into a little-endian long
void putlong(unsigned char *dest, unsigned long source)
{
	dest[0] = source & 0xff;
	dest[1] = (source & 0xff00) >> 8;
	dest[2] = (source & 0xff0000) >> 16;
	dest[3] = (source & 0xff000000) >> 24;
}

// put a long into a big-endian long
void putblong(unsigned char *dest, unsigned long source)
{
	dest[0] = (source & 0xff000000) >> 24;
	dest[1] = (source & 0xff0000) >> 16;
	dest[2] = (source & 0xff00) >> 8;
	dest[3] = source & 0xff;
}

// convert MS-DOS-style date/time to struct tm
struct tm *getdostime(unsigned long packed)
{
	static struct tm unpacked;

	unpacked.tm_mday = packed & 0x1f;
	packed >>= 5;
	unpacked.tm_mon = (packed & 0x0f) - 1;
	packed >>= 4;
	unpacked.tm_year = (packed & 0x7f) + 80;
	packed >>= 7;

	unpacked.tm_sec = (packed & 0x1f) << 1;
	packed >>= 5;
	unpacked.tm_min = packed & 0x3f;
	packed >>= 6;
	unpacked.tm_hour = packed & 0x1f;

	return &unpacked;
}

// convert struct tm to MS-DOS-style date/time
unsigned long mkdostime(struct tm *unpacked)
{
	unsigned long packed;

	packed = unpacked->tm_hour;
	packed <<= 6;
	packed |= unpacked->tm_min;
	packed <<= 5;
	packed |= unpacked->tm_sec >> 1;
	packed <<= 7;

	packed |= unpacked->tm_year - 80;
	packed <<= 4;
	packed |= unpacked->tm_mon + 1;
	packed <<= 5;
	packed |= unpacked->tm_mday;

	return packed;
}

// takes off the spaces from the end of a string
char *cropesp(char *st)
{
	char *p;

	for (p = st + strlen(st) - 1; (p > st) && (*p == ' '); p--);
	p[1] = '\0';
	return st;
}

// converts spaces to underline characters
char *unspace(char *source)
{
	for (unsigned c = 0; c < strlen(source); c++)
		if (source[c] == ' ')
			source[c] = '_';
	return source;
}

// allocate and copy a string
char *strdupplus(const char *original)
{
	char *tmp;

	if (original) {
		tmp = new char[strlen(original) + 1];
		strcpy(tmp, original);
	} else
		tmp = 0;

	return tmp;
}

// allocate but do not copy
char *strdupblank(const char *original)
{
	return original ? new char[strlen(original) + 1] : 0;
}

// tmp string = Path + filename
char *fullpath(const char *dir, const char *name)
{
	char *fp = new char[strlen(dir) + strlen(name) + 4];
	sprintf(fp, "%s/%s", dir, name);

	return fp;
}

// If there's a space in the string, return it quoted
char *quotespace(const char *pathname)
{
	char *result;
	const char *sp = strchr(pathname, ' ');

	if (sp) {
		result = new char[strlen(pathname) + 3];
		sprintf(result, "\"%s\"", pathname);
	} else
		result = strdupplus(pathname);

	return result;
}

const char *findBaseName(const char *fileName)
{
	const int maxbaselen = 30;
	int c, d;
	static char tmp[maxbaselen + 2];

	for (c = 0; (fileName[c] != '.') && (fileName[c]); c++);

	if (c > maxbaselen)
		c = maxbaselen;

	for (d = 0; d < c; d++)
		tmp[d] = tolower(fileName[d]);
	tmp[d] = '\0';

	return tmp;
}

// For consistency, no path should end in a slash:
char *fixPath(const char *path)
{
	char *tmp;

	int len = strlen(path);
	char d = path[len - 1];

	if ((d == '/') || (d == '\\')) {
		tmp = new char[len + 2];
		sprintf(tmp, "%s.", path); 
	} else
		tmp = strdupplus(path);

	return tmp;
}

int getNumExt(const char *fileName)
{
	int retval = -1;
	const char *lastp = strrchr(fileName, '.');

	if (lastp) {
		lastp++;
		if (strlen(lastp) == 3) {
			bool isnum = true;

			for (int x = 0; x < 3; x++)
				isnum = isnum && isdigit(lastp[x]);
			if (isnum)
				retval = atoi(lastp);
		}
	}
	return retval;
}

const char *stripre(const char *subject)
{
	while (!strncasecmp(subject, "re: ", 4))
		subject += 4;
	return subject;
}

// basically the equivalent of "strcasestr()", if there were such a thing
const char *searchstr(const char *source, const char *item, int slen)
{
	const char *s;
	char first[3], oldc = '\0';
	char *end = (-1 != slen) ? ((char *) source + slen) : 0;

	int ilen = strlen(item) - 1;
	bool found = false;

	first[0] = tolower(*item);
	first[1] = toupper(*item);
	first[2] = oldc;

	if (end) {
		oldc = *end;
		*end = '\0';
	}

	item++;

	do {
		s = strpbrk(source, first);
		if (s) {
			source = s + 1;
			found = !strncasecmp(source, item, ilen);
		}
	} while (s && !found && *source);

	if (end)
		*end = oldc;

	return found ? s : 0;
}

// Find the address in "Foo <foo@bar.baz>" or "foo@bar.baz (Foo)"
const char *fromAddr(const char *source)
{
	static char tmp[100];
	const char *index = source;

	while (*index) {
		if (*index == '"')
			do
				index++;
			while (*index && (*index != '"'));
		if ((*index == '<') ||
		    ((*index == '(') && (index > source)))
			break;
		if (*index)
			index++;
	}

	bool bracket = (*index == '<');
	const char *end = bracket ? strchr(index, '>') :
		index - (*index == '(');
	index = bracket ? (index + 1) : source;

	if (end) {
		int len = end - index;
		if (len > 99)
			len = 99;
		strncpy(tmp, index, len);
		tmp[len] = '\0';
		return tmp;
	}
	return source;
}

// Find the name portion of the address
const char *fromName(const char *source)
{
	static char tmp[100];
	const char *end = 0, *fr = source;

	while (*fr) {
		if (*fr == '"') {
			fr++;
			end = strchr(fr, '"');
			break;
		}
		if (*fr == '(') {
			fr++;
			end = strchr(fr, ')');
			break;
		}
		if ((*fr == '<') && (fr > source)) {
			end = fr - 1;
			if (*end != ' ')
				end++;
			fr = source;
			break;
		}
		if (*fr)
			fr++;
	}

	if (end) {
		int len = end - fr;
		if (len > 99)
			len = 99;
		strncpy(tmp, fr, len);
		tmp[len] = '\0';
		return tmp;
	}
	return source;
}

// Should a name be quoted in an address?
bool quoteIt(const char *s)
{
	bool flag = false;

	while (*s) {
		int c = toupper(*s++);
		if (!(((c >= 'A') && (c <= 'Z')) || (c == ' '))) {
			flag = true;
			break;
		}
	}
	return flag;
}

// MIME decoding for header lines (=?iso-8859-1?Q? and =?iso-8859-1?B?)
void headdec(const char *source, const char *cset, char *dest)
{
	// (c) Copyright 1999 Peter Karlsson
	// Modified by William McBrine
	// May be used in any way as long as this copyright is included
	// in the derived work.

	bool isqp, isb64;
	int c, b64buf, b64count;

	if (source)
	    for (isqp = isb64 = false, b64buf = b64count = 0; *source;
	      source++) {
		c = *source;

		if (isqp) {
		    if ('_' == c)		// QP space
			*dest++ = ' ';
		    else
			if (' ' == c) {		// QP end
				*dest++ = ' ';
				isqp = false;
			} else
				if ('?' == c && '=' == source[1]) {
					source++;
					isqp = false;
				} else {
					if ('=' == c) {	// QP escape
						char hex[3] = "00";

						hex[0] = *++source;
						hex[1] = *++source;
						sscanf(hex, "%x",
						    (unsigned *) &c);
					}
					*dest++ = c;
				}
		} else
		    if (isb64) {
			if ('?' == c && '=' == source[1]) {
				source++;
				isb64 = false;
			} else {
				// Update base64 buffer
				static const char *base64 =
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"	// 0-25
				"abcdefghijklmnopqrstuvwxyz"	// 26-51
				"0123456789+/=";		// 52-63 + pad

				int b64val;
				const char *p = strchr(base64, c);

				b64val = p ? (p - base64) : 0;

				if (64 == b64val)
					b64val = 0;

				b64buf = (b64buf << 6) | b64val;
				b64count++;

				switch (b64count) {
				case 2:
					c = (b64buf & 0xff0) >> 4;
					if (c)
						*dest++ = c;
					b64buf &= 0x0f;
					break;
				case 3:
					c = (b64buf & 0x3fc) >> 2;
					if (c)
						*dest++ = c;
					b64buf &= 0x03;
					break;
				case 4:
					if (b64buf)
						*dest++ = b64buf;
					b64buf = b64count = 0;
				}
			}

		    } else
			if ('=' == c && '?' == source[1]) {
			    int clen = strlen(cset);

			    if (!strncasecmp(source + 2, cset, clen)) {

				char d = toupper(source[3 + clen]);

				if (('Q' == d) || ('B' == d)) {
					source += 4 + clen;

					if ('Q' == d)
						isqp = true;
					else {
						b64buf = b64count = 0;
						isb64 = true;
					}
				} else
					*dest++ = c;
			    } else
				    *dest++ = c;
			} else
				*dest++ = c;
	    }
	*dest = '\0';
}

// write a string to a file in RFC 2047 (pseudo-QP) form
void headenc(const unsigned char *in, const char *cset, FILE *out)
{
	bool qp = false;

	while (*in) {
		if (!qp && (*in & 0x80)) {
			fprintf(out, "=?%s?Q?", cset);
			qp = true;
		}
		if (qp)
			switch (*in) {
			case ' ':
				if ('<' != in[1])
					fputc('_', out);
				else {
					fprintf(out, "?= ");
					qp = false;
				}
				break;
			case '\"':
				fprintf(out, "?=\"");
				qp = false;
				break;
			default:
				if (*in & 0x80 || '=' == *in ||
				    '?' == *in || '_' == *in)
					fprintf(out, "=%02x", *in);
				else
					fputc(*in, out);
			}
		else
			fputc(*in, out);

		in++;
	}
	if (qp)
		fprintf(out, "?=");
}

// decode quoted-printable text in the body of a message
unsigned char *qpdecode(unsigned char *source)
{
	unsigned char *dest = source;

	while (*source) {
		*dest = *source++;
		if (*dest == '=') {
			unsigned char hex[3];

			hex[0] = *source++;

			if (hex[0] == '\n')
				dest--;
			else {
				unsigned i;

				hex[1] = *source++;
				hex[2] = '\0';
				sscanf((char *) hex, "%x", &i);
				*dest = i;
			}
		}
		dest++;
	}
	return dest;
}

long qpdecode(FILE *source, FILE *dest)
{
	int c;
	long count = 0;

	while ((c = fgetc(source)) != EOF) {
		if ('=' == c) {
			unsigned char hex[3];

			hex[0] = fgetc(source);

			if (hex[0] != '\n') {
				unsigned i;

				hex[1] = fgetc(source);
				hex[2] = '\0';
				sscanf((char *) hex, "%x", &i);
				fputc(i, dest);
				count++;
			}
		} else {
			fputc(c, dest);
			count++;
		}
	}

	return count;
}

// write text to a file in Quoted-Printable form
void qpencode(FILE *src, FILE *dest)
{
	unsigned char buf[77];
	int c, col = 0, lastcol = 0;
	long lastsp = 0;

	while ((c = fgetc(src)) != EOF) {
		bool quotethis = (c & 0x80) || ('=' == c);

		if ((quotethis && (col > 72)) || (col > 74)) {

		    int d = fgetc(src);
		    ungetc(d, src);

		    if ((d != EOF) && (d != '\n')) {

			if (lastcol) {
				fseek(src, lastsp, SEEK_SET);
				col = lastcol + 1;
			} else	// Warning! Second ungetc() not guaranteed!
				ungetc(c, src);

			buf[col++] = '=';
			c = '\n';
		    }
		}

		if ('\n' == c) {
			buf[col] = '\0';
			fprintf(dest, "%s\n", buf);
			col = lastcol = 0;
		} else {
			if (quotethis) {
				sprintf((char *) buf + col, "=%02X", c);
				col += 3;
			} else {
				if (' ' == c) {
					lastcol = col;
					lastsp = ftell(src);
				}
				buf[col++] = c;
			}
		}
	}

	if (col) {
		buf[col] = '\0';
		fprintf(dest, "%s\n", buf);
	}
}

