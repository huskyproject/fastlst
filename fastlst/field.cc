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

// Field.Cpp

#include "field.hpp"


/* Returns the next field that ends with a comma */

static char *p;
char *&remainder = p;

char *nextfield (char *txt)
{
    char *f;

    if (txt)
        p = txt;

    f = p;

    while (*p) {
        if (*p == ',') {
            *p = '\0';
            p ++;       // advance to next field if available
            break;
        }
        p++;
    }

    return f;
}

