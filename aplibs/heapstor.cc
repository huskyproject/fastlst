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


// HeapStor.Cpp

#ifdef __OS2__
    #define INCL_DOS
    #include <os2.h>
#endif
#include "apgenlib.hpp"
#include <string.h>

#pragma pack (1)

typedef void *voidp;

#define WordFromP(p)  (*((word *)(p)))
#define ByteP(p) ((byte *)(p))


struct HeapBlk {
    HeapBlk *next;  // pointer to next block in heap chain
    byte *buf;      // buffer for heap block (blksize)
};


HeapStore::HeapStore (size_t blksize)
{
    this->blksize = blksize;
    HeapCur = HeapTail = HeapHead = NULL;
    ni = 0;
    tailofs = blksize;   // i.e. no space available
    cmpf = NULL;
    ptr = NULL;
}


HeapStore::~HeapStore ()
{
    if (ptr)
        delete[] ptr;

    HeapBlk *hb = HeapHead;
    while (hb) {
        HeapBlk *next = hb->next;
        delete hb;
        hb = next;
    }
}


void HeapStore::Store (const void *p)
{
    word size = WordFromP (p);

    if ((size + sizeof (word)) > blksize)     // it should never happen...
        return;

    if ((size + sizeof (word)) > (blksize - tailofs)) {   // alloc new HeapBlk
        HeapBlk **nextp = HeapTail ? &HeapTail->next : &HeapHead;
        HeapTail = *nextp = (HeapBlk *) new byte[sizeof (HeapBlk *) + blksize];
        HeapTail->next = NULL;
        tailofs = 0;
    }

    memcpy (HeapTail->buf+tailofs, p, size);
    ni ++;
    tailofs += size;
    WordFromP (HeapTail->buf+tailofs) = 0; // mark end of list

    if (ptr) {
        delete[] ptr;
        ptr = NULL;
        cmpf = NULL;
    }
}


uint HeapStore::NTot ()
{
    return ni;
}


void HeapStore::LoadArray ()
{
    if (ni < 1)
        return;

    ptr = new voidp[ni];

    HeapBlk *hb = HeapHead;
    uint iofs = 0;
    for (uint i = 0; i < ni; i ++) {
        void *p = hb->buf+iofs;
        size_t size = WordFromP (p);
        if (size == 0) {        // switch to next block
            hb = hb->next;
            iofs = 0;
            p = hb->buf;
            size = WordFromP (p);
        }
        ptr[i] = p;
        iofs += size;
    }
}


void HeapStore::Sort (CMPF cmpf)
{
    if (!ptr)
        LoadArray ();
    if (!ptr)
        return;

    this->cmpf = cmpf;

    if (ni > 1)
        qsort (ptr, ni, sizeof (void *), cmpf);
}


const void *HeapStore::Retrieve (uint n)
{
    if (!ptr)
        LoadArray ();
    if (n >= ni)
        return NULL;

    return ptr[n];
}


const void *HeapStore::Retrieve (const void *key, uint *n, CMPF bscmpf)
{
    if (!cmpf)        // no Sort done
        return NULL;

    void **ret = (void **) bsearch (key, ptr, ni, sizeof (void *), bscmpf ? bscmpf : cmpf);
    if (!ret)
        return NULL;

    if (n)             
        *n = uint (ret - ptr);

    return *ret;
}


const void *HeapStore::GetFirst ()
{
    HeapCur = HeapHead;
    curofs = 0;
    return GetNext ();
}


const void *HeapStore::GetNext ()
{
    if (!HeapCur)
        return NULL;

    const void *p = HeapCur->buf+curofs;
    word size = WordFromP (p);
    if (size == 0) { // switch to next blk
        HeapCur = HeapCur->next;
        if (!HeapCur)
            return NULL;
        curofs = 0;
        p = HeapCur->buf;
        size = WordFromP (p);
    }

    curofs += size;
    return p;
}


void HeapStore::Process (uint n, HSPF hspf, void *prm)
{
    if (!ptr)
        LoadArray ();

    if (n >= ni)
        return;

    hspf (ptr[n], prm);
}



const void *HeapStore::LRetrieve (const void *key, CMPF cmpf, uint *n)
{
    const void *vp = GetFirst ();
    uint i = 0;
    while (vp) {
        if (cmpf (key, vp) == 0)
            break;
        vp = GetNext ();
        i ++;
    }

    if (!vp)
        return NULL;

    if (n)
        *n = i;

    return vp;
}

