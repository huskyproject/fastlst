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

// CfgData.hpp

#ifndef CFGDATA_HPP
#define CFGDATA_HPP

#include "types.hpp"
#include "defines.hpp"
#include "outblk.hpp"

class AH_Archiver;
class AH_ComprCfg;


class ARCMETHOD {
  public:
    const AH_Archiver *archiver;
    char first;
    class ARCMETHOD *next;
    ARCMETHOD (const AH_Archiver *a, char initial);
};


extern char NullStr[];        // zero-length string

extern BOOL nocrcexit;
extern BOOL ForceComp;
extern BOOL IgnoreDat;

extern char *InputPath,
            *ArcPath,
            *DatFile;
                            // for ArcDate

#define _ArcDateCreation_  0x00
#define _ArcDateWrite_     0x01

extern byte ArcDate;

extern CS   CostNullPhone;
extern CS   CostVerbatimPhone;
extern CO   *co_head;
extern DL   *dl_head;
extern TD   *td_head;
extern FD   *fd_head;


extern BOOL killafter,
            killsource,
            dash2comma,
            noreport,
            V7BugFix,
            BitType,
            NoRedir;

extern char *BeforeKillSource;

extern OutncBlk *nocomp;
extern OUTBLK   *outblk;

extern AH_ComprCfg *Compr;

extern char  *MsgLogAreaPath,
             *MsgRemAreaPath;
extern word  MsgLogAreaType,
             MsgRemAreaType;

extern ADR   MsgFromNode,
             MsgToNode;
extern char  *MsgTo;
extern dword MsgAttr;
extern uint  MsgSize;

#endif
