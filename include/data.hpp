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

// Data.hpp

#ifndef DATA_HPP
#define DATA_HPP

#if defined (__OS2__)
  #define OS_2
#elif defined (__NT__)
  #define NT
#endif


#include <stdio.h>
#include "types.hpp"
#include "defines.hpp"
#include "v7.hpp"
#include "response.hpp"

extern "C" {
#include <smapi/msgapi.h>
};

extern long  StartTime;            // when FastLst began operating

extern FILE  *logfile;
extern BOOL  ApiOpened;

extern HAREA MsgLog,
             MsgRem;

extern _RSP  *MsgLogRsp,
             *MsgRemRsp;

extern int   errorlevel;

extern BOOL  Break;

extern int fb_cpos;           // columns for continuation (-1 = disabled)
extern char fb_cont;         // continuation character.

#ifdef __DOS__
extern dword MinMemFree;
#endif


#endif
