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


void WordStore::Init ()
{
    usedsize = strlen (storebuf);
    nextp = storebuf;
    lastp = NULL;
}


WordStore::WordStore (char *buf, int size, int flags)
{
    storebuf = buf;
    sflags = flags;

    if (size) {
        storesize = size - 1;
        if (!(flags & WST_NoClear))
            Clear ();
        else
            Init ();
    } else {
        storesize = 0;
        Init ();
    }
}


void WordStore::Clear ()
{
    storebuf[0] = '\0';
    Init ();
}


int WordStore::Store (const char *tok)
{
    if (!storesize)
        return -1;

    bool first = (usedsize == 0);

    int toklen = strlen (tok);
    int totlen = toklen + first ? 0 : 1;

    if (totlen > (storesize - usedsize))
        return -1;

    if (!first)
        storebuf[usedsize++] = ' ';

    strcpy (storebuf+usedsize, tok);
    usedsize += toklen;

    return 0;
}


char *WordStore::GetFirst ()
{
    nextp = storebuf;
    return GetNext ();
}


char *WordStore::GetNext ()
{
    lastp = nextp;
    if (GetName (nextp, retbuf, PATH_MAX) <= 0)
        return NULL;
    return retbuf;
}


void WordStore::RemoveLast ()
{
    if (!lastp)
        return;
    if (*lastp == '\0')
        return;

    if (*nextp == '\0') {
        while (lastp > storebuf)        // remove useless space
            if (*(lastp-1) == ' ')
                lastp --;
    }

    int movelen = strlen (nextp) + 1;   // includes terminating NULL

    memmove ((char *)lastp, nextp, movelen);

    if (storesize)
        usedsize -= (nextp - lastp);

    nextp = lastp;
    lastp = NULL;
}


int AddWord (const char *src, char *dest, size_t totsize)
{
    if (*src == '\0')           // no word to add
        return 1;

    size_t curlen = strlen (dest);
    size_t srclen = strlen (src);
    if (curlen != 0)        // separating space
        srclen ++;

    if ((curlen + srclen) >= totsize)
        return -1;

    if (curlen != 0)
        dest[curlen++] = ' ';
    strcpy (dest+curlen, src);

    return 0;
}



