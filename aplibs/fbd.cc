/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1995-1996  Alberto Pasquale                 */
/*                 Portions (C) Copyright 1999 Per Lundberg                  */
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


// fbd.cpp

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fbd.hpp"
#include "fb.hpp"
#include "anumlst.hpp"
#include "apgenlib.hpp"


typedef int (*QSF) (const void *, const void *);

char *strnupcpy (char *dest, const char *src, int n);
// strncpy (dest, src, n), dest converted to uppercase
// Implemented in FbLib.Cpp


#define BLKSIZE 1000
typedef FIDX *FIDXP;


struct _idxdata {
    FIDX idx[BLKSIZE];          // array of index entries
    _idxdata *next;
};


static int IdxSortCmp (FIDXP *i1, FIDXP *i2)
{
    int ret = strncmp ((char *)(*i1)->name, (char *)(*i2)->name, MAX_FN_LEN);
    if (ret)
        return ret;
    return (*i1)->anum - (*i2)->anum;
}


static int IdxDupeCmp (FIDXP i1, FIDXP i2)
{
    return strncmp ((char *)i1->name, (char *)i2->name, MAX_FN_LEN);
}


FBD::FBD (void)
{
    ntot = ncur = 0;
    idxcur = idxhead = NULL;
    idxnext = &idxhead;
    anumlist = new ANumLst;
    oldidx = NULL;
    oldtot = oldrem = 0;
}


FBD::~FBD (void)
{
    _idxdata *next;

    if (oldidx)
        delete[] oldidx;

    delete anumlist;

    idxcur = idxhead;
    while (idxcur) {
        next = idxcur->next;
        delete idxcur;
        idxcur = next;
    }
}


void FBD::AddArea (word areanum)
{
    anumlist->Add (areanum);
}


void FBD::Store (const char *filename, word areanum, word datpos)
{
    if (ncur % BLKSIZE == 0) {
        idxcur = *idxnext = new _idxdata;
        idxcur->next = NULL;
        idxnext = &idxcur->next;
        ncur = 0;
    }
    FIDX *fidx = &idxcur->idx[ncur++];
    strnupcpy ((char *)fidx->name, filename, MAX_FN_LEN);
    fidx->anum = areanum;
    fidx->fpos = datpos;
    ntot ++;
}


dword FBD::GetCurn (void)
{
    return ntot;
}


int FBD::GetOldIdx (const char *idxname)
{
    FILE *fidx;

    fidx = fopen (idxname, "rb");
    if (!fidx)
        return -1;
    long idxlen = filelength (fileno (fidx));
    oldtot = idxlen / sizeof (FIDX);
    if (oldtot == 0) {
        fclose (fidx);
        return 0;
    }
    oldidx = new FIDX[oldtot];
    if (fread (oldidx, sizeof (FIDX), oldtot, fidx) != oldtot) {
        delete oldidx;
        oldidx = NULL;
        fclose (fidx);
        return -1;
    }
    fclose (fidx);

                    // mark entries to be removed

    word area = anumlist->Get (1);
    while (area != USHRT_MAX) {
        for  (dword i = 0; i < oldtot; i++)
            if (oldidx[i].anum == area) {
                oldidx[i].anum = USHRT_MAX;
                oldrem ++;
            }
        area = anumlist->Get ();
    }
    
    return 0;
}


int FBD::Build (dword first, const char *idxname, const char *nodupeidxname,
                DupeF df)
{
    dword idxtot;

    if (first != ULONG_MAX)       // full build
        idxtot = ntot - first;
     else {         // update
        if (GetOldIdx (idxname))
            return -1;
        idxtot = ntot + oldtot - oldrem;
    }

    FILE *fidx;

    if (idxtot == 0) {
        fidx = fopen (idxname, "wb");
        if (!fidx)
            return -1;
        if (fclose (fidx))
            return -1;

        if (nodupeidxname) {        // no-dupe idx
            fidx = fopen (nodupeidxname, "wb");
            if (!fidx)
                return -1;
            if (fclose (fidx))
                return -1;
        }
        return 0;
    }

                // idxtot > 0

    FIDXP *fidxp = new FIDXP[idxtot];

    dword i;
    _idxdata *id = idxhead;

    if (first != ULONG_MAX) {      // full build of local or main index
        dword nc = 0;
        while (first - nc >= BLKSIZE) {   // skip whole blocks
            nc += BLKSIZE;
            id = id->next;
        }
        ncur = first % BLKSIZE;    // in the current block
        i = 0;
    } else {                // update build of main index
        dword oi;
        for (i = oi = 0; i < oldtot - oldrem; i ++) {    // load pointers to old idx
            while (oldidx[oi].anum == USHRT_MAX)    // skip marked entries
                oi ++;
            fidxp[i] = &oldidx[oi++];
        }
        ncur = 0;
    }


    for (; i < idxtot; i ++) {   // load pointers to new entries
        if (ncur == BLKSIZE) {
            ncur = 0;
            id = id->next;
        }
        fidxp[i] = &id->idx[ncur++];
    }

    qsort (fidxp, idxtot, sizeof (FIDXP), (QSF) IdxSortCmp);

    fidx = fopen (idxname, "wb");
    if (!fidx) {
        delete[] fidxp;
        return -1;
    }
    setvbuf (fidx, NULL, _IOFBF, 8192);

    for (i = 0; i < idxtot; i ++) {
        if (fwrite (fidxp[i], sizeof (FIDX), 1, fidx) != 1) {
            delete[] fidxp;
            fclose (fidx);
            return -1;
        }
    }

    if (fclose (fidx)) {
        delete[] fidxp;
        return -1;
    }

    if (nodupeidxname) {        // build no-dupe idx
        fidx = fopen (nodupeidxname, "wb");
        if (!fidx) {
            delete[] fidxp;
            return -1;
        }
        setvbuf (fidx, NULL, _IOFBF, 8192);

        FIDX *prev;

        if (fwrite (prev = fidxp[0], sizeof (FIDX), 1, fidx) != 1) {
            delete[] fidxp;
            fclose (fidx);
            return -1;
        }

        BOOL InDupes = FALSE;

        for (i = 1; i < idxtot; i ++) {
            if (IdxDupeCmp (prev, fidxp[i]) != 0) {
                if (fwrite (prev = fidxp[i], sizeof (FIDX), 1, fidx) != 1) {
                    delete[] fidxp;
                    fclose (fidx);
                    return -1;
                }
                InDupes = FALSE;
            } else {        // dupe
                if (!InDupes) {
                    if (df)
                        df (prev, TRUE);
                    InDupes = TRUE;
                }
                if (df)
                    df (fidxp[i], FALSE);
            }
        }

        if (fclose (fidx)) {
            delete[] fidxp;
            return -1;
        }
    }

    delete[] fidxp;
    return 0;
}



