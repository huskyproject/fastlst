/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1995       Alberto Pasquale                 */
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

// AnumLst.Cpp

#include "anumlst.hpp"
#include <stdlib.h>

#define _ANUM_BLK_SIZE 100


struct _anumlist {
    word anum[_ANUM_BLK_SIZE];
    _anumlist *next;
};


ANumLst::ANumLst (void)
{
    anlcur = anlhead = NULL;
    anlnext = &anlhead;
    narea = 0;
}


ANumLst::~ANumLst (void)
{
    _anumlist *anl = anlhead, *next;
    while (anl) {
        next = anl->next;
        delete anl;
        anl = next;
    }
}


void ANumLst::Add (word anum)
{
    uint i = narea % _ANUM_BLK_SIZE;
    if (i == 0) {
        anlcur = *anlnext = new _anumlist;
        anlnext = &anlcur->next;
        anlcur->next = NULL;
    }
    anlcur->anum[i] = anum;
    narea ++;
}


word ANumLst::Get (int first)
{
    static uint i;  // absolute number of entry

    if (first) {
        anlcur = anlhead;
        i = 0;
    }

    if (i >= narea)
        return USHRT_MAX;

    word ret = anlcur->anum[i % _ANUM_BLK_SIZE];

    i ++;
    if (i % _ANUM_BLK_SIZE == 0)
        anlcur = anlcur->next;

    return ret;
}

