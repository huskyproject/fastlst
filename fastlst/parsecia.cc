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

// ParseCia.Cpp

#ifdef __OS2__
    #define INCL_DOS
    #include <os2.h>
#endif

#include <string.h>
#include "parsetyp.hpp"
#include "parsecia.hpp"
#include "misc.hpp"


BOOL ParseCia (CfgFile &f1, InpAll *cia)
{
    pcsz tok = f1.ReGetToken ();
    pcsz tkp = f1.RestOfLine ();

    if (stricmp (tok, "NeededBeforeKill") == 0) {
        cia->NeededBeforeKill = TRUE;
        return TRUE;
    }

    if (stricmp (tok, "BeforeArcList") == 0) {
        if (*tkp == '\0')
            CfgError (f1);
        cia->BeforeArcList = getallocline (tkp);
        return TRUE;
    }

    if (stricmp (tok, "AfterArcList") == 0) {
        if (*tkp == '\0')
            CfgError (f1);
        cia->AfterArcList = getallocline (tkp);
        return TRUE;
    }

    if (stricmp (tok, "BeforeUnArcList") == 0) {
        if (*tkp == '\0')
            CfgError (f1);
        cia->BeforeUnArcList = getallocline (tkp);
        return TRUE;
    }

    if (stricmp (tok, "AfterUnArcList") == 0) {
        if (*tkp == '\0')
            CfgError (f1);
        cia->AfterUnArcList = getallocline (tkp);
        return TRUE;
    }

    if (stricmp (tok, "BeforeArcDiff") == 0) {
        if (*tkp == '\0')
            CfgError (f1);
        cia->BeforeArcDiff = getallocline (tkp);
        return TRUE;
    }

    if (stricmp (tok, "AfterArcDiff") == 0) {
        if (*tkp == '\0')
            CfgError (f1);
        cia->AfterArcDiff = getallocline (tkp);
        return TRUE;
    }

    if (stricmp (tok, "BeforeUnArcDiff") == 0) {
        if (*tkp == '\0')
            CfgError (f1);
        cia->BeforeUnArcDiff = getallocline (tkp);
        return TRUE;
    }

    if (stricmp (tok, "AfterUnArcDiff") == 0) {
        if (*tkp == '\0')
            CfgError (f1);
        cia->AfterUnArcDiff = getallocline (tkp);
        return TRUE;
    }

    if (stricmp (tok, "BeforeEdit") == 0) {
        if (*tkp == '\0')
            CfgError (f1);
        cia->BeforeEdit = getallocline (tkp);
        return TRUE;
    }

    if (stricmp (tok, "AfterEdit") == 0) {
        if (*tkp == '\0')
            CfgError (f1);
        cia->AfterEdit = getallocline (tkp);
        return TRUE;
    }

    if (stricmp (tok, "ArcMethod") == 0) {
        if (!GetArcMethods (tkp, &cia->ArcMethHead))
            CfgError (f1);
        return TRUE;
    }

    if (stricmp (tok, "ArcDiffMethod") == 0) {
        if (!GetArcMethods (tkp, &cia->ArcDiffMethHead))
            CfgError (f1);
        return TRUE;
    }

    return FALSE;
}

