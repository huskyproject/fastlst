/*****************************************************************************/
/*                                                                           */
/*                (C) Copyright 1992-94  by  Alberto Pasquale                */
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

// Edit.hpp

#ifndef EDIT_HPP
#define EDIT_HPP

#include <stdio.h>
#include "types.hpp"


class EDIT {
    private:
      char *BeforeCmd,
           *AfterCmd;

      char *NodeList,
           *NodeDiff;

      char *old_name,
           *diff_name,
           *new_name;

      int  ListDay,
           DiffDay;

      FILE *Old,
           *New,
           *Diff;

      int  Open (void); // 0 = success, -1 = error, 1 = day mismatch
      int  DoEdit (void);   // 0 = success, -1 = error
      void Close (BOOL Ok);

    public:
      EDIT (char *BeforeCmd=NULL, char *AfterCmd=NULL);
      ~EDIT (void);
      int Apply (char *NodeList, int ListDay, char *NodeDiff, int DiffDay); // 0 = success, -1 = error, 1 = day mismatch
};

#endif

