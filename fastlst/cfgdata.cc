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

// CfgData.Cpp

#ifdef __OS2__
    #define INCL_DOS
    #include <os2.h>
#endif

#include <limits.h>
#include <bbsgenlb.hpp>
#include "data.hpp"
#include "misc.hpp"
#include "cfgdata.hpp"


char NullStr[] = "";        // zero-length string

BOOL nocrcexit = FALSE;
BOOL ForceComp = FALSE;
BOOL IgnoreDat = FALSE;

char *InputPath = NullStr,
     *ArcPath = NullStr,
     *DatFile = "fastlst.dat";

byte ArcDate = _ArcDateCreation_;

CS   CostNullPhone = { USHRT_MAX, 0 };
CS   CostVerbatimPhone = { 0, 0 };
CO   *co_head  = NULL;
DL   *dl_head  = NULL;
TD   *td_head  = NULL;
FD   *fd_head  = NULL;


BOOL killafter  = FALSE,
     killsource = FALSE,
     dash2comma = FALSE,
     noreport   = FALSE,
     V7BugFix   = FALSE,
     BitType    = FALSE,
     NoRedir    = FALSE;

char *BeforeKillSource = NULL;

OutncBlk *nocomp = NULL;   // Nocompile "output" block
OUTBLK   *outblk = NULL;   // head of out block list

AH_ComprCfg *Compr = NULL;    // pointer to istance of class of Compress Configuration

char  *MsgLogAreaPath = NULL,
      *MsgRemAreaPath = NULL;
word  MsgLogAreaType  = MSGTYPE_SDM,
      MsgRemAreaType  = MSGTYPE_SDM;

ADR   MsgFromNode,
      MsgToNode;
char  *MsgTo = "SysOp";
dword MsgAttr = 0;
uint  MsgSize = 7168;


ARCMETHOD::ARCMETHOD (const AH_Archiver *a, char initial)
{
    archiver = a;
    first = initial ? initial : a->ext ? *a->ext : 'a';
    next = NULL;
}

