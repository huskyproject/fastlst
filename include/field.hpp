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

// Field.hpp

extern char *&remainder;

char *nextfield (char *txt);

// Returns the pointer to next ',' divided token.
// As strtok (txt, ','), but returns the pointer to the null string
// instead of the NULL pointer when no more tokens available.
// If (txt != NULL) looks for ',' in txt, changes it to '\0' and returns
// txt. If (txt == NULL) looks for ',' starting after the previously
// found ',' and returns the pointer to the char after the previous ','.


