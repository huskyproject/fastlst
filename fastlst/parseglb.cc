/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1992-1997  Alberto Pasquale                 */
/*                 Portions (C) Copyright 1999 Per Lundberg                  */
/*                                                                           */
/*                   A L L   R I G H T S   R E S E R V E D                   */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/* How to contact the author:  Alberto Pasquale of 2:332/504@fidonet         */
/*                             Viale Verdi 106                               */
/*                             41100 Modena                                  */
/*                             Italy                                         */
/*                                                                           */
/*****************************************************************************/

#include <ctype.h>

#include <string.h>
#include <limits.h>
#include <apgenlib.hpp>
#include <bbsgenlb.hpp>
#include "misc.hpp"
#include "data.hpp"
#include "parsecfg.hpp"
#include "parseglb.hpp"

void GetCostData (CfgFile &f, CO *tco)
{
    pcsz tok;

    tok = f.GetToken ();      // cost
    if (tok)
        tco->cost = (word) atoi (tok);
    else
        tco->cost = USHRT_MAX;

    tok = f.GetToken ();      // ucost
    if (tok)
        tco->ucost = (word) atoi (tok);
    else
        tco->ucost = tco->cost;
                                           // cost/ucost assigned

    tok = f.GetToken ();      // costdig
    if (!tok) {
        tco->costdig  = tco->cost;
        tco->ucostdig = tco->ucost;
        return;
    }

    tco->costdig = (word) atoi (tok);

    tok = f.GetToken ();      // ucostdig
    if (tok)
        tco->ucostdig = (word) atoi (tok);
    else
        tco->ucostdig = tco->costdig;
}


void GetCostTable (CfgFile &f, CO **co_head)
{
    CO **co; CO *tco;

    co = co_head;
    tco = NULL;

    while (f.GetLn ()) {

        pcsz tok = f.GetToken ();

        if (strcasecmp (tok, "end") == 0)
            break;
        tco = *co = new CO;

        if (strcmp (tok, "-") == 0)
            tok = NullStr;

        tco->mstr = newcpy (tok);       /* string to be substituted */

        GetCostData (f, tco);

        tco->next = NULL;
        co = &(tco->next);
    }

    if (!tco) {
        vprintlog ("Cost table MUST NOT be empty !\n");
        myexit (CFG_ERROR);
    }

    if (*tco->mstr != '\0') {
        vprintlog ("Last entry in Cost Table must have a single dash '-' as the first field !\n");
        myexit (CFG_ERROR);
    }
}



void GetDialPrePost (CfgFile &f, DL *tdl)
{
    pcsz tok2;
    int itmp;

    pcsz tok = f.GetToken ();    // get prefix/suffix
    if (!tok)
        tok = NullStr;
    itmp = strcspn (tok, "/");   /* find length of prefix */
    tdl->pre = new char[itmp + 1];
    strncpy (tdl->pre, tok, itmp);
    *(tdl->pre+itmp) = '\0';

    tok2 = tok + itmp;
    if (*tok2 == '/')     /* point to suffix */
        tok2++;
    tdl->post = newcpy (tok2);
}


void GetDialTable (CfgFile &f, DL **dl_head, BOOL GetCost)
{
    DL **dl = dl_head;
    DL *tdl = NULL;
    StrChain **pple = NULL;

    while (f.GetLn ()) { /* dial lines */

        pcsz tok = f.GetToken ();

        if (strcasecmp (tok, "LocalExchanges") == 0) {
            if (!pple) {
                vprintlog ("LocalExchanges must follow some Dial translation entry !\n");
                CfgError (f);
            }
            while ((tok = f.GetToken ()) != NULL) {
                *pple = new StrChain;
                (*pple)->text = newcpy (tok);
                pple = &(*pple)->next;
            }
            *pple = NULL;     // close the list
            continue;
        }

        if (strcasecmp (tok, "LocalValues") == 0) {
            tok = f.GetToken ();               // Ignore "LocalValues"
            if (!tok)
                continue;
        }

        if (strcasecmp (tok, "end") == 0)
            break;

        tdl = *dl = new DL;
        tdl->Exchange_head = NULL;
        pple = &tdl->Exchange_head;

        if (strcmp (tok, "-") == 0)
            tok = NullStr;

        tdl->mstr = newcpy (tok);       /* string to be substituted */

        GetDialPrePost (f, tdl);

        if (GetCost) {
            tdl->co = new CO;
            GetCostData (f, tdl->co);
        } else {
            tdl->co = NULL;
            if (f.TokenPtr ()) {
                vprintlog ("Cannot define costs in the Dial table when a Cost table exists !\n");
                CfgError (f);
            }
        }

        tdl->next = NULL;
        dl = &(tdl->next);
    }

    if (!tdl) {
        vprintlog ("Dial table MUST NOT be empty !\n");
        myexit (CFG_ERROR);
    }

    if (*tdl->mstr != '\0') {
        vprintlog ("Last entry in Dial Table must have a single dash '-' as the first field !\n");
        myexit (CFG_ERROR);
    }
}


int GetTypeDef (CfgFile &f, TD **td_head)
{
                            // all Flags are stored as uppercase !
    TD **td; TD *ttd;

    td = td_head;

    while (f.GetLn ()) {

        pcsz tok = f.GetToken ();

        if (strcasecmp (tok, "end") == 0)
            break;
        ttd = *td = new TD;
        strzcpy (ttd->flag, tok, sizeof (ttd->flag));
        fl_strupr (ttd->flag);
        tok = f.GetToken ();
        if (tok)
            ttd->type = (byte) atoi (tok);
        else
            ttd->type = 0;

        ttd->info = 0;
        ttd->pt   = NULL;

        pcsz tkp = f.TokenPtr ();

        if (tkp) {         // something after modem type
          if (isdigit (*tkp)) {   // PhoneTrans
            ttd->pt = new PT;
            if (ttd->pt->Get (tkp))
                return -1;
          } else {
            tok = f.GetToken ();
            if (strcasecmp (tok, "DIGITAL") == 0)
                ttd->info |= TD_DIGITAL;
            else if (strcasecmp (tok, "ANALOG") == 0)
                ;
            else
                return -1;
          }
        }

        ttd->next = NULL;
        td = &(ttd->next);
    }

    return 0;
}


void GetFlagDef (CfgFile &f, FD **fd_head)
{
                            // all Flags are stored as uppercase !
    FD **fd; FD *tfd;

    fd = fd_head;

    while (f.GetLn ()) { /* FlagDef lines */

        pcsz tok = f.GetToken ();

        if (strcasecmp (tok, "end") == 0)
            break;
        tfd = *fd = new FD;
        strzcpy (tfd->flag, tok, sizeof (tfd->flag));
        fl_strupr (tfd->flag);
        tok = f.GetToken ();
        tfd->flag_w = 0;
        if (tok)
            if (GetUserFlags (tok, &tfd->flag_w)) {
                vprintlog ("Invalid flags !\n");
                CfgError (f);
            }
        tfd->next = NULL;
        fd = &(tfd->next);
    }
}

BOOL ParseGlb (CfgFile &f1)
{
    pcsz tok = f1.ReGetToken ();

    static BOOL CostNeeded = TRUE;  // Cost table not acquired yet.

    if (strcasecmp (tok, "MultiLineDesc") == 0) {  // MultiLineDesc
        pcsz tkp = f1.RestOfLine ();
        if (sscanf (tkp, "%d %c", &fb_cpos, &fb_cont) < 1)
            CfgError (f1);
        if (fb_cpos != -1)
            vwritelog ("Using MultiLineDesc %d '%c'", fb_cpos, fb_cont);
        return TRUE;
    }

    if (strcasecmp (tok, "CompressCfg") == 0) {
        char path[PATH_MAX];
        tok = f1.GetToken ();
        if (!tok)
            CfgError (f1);
        getpath (tok, path, Build);
        Compr = new AH_ComprCfg (path, vpwlog);
        return TRUE;
    }

    if (strcasecmp (tok, "statuslog") == 0) {
        tok = f1.GetToken ();
        if (!tok)
            CfgError (f1);
        char logname[PATH_MAX];
        getpath (tok, logname, Build);
        if ((logfile = fopen (logname, "at")) == NULL) {
            fprintf(stderr, "\nError opening log file \"%s\" !\n\n", logname);
            myexit(OPEN_ERR);
        } else {
            fprintf (logfile, "\n");
            vwritelog ("Begin, FastLst "VER);
        }
        return TRUE;
    }

    if (strcasecmp (tok, "InputPath") == 0) {
        tok = f1.GetToken ();
        if (!tok)
            CfgError (f1);
        getallocpath (tok, &InputPath, Slash | MkDir | Build);
        return TRUE;
    }

    if (strcasecmp (tok, "ArcPath") == 0) {
        tok = f1.GetToken ();
        if (!tok)
            CfgError (f1);
        getallocpath (tok, &ArcPath, Slash | Build);
        return TRUE;
    }

    if (strcasecmp (tok, "ArcDate") == 0) {      /* ArcDate */
        tok = f1.GetToken ();
        if (tok) {
            if (strcasecmp (tok, "Write") == 0)
                ArcDate = _ArcDateWrite_;
            else if (strcasecmp (tok, "Creation") == 0)
                ArcDate = _ArcDateCreation_;
            else
                CfgError (f1);
        } else
            ArcDate = _ArcDateCreation_;
        return TRUE;
    }

    if (strcasecmp (tok, "killafter") == 0) {
        killafter = TRUE;
        return TRUE;
    }

    if (strcasecmp (tok, "KillSource") == 0) {
        killsource = TRUE;
        return TRUE;
    }

    if (strcasecmp (tok, "BeforeKillSource") == 0) {
        BeforeKillSource = getallocline (f1.RestOfLine ());
        return TRUE;
    }

    if (strcasecmp (tok, "dash2comma") == 0) {
        dash2comma = TRUE;
        return TRUE;
    }

    if (strcasecmp (tok, "noreport") == 0) {
        noreport = TRUE;
        return TRUE;
    }

    if (strcasecmp (tok, "V7BugFix") == 0) {
        V7BugFix = TRUE;
        return TRUE;
    }

    if (strcasecmp (tok, "cost") == 0) {
        if (!CostNeeded) {
            vprintlog ("Error: The Cost table must precede the Dial table !\n");
            CfgError (f1);
        }
        GetCostTable (f1, &co_head);
        CostNeeded = FALSE;
        return TRUE;
    }

    if (strcasecmp (tok, "dial") == 0) {
        if (dl_head) {
            vprintlog ("Error: The Dial table must be unique !\n");
            CfgError (f1);
        }
        GetDialTable (f1, &dl_head, CostNeeded);
        CostNeeded = FALSE;
        return TRUE;
    }

    if (strcasecmp (tok, "NoRedir") == 0) {
        NoRedir = TRUE;
        return TRUE;
    }

    if (strcasecmp (tok, "TypeDef") == 0) {
        if (GetTypeDef (f1, &td_head)) {
            vprintlog ("Error in TypeDef !\n");
            CfgError (f1);
        }
        return TRUE;
    }

    if (strcasecmp (tok, "BitType") == 0) {
        BitType = TRUE;
        return TRUE;
    }

    if (strcasecmp (tok, "FlagDef") == 0) {
        GetFlagDef (f1, &fd_head);
        return TRUE;
    }

    if (strcasecmp (tok, "MsgLogArea") == 0) {
        tok = f1.GetToken ();
        if (!tok)
            return TRUE;
        MsgLogAreaPath = GetAllocName (tok, GN_BSlash);
        MsgLogAreaType = MSGTYPE_SDM;
        if ((tok = f1.GetToken ()) != NULL)
            if (strcmp (tok, "-$") == 0) {
                MsgLogAreaType = MSGTYPE_SQUISH;
                strubslash (MsgLogAreaPath);
            }
        return TRUE;
    }

    if (strcasecmp (tok, "MsgRemArea") == 0) {
        tok = f1.GetToken ();
        if (!tok)
            return TRUE;
        MsgRemAreaPath = GetAllocName (tok, GN_BSlash);
        MsgRemAreaType = MSGTYPE_SDM;
        if ((tok = f1.GetToken ()) != NULL)
            if (strcmp (tok, "-$") == 0) {
                MsgRemAreaType = MSGTYPE_SQUISH;
                strubslash (MsgRemAreaPath);
            }
        return TRUE;
    }

    if (strcasecmp (tok, "MsgSize") == 0) {
        tok = f1.GetToken ();
        if (!tok)
            CfgError (f1);
        if (str2uint (tok, &MsgSize))
            CfgError (f1);
        MsgSize = __min (MsgSize, UINT_MAX - 256 - 160);
        return TRUE;
    }

    if (strcasecmp (tok, "MsgFromNode") == 0) {
        if ((tok = f1.GetToken ()) == NULL) {
            CfgError (f1);
            return TRUE;
        }
        strto4Dadr (tok, &MsgFromNode);
        return TRUE;
    }

    if (strcasecmp (tok, "MsgToNode") == 0) {
        if ((tok = f1.GetToken ()) == NULL) {
            CfgError (f1);
            return TRUE;
        }
        strto4Dadr (tok, &MsgToNode);
        return TRUE;
    }

    if (strcasecmp (tok, "MsgTo") == 0) {
        MsgTo = GetAllocLn (f1.RestOfLine ());
        return TRUE;
    }

    if (strcasecmp (tok, "MsgAttr") == 0) {
        MsgAttr = 0;
        tok = f1.RestOfLine ();
        if (strpbrk (tok, "Pp"))
            MsgAttr |= MSGPRIVATE;
        if (strpbrk (tok, "Kk"))
            MsgAttr |= MSGKILL;
        if (strpbrk (tok, "Cc"))
            MsgAttr |= MSGCRASH;
        if (strpbrk (tok, "Hh"))
            MsgAttr |= MSGHOLD;
        if (strpbrk (tok, "Dd"))
            MsgAttr |= (MSGCRASH|MSGHOLD);
        // no special action necessary for 'N' and 'O' //
        return TRUE;
    }

    if (strcasecmp (tok, "CostNullPhone") == 0) {
        tok = f1.GetToken ();      // cost
        if (tok) {
            CostNullPhone.cost = (word) atoi (tok);
            tok = f1.GetToken ();      // ucost
            if (tok)
                CostNullPhone.ucost = (word) atoi (tok);
            else
                CostNullPhone.ucost = CostNullPhone.cost;
        }
        return TRUE;
    }
 
    if (strcasecmp (tok, "CostVerbatimPhone") == 0) {
        tok = f1.GetToken ();      // cost
        if (tok) {
            CostVerbatimPhone.cost = (word) atoi (tok);
            tok = f1.GetToken ();      // ucost
            if (tok)
                CostVerbatimPhone.ucost = (word) atoi (tok);
            else
                CostVerbatimPhone.ucost = CostVerbatimPhone.cost;
        }
        return TRUE;
    }
 
    return FALSE;
}
