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

// ParseCsa.Cpp

#ifdef __OS2__
    #define INCL_DOS
    #include <os2.h>
#endif

#include <string.h>
#include "parsetyp.hpp"
#include "parsecsa.hpp"
#include "misc.hpp"
#include "export.hpp"

BOOL ParseCsa (CfgFile &f1, SegAll *csa)
{
    pcsz tok = f1.ReGetToken ();
    pcsz tkp = f1.RestOfLine ();

    if (stricmp (tok, "ArcExportMethod") == 0) {
        if (!GetArcMethods (tkp, &csa->ArcMethHead))
            CfgError (f1);
        return TRUE;
    }

    if (stricmp (tok, "BeforeArcExport") == 0) {
        if (*tkp == '\0')
            CfgError (f1);
        csa->BeforeArc = getallocline (tkp);
        return TRUE;
    }

    if (stricmp (tok, "AfterArcExport") == 0) {
        if (*tkp == '\0')
            CfgError (f1);
        csa->AfterArc = getallocline (tkp);
        return TRUE;
    }

    if (stricmp (tok, "ExportNeededBeforeKill") == 0) {
        csa->NeededBeforeKill = TRUE;
        return TRUE;
    }

    return FALSE;
}

