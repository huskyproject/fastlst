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

// NameList.Hpp

#ifndef _NAMELIST_HPP_
#define _NAMELIST_HPP_

struct _namelistel;


class namelist {
    private:
    _namelistel *head,      // head of list
                *cur;       // current element
    _namelistel **tailp;    // pointer to tail
    public:
      namelist (void);      // constructor
      ~namelist (void);     // destructor
      void add (const char *name);
      char *get (int first = 0); // get (1): first, get (0): next; NULL = end of list
};

#endif
