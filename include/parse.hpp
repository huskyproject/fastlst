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

// Parse.hpp

#include "defines.hpp"

#define IsDigit(c) ((c >= '0') && (c <= '9'))
BOOL isBlank (char c);
char *SkipBlank (const char *c);
char *SkipNotBlank (const char *c);
char *SkipToken (const char *c);
char *SkipNum (const char *c);
char *GetToken (const char **c);         // get token and return ptr to token
                                   // advance c, return NULL if no token

