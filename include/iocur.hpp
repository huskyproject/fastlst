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

// IOcur.hpp


#ifndef IOCUR_HPP
#define IOCUR_HPP

#include <stdio.h>

class InpOut;
class InMem;
class PhoneIndex;


class ALTAB {     // pointers to sorted array of pointers to InpOut lists
  public:
    ADRDATA **pw,
            **ph,
            **nf,
            **fl,
            **cs;

    ALTAB (InpOut *o);
    ~ALTAB (void);
};


#define HeaderSize 80

class INPCUR {
  public:
    char  header[HeaderSize];
    short txt_pg_lines;
    short txt_pagenum;
    FILE  *nodelist_txt;
    FILE  *nodelist_prn;

    ALTAB *tab;

    INPCUR (InpOut *o);
    ~INPCUR (void);
};


class OUTCUR {
  public:

    dword datofs,
          dtpofs;

    FILE  *nodex_dat,
          *nodex_dtp;

    dword nzones,
          nregions,
          nnets,
          nhubs,
          naddr,
          nphones,
          nsysops,
          npoints,
          nnodes,
          nexcluded,
          ndown,
          nunknown,
          nredirect,
          nnullphone;


    ALTAB *tab;

    FILE  *fidouser_sss;
    FILE  *nodexndx_sss;
    InMem *heap;

    OUTCUR (InpOut *o);
    void FreeTab (void);
    ~OUTCUR (void);
};


ADRDATA *GetData (ADR *adr, ADRDATA **o, uint n, ADRDATA **cobo, uint cobn);


#endif
