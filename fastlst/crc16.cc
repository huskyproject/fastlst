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

// Crc16.Cpp

#include <string.h>
#include "defines.hpp"
#include "types.hpp"
#include "crc16tab.hpp"


/* updcrc macro derived from article Copyright (C) 1986 Stephen Satchell.
    NOTE: First argument must be in range 0 to 255.
 */

#define updcrc(cp, crc) ((crctab[((crc >> 8) & 255) ^ cp]) ^ (word)(crc << 8))


word crcstr (char *buf, word crc, int *len)
{
    char *p;

    p = buf;

    while (*p)
        crc = updcrc (*p++, crc);

    if (len)
	*len = int (p - buf);

    return (crc);
}
