/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1992-1996  Alberto Pasquale                 */
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

// DayDir.hpp

#ifndef DAYDIR_HPP
#define DAYDIR_HPP

#include "defines.hpp"
#include <time.h>

class InpncBlk;


#define NODELIST 0   // for Select
#define ARCLIST  1
#define NODEDIFF 2
#define ARCDIFF  3

#define DD_SIZE 400   // size of dd_data array


struct dd_data {
    short day;    // number of day, resolved for circular order
    char  first;  // initial character, don't care for 3 digit ext
};


class DAYDIR {      // for variable numeric extensions
  private:
    int  Select;
    char *mask;
    BOOL arc;
    char *fname;

    int  keep;
    BOOL VarList;
    time_t OldTime;

    int firstnew;       // index to first new entry
    int nfound;         // number of entries found
    dd_data *dir;       // pointer to array of directory data
    time_t latestime;   // Unix time of latest file in above dir
    time_t fixtime;     // Unix time of latest fixed extension
    char *fixext;       // Pointer to extension of latest fixed extension
    BOOL SrcNodeMissing;  // TRUE when Latest == -1 in constructor
    int FutDiff;        // first nodediff that caused a day mismatch, if != 0
    int FirstKeep (int keep);   // returns index to first entry to be kept; keep is the number of items to be kept (for each archiver)
    char *Name (int dd_idx=-1); // returns pointer to static area with full resolved name; if dd_idx omitted, fixext is taken
    int FindNew (BOOL first); // return first/next new index
    void CommInit (int Latest = -1);

  public:
    DAYDIR (int Select, InpncBlk *cib, int Latest=-1, int FutDiff = 0);
                // FutDiff is used for NodeDiff/ArcDiff
                // FutDiff is day of first nodediff that caused a day mismatch
    DAYDIR (int Select, char *List, int nKeep = 1);
    ~DAYDIR (void);

    BOOL NewAvail (void);   // returns TRUE if something new available
    char *FindNewName (BOOL first=FALSE);  // returns first/next new name
    int  FindNewDay (BOOL first=FALSE);   // returns first/next new day
    char *LatestName ();    // Returns name of latest file
    int  LatestDay (void);  // Returns day of latest file, -1 if none
    void KillOld (void);    // kills all files older than configured
    void KillAll (void);    // kills all files
};


#endif
