/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1991-1997  Alberto Pasquale                 */
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

// V7lib.hpp 1.00

#ifndef _V7LIB_HPP_
#define _V7LIB_HPP_

#include <stdlib.h>
#include <typedefs.h>

#pragma pack (1)

#include <smapi/msgapi.h>
typedef NETADDR v7ADR;



struct _vers7 {
    word Zone;
    word Net;
    word Node;
    word HubNode;          // Hub or point
    word CallCost;         /* phone company's charge */
    word MsgFee;           /* Amount charged to user for a message */
    word NodeFlags;        /* set of flags (see below) */
    byte ModemType;        /* RESERVED for modem type */
    byte Phone_len;        // length of phone string
    byte Password_len;     // length of pwd
    byte Bname_len;        // length of Board in packed blk
    byte Sname_len;        // length of SysOp in packed blk
    byte Cname_len;        // length of city in packed blk
    byte pack_len;         // length of packed blk
    byte BaudRate;         /* baud rate divided by 300 */
};


// for NodeFlags


#define v7nf_hub      0x0001  // node is a net hub
#define v7nf_host     0x0002  // node is a net host
#define v7nf_region   0x0004  // node is region coord
#define v7nf_zone     0x0008  // node is a zone coord
#define v7nf_CM       0x0010  // runs continuous mail
#define v7nf_b5       0x0020  //
#define v7nf_b6       0x0040  //
#define v7nf_b7       0x0080  //
#define v7nf_b8       0x0100  //
#define v7nf_b9       0x0200  //
#define v7nf_bA       0x0400  //
#define v7nf_bB       0x0800  //
#define v7nf_point    0x1000  // node is a point
#define v7nf_bD       0x2000  //
#define v7nf_bE       0x4000  //
#define v7nf_bF       0x8000  //


// compares the addresses pointed to by key and desired.
// Returns 0: equal, <0: *key < *desired, >0 *key > *desired
// len can be 6 (key->point taken = 0) or 8.

int v7ADRcmp (const v7ADR *key, const v7ADR *desired, int len = 8);


struct _ndx;
class Busy;


class v7lib {

    private:
        int dath,           // handle for .dat
            ndxh;           // handle for .ndx
        Busy *v7bsy;
        _ndx *nodeidx,
             *ctlblk;
        int  nentry;       // number of entry in leaf; <0 none.
        long nextdatofs;   // offset in DAT of next entry to load

        int get7blk (long nblk);      // 0 success, -1 error

        int get7rec (_vers7 *v7rec);    // loads nentry in v7rec
                            // 0 success, -2 error

        int btree (const v7ADR *desired);   // >= 0 nentry in nodeidx
                                          // -1 not found
                                          // -2 error
        int getnext ();                   // >= 0 nentry
                                          // -1   no more
                                          // -2   error

    public:
        v7lib ();
        ~v7lib ();
        int v7open (const char *nodex);      // 0 on success, -1 on timeout
                                       // no need for close between opens
        void v7close ();               // automatic on 2+ open and destructor

        int get (const v7ADR *adr = NULL,  // NULL -> load next in index
                 _vers7 *v7rec = NULL);  // NULL -> check existence only
                    //  0 success
                    // -1 adr not found
                    // -2 error

        int getfirst (_vers7 *v7rec = NULL); // get 1st entry in index
                    // 0 success
                    // -2 error

        // the following functions do not modify the "index position"
        // for get/getfirst

        int getfirstdat (_vers7 *v7rec); // get 1st entry in DAT
                    // 0 success
                    // -2 error

        int getnextdat (_vers7 *v7rec); // get next entry in DAT
                    //  0 success     // loads the entry following
                    // -2 error       // the last loaded one.
                                      // Works also after get/getfirst
                                      // provided v7rec != NULL.
};

#endif
