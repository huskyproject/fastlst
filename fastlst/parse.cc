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

// Parse.Cpp

#ifdef __OS2__
    #define INCL_DOS
    #include <os2.h>
#endif

#include <string.h>
#include <stdlib.h>
#include "parse.hpp"
#include "misc.hpp"

#define TokenSize 128

char Blank[] = " ";
char Num[]   = "0123456789";


BOOL isBlank (char c)
{
    if (strchr (Blank, c))
        return TRUE;
    else
        return FALSE;
}


char *SkipBlank (const char *c)
{
    return (char *)(c + strspn (c, Blank));
}


char *SkipNotBlank (const char *c)
{
    return (char *)(c + strcspn (c, Blank));
}


char *SkipToken (const char *c)
{
    c = SkipNotBlank (c);
    return (SkipBlank (c));
}


char *SkipNum (const char *c)
{
    return (char *)(c + strspn (c, Num));
}


char *GetToken (const char **c)         // get token and return ptr to token
{                                 // advance c, return NULL if no token
    static char Token[TokenSize];
    char *d;
    int toklen;

    *c = SkipBlank (*c);
    d = SkipNotBlank (*c);
    toklen = __min (TokenSize-1, (int) (d-(*c)));
    strncpy (Token, *c, toklen);
    Token[toklen] = '\0';
    d = SkipBlank (d);
    *c = d;
    if (*Token)
        return Token;
    else
        return NULL;
}



