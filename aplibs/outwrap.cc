/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1991-1996  Alberto Pasquale                 */
/*                                                                           */
/*                   A L L   R I G H T S   R E S E R V E D                   */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/* This source code is NOT in the public domain and it CANNOT be used or     */
/* distributed without written permission from the author.                   */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*   How to contact the author:  Alberto Pasquale of 2:332/504@fidonet       */
/*                               Viale Verdi 106                             */
/*                               41100 Modena                                */
/*                               Italy                                       */
/*                                                                           */
/*****************************************************************************/

// OutWrap.Cpp

#ifdef __OS2__
    #define INCL_DOS
    #include <os2.h>
#endif
#include <apgenlib.hpp>
#include <string.h>
#include <stdlib.h>
#include "bbsgenlb.hpp"

#define WrapColOff 0
#define WrapEndOff 1023
#define WrapColDef 0
#define WrapEndDef 79


OutWrap::OutWrap (const char *WrapCfg)
{
    OutWrap::WrapCfg = newcpy (WrapCfg);
    wrapcol = WrapColOff;
    wrapend = WrapEndOff;
    CfgDone = FALSE;
    LineBuf = NULL;
}


OutWrap::~OutWrap ()
{
    if (LineBuf)
        delete[] LineBuf;
    delete[] WrapCfg;
}


int OutWrap::ParseCfg (const char *clnline)
{
    const char *tkp = clnline;
    char *tok = GetStatName (tkp);

    if (!tok)
        return OWcfgNotFound;

    if (stricmp (tok, WrapCfg) ==  0) {
        if (CfgDone)
            return OWcfgDupe;
        CfgDone = TRUE;
        wrapcol = WrapColDef;
        wrapend = WrapEndDef;
        if ((tok = GetStatName (tkp)) != NULL) {
            wrapcol = atoi (tok);
            tok = GetStatName (tkp);
            if (tok != NULL)
                wrapend = atoi (tok);
        }
        if ((wrapend <= wrapcol) || (wrapcol < 0)) {
            wrapcol = WrapColDef;
            wrapend = WrapEndDef;
            return OWcfgError;
        }

        return OWcfgFound;
    }

    return OWcfgNotFound;
}


static int n2wr (const char *b, int maxlen)       // characters before wrap
{
    int lnlen = strcspn (b, "\n");
    if (maxlen >= lnlen)             // full line shorter than maxlen
        return lnlen;

    const char *e = b + maxlen;         // points after last allowable char
    while ((*e != ' ') && (e > b))
        e--;
    if (*e != ' ')          // long token: can't word wrap
        return maxlen;

    while ((*e == ' ') && (e > b))
        e--;
    if (*e == ' ')      // long space: can't word wrap
        return maxlen;

    e++;        // points after last character to output

    return (int)(e-b);
}


int OutWrap::fwrap (const char *src, int begin, OWShow ows, void *prm)
{                                  // return n written from b, EOF on error
    const char *b = src;
    int nspaces = begin ? 0 : wrapcol;      // number of heading spaces
    int maxlen = wrapend - nspaces;         // size for remaining string
    int nwr = n2wr (b, maxlen);           // size of string to be copied

    sprintf (LineBuf, "%*s%.*s", nspaces, "", nwr, b);
    if (ows (LineBuf, prm))
        return EOF;

    b += nwr;
    if ((*b == '\n') || (*b == ' '))
        b ++;

    return int (b - src);
}


int OutWrap::Out (const char *source, OWShow ows, void *prm)
{
    if (!LineBuf)
        LineBuf = new char[wrapend+1];

    const char *b = source;

    int nwr = fwrap (b, 1, ows, prm);
    if (nwr == EOF)
        return EOF;
    b += nwr;

    while (*b) {
        nwr = fwrap (b, 0, ows, prm);
        if (nwr == EOF)
            return EOF;
        b += nwr;
    }

    return 0;
}

