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

// ParseCsl.Cpp


#ifdef __OS2__
    #define INCL_DOS
    #include <os2.h>
#endif

#include <limits.h>
#include <string.h>

#ifdef __QNXNTO__
   #include <strings.h>
#endif // __QNXNTO__

#include "parsecsl.hpp"
#include "misc.hpp"
#include "export.hpp"

BOOL ParseCsl (CfgFile &f1, SegLoc *csl)
{
    pcsz tok = f1.ReGetToken ();
    pcsz tkp = f1.RestOfLine ();

    if (stricmp (tok, "ArcExport") == 0) {
        if (*tkp == '\0')
            CfgError (f1);
        if (Compr == NULL) {
            vprintlog ("Error: CompressCfg not defined\n");
            CfgError (f1);
        }
        tkp = getallocpath (tkp, &csl->ArcName, Build, ArcPath);
        csl->ArcKeep = (*tkp) ? atoi (tkp) : INT_MAX;
        if (!IsDayNode (csl->ArcName)) {
            vprintlog ("ArcExport MUST have a .??? extension\n");
            CfgError (f1);
        }
        return TRUE;
    }

    if (stricmp (tok, "ArcExportDesc") == 0) {
        csl->ArcDesc = getallocline (tkp);
        return TRUE;
    }

    return FALSE;
}

