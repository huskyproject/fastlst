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

#ifndef OUTBLK_HPP
#define OUTBLK_HPP

#include "types.hpp"
#include "inpblk.hpp"


#define OB_All        0
#define OB_NeededOnly 1     // for prepare
#define OB_NewOnly    2

//#pragma pack (1)        // necessary for OutSave (FastLst.Dat file) !

class OutSave {
  public:
    time_t PwdFileTime;

    OutSave (void);
};


        // types for OutLoc.NLname.nltype

#define NL_DAT 0x01
#define NL_NDX 0x02
#define NL_SDX 0x03
#define NL_DTP 0x04
#define NL_PDX 0x05

#define NL_DAs 0x81     // Dat & 0x80
#define NL_NDs 0x82
#define NL_SDs 0x83
#define NL_DTs 0x84
#define NL_PDs 0x85

#define MaxNL   2


class OutLoc {
  private:
    char filename[MaxNL][PATH_MAX];
    int filename_i;

  public:
    V7DATA v7data;
    char   *FidoUserLst;

    OutLoc (void);

    // return pointer to the created filename.
    // Do not use more than MaxNL calls in a single parameter line !
    // If killafter is not used, the NL_??s types are the same as NL_???.
    // Returns NULL if not available (NL_SD?, NL_DT?, NL_PD?)

    char *NLname (byte nltype);
};


class OutncBlk {        // for nocompile
  public:
    InpAll *a;
    InpncBlk *InpBlk;         // list of Input Blocks

    OutncBlk (InpAll *ia = NULL);
    void InpSavInit (void);
    BOOL ChkNew (void);         // TRUE if something new to process
    BOOL Prepare (int Action = OB_All, BOOL *SomeNew = NULL); // TRUE when prepared a needed list
    void KillSource (void);
};


class OUTBLK : public OutncBlk {     // output block info
  public:
    InpNnc  *n;
    InpOut  *o;
    SegAll  *sa;
    OutLoc  *l;
    OutSave *s;
    OUTBLK *next;

    OUTBLK (InpAll *ia = NULL, InpNnc *in = NULL, SegAll *isa = NULL);
    BOOL Process (void);        // true if some exported nodelists are NeededBeforeKill
    void KillSource (void);
    void PrepareExportNeeded (void);
    BOOL Prepare (int Action = OB_All, BOOL *SomeNew = NULL);

  private:
    void Open (OUTCUR *ocp);
    void Close (OUTCUR *ocp, BOOL Ok);
};


#endif
