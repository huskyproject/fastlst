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

// FilesBbs.Hpp


void KillFile (char *fullname);
// Kills file and its description

void SetDesc (char *fullname, char *desc, int day, char *arcname);
// Sets description of fullname, using desc, with day as %d and arcname as %a

