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

// Types.hpp


#ifndef TYPES_HPP
#define TYPES_HPP

#include "addrs.hpp"
#include "v7.hpp"
#include "phtrans.hpp"


struct StrChain {
    char *text;
    StrChain *next;
};


struct CS {
    word cost;
    word ucost;
};


struct CO {        // cost table
    char *mstr;
    word cost;          // call cost (analog)
    word ucost;         // user cost (analog)
    word costdig;       // call cost (digital) (def. =cost)
    word ucostdig;      // user cost (digital) (def. =ucost)
    CO   *next;
};


struct DL {        // dial table
    char  *mstr;
    char  *pre;
    char  *post;
    StrChain *Exchange_head; // used for LocalValues/LocalExchanges
    CO    *co;       // used when combined dial/cost is used
    DL    *next;
};


#define TD_DIGITAL  0x01

struct TD {         // TypeDef
   char flag[50];
   byte type;
   byte info;       // additional info on the flag, bit-wise
   PT   *pt;        // Phone Trans pointer, NULL if none.
   TD   *next;
};


struct FD {         // FlagDef
   char flag[50];
   word flag_w;
   FD   *next;
};


struct ADRDATA {        // password, phone, flags, cost
   ADR adr;
   union {
      char *txt;
      struct {
        word w1;
        word w2;
      } w;
   };
   ADRDATA *next;
};


                        // V7DATA.flags

#define V7DTP_F     0x0001  // Make v7+ DTP data file
#define V7PDX_F     0x0002  // Make v7+ PDX Phone Index
#define V7Dsk_F     0x0004  // Link using Disk (otherwise memory)
#define Stats_F     0x0008  // Log Statistics for this output nodelist


struct V7DATA {
    char *nodex;            // No extension
    char *sysopndx;
    word flags;
};



#endif

