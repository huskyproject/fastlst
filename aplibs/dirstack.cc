/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1996       Alberto Pasquale                 */
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

// DirStack.Cpp

#include "apgenlib.hpp"
#include <string.h>
#include <unistd.h>

struct _DirStackEl {
    char Dir[PATH_MAX+1];
    _DirStackEl *lower;
};



DirStack::DirStack ()
{
    Top = NULL;
}


DirStack::~DirStack ()
{
    _DirStackEl *el = Top;
    while (el) {
        _DirStackEl *lower = el->lower;
        delete el;
        el = lower;
    }
}


int DirStack::Push (const char *newdir)
{
    char startpath[PATH_MAX+1];

    if (!getcwd (startpath, sizeof (startpath)))    // save current path
        return 1;

    if (cdd (newdir))
        return 1;

                // change successful: record it

    _DirStackEl *lower = Top;
    Top = new _DirStackEl;
    strcpy (Top->Dir, startpath);
    Top->lower = lower;

    return 0;
}


int DirStack::Pop ()
{
    if (!Top)
        return 1;
    if (cdd (Top->Dir))
        return 1;

            // popdir successfull: release current block and move pointer

    _DirStackEl *lower = Top->lower;
    delete Top;
    Top = lower;

    return 0;
}
