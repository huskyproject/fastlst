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

// ParseCio.Cpp

#ifdef __OS2__
    #define INCL_DOS
    #include <os2.h>
#endif

#include <string.h>

#ifdef __QNXNTO__
   #include <strings.h>
#endif // __QNXNTO__

#include "misc.hpp"
#include "parsecio.hpp"
#include "parsetyp.hpp"
#include "parsecfg.hpp"



ADRDATA **getflags (const char *p, ADRDATA **tail, uint *n)   // get user flags
{
    ADRDATA *c = *tail = new ADRDATA;

    if (!get_addr (&p, &c->adr))
        return NULL;

    c->w.w1 = 0;
    if (GetUserFlags (p, &c->w.w1)) {
        vprintlog ("Invalid flags !\n");
        return NULL;
    }

    c->next = NULL;

    (*n) ++;
    return &c->next;
}


ADRDATA **gettxt4adr (const char **p, const ADR *adr, ADRDATA **tail, uint *n)  // get text token
{
    ADRDATA *c = *tail = new ADRDATA;
    c->adr = *adr;
    char *tok = GetToken (p);
    if (!tok)
        c->txt = newcpy ("");
    else
        c->txt = newcpy (tok);
    c->next = NULL;

    (*n) ++;
    return &c->next;
}


ADRDATA **getadrtxt (const char **p, ADRDATA **tail, uint *n)  // get text token
{
    ADR adr;

    if (!get_addr (p, &adr))
        return NULL;

    return gettxt4adr (p, &adr, tail, n);
}


ADRDATA **getcost4adr (const char *p, const ADR *adr, ADRDATA **tail, uint *n)
{
    ADRDATA *c = *tail = new ADRDATA;

    c->adr = *adr;

    char *tok = GetToken (&p);
    c->w.w1 = tok ? (word) atoi (tok) : (word) USHRT_MAX;
    tok = GetToken (&p);
    c->w.w2 = tok ? (word) atoi (tok) : c->w.w1;

    c->next = NULL;

    (*n) ++;
    return &c->next;
}


ADRDATA **getcost (const char *p, ADRDATA **tail, uint *n)
{
    ADR adr;

    if (!get_addr (&p, &adr))
        return NULL;

    if (!*p)
        return NULL;

    return getcost4adr (p, &adr, tail, n);
}


ADRDATA **ReadPassword (char *filename, ADRDATA **tail, uint *n, char *subject)
{
    CfgFile f (CFGLINESIZE);

    if (f.Open (filename)) {
        vprintlog ("Cannot open PasswordFile \"%s\"\n", filename);
        myexit (NO_CONFIG);
    }

    vwritelog ("Including Password File \"%s\" for \"%s\"\n", filename, subject);

    while (f.GetLn ()) {
        pcsz tok = f.GetToken ();
        pcsz tkp;
        if (stricmp (tok, "Password") == 0)
            tkp = f.RestOfLine ();
        else
            tkp = f.ReGetLn ();
        tail = getadrtxt (&tkp, tail, n);
        if (!tail)
            CfgError (f);
    }

    f.Close ();

    vwritelog ("Finished Password File \"%s\"\n", filename);

    return tail;
}



BOOL ParseCio (CfgFile &f1, InpOut *cio, CioThis *ciothis, InpNnc *cin)
{
    pcsz tok = f1.ReGetToken ();
    pcsz tkp = f1.RestOfLine ();

    if (stricmp (tok, "PasswordFile") == 0) {
        char path[PATH_MAX];
        getpath (tkp, path, Build);
        if (*path == '\0')
            CfgError (f1);
        *ciothis->PwdFileTime = __max (*ciothis->PwdFileTime, DosFileTime (path));
        ciothis->pw = ReadPassword (path, ciothis->pw, &cio->pwn, ciothis->NodeName);
        return TRUE;
    }

    if (stricmp (tok, "IncAddr") == 0) {
        ciothis->ia = get_part_addr_lst (&tkp, ciothis->ia);
        if (cin->IncCoord == 0)
            cin->IncCoord = HIGHER;
        return TRUE;
    }

    if (stricmp (tok, "ExcAddr") == 0) {
        ciothis->ea = get_part_addr_lst (&tkp, ciothis->ea);
        if (cin->IncCoord == 0)
            cin->IncCoord = HIGHER;
        return TRUE;
    }

    if (stricmp (tok, "Password") == 0) {
        ciothis->pw = getadrtxt (&tkp, ciothis->pw, &cio->pwn);
        if (!ciothis->pw)
            CfgError (f1);
        return TRUE;
    }

    if (stricmp (tok, "Phone") == 0) {
        ADR adr;
        if (!get_addr (&tkp, &adr))
            CfgError (f1);

        if (*tkp == '\0')
            CfgError (f1);

        ciothis->ph = gettxt4adr (&tkp, &adr, ciothis->ph, &cio->phn);
        if (*tkp)
            ciothis->nf = gettxt4adr (&tkp, &adr, ciothis->nf, &cio->nfn);
        if (*tkp)
            ciothis->cs = getcost4adr (tkp, &adr, ciothis->cs, &cio->csn);

        return TRUE;
    }

    if (stricmp (tok, "NodeFlags") == 0) {
        ciothis->nf = getadrtxt (&tkp, ciothis->nf, &cio->nfn);
        if (!ciothis->nf)
            CfgError (f1);
        return TRUE;
    }

    if (stricmp (tok, "Flags") == 0) {
        ciothis->fl = getflags (tkp, ciothis->fl, &cio->fln);
        if (!ciothis->fl)
            CfgError (f1);
        return TRUE;
    }

    if (stricmp (tok, "Cost") == 0) {
        ciothis->cs = getcost (tkp, ciothis->cs, &cio->csn);
        if (!ciothis->cs)
            CfgError (f1);
        return TRUE;
    }

    return FALSE;
}

