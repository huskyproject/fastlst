/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1991-1994  Alberto Pasquale                 */
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

// MsgApiEr.Cpp


#include "msgapier.hpp"
#include <unistd.h>

void vprintlog (char[], ...);

void wr_mapi_err (void)
{
    switch (msgapierr) {
        case MERR_BADH:  vprintlog ("MSGAPI Error: Invalid handle !\n");
                         break;
        case MERR_BADF:  vprintlog ("MSGAPI Error: Corrupted file !\n");
                         break;
        case MERR_NOMEM: vprintlog ("MSGAPI Error: Not enough memory !\n");
                         break;
        case MERR_NODS:  vprintlog ("MSGAPI Error: Disk Full !\n");
                         break;
        case MERR_NOENT: vprintlog ("MSGAPI Error: File/Message not found !\n");
                         break;
        case MERR_BADA:  vprintlog ("MSGAPI Error: Bad argument !\n");
                         break;
        case MERR_EOPEN: vprintlog ("MSGAPI Error: Open messages: cannot close Area !\n");
                         break;
//        case MERR_NOLOCK:vprintlog ("MSGAPI Error: Base needs to be locked !\n");
//                         break;
//        case MERR_SHARE: vprintlog ("MSGAPI Error: Resource in use by other process !\n");
//                         break;
//        case MERR_EACCES:vprintlog ("MSGAPI Error: Access denied !\n");
//                         break;
//        case MERR_BADMSG:vprintlog ("MSGAPI Error: Bad Message Frame !\n");
//                         break;
//        case MERR_TOOBIG:vprintlog ("MSGAPI Error: Too much text/ctrlinfo !\n");
//                         break;

        default:         vprintlog ("MSGAPI Error: Unknown !\n");
    }
}


HAREA MsgSOpenArea (byte *name, word mode, word type)
{
    HAREA ret;
    int i;

    i = 0;
    ret = MsgOpenArea (name, mode, type);

    while ((ret == NULL) && (i < 10)) {
        sleep (1);
        i++;
        ret = MsgOpenArea (name, mode, type);
    };

    return ret;
}
