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

#include "apgenlib.hpp"
#include "types.hpp"
#include "outblk.hpp"
#include "misc.hpp"
#include "cfgdata.hpp"
#include "data.hpp"
#include "export.hpp"
#include "inmem.hpp"
#include <stdlib.h>
#include <string.h>
#if !defined(__FreeBSD__)
  #include <malloc.h>
#endif
#include <unistd.h>


OutLoc::OutLoc (void)
{
    memset (this, 0, sizeof (*this));
}


char *OutLoc::NLname (byte nltype)
{
    char *p = filename[filename_i++];
    filename_i %= MaxNL;

    switch (nltype & 0x7F) {

        case NL_DAT:
            sprintf (p, "%s.dat", v7data.nodex);
            break;

        case NL_NDX:
            sprintf (p, "%s.ndx", v7data.nodex);
            break;

        case NL_SDX:
            if (!v7data.sysopndx)
                return NULL;
            strcpy (p, v7data.sysopndx);
            break;

        case NL_DTP:
            if (!(v7data.flags & V7DTP_F))
                return NULL;
            sprintf (p, "%s.dtp", v7data.nodex);
            break;

        case NL_PDX:
            if (!(v7data.flags & V7PDX_F))
                return NULL;
            sprintf (p, "%s.pdx", v7data.nodex);
            break;
    }


    if (killafter && (nltype & 0x80))
        p[strlen(p)-1] = '$';

    return p;
}




OutSave::OutSave (void)
{
    memset (this, 0, sizeof (*this));
}


OutncBlk::OutncBlk (InpAll *ia)
{
    a = new InpAll (ia);
    InpBlk = NULL;
}


void OutncBlk::InpSavInit (void)
{
    InpncBlk *cib = InpBlk;
    while (cib) {
        cib->savinit ();
        cib = cib->next;
    }
}


OUTBLK::OUTBLK (InpAll *ia, InpNnc *in, SegAll *isa)
{
    a->Init (ia);
    n = new InpNnc (in);
    o = new InpOut;
    sa = new SegAll (isa);
    l = new OutLoc;
    s = new OutSave;
    next = NULL;
}


static void SizeReport (const OUTCUR *ocp, _RSP *rsp)
{
   printf ("\n");
   vprintlogrsp (rsp, "+-----------------------------------+\n");
   vprintlogrsp (rsp, "|Total Systems              = %6lu|\n", ocp->nnodes + ocp->npoints + ocp->ndown + ocp->nunknown);
   vprintlogrsp (rsp, "+-----------------------------------+\n");
   vprintlogrsp (rsp, "|Zone Coordinators          = %6lu|\n", ocp->nzones);
   vprintlogrsp (rsp, "|Regional Coordinators      = %6lu|\n", ocp->nregions);
   vprintlogrsp (rsp, "|Network Coordinators       = %6lu|\n", ocp->nnets);
   vprintlogrsp (rsp, "|Hub Coordinators           = %6lu|\n", ocp->nhubs);
   vprintlogrsp (rsp, "+-----------------------------------+\n");
   vprintlogrsp (rsp, "|Total Nodes                = %6lu|\n", ocp->nnodes);
   vprintlogrsp (rsp, "|Total Points               = %6lu|\n", ocp->npoints);
   vprintlogrsp (rsp, "|Total Down Systems         = %6lu|\n", ocp->ndown);
 if (ocp->nunknown)
   vprintlogrsp (rsp, "|Total Unknown Systems      = %6lu|\n", ocp->nunknown);
   vprintlogrsp (rsp, "+-----------------------------------+\n");
 if (ocp->nexcluded)
   vprintlogrsp (rsp, "|Excluded Systems           = %6lu|\n", ocp->nexcluded);
   vprintlogrsp (rsp, "|Total Compiled Systems     = %6lu|\n", ocp->nnodes + ocp->npoints - ocp->nexcluded);
   vprintlogrsp (rsp, "+-----------------------------------+\n");
 if (!NoRedir)
   vprintlogrsp (rsp, "|ReDirected Systems         = %6lu|\n", ocp->nredirect);
   vprintlogrsp (rsp, "|Null Phone Systems         = %6lu|\n", ocp->nnullphone);
   vprintlogrsp (rsp, "+-----------------------------------+\n");
   vprintlogrsp (rsp, "|Unique Addresses           = %6lu|\n", ocp->naddr);
 if (ocp->nphones)
   vprintlogrsp (rsp, "|Unique Phone Numbers       = %6lu|\n", ocp->nphones);
 if (ocp->nsysops)
   vprintlogrsp (rsp, "|Unique SysOp Names         = %6lu|\n", ocp->nsysops);
   vprintlogrsp (rsp, "+-----------------------------------+\n");
}


static int renameoutfile (const char *tmpname, const char *newname)
{
    if (access (newname, 0) == 0)       // erase existing file
        EraseFile (newname, 30);

    if (rename (tmpname, newname)) {    // rename temp -> new
        vprintlog ("Cannot rename \"%s\" to \"%s\"\n", tmpname, newname);
        errorlevel = NO_NEW;
        return -1;
    }

    return 0;           // All Ok
}


static void renameout (OutLoc *l)
{
    if (renameoutfile (l->NLname (NL_DAs), l->NLname (NL_DAT)))
        return;

    renameoutfile (l->NLname (NL_NDs), l->NLname (NL_NDX));

    if (l->v7data.sysopndx)
        renameoutfile (l->NLname (NL_SDs), l->NLname (NL_SDX));

    if (l->v7data.flags & V7DTP_F)
        renameoutfile (l->NLname (NL_DTs), l->NLname (NL_DTP));

    if (l->v7data.flags & V7PDX_F)
        renameoutfile (l->NLname (NL_PDs), l->NLname (NL_PDX));
}


BOOL OUTBLK::Process (void)         // public
{
    BOOL SomeNeeded = FALSE;        // True if some exported lists are NeededBeforeKill

    OUTCUR outcur (o);

    Busy *bsy = new Busy (l->v7data.nodex);     // Name for .bsy flag

    if (!killafter)                 // Make the .bsy
        if (bsy->Wait (60)) {
            vprintlogrsp (MsgLogRsp, "\nTimeOut on \"%s.bsy\"\n\n", l->v7data.nodex);
            errorlevel = ERR_TIMEOUT;
            delete bsy;
            return SomeNeeded;
        }

    Open (&outcur); /* Open output files for current OUTBLK */

    vprintlogrsp (MsgLogRsp, "\nCompiling to \"%s\"\n\n", l->v7data.nodex);

    BOOL Ok = TRUE;

    INPBLK *cib = (INPBLK *)InpBlk;
    while (cib) {
        if (!cib->Process (&outcur, this, &SomeNeeded)) {
            Ok = FALSE;
            break;
        }
        cib = (INPBLK *)cib->next;
    }

    outcur.FreeTab ();

    printf ("%15s\n", "");

    vprintlog ("Nodelist Input Completed.\n\n");

    Close (&outcur, Ok);           /* Close output files for current OUTBLK */

    if (!Ok) {
        vprintlog ("Processing of \"%s\" aborted\n\n", l->v7data.nodex);
        delete bsy;
        return SomeNeeded;
    }

    if (killafter) {
        if (bsy->Wait (60)) {
            vprintlogrsp (MsgLogRsp, "\nTimeOut on \"%s.bsy\"\n\n", l->v7data.nodex);
            errorlevel = ERR_TIMEOUT;
            delete bsy;
            return SomeNeeded;
        }
        renameout (l);
    }

    delete bsy;

    if (!noreport)
        SizeReport (&outcur, l->v7data.flags & Stats_F ? MsgLogRsp : NULL);

    vprintlogrsp (MsgLogRsp, "\nCompilation of \"%s\" Completed.\n\n", l->v7data.nodex);

    return SomeNeeded;
}


static dword WriteDtpHead (FILE *f)
{
    const _DTPHead DTPHead = {
        {
          sizeof (_DTPCtl),
          V7PLUS_VERSION,
          sizeof (_DTPAllLnk),
          sizeof (_DTPNodeLnk)
        },{
          0,            // ndowns (top level nodes)
          OfsNoLink     // pointer to first downlink (first top node)
        }
    };

    if (fwrite (&DTPHead, sizeof (DTPHead), 1, f) != 1)
        DiskFull (f);

    return sizeof (DTPHead);
}


void OUTBLK::Open (OUTCUR *ocp)  // open out files for current OUTBLK
{
                                        // Open DAT/DA$
    if ((ocp->nodex_dat = fopen (l->NLname (NL_DAs), "wb")) == NULL) {
    	vprintlog ("Could not open V7 Nodelist Index File \"%s\"\n", l->NLname (NL_DAs));
    	myexit(ERR_WRITE_V7);
    }
    if (l->v7data.flags & V7DTP_F) {        // Open DTP/DT$
        ocp->nodex_dtp = fopen (l->NLname (NL_DTs), "wb");
        ocp->dtpofs = WriteDtpHead (ocp->nodex_dtp);
    }

    ocp->heap->Open ();
}


static void MyClose (FILE *f)
{
    if (fclose (f) == EOF)
        DiskFull (f);
}


void OUTBLK::Close (OUTCUR *ocp, BOOL Ok)
{
    if (Ok)
        ocp->heap->Close ();        // build unique block of pointers

    MyClose (ocp->nodex_dat);       // close DAT/DA$

    if (l->v7data.flags & V7DTP_F)             // close DTP/DT$
        MyClose (ocp->nodex_dtp);

    if (!Ok) {
        unlink (l->NLname (NL_DAs));
        if (l->v7data.flags & V7DTP_F)
            unlink (l->NLname (NL_DTs));
        return;
    }

    printflush ("Creating Address Index\n");
    ocp->naddr = ocp->heap->MkNodeNdx (l->NLname (NL_NDs));

    if (l->v7data.flags & V7DTP_F) {
        ocp->heap->DTPLnkOpen ();

        printflush ("Linking Fido Hierarchy\n");
        ocp->heap->FidoLnk ();
    }

    if (l->v7data.flags & V7PDX_F) {
        printflush ("Creating Phone Index\n");
        ocp->nphones = ocp->heap->MkPhLst (l->NLname (NL_PDs));
    }

    if (l->v7data.sysopndx || l->FidoUserLst) {
        printflush ("Creating SysOp Index\n");
        ocp->nsysops = ocp->heap->MkSysLst (l->NLname (NL_SDs), l->FidoUserLst);
    }

    if (l->v7data.flags & V7DTP_F) {
        printflush ("Linking DTP file");
        ocp->heap->DTPLnkClose (l->NLname (NL_DTs), l->v7data.flags & V7Dsk_F);
        printflush ("\n");
    }

}


BOOL OutncBlk::ChkNew (void)
{
    BOOL New = FALSE;
    InpncBlk *cib = InpBlk;
    while (cib && !New) {
        New = cib->ChkNew ();
        cib = cib->next;
    }
    return New;
}


void OUTBLK::PrepareExportNeeded (void)
{
    INPBLK *cib = (INPBLK *)InpBlk;
    while (cib) {
        SEGEXPORT::PrepareAllNeeded (cib->SegExpHead);
        cib = (INPBLK *)cib->next;
    }
}


BOOL OUTBLK::Prepare (int Action, BOOL *SomeNew)
{
    BOOL SomeNeededBeforeKill = OutncBlk::Prepare (Action, SomeNew);
    if (Action == OB_NeededOnly)
        PrepareExportNeeded ();
    return SomeNeededBeforeKill;
}


BOOL OutncBlk::Prepare (int Action, BOOL *SomeNew)
{
    BOOL SomeNeededBeforeKill = FALSE;

    InpncBlk *cib = InpBlk;
    while (cib) {
        if (Action == OB_NewOnly)
            if (!cib->ChkNew ()) {
                cib = cib->next;
                continue;
            }
        if (cib->Prepare (Action == OB_NeededOnly, SomeNew))
            SomeNeededBeforeKill = TRUE;
        cib = cib->next;
    }
    return SomeNeededBeforeKill;
}


void OUTBLK::KillSource (void)
{
    INPBLK *cib = (INPBLK *)InpBlk;
    while (cib) {
        cib->KillSource ();
        cib = (INPBLK *)cib->next;
    }
}


void OutncBlk::KillSource (void)
{
    InpncBlk *cib = InpBlk;
    while (cib) {
        cib->KillSource ();
        cib = cib->next;
    }
}

