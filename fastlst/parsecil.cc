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

// ParseCil.Cpp


#ifdef __OS2__
    #define INCL_DOS
    #include <os2.h>
#endif

#include <string.h>
#include <limits.h>
#include "misc.hpp"
#include "parsecil.hpp"


BOOL ParseCil (CfgFile &f1, InpLoc *cil)
{
    pcsz tok = f1.ReGetToken ();
    pcsz tkp = f1.RestOfLine ();

    if (stricmp (tok, "NodeDiff") == 0) {
        if (*tkp == '\0')
            CfgError (f1);
        getallocpath (tkp, &cil->NodeDiff, Build, InputPath);
        if (!cil->VarNodeList) { // cfg error, it should be var nodelist name
            vprintlog ("Cannot accept fixed \"%s\" with NodeDiff !\n", cil->NodeList);
            myexit (CFG_ERROR);
        }
        return TRUE;
    }

    if (stricmp (tok, "ArcList") == 0) {
        if (*tkp == '\0')
            CfgError (f1);
        if (Compr == NULL) {
            vprintlog ("Error: CompressCfg not defined\n");
            CfgError (f1);
        }
        tkp = getallocpath (tkp, &cil->ArcList, Build, ArcPath);
        cil->ArcListKeep = (*tkp) ? atoi (tkp) : INT_MAX;
        if ((cil->ArcListKeep == 0) && (killsource)) {
            vprintlog ("Cannot accept to keep 0 archives when KillSource is active !\n");
            myexit (CFG_ERROR);
        }
        return TRUE;
    }

    if (stricmp (tok, "ArcListDesc") == 0) {
        cil->ArcListDesc = getallocline (tkp);
        return TRUE;
    }

    if (stricmp (tok, "ArcDiff") == 0) {
        if (*tkp == '\0')
            CfgError (f1);
        if (Compr == NULL) {
            vprintlog ("Error: CompressCfg not defined\n");
            CfgError (f1);
        }
        tkp = getallocpath (tkp, &cil->ArcDiff, Build, ArcPath);
        cil->ArcDiffKeep = (*tkp) ? atoi (tkp) : INT_MAX;
        return TRUE;
    }

    if (stricmp (tok, "ArcDiffDesc") == 0) {
        cil->ArcDiffDesc = getallocline (tkp);
        return TRUE;
    }

    return FALSE;
}
