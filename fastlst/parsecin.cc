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

// ParseCin.Cpp

#ifdef __OS2__
    #define INCL_DOS
    #include <os2.h>
#endif

#include <string.h>

#ifdef __QNXNTO__
   #include <strings.h>
#endif // __QNXNTO__

#include "misc.hpp"
#include "parsecin.hpp"
#include "apgenlib.hpp"


BOOL ParseCin (CfgFile &f1, InpNnc *cin)
{
    pcsz tok = f1.ReGetToken ();
    pcsz tkp = f1.RestOfLine ();

    if (stricmp (tok, "MsgRem") == 0) {
        cin->MsgRem = GetAllocLn (tkp, GL_Empty);
        return TRUE;
    }

    if (stricmp (tok, "MsgLog") == 0) {
        while ((tok = GetToken (&tkp)) != NULL) {
            if (stricmp (tok, "NullPhone") == 0)
                cin->MsgLog |= MsgLogNullPhone;
            else if (stricmp (tok, "Redirected") == 0)
                cin->MsgLog |= MsgLogRedirected;
            else if (stricmp (tok, "Points") == 0)
                cin->MsgLog |= MsgLogPoints;
            else CfgError (f1);
        }
        return TRUE;
    }

    if (stricmp (tok, "GermanPointList") == 0) {
        cin->flags |= GermanPointLst;
        return TRUE;
    }

    if (stricmp (tok, "NoPointLstPhone") == 0) {
        cin->flags |= NoPointLstPhone;
        return TRUE;
    }

    if (stricmp (tok, "BeforeCompile") == 0) {
        if (*tkp == '\0')
            CfgError (f1);
        cin->BeforeCompile = getallocline (tkp);
        return TRUE;
    }

    if (stricmp (tok, "AfterCompile") == 0) {
        if (*tkp == '\0')
            CfgError (f1);
        cin->AfterCompile = getallocline (tkp);
        return TRUE;
    }

    if (stricmp (tok, "FidoTxt") == 0) {
        getallocpath (*tkp ? tkp : "Nodelist.Txt", &cin->FidoTxt, Build);
        return TRUE;
    }

    if (stricmp (tok, "FidoPrn") == 0) {
        getallocpath (*tkp ? tkp : "Nodelist.Prn", &cin->FidoPrn, Build);
        return TRUE;
    }

    if (stricmp (tok, "IncCoord") == 0) {
        tok = GetToken (&tkp);
        if (!tok)
            CfgError (f1);
        if (stricmp (tok, "ZC") == 0)
            cin->IncCoord = ZONE;
        else if (stricmp (tok, "RC") == 0)
            cin->IncCoord = REGION;
        else if (stricmp (tok, "NC") == 0)
            cin->IncCoord = HOST;
        else if (stricmp (tok, "HC") == 0)
            cin->IncCoord = HUB;
        else
            CfgError (f1);
        return TRUE;
    }

    return FALSE;
}


