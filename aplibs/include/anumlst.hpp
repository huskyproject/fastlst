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

// AnumLst.hpp

#include <limits.h>

struct _anumlist;
typedef unsigned int uint;
typedef unsigned short word;


class ANumLst {
    private:
        uint narea;
        _anumlist *anlhead, *anlcur;
        _anumlist **anlnext;
    public:
        ANumLst ();
        ~ANumLst ();
        void Add (word anum);
        word Get (int first = 0);   // USHRT_MAX for no more areas
};
