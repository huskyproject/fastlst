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
#include "bbsgenlb.hpp"
#include "cfgdata.hpp"
#include "inpblk.hpp"
#include "daydir.hpp"
#include "misc.hpp"
#include "edit.hpp"
#include "filesbbs.hpp"
#include "export.hpp"
#include <stdlib.h>
#include <string.h>

BOOL InpncBlk::ChkNew (void)  // must this nodelist trigger recompilation ?
{
    if (s->NodeTime == 0)  // this nodelist must be recompiled (new cfg)
        return TRUE;

    if (ChkNewList (s->NodeDay))
        return TRUE;

    if (l->NodeDiff)
        if (ChkNewDiff (s->NodeDay))
            return TRUE;

    if (l->ArcList)
        if (ChkNewArcList (s->NodeDay))
            return TRUE;

    if (l->ArcDiff)
        if (ChkNewArcDiff (s->NodeDay))
            return TRUE;

    return FALSE;
}

BOOL InpncBlk::ChkNewList (int LatestList)
{
    if (l->VarNodeList) {    // List is .nnn
        DAYDIR DayDir (NODELIST, this, LatestList);
        return DayDir.NewAvail ();
    } else {        // List is fixed
        time_t newtime = DosFileTime (l->NodeList);
        if (newtime != 0)
            if (newtime != s->NodeTime)    // this nodelist is new
                return TRUE;
        return FALSE;
    }
}


BOOL InpncBlk::ChkNewDiff (int LatestList)
{
    DAYDIR DayDir (NODEDIFF, this, LatestList, s->FutDiff);
    return DayDir.NewAvail ();
}


BOOL InpncBlk::ChkNewArcList (int LatestList)
{
    DAYDIR DayDir (ARCLIST, this, l->VarNodeList ? LatestList : -2);
    return DayDir.NewAvail ();
}


BOOL InpncBlk::ChkNewArcDiff (int LatestList)
{
    DAYDIR DayDir (ARCDIFF, this, LatestList, s->FutDiff);
    return DayDir.NewAvail ();
}


BOOL InpncBlk::Prepare (BOOL NeededOnly, BOOL *SomeNew)
{
    if (v->AlreadyPrepared || (NeededOnly && !a->NeededBeforeKill))
        return a->NeededBeforeKill;

    if (l->VarNodeList) {     // var name nodelist
        int LatestList = GetLatestList ();  // -1 if none available
        if (l->ArcList) {
            DoArcList (LatestList); // Unarc newer list if available
            LatestList = GetLatestList ();
        }
        if (LatestList != -1) {      // nodelist available
            if (l->ArcDiff)
                DoArcDiff (LatestList);
            if (l->NodeDiff)
                LatestList = ApplyNewDiffs (LatestList);
            SetDay3 (l->NodeList, LatestList);     // resolve NodeList
            if (short (LatestList) != s->NodeDay) {
                s->NodeDay = (short)LatestList;
                if (SomeNew)
                    *SomeNew = TRUE;
            }
        }
    } else {            // fixed name nodelist
        if (l->ArcList)
            DoArcList (fexist (l->NodeList) ? -2 : -1);  // UnArc new list if available
    }

    time_t newtime = DosFileTime (l->NodeList);
    if (newtime == 0) {
        vprintlog ("Cannot find \"%s\"\n", l->NodeList);
        myexit (NO_NODELIST);
    }

    if (s->NodeTime != newtime) {  // new nodelist: Arc it !
        if (l->ArcList) {
            if (l->ArcListKeep > 0)
                ExecArcList ();
            DAYDIR DayDir (ARCLIST, this);
            DayDir.KillOld ();
        }
        s->NodeTime = newtime;
        if (SomeNew)
            *SomeNew = TRUE;
    }

    v->AlreadyPrepared = TRUE;

    return a->NeededBeforeKill;
}


void InpncBlk::DoArcList (int LatestList)
{
    DAYDIR DayDir (ARCLIST, this, LatestList);

    if (DayDir.NewAvail ()) {
        char *ArcFile = DayDir.LatestName ();
        ExecUnarcList (ArcFile);
        s->ArcTime = arcfiletime (ArcFile);
    }

    DayDir.KillOld ();
}


void InpncBlk::DoArcDiff (int LatestList)
{
    DAYDIR DayDir (ARCDIFF, this, LatestList, s->FutDiff);

    if (l->NodeDiff) {
        char *arcdiffname = DayDir.FindNewName (TRUE);
        while (arcdiffname) {
            ExecUnarcDiff (arcdiffname);
            arcdiffname = DayDir.FindNewName ();
        }
    }

    DayDir.KillOld ();
}


int InpncBlk::ApplyNewDiffs (int LatestList)
{
    DAYDIR DayDir (NODEDIFF, this, LatestList, s->FutDiff);

    DayDir.KillOld ();

    int difres,
        NewList = LatestList;

    int day;

    if (l->ArcDiff) {
        day = DayDir.FindNewDay (TRUE); // arc diffs if ArcDiffMethod used
        while (day != -1) {
            ExecArcDiff (day);
            day = DayDir.FindNewDay ();
        }
    }

    day = DayDir.FindNewDay (TRUE);     // apply all diffs
    while (day != -1) {
        difres = ApplyDiff (NewList, day);
        if (difres == -1)       // ApplyDiff failed
            break;
        NewList = difres;
        day = DayDir.FindNewDay ();
    }

    if (NewList != LatestList) // If some Diff applied, do not check CRC again
        v->crcchk = FALSE;

    return NewList;
}


int InpncBlk::ApplyDiff (int ListDay, int DiffDay)    // -1 on error
{                                                   // DiffDay on success
    int ret;

    EDIT* edit = new EDIT (a->BeforeEdit, a->AfterEdit);
    switch (edit->Apply (l->NodeList, ListDay, l->NodeDiff, DiffDay)) {
        case 0:     // success
            s->FutDiff = 0;
            ret = DiffDay;
            break;
        case 1:     // day mismatch
            s->FutDiff = (short)DiffDay;
            ret = -1;
            break;
	default:
        case -1:    // error
            s->FutDiff = 0;
            ret = -1;
            break;
    }
    delete edit;

    return ret;
}


int InpncBlk::GetLatestList (void)  // 0 -> 366, -1 if none found
{
    DAYDIR DayDir (NODELIST, this);
    DayDir.KillOld ();
    return DayDir.LatestDay ();
}


void InpncBlk::ExecUnarcList (char *ArcFile)
{
    ExecUnarc (ArcFile, l->NodeList, a->BeforeUnArcList, a->AfterUnArcList);
}


void InpncBlk::ExecUnarcDiff (char *ArcFile)
{
    ExecUnarc (ArcFile, l->NodeDiff, a->BeforeUnArcDiff, a->AfterUnArcDiff);
}


void InpncBlk::ExecArcList ()
{
    time_t ftime = ExecArc (l->ArcList, l->NodeList, s->NodeDay,
             a->ArcMethHead, a->BeforeArcList, a->AfterArcList,
             l->ArcListDesc);

    s->ArcTime = __max (s->ArcTime, ftime); // make sure ArcTime is latest. Usually ftime is less than ArcTime
}


void InpncBlk::ExecArcDiff (int day)
{
    ExecArc (l->ArcDiff, l->NodeDiff, day,
             a->ArcDiffMethHead, a->BeforeArcDiff, a->AfterArcDiff,
             l->ArcDiffDesc);
}

