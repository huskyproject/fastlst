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

// PhTrans.Hpp

#ifndef _PHTRANS_HPP_
#define _PHTRANS_HPP_

#include <typedefs.h>

#define PHTR_SIZE 6     // char to be substituted with string of 4 chars
#define PHTR_MAXN 15    // number of available translations

class PT {

    private:
        word cost;
        word ucost;

        int n;      // number of trans configured
        char phtr[PHTR_MAXN][PHTR_SIZE];

    public:
        PT ();
        int Get (pcsz cfg);     // gets phonetrans, returns 0 on success
                                // -1 on error, 1 on more than PHTR_MAXN
        int Apply (word *cost,  // Call Cost
                   word *ucost, // User Cost
                   psz dest,    // destination phone
                   pcsz src,    // source phone
                   int size);   // size of dest
                                // returns 0 on success
                                // if size exceeded, returns -1, dest is
                                // guaranteed NULL terminated.
};

// The first char is substituted by the following string.
// E.g.   .* " \ "
// '.' becomes '*'
// ' ' becomes '\ '

#endif
