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

// NameList.Cpp

#include <string.h>
#include <apgenlib.hpp>
#include "namelist.hpp"

struct _namelistel {
    char *name;
    _namelistel *next;
};


namelist::namelist (void)
{
    head = NULL;
    tailp = &head;
    cur = NULL;
}


namelist::~namelist (void)
{
    _namelistel *next;

    cur = head;
    while (cur) {
        next = cur->next;
        delete[] cur->name;
        delete cur;
        cur = next;
    }
}


void namelist::add (const char *name)
{
    *tailp = new _namelistel;
    (*tailp)->name = newcpy (name);
    (*tailp)->next = NULL;
    tailp = &(*tailp)->next;
}


char *namelist::get (int first)
{
    if (first)
        cur = head;

    if (!cur)
        return NULL;

    char *ret = cur->name;
    cur = cur->next;

    return ret;
}
