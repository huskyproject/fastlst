/*****************************************************************************/
/*                                                                           */
/*                 (C)  Copyright 1993-1997 Alberto Pasquale                 */
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

// ParseTyp.hpp

#ifndef PARSETYP_HPP
#define PARSETYP_HPP

#include "inpblk.hpp"
#include "parse.hpp"


char *fgetln (char *buff, int n, FILE *f, char **tkp);
char *getallocline (const char *p);
BOOL IsDayNode (char *name); // is name a variable named nodelist (.nnn) ?
BOOL GetArcMethods (const char *tkp, ARCMETHOD **am);  // FALSE on error
ARCMETHOD *CopyArcMethod (ARCMETHOD *iam);


class CioThis {
public:
    char    *NodeName;
    time_t  *PwdFileTime;
    ADRLST  **ia;
    ADRLST  **ea;
    ADRLST  **is;
    ADRDATA **pw;
    ADRDATA **ph;
    ADRDATA **nf;
    ADRDATA **fl;
    ADRDATA **cs;

    void Init (InpOut *cio, time_t *pft, char *name);
};


#endif


