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

// Export.Cpp

#include <string.h>
#include <apgenlib.hpp>
#include <bbsgenlb.hpp>
#include <unistd.h>
#include "data.hpp"
#include "misc.hpp"
#include "export.hpp"
#include "parsetyp.hpp"
#include "filesbbs.hpp"


SegAll::SegAll (SegAll *isa)
{
    if (!isa)
        memset (this, 0, sizeof (*this));
    else {
        *this = *isa;
        ArcMethHead = CopyArcMethod (isa->ArcMethHead);
    }
}


SegLoc::SegLoc (void)
{
    memset (this, 0, sizeof (*this));
}


SegVar::SegVar (void)
{
    memset (this, 0, sizeof (*this));
}


SEGEXPORT::SEGEXPORT (SegAll *isa)
{
    a = new SegAll (isa);
    l = new SegLoc;
    v = new SegVar;
    next = NULL;
}


void SEGEXPORT::PrepareAllNeeded (SEGEXPORT *Head)
{
    DAYDIR *dd;
    BOOL exist;

    SEGEXPORT *s = Head;
    while (s) {
        if ((!s->a->NeededBeforeKill) || (s->v->f) || (!s->l->ArcName)) {   
            s = s->next;  // not necessary or just created or not archived
            continue;
        }

        if (s->l->Varname) {        // does the requested list already exist ?
            dd = new DAYDIR (NODELIST, s->l->name);
            exist = (dd->LatestDay () != -1);
            delete dd;
        } else {
            exist = fexist (s->l->name);
        }
        if (exist) {       
            s = s->next;
            continue;
        }                    // if not, decompress it

        dd = new DAYDIR (ARCLIST, s->l->ArcName);
        if (dd->NewAvail ())
            ExecUnarc (dd->LatestName (), s->l->name);
        delete dd;

        s = s->next;
    }
}


static BOOL ArcOk (char *ArcTpl, ARCMETHOD *amh, time_t NodeTime, int NodeDay = -1)
{
    char ArcName[PATH_MAX];

    strcpy (ArcName, ArcTpl);
    BOOL Var = (NodeDay > 0);

    char *p = ArcName + strlen (ArcName) - 3;   // pointer to extension
    if (Var)                               // day name
        sprintf (p+1, "%02d", NodeDay % 100);

    ARCMETHOD *am = amh;
    while (am) {
        if (Var) {
            *p = am->first;
            if (!fexist (ArcName))
                return FALSE;
        } else {
            strcpy (p, am->archiver->ext);
            if (arcfiletime (ArcName) != NodeTime)   // ArcName missing or not up to date
                return FALSE;
        }

        am = am->next;
    }

    return TRUE;
}


BOOL SEGEXPORT::OpenAll (SEGEXPORT *SegExpHead, time_t NodeTime, int NodeDay, char *NodeList)
{
    BOOL SomeNeeded = FALSE;        // Varname not accepted in cfg if NodeList is not variable

    SEGEXPORT *s = SegExpHead;
    while (s) {
        s->v->FTime = NodeTime;
        s->v->FDay  = s->l->Varname ? NodeDay : -1;
        if (s->l->Varname) {
            s->l->sname = newcpy (s->l->name);
            SetDay3 (s->l->sname, NodeDay);
        } else
            s->l->sname = s->l->name;
        time_t oldt = DosFileTime (s->l->sname);

        if (!IgnoreDat && !s->l->Append) {
            if (oldt == s->v->FTime)
                goto OAc;           // export file already existing, equal date
            if (s->l->Varname)
                if (oldt != 0)
                    goto OAc;       // export file already existing, equal day
            if (oldt == 0)      // export file not existing
                if (s->l->ArcName)
                    if (ArcOk (s->l->ArcName, s->a->ArcMethHead, NodeTime, s->v->FDay))
                        goto OAc;       // arced export file already existing
        }

        if (s->l->Varname) {
            DAYDIR *dd = new DAYDIR (NODELIST, s->l->name);
            dd->KillAll ();
            delete dd;
        }

        if (s->l->Append)
            s->v->f = SmartOpen (s->l->sname, ";\n; Start of New Export Segment\n;\n");
        else
            s->v->f = fopen (s->l->sname, "wt");

        if (s->v->f == NULL)
            vprintlogrsp (MsgLogRsp, "Cannot open \"%s\"\n", s->l->sname);
        else {
            SomeNeeded = SomeNeeded || s->a->NeededBeforeKill;
            vprintlogrsp (MsgLogRsp, "Exporting to \"%s\"\n", s->l->sname);
            if (fprintf (s->v->f, "; Exported from \"%s\"\n;\n", NodeList) < 0) {
                vprintlogrsp (MsgLogRsp, "Error writing to \"%s\"\n", s->l->sname);
                myexit (DISK_FULL);
            }
        }
  OAc:  s = s->next;
    }

    return SomeNeeded;
}


void SEGEXPORT::WriteAll (SEGEXPORT *SegExpHead, const EXTADR *adr, char *buff)
{
    SEGEXPORT *s = SegExpHead;
    while (s) {
        if (s->v->f) {
            if ((s->l->Seg == NULL) || InPartAdrLst (adr, s->l->Seg))
                if (fprintf (s->v->f, "%s\n", buff) < 0) {
                    vprintlogrsp (MsgLogRsp, "Error writing to \"%s\"\n", s->l->sname);
                    myexit (DISK_FULL);
                }
        }
        s = s->next;
    }
}


void SEGEXPORT::CloseAll (SEGEXPORT *SegExpHead)
{
    SEGEXPORT *s = SegExpHead;
    while (s) {
        if (s->v->f) {
            if (fclose (s->v->f)) {
                vprintlogrsp (MsgLogRsp, "Cannot close \"%s\"\n", s->l->sname);
                myexit (DISK_FULL);
            }
            if (!s->l->Append)
                if (!touchf (s->l->sname, s->v->FTime))
                    vprintlogrsp (MsgLogRsp, "Cannot set datime of \"%s\"\n", s->l->sname);
            if (s->l->ArcName) {
                DAYDIR *dd = new DAYDIR (ARCLIST, s->l->ArcName, s->l->ArcKeep - 1);
                dd->KillOld ();
                delete dd;
                ExecArc (s->l->ArcName, s->l->sname, s->v->FDay,
                         s->a->ArcMethHead, s->a->BeforeArc, s->a->AfterArc,
                         s->l->ArcDesc);
            }
        }
        if (s->l->Varname)
            delete[] s->l->sname;
        s = s->next;
    }
}


time_t ExecArc (char *ArcName, char *name, int NodeDay, ARCMETHOD *ArcMethHead, char *BeforeArc, char *AfterArc, char *ArcDesc)
{
    char *prm_a = new char[PATH_MAX];
    char *prm_f = new char[PATH_MAX];
    char *e_dir = new char[PATH_MAX];
    char *c_dir = new char[PATH_MAX];

    BOOL Var = (NodeDay > 0);

    strcpy (prm_a, ArcName);
    char *p = prm_a + strlen (prm_a) - 3;   // pointer to extension
    if (Var)                               // day name
        sprintf (p+1, "%02d", NodeDay % 100);

    strcpy (e_dir, name);        // already resolved
    MoveName (e_dir, prm_f);
    if (Var)                        // make sure there is correct extension
        SetDay3 (prm_f, NodeDay);

    mkdirpath (e_dir);          // make sure extract dir exists
    getcwd (c_dir, PATH_MAX);    // save current dir
    cdd (e_dir);                // change to extract dir

    time_t ftime = DosFileTime (prm_f);

    ARCMETHOD *am = ArcMethHead;
    while (am) {
        if (Var) {
            *p = am->first;
            if (fexist (prm_a)) {
                if (!toucharcfile (prm_a, ftime))
                    vprintlog ("Error setting date and time of \"%s\"\n", prm_a);
                am = am->next;
                continue;
            }
        } else {
            strcpy (p, am->archiver->ext);
            time_t arctime = arcfiletime (prm_a);
            if (arctime == ftime) {
                am = am->next;
                continue;
            }
            if (arctime > 0)
                DeleteFile (prm_a);
        }

        if (BeforeArc) {
            RunCmd (BeforeArc, RCf, "af", prm_a, prm_f);
            if (DosFileTime (prm_f) != ftime)
                if (!touchf (prm_f, ftime))
                    vprintlog ("Error re-setting date and time of \"%s\"\n", prm_f);
        }

        Compr->Arc (am->archiver, prm_a, prm_f);

        if (!toucharcfile (prm_a, ftime))
            vprintlog ("Error setting date and time of \"%s\"\n", prm_a);

        if (ArcDesc)
            SetDesc (prm_a, ArcDesc, NodeDay, am->archiver->name);

        if (AfterArc)
            RunCmd (AfterArc, RCf, "af", prm_a, prm_f);

        am = am->next;
    }

    cdd (c_dir);                // restore dir

    delete[] c_dir;
    delete[] e_dir;
    delete[] prm_f;
    delete[] prm_a;

    return ftime;
}


void ExecUnarc (char *ArcFile, char *file, char *BeforeUnArc, char *AfterUnarc)
{
    char *prm_f = new char[PATH_MAX];
    char *e_dir = new char[PATH_MAX];
    char *c_dir = new char[PATH_MAX];

    strcpy (e_dir, file);
    MoveName (e_dir, prm_f);

    BOOL FFix = (strpbrk (prm_f, "?*") == NULL);  // no wildcards: prm_f fixed

    mkdirpath (e_dir);          // make sure extract dir exists
    getcwd (c_dir, PATH_MAX);    // save current dir
    cdd (e_dir);                // change to extract dir

    if (BeforeUnArc)
        RunCmd (BeforeUnArc, RCf, "af", ArcFile, prm_f);

    if (FFix)                           // If prm_f fixed -> delete
        if (Exist (prm_f))
            if (tunlink (prm_f, 60))
                myexit (ERR_UNARCHIVE);

    Compr->UnArc (ArcFile, prm_f);

    if (FFix) {                         // If prm_f fixed -> touch ArcFile
        time_t nodetime = DosFileTime (prm_f);
        if (nodetime == 0)
            myexit (ERR_UNARCHIVE);
        if (!toucharcfile (ArcFile, nodetime))
            vprintlog ("Error setting date and time of \"%s\"\n", ArcFile);
    }

    if (AfterUnarc)
        RunCmd (AfterUnarc, RCf, "af", ArcFile, prm_f);

    cdd (c_dir);                // restore dir

    delete[] c_dir;
    delete[] e_dir;
    delete[] prm_f;
}


void SEGEXPORT::KillSource (SEGEXPORT *SegExpHead)
{
    SEGEXPORT *s = SegExpHead;
    while (s) {
        if (s->l->ArcName && s->a->ArcMethHead) {
            if (!s->l->Varname)
                DeleteFile (s->l->name, CHK_EXIST);
            else {
                DAYDIR *dd = new DAYDIR (NODELIST, s->l->name);
                dd->KillAll ();
                delete dd;
            }
        }
        s = s->next;
    }
}


