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

#ifndef INPBLK_HPP
#define INPBLK_HPP

#include "types.hpp"
#include "daydir.hpp"
#include "iocur.hpp"
#include <time.h>

#define IB_NeededOnly TRUE     // for Prepare

class DAYDIR;               // forward reference
class OUTBLK;
class OutncBlk;
class ARCMETHOD;
class OutSave;
class SEGEXPORT;
class SegAll;


class InpAll {  // can be anywhere (global section, outblk, inpblk, nocomp)
  public:
    BOOL        NeededBeforeKill;
    char        *BeforeArcList;     // external commands
    char        *AfterArcList;      //
    char        *BeforeUnArcList;   //
    char        *AfterUnArcList;    //
    char        *BeforeArcDiff;     //
    char        *AfterArcDiff;      //
    char        *BeforeUnArcDiff;   //
    char        *AfterUnArcDiff;    //
    char        *BeforeEdit;        //
    char        *AfterEdit;         //
    ARCMETHOD   *ArcMethHead;   // list of ARC methods
    ARCMETHOD   *ArcDiffMethHead;

    InpAll (InpAll *ia = NULL);
    void Init (InpAll *ia = NULL);
};


#define MsgLogNullPhone     0x0001  // for MsgLog
#define MsgLogRedirected    0x0002
#define MsgLogPoints        0x0004


                                // bit-wise for InpNnc.flags

#define GermanPointLst      0x01
#define NoPointLstPhone     0x02


class InpNnc {  // not acceptable in NoCompile block
  public:
    char        *MsgRem;         // when !NULL, selected comments go to MsgRemArea msg.
    int         MsgLog;
    char        *BeforeCompile;     //
    char        *AfterCompile;      //
    char        *FidoTxt;
    char        *FidoPrn;
    int         IncCoord;
    byte        flags;

    InpNnc (InpNnc *in = NULL);
};


class InpOut { // can be in outblk or inpblk; lists that must NOT be inherited
  public:
    ADRLST      *IncAddr;
    ADRLST      *ExcAddr;
    ADRDATA     *Password;
    ADRDATA     *Phone;
    ADRDATA     *NodeFlags;
    ADRDATA     *Flags;
    ADRDATA     *Cost;
    uint        pwn;
    uint        phn;
    uint        nfn;
    uint        fln;
    uint        csn;

    InpOut (void);
};


class InpLoc {   // Nodelist specific data, cannot be inherited
  public:
    char        *NodeList;
    BOOL        VarNodeList;    // true when Nodelist.???
    EXTADR      PartAddr;
    char        *NodeDiff;
    char        *ArcList;
    int         ArcListKeep;
    char        *ArcListDesc;   // description for ArcLists
    char        *ArcDiff;
    int         ArcDiffKeep;
    char        *ArcDiffDesc;   // description for ArcDiffs

    InpLoc (void);
};


class InpVar {  // Locale variables
  public:
    BOOL        AlreadyPrepared;
    BOOL        crcchk;

    InpVar (void);
};

//#pragma pack (1)        // necessary for InpSave !

class InpSave {        // to be saved in FastLst.Dat
  public:
    time_t      NodeTime;
    time_t      ArcTime;
    short       NodeDay;
    short       FutDiff;        // day of oldest nodediff, future to NodeDay but not yet applicable; 0 if none.

    InpSave ();
    void Init ();
};


class InpncBlk {
  protected:
    InpAll      *a;
    InpLoc      *l;
    InpVar      *v;
    InpSave     *s;

    BOOL ChkNewList (int LatestList);
    BOOL ChkNewDiff (int LatestList);
    BOOL ChkNewArcList (int LatestList);
    BOOL ChkNewArcDiff (int LatestList);
    int  GetLatestList (void);
    void DoArcDiff (int LatestList);
    void DoArcList (int LatestList = -1);
    int  ApplyNewDiffs (int LatestList);
    int  ApplyDiff (int ListDay, int DiffDay); // return DiffDay, -1 on error
    void ExecUnarcList (char *ArcFile);
    void ExecUnarcDiff (char *ArcFile);
    void ExecArcList ();
    void ExecArcDiff (int day);

  public:
    InpncBlk    *next;

    InpncBlk (InpAll *ia = NULL);
    void savinit (void);
    BOOL ChkNew (void);     // TRUE if something new
    BOOL Prepare (BOOL NeededOnly = FALSE, BOOL *SomeNew = NULL); // TRUE when prepared a needed list
        // NeededOnly = TRUE -> prepare only nodelists needed before KillAfter
        // SomeNew is set to TRUE when the prepared nodelist is new.
    void KillSource (void);

    friend class DAYDIR;
    friend void read_cob (FILE *f, OutncBlk *cob, OutSave *os);
    friend int save_cob (FILE *f, OutncBlk *cob, OutSave *os);
    friend class Config;
};


class INPBLK : public InpncBlk {
  private:
    InpNnc      *n;
    InpOut      *o;
    SegAll      *sa;
    SEGEXPORT   *SegExpHead;    // Head of segments to be exported

    void Open (INPCUR *icp);
    void Close (INPCUR *icp);
    BOOL InSegment (const EXTADR *adr, ADRLST *cobIncAddr, ADRLST *cobExcAddr, ADRLST *IncAddr, ADRLST *ExcAddr);
    BOOL ProcessFile (char *fname, INPCUR *icp, OUTCUR *ocp, OUTBLK *cob, BOOL *SomeNeeded);

  public:
    INPBLK (InpAll *ia = NULL, InpNnc *in = NULL, SegAll *isa = NULL);  // constructor initializer

    BOOL Process (OUTCUR *ocp, OUTBLK *cob, BOOL *SomeNeeded); // FALSE on error
    void KillSource (void);

    friend class OUTBLK;
    friend class Config;
};


#endif
