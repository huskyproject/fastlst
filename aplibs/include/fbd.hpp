/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1995-1997  Alberto Pasquale                 */
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


// fbd.hpp

#ifdef __OS2__
    #include <os2def.h>
#else
    #include <stdlib.h>
    #define TRUE 1
    #define FALSE 0
    typedef int BOOL;
#endif


typedef unsigned short word;
typedef unsigned long dword;


struct FIDX;

typedef void (*DupeF) (FIDX *p, BOOL included);

struct _idxdata;
class ANumLst;

#define FBD_Update      ULONG_MAX
#define FBD_BuildAll    0


class FBD {
  private:
    dword ntot, ncur;
    _idxdata *idxhead,
             *idxcur;
    _idxdata **idxnext;
    ANumLst *anumlist;
    dword oldtot, oldrem;   // not significant when oldidx = NULL
    FIDX *oldidx;

    int GetOldIdx (const char *idxname);

  public:
    FBD (void);
    ~FBD(void);
    void AddArea (word areanum);
    void Store (const char *filename, word areanum, word datpos);
    dword GetCurn (void);
    int Build (dword first, const char *idxname,
               const char *nodupeidxname = NULL, DupeF df = NULL);
            // first = FBD_Update for update
            // returns 0 on success
};



