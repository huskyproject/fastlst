/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1992-1997  Alberto Pasquale                 */
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
/* How to contact the author:  Alberto Pasquale of 2:332/504@fidonet         */
/*                             Viale Verdi 106                               */
/*                             41100 Modena                                  */
/*                             Italy                                         */
/*                                                                           */
/*****************************************************************************/

// IOcur.cpp

#ifdef __OS2__
    #define INCL_DOS
    #include <os2.h>
#endif

#include <string.h>
#include "types.hpp"
#include "iocur.hpp"
#include "inpblk.hpp"
#include "misc.hpp"
#include "inmem.hpp"

typedef ADRDATA *ADP;



static int sercmp (ADR *p1, ADRDATA **p2)
{
    return adrcmp (p1, &(*p2)->adr);
}


ADRDATA *GetData (ADR *adr, ADRDATA **o, uint n, ADRDATA **cobo, uint cobn)
{
    ADRDATA **f = NULL;

    if (n)
        f = (ADRDATA **) bsearch (adr, o, n, sizeof (ADRDATA *), (QSF) sercmp);
    if (!f)
        if (cobn)
            f = (ADRDATA **) bsearch (adr, cobo, cobn, sizeof (ADRDATA *), (QSF) sercmp);

    if (f)
        return *f;
    else
        return NULL;
}


static int tabcmp (ADRDATA **p1, ADRDATA **p2)
{
    return adrcmp (&(*p1)->adr, &(*p2)->adr);
}


static ADRDATA **build (uint n, ADRDATA *ad)
{
    ADRDATA **ret;
    int i;

    if (n == 0)
        return NULL;

    ADRDATA **a = ret = new ADP[n];

    for (i = 0; (i < n) && (ad != NULL); i++, a++, ad = ad->next)
        *a = ad;

    if ((i < n) || (ad != NULL))
        vprintlog ("Error: Internal Inconsistency\n");

    qsort (ret, n, sizeof (ADRDATA *), (QSF) tabcmp);

    return ret;
}


ALTAB::ALTAB (InpOut *o)
{
    pw = build (o->pwn, o->Password);
    ph = build (o->phn, o->Phone);
    nf = build (o->nfn, o->NodeFlags);
    fl = build (o->fln, o->Flags);
    cs = build (o->csn, o->Cost);
}


ALTAB::~ALTAB (void)
{
    if (cs) delete[] cs;
    if (fl) delete[] fl;
    if (nf) delete[] nf;
    if (ph) delete[] ph;
    if (pw) delete[] pw;
}


OUTCUR::OUTCUR (InpOut *o)
{
    memset (this, 0, sizeof (*this));
    heap = new InMem;
    tab = new ALTAB (o);
}


void OUTCUR::FreeTab (void)
{
    delete tab;
    tab = NULL;
}


OUTCUR::~OUTCUR (void)
{
    if (tab)
        delete tab;
    delete heap;
}


INPCUR::INPCUR (InpOut *o)
{
    memset (this, 0, sizeof (*this));
    tab = new ALTAB (o);
}


INPCUR::~INPCUR (void)
{
    delete tab;
}
