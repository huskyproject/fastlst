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

// ParseCfg.Hpp

#ifndef __PARSECFG_HPP_
#define __PARSECFG_HPP_

#include "export.hpp"
#include "parsetyp.hpp"


class Config {

  private:

    OUTBLK *cob;     // current outblk
    OUTBLK **ob;     // pointer to first outblk

    InpAll inpall;   // inpblk default (from global section)
    InpAll *cia;     // current InpAll blk
    InpNnc inpnnc;   // inpblk default (from global section)
    InpNnc *cin;     // current InpNnc blk

    InpLoc *cil;     // current InpLoc blk
    InpOut *cio;     // current InpOut blk
    CioThis ciothis; // current data for cio: pointers to tail of lists

    InpncBlk **cib;  // address of pointer to current inpblk

    SegAll segall;   // current "all" blk for Export.
    SegAll *csa;
    SegLoc *csl;     // current "loc" blk for Export.

    int cs;          // cfg parsing status

    time_t maxtime;  // max time of cfg files

    void ParseCfg (pcsz filename);

  public:
    Config ();

    time_t parse (pcsz filename); // returns max time of cfg files
};


int GetUserFlags (const char *p, word *flags);

// Adds to *flags the user defined flags specified in p.
// Other bits of *flags are NOT changed: it's up to you to correctly
// initialize *flags !
// Returns 0 on success; in case of invalid flags -1 is returned.

#endif
