/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1991-1995  Alberto Pasquale                 */
/*                 Portions (C) Copyright 1999 Per Lundberg                  */
/*                                                                           */
/*                   A L L   R I G H T S   R E S E R V E D                   */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*   How to contact the author:  Alberto Pasquale of 2:332/504@fidonet       */
/*                               Viale Verdi 106                             */
/*                               41100 Modena                                */
/*                               Italy                                       */
/*                                                                           */
/*****************************************************************************/

// StrLst.Cpp

#include "bbsgenlb.hpp"
#include <apgenlib.hpp>
#include <string.h>

StrLst::StrLst (char *OutNameCfg, BOOL Append, char *OutName)
{
    StrLst::OutNameCfg = newcpy (OutNameCfg);
    StrLst::Append = Append;
    if (OutName && *OutName)
        StrLst::OutName = newcpy (OutName);
    else
        StrLst::OutName = NULL;
    Outf = NULL;
}


StrLst::~StrLst (void)
{
    delete[] OutNameCfg;
    if (OutName)
        delete[] OutName;
    if (Outf)
        Close ();
}


void StrLst::SetDefCfg (char *OutName)
{
    if (!StrLst::OutName && *OutName)
        StrLst::OutName = newcpy (OutName);
}


BOOL StrLst::ParseCfg (const char *clnline)
{
    const char *tkp = clnline;
    char *tok = GetStatName (tkp);

    if (!tok)
        return FALSE;

    if (stricmp (tok, OutNameCfg) ==  0) {
        if (*tkp == '+') {
            Append = TRUE;
            tkp ++;
        }
        if (OutName)
            delete[] OutName;
        OutName = GetAllocName (tkp);
        return TRUE;
    }

    return FALSE;
}


int StrLst::Add (const char *tag)
{
    if (!OutName)
        return 0;

    if (!Outf) {
        Outf = fopen (OutName, Append ? "at" : "wt");
        if (!Outf)
            return -1;
    }

    if (fprintf (Outf, "%s\n", tag) < 0)
        return -1;

    return 0;
}


int StrLst::Close (void)
{
    if (Outf) {
        if (fclose (Outf))
            return -1;
        Outf = NULL;
    }
    return 0;
}
