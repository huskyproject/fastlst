/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1995       Alberto Pasquale                 */
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

// MaxPrm.Cpp

#include <string.h>
#include <apgenlib.hpp>
#include "bbsgenlb.hpp"

MAXPRM::MAXPRM ()
{
    buffer = NULL;
}

MAXPRM::~MAXPRM ()
{
    if (buffer)
        delete[] buffer;
}


int MAXPRM::Read (char *prmname)
{
    char *fullname = new char[PATH_MAX];
    strcpy (fullname, prmname);
    addext (fullname, ".PRM");
    FILE *prmf = fopen (fullname, "rb");
    delete[] fullname;

    if (!prmf)
        return 1;

    long prmlen = filelength (fileno (prmf));
    if (prmlen < 0) {
       fclose (prmf);
       return 1;
    }

    buffer = new byte[prmlen];
    int error = (fread (buffer, (size_t) prmlen, 1, prmf) != 1);
    fclose (prmf);
    if (error)
        return 1;

    prm = (m_pointers *) buffer;
    prmheap = (char *) (buffer + prm->heap_offset);

    if (prm->id != 'M')
        return 1;
    if (prm->version != CTL_VER)
        return 1;

    return 0;
}



