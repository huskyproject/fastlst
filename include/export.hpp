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

// Export.hpp

#ifndef EXPORT_HPP
#define EXPORT_HPP

#include "types.hpp"
#include "defines.hpp"

class CfgFile;


time_t ExecArc (char *ArcName, char *name, int NodeDay, ARCMETHOD *ArcMethHead,
              char *BeforeArc, char *AfterArc, char *ArcDesc);
// returns time of NodeList (ArcList are touched to the same value).

void ExecUnarc (char *ArcFile, char *file, char *BeforeUnArc = NULL, char *AfterUnarc = NULL);

class SegAll {
  public:
    ARCMETHOD *ArcMethHead;
    char *BeforeArc;
    char *AfterArc;
    BOOL NeededBeforeKill;

  public:
    SegAll (SegAll *isa = NULL);
};


class SegLoc {
  public:
    char *name;
    char *sname;
    BOOL Varname;       // TRUE if name ending with ".???"
    BOOL Append;
    ADRLST *Seg;
    char *ArcName;      // has .??? extension
    int  ArcKeep;
    char *ArcDesc;

  public:
    SegLoc (void);

};


class SegVar {
  public:
    FILE *f;
    time_t FTime;
    int FDay;

  public:
    SegVar (void);
};


class SEGEXPORT {
  private:
    SegAll *a;
    SegLoc *l;
    SegVar *v;

  public:
    SEGEXPORT *next;

    SEGEXPORT (SegAll *isa = NULL);

  static BOOL OpenAll (SEGEXPORT *SegExpHead, time_t NodeTime, int NodeDay, char *NodeList);
  static void WriteAll (SEGEXPORT *SegExpHead, const EXTADR *adr, char *buff);
  static void CloseAll (SEGEXPORT *SegExpHead);
  static void KillSource (SEGEXPORT *SegExpHead);
  static void PrepareAllNeeded (SEGEXPORT *Head);

  friend BOOL ParseCil (CfgFile &f1, InpLoc *cil);
  friend class Config;
};


#endif
