/*****************************************************************************/
/*                                                                           */
/*                 (C)  Copyright 1992-1997 Alberto Pasquale                 */
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

// ParseTyp.cpp

#ifdef __OS2__
    #define INCL_DOS
    #include <os2.h>
#endif

#include <string.h>
#include "misc.hpp"
#include "parsetyp.hpp"


void CioThis::Init (InpOut *cio, time_t *pft, char *name)
{
    PwdFileTime = pft;
    NodeName    = name;
    ia = &cio->IncAddr;    // Current Tail for Lists
    ea = &cio->ExcAddr;
    pw = &cio->Password;
    ph = &cio->Phone;
    nf = &cio->NodeFlags;
    fl = &cio->Flags;
    cs = &cio->Cost;
}



