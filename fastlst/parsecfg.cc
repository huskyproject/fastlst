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

#include <limits.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#include "apgenlib.hpp"
#include "apgenlb2.hpp"
#include "bbsgenlb.hpp"
#include "types.hpp"
#include "parsetyp.hpp"
#include "misc.hpp"
#include "defines.hpp"
#include "parse.hpp"
#include "addrs.hpp"
#include "apgenlib.hpp"
#include "cfgdata.hpp"
#include "data.hpp"
#include "parsecfg.hpp"
#include "parseglb.hpp"
#include "parsecia.hpp"
#include "parsecin.hpp"
#include "parsecil.hpp"
#include "parsecio.hpp"
#include "parsecsa.hpp"
#include "parsecsl.hpp"

char *getallocline (const char *p)
{
    char *line;
    const char *d;
    int linelen;

    p = SkipBlank (p);      // begin

    d = p + strlen (p);     // 0 termination

    while (d > p) {
        if (!isBlank (*(d-1)))
            break;
        d--;
    }

    linelen = (int) (d - p);

    line = new char[linelen + 1];
    strncpy (line, p, linelen);
    line[linelen] = '\0';

    return line;
}


BOOL IsDayNode (char *name) // is name a variable named nodelist (.nnn) ?
{
    if (strchr (name, '*'))     // make sure no expandable char present
        return FALSE;

    int namelen = strlen (name);
    if (namelen < 5)            // at least 1 char + extension
        return FALSE;

    return (strcmp (name + namelen - 4, ".???") == 0);
}


void AddArcMethod (char *method, char first, ARCMETHOD **am)   // add method to cfg
{
    const AH_Archiver *a;

    if ((a = Compr->AddDefined (method)) == NULL) {
        vprintlog ("Error: %s not a defined compressing method.\n", method);
        myexit (CFG_ERROR);
    }

    while (*am)                 // go to last method
        am = &(*am)->next;

    *am = new ARCMETHOD (a, first);
}


BOOL GetArcMethods (const char *tkp, ARCMETHOD **am)  // FALSE on error
{
    char *tok;

    while ((tok = GetToken (&tkp)) != NULL) {

        char *c = strchr (tok, ',');
        if (c != NULL) {
            *c = '\0';
            c++;
            if (strlen (c) > 1) {
                vprintlog ("Error: only first character of extension allowed\n");
                return FALSE;
            }
        } else
            c = strchr (tok, '\0');

        AddArcMethod (tok, *c, am);
    }

    return TRUE;
}



ARCMETHOD *CopyArcMethod (ARCMETHOD *iam)
{
    ARCMETHOD *ret = NULL;
    ARCMETHOD **am = &ret;

    while (iam) {
        *am = new ARCMETHOD (iam->archiver, iam->first);
        am = &(*am)->next;
        iam = iam->next;
    }

    return ret;
}


int GetUserFlags (const char *p, word *flags)
{
    int ret = 0;

    while (*p) {
        switch (*p) {
            case '5': *flags |= 0x0020;
                      break;
            case '6': *flags |= 0x0040;
                      break;
            case '7': *flags |= 0x0080;
                      break;
            case '8': *flags |= 0x0100;
                      break;
            case '9': *flags |= 0x0200;
                      break;
            case 'a':
            case 'A': *flags |= 0x0400;
                      break;
            case 'b':
            case 'B': *flags |= 0x0800;
                      break;
            case 'd':
            case 'D': *flags |= 0x2000;
                      break;
            case 'e':
            case 'E': *flags |= 0x4000;
                      break;
            case 'f':
            case 'F': *flags |= 0x8000;
                      break;
            default : ret = -1;         // invalid flag
        }
        p++;
    }
    return ret;
}



#define StGlobal 1      // cia cin csa - global section
#define StNoComp 2      // cia - NoCompile section
#define StNoCInp 3      // cia cil - NodeList blk in NoCompile
#define StOutBlk 4      // cia cin cio csa - After version7, before first Nodelist of block
#define StOutInp 5      // cia cin cio cil csa - After Nodelist, in version7 output block
#define StExport 6      // csa csl - Export blk


#define CIB ((INPBLK **)cib)


Config::Config ()
{
    cob = NULL;         // current outblk
    ob = &outblk;       // pointer to first outblk

    cia = &inpall;      // current InpAll blk
    cin = &inpnnc;      // current InpNnc blk

    cil = NULL;         // current InpLoc blk
    cio = NULL;         // current InpOut blk

    cib = NULL;         // address of pointer to current inpblk

    csa = &segall;
    csl = NULL;         // current "loc" blk for Export.

    cs = StGlobal;      // cfg parsing status

    maxtime = 0;
}


time_t Config::parse (pcsz filename)
{                       // don't alloc memory to be released,
                        // since it would fragment memory due to the
                        // not released alloc-ed config data

    ParseCfg (filename);

    if (!dl_head) {
        vprintlog ("Dial table MUST be defined !\n");
        myexit (CFG_ERROR);
    }

    return maxtime;
}


void Config::ParseCfg (pcsz filename)
{
    maxtime = __max (maxtime, DosFileTime (filename));

    /* Need to read in the configuration options from control file */

    CfgFile f1 (CFGLINESIZE);

    if (f1.Open (filename)) {
        vprintlog ("Cannot open configuration file \"%s\"\n", filename);
        myexit (NO_CONFIG);
    }

    while (f1.GetLn ()) {
      pcsz tok = f1.GetToken ();

      if (cs == StGlobal) {                // Global section

        if (ParseGlb (f1))
            continue;

        if (strcasecmp (tok, "NoCompile") == 0) {
            nocomp = new OutncBlk (&inpall);
            cs = StNoComp;
            cib = &nocomp->InpBlk;
            cia = nocomp->a;
            cin = NULL;
            cio = NULL;
            cil = NULL;
            csa = NULL;
            continue;
        }

      }


      // everywhere...

        if ((strcasecmp (tok, "Version7") == 0) ||
            (strcasecmp (tok, "Version7+") == 0)) {

            cs = StOutBlk;

            cob = *ob = new OUTBLK (&inpall, &inpnnc, &segall);
            ob = &(cob->next);
            cia = cob->a;
            cin = cob->n;
            cio = cob->o;
            cil = NULL;

            csa = cob->sa;
            csl = NULL;

            cib = &cob->InpBlk;

            if (tok[8] == '+')
                cob->l->v7data.flags |= (V7DTP_F | V7PDX_F);

            char *v7path;

            pcsz tkp = f1.TokenPtr ();
            if (!tkp)
                CfgError (f1);
                                      // get V7 path
            tkp = getallocpath (tkp, &v7path, Slash | MkDir | Build);

            if (*tkp == '\0')       // no NODEX spec.
                CfgError (f1);
                                        // get NODEX
            tkp = getallocpath (tkp, &cob->l->v7data.nodex, Build, v7path);

            if (*tkp) {             // Sysop Index name

                tkp = getallocpath (tkp, &cob->l->v7data.sysopndx, Build, v7path, 4);
                if (!hasext (cob->l->v7data.sysopndx)) { // no extension given
                    if (strcasecmp (cob->l->v7data.nodex, cob->l->v7data.sysopndx) == 0)
                        strcat (cob->l->v7data.sysopndx, ".sdx");
                    else
                        strcat (cob->l->v7data.sysopndx, ".ndx");
                }

            } else if (cob->l->v7data.flags & V7DTP_F) {

                getallocpath (cob->l->v7data.nodex, &cob->l->v7data.sysopndx, Normal, NULL, 4);
                strcat (cob->l->v7data.sysopndx, ".sdx");

            }

            delete[] v7path;

            ciothis.Init (cio, &cob->s->PwdFileTime, cob->l->v7data.nodex);

            continue;
        }

        if (strcasecmp (tok, "Include") == 0) {        // Include
            tok = f1.GetToken ();
            if (!tok)
                CfgError (f1);
            vwritelog ("Including \"%s\"\n", tok);
            ParseCfg (tok);
            vwritelog ("Finished \"%s\"\n", tok);
            continue;
        }


      if (cs != StGlobal) {

        if (strcasecmp (tok, "NodeList") == 0) {

            pcsz tkp = f1.TokenPtr ();
            if (!tkp)
                CfgError (f1);

            if (*cib)
                cib = &((*cib)->next);

            switch (cs) {
                case StNoComp:
                case StNoCInp:
                    cs = StNoCInp;
                    *cib = new InpncBlk (nocomp->a);
                    cin = NULL;
                    cio = NULL;
                    break;
                case StOutBlk:
                case StOutInp:    
                case StExport:
                    cs = StOutInp;
                    *cib = new INPBLK (cob->a, cob->n, cob->sa);
                    cin = (*CIB)->n;
                    cio = (*CIB)->o;

                    csa = (*CIB)->sa;
                    csl = NULL;

                    break;
            }

            cia = (*cib)->a;
            cil = (*cib)->l;

            tkp = getallocpath (tkp, &cil->NodeList, Build, InputPath);

            if (cs == StOutInp) {
                get_part_addr (&tkp, &cil->PartAddr);
                ciothis.Init (cio, &cob->s->PwdFileTime, cil->NodeList);
            }
            cil->VarNodeList = IsDayNode (cil->NodeList);
            continue;
        }

      }


      if ((cs == StOutInp) || (cs == StExport)) {

        if (strcasecmp (tok, "Export") == 0) {

            pcsz tkp = f1.TokenPtr ();
            if (!tkp)
                CfgError (f1);

            SEGEXPORT **s = &(*CIB)->SegExpHead;
            while (*s)
                s = &(*s)->next;
            *s = new SEGEXPORT ((*CIB)->sa);

            if (strncmp (tkp, "+ ", 2) == 0) {
                (*s)->l->Append = TRUE;
                tok = f1.GetToken ();
                tkp = f1.TokenPtr ();
                if (!tkp)
                    CfgError (f1);
            }

            tkp = getallocpath (tkp, &(*s)->l->name, Build, InputPath);
            (*s)->l->Varname = IsDayNode ((*s)->l->name);
            if ((*s)->l->Varname && !(*cib)->l->VarNodeList) {
                vprintlog ("Cannot accept .??? extension in Export when NodeList is fixed.\n");
                CfgError (f1);
            }

            get_part_addr_lst (&tkp, &(*s)->l->Seg);

            cs = StExport;
            csa = (*s)->a;
            csl = (*s)->l;
            cia = NULL;
            cil = NULL;
            cin = NULL;
            cio = NULL;

            continue;
        }

      }


      if (cs == StOutBlk) {   // Output Block section

        if (strcasecmp (tok, "FidoUserLst") == 0) {
            tok = f1.GetToken ();
            getallocpath (tok ? tok : "fidouser.lst", &cob->l->FidoUserLst, Build);
            continue;
        }

        if (strcasecmp (tok, "LogStats") == 0) {
            cob->l->v7data.flags |= Stats_F;
            continue;
        }

        if (strcasecmp (tok, "LinkOnDisk") == 0) {
            cob->l->v7data.flags |= V7Dsk_F;
            continue;
        }

      }


      if (cia)
        if (ParseCia (f1, cia))
            continue;

      if (cin)      // NoComp excluded
        if (ParseCin (f1, cin))
            continue;

      if (cil)      // input blocks only (V7 and nocomp)
        if (ParseCil (f1, cil))
            continue;

      if (cio)      // V7 and its Nodelists
        if (ParseCio (f1, cio, &ciothis, cin))
            continue;

      if (csa)
        if (ParseCsa (f1, csa))
            continue;

      if (csl)
        if (ParseCsl (f1, csl))
            continue;

      vprintlogrsp (MsgLogRsp, "\nUnknown or out of sequence cfg line %d:\n", f1.LineN ());
      vprintlogrsp (MsgLogRsp, "%s\n\n", f1.ReGetLn ());
    }
    f1.Close ();

}

