/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1992-1997  Alberto Pasquale                 */
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

// Data.Cpp

#include "data.hpp"
#include <limits.h>

long  StartTime;            // when FastLst began operating

FILE  *logfile = NULL;
BOOL  ApiOpened = FALSE;

HAREA MsgLog = NULL,
      MsgRem = NULL;

_RSP  *MsgLogRsp = NULL,
      *MsgRemRsp = NULL;

int errorlevel = OK;

BOOL Break = FALSE;

int fb_cpos = -1;           // columns for continuation (-1 = disabled)
char fb_cont = ' ';         // continuation character.


#ifdef __DOS__
    dword MinMemFree = ULONG_MAX;
#endif
