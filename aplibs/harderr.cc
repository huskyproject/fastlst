/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1994-1996  Alberto Pasquale                 */
/*                 Portions (C) Copyright 1999 Per Lundberg                  */
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
/*   How to contact the author:  Alberto Pasquale of 2:332/504@fidonet       */
/*                               Viale Verdi 106                             */
/*                               41100 Modena                                */
/*                               Italy                                       */
/*                                                                           */
/*****************************************************************************/


/* HardErr.Cpp */

#include "apgenlib.hpp"
#include <stdlib.h>

#ifdef __NT__
 #include <windows.h>
#endif


#ifdef __DOS__

#pragma off (unreferenced)
int __far critical_error_handler (unsigned deverr, unsigned errcode, unsigned far *devhdr)
{
   return _HARDERR_FAIL;
}
#pragma on (unreferenced)

#endif


void HardErrDisable (void)
{
#ifdef __OS2__
    if (DosError (FERR_DISABLEHARDERR))
        abort ();
#elif defined (__NT__)
    SetErrorMode (SEM_FAILCRITICALERRORS);
#elif defined (__linux__)
#else   // __DOS__
    _harderr (critical_error_handler);
#endif
}

