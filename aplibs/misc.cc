/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1994-1996  Alberto Pasquale                 */
/*                 Portions (C) Copyright 1999 Per Lundberg                  */
/*                                                                           */
/*                   A L L   R I G H T S   R E S E R V E D                   */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*   How to contact the author:  Alberto Pasquale of 2:332/504@fidonet       */
/*                               Viale Verdi 106                             */
/*                               41100 Modena                                */
/*                               Italy                                       */
/*                                                                           */
/*****************************************************************************/

#include "apgenlib.hpp"
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

char *newcpy (pcsz text)
{
    char *tmp = new char[strlen (text) + 1];
    strcpy (tmp, text);
    return tmp;
}


char *newncpy (pcsz text, size_t len)
{
    char *tmp = new char[len + 1];
    strncpy (tmp, text, len);
    tmp[len] = '\0';
    return tmp;
}


char *strzcpy (char *dest, const char *src, size_t maxlen)
{
    *dest = '\0';
    return strncat (dest, src, maxlen-1);
}


char *stpcpy (char *dest, const char *src)
{
    while (*src)
        *(dest++) = *(src++);
    *dest = '\0';   // set terminating null
    return dest;
}


char *stpzcpy (char *dest, const char *src, size_t maxlen)
{
    size_t count = 0;
    maxlen --;

    while (*src && (count < maxlen)) {
        *(dest++) = *(src++);
        count ++;
    }
    *dest = '\0';   // set terminating null
    return dest;
}

#if 0
char *strlcat (char *dest, const char *src, size_t totsize)
{
    size_t curlen = strlen (dest);
    size_t srclen = strlen (src);
    if ((curlen + srclen) >= totsize)
        return NULL;
    strcpy (dest+curlen, src);
    return dest;
}
#endif


char *stristr (const char *str, const char *substr)
{
    int slen = strlen (substr);
    if (slen == 0)
        return NULL;

    const char *d = str;
    while (*d) {            // look for 1st char of substr
        if (strncasecmp (d, substr, slen) == 0)
            return (char *)d;
        d ++;
    }

    return NULL;
}


char *StrChg (char *src, char from, char to)
{
    char *p = src;
    while (*p) {
        if (*p == from)
            *p = to;
        p ++;
    }

    return src;
}


char *_fgets (char *buf, size_t n, FILE *fp, int *buflen)
{
    char *ret = fgets (buf, n, fp);

    if (!ret)
        return NULL;

    int blen = strlen (buf);
    if (blen > 0) {
        if (buf[blen-1] == '\n') {  // remove trailing lf
            buf[blen-1] = '\0';
            blen --;
        } else {        // skip to EOL
            int c;
            do
                c = fgetc (fp);
            while ((c != EOF) && (c != '\n'));
        }
    }

    if (buflen)
        *buflen = blen;

    return ret;
}


int _fputs (const char *buf, FILE *fp)
{
    int ret = fputs (buf, fp);
    if (fputc ('\n', fp) == EOF)
        return EOF;
    return ret;
}

static int waitopen (const char *bsyname, int timeout) // -1 on timeout
{
    int ret = -1;
    int i = 0;

    do {
        if (i > 0)
            sleep (1);
        if (access (bsyname, F_OK)) {       // file not existent
            int handle = open (bsyname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (handle != -1)
                close (handle);
        }
        ret = open (bsyname, O_RDONLY);
        i ++;
    } while ((ret == -1) && (i < timeout) && (errno == ENOENT));

    return ret;
}

void strupr (char *string)
{
  while (*string)
    *(string++) = toupper (*string);
}

int strto4Dadr (const char *&adrs, ADR *adr, byte flags = 0)
{
  sscanf (adrs, "%u:%u/%u.%u", adr->zone, adr->net, adr->node, adr->point);
}


Busy::Busy (pcsz refname)
{
    bsyname = new char[PATH_MAX];
    strcpy (bsyname, refname);
    setext (bsyname, ".bsy");
    bsyh = -1;
}


int Busy::Wait (int timeout, byte shflg)
{
    if (shflg == BSY_EXCL)
        bsyh = open (bsyname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    else  // BSY_SHARE
        bsyh = waitopen (bsyname, timeout);

    if (bsyh == -1)
        return -1;
    return 0;
}

int filelength (int fd)
{
  lseek (fd, 0, SEEK_END);
  return tell (fd);
}

Busy::~Busy ()
{
    if (bsyh != -1) {
        close (bsyh);
        unlink (bsyname);
    }
    delete[] bsyname;
}
