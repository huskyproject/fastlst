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


char *fl_stpcpy (char *dest, const char *src)
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

#if !defined(__FreeBSD__) && !defined(__CYGWIN__)

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

#if !defined(__CYGWIN32__)

void strupr (char *string)
{
  while (*string)
    *(string++) = toupper (*string);
}

#endif

int strto4Dadr (const char *&adrs, ADR *adr, byte flags = 0)
{
  unsigned zone, net, node, point=0;
  if(strchr(adrs, '.'))
    sscanf (adrs, "%u:%u/%u.%u", &zone, &net, &node, &point);
  else
    sscanf (adrs, "%u:%u/%u", &zone, &net, &node);
  adr->zone = zone;
  adr->net  = net;
  adr->node = node;
  adr->point = point;
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



BOOL TagMatch (const char *Tag, const char *WildTag)
{
    if (Tag[0] == '<')      // exclude special tags
        return (stricmp (Tag, WildTag) == 0);

    return  (fnamecmp (Tag, WildTag) == 0);
}


    // gets keys from src, stores in *keys, advances src
    // returns length, -1 on error

static int getkeys (const char *&src, dword *keys)
{
    *keys = 0;

    const char *p = src;
    while (*p) {
        if ((*p == ' ') || (*p == '\t'))
            break;

        int u = toupper (*p);
        int nbit;
        if ((u >= '1') && (u <= '8'))
            nbit = u - '1';
        else if ((u >= 'A') && (u <= 'X'))
            nbit = u - 'A' + 8;
        else
            return -1;

        *keys |= (1L << nbit);
        p ++;
    }

    int slen = int (p - src);

    src += slen;

    return slen;
}


int GetLevKey (const char *&src, word *level, dword *keys, byte flags)
{
    const char *p = src;

    p += strspn (p, " \t");     // skip leading blanks

    if (!isdigit (*p)) {                 // decimal level ?
        if ((flags & MSC_Allow_NoLevel) && (*p == '/'))
            *level = USHRT_MAX;
        else
            return -1;
    } else
        *level = (word) strtoul (p, (char **)&p, 10);  // convert to ulong


    if (keys)
        *keys = 0;

    if (*p == '/') {          // keys present
        if (!keys)
            return -1;
        p ++;
        if (getkeys (p, keys) == -1)
            return -1;
    }

    switch (*p) {
        case '\0':
            break;
        case ' ':               // skip trailing blanks
        case '\t':
            p += strspn (p, " \t");
            break;
        default:                // illegal chars
            return -1;
    }

    int slen = int (p - src);

    if (flags & MSC_SrcMov)
        src += slen;

    return slen;
}


char Ibm2Ascii (char c)
{
    if ((unsigned char)c <= 127)       // 0->127, return unchanged
        return c;

    static char cvt[129] = "CueaaaaceeeiiiAAEaAooouuyOUcLYPfaiounNao?++24!<>XXX|++++++|+++++++++-++++++++-+++++++++++++XXXXXabgpEouTOOOqooeU=+><()%=o../n2X";

    return cvt[c-128];
}


static char keych[33] = "12345678abcdefghijklmnopqrstuvwx";


void PrintLevKey (char *buffer, word level, dword keys)
{
#ifdef UNIX
    sprintf(buffer,"%u",level);
#else
    utoa (level, buffer, 10);       // write the level
#endif
    char *p = strchr (buffer, '\0'); // points after level
    if (keys == 0UL)
        return;

    *(p++) = '/';

    dword ibit = 0x00000001;
    for (int i = 0;  i < 32; i ++) {
        if (ibit & keys)
            *(p++) = keych[i];
        ibit <<= 1;
    }

    *p = '\0';
}


bool eq4Dadr (const ADR *adr1, const ADR *adr2)
{
    if (adr1->zone  != adr2->zone)  return false;
    if (adr1->net   != adr2->net)   return false;
    if (adr1->node  != adr2->node)  return false;
    if (adr1->point != adr2->point) return false;
    return true;
}

