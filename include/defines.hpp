/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1992-1997  Alberto Pasquale                 */
/*                 Portions (C) Copyright 1999 Per Lundberg                  */
/*                                                                           */
/*                   A L L   R I G H T S   R E S E R V E D                   */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/* How to contact the author:  Alberto Pasquale of 2:332/504@fidonet         */
/*                             Viale Verdi 106                               */
/*                             41100 Modena                                  */
/*                             Italy                                         */
/*                                                                           */
/*****************************************************************************/

//  _DEBUG_ : For supplemental debug output

#ifndef DEFINES_HPP
#define DEFINES_HPP

#include "config.h"

#ifdef __linux
   #define VER VERSION"/Linux"
#elif defined (__FreeBSD__)
   #define VER VERSION"/FreeBSD"
#elif defined (__CYGWIN32__)
   #define VER VERSION"/Cygwin"
   #define BufSize 32768
#elif defined(UNIX)
   #define VER VERSION"/Unix"
#endif

#if defined(__linux) || defined(UNIX)
   #define BufSize 32768
#endif

typedef int BOOL;

#include "typedefs.h"

#define V7PLUS_VERSION 0x00     // version of current V7+

#define LINESIZE    512         // nodelist line size
#define SYSOPSIZE   100         // SysOp Name size
#define PHONESIZE   100         // Phone Size

typedef int (*QSF) (const void *, const void *);

                                    // errorlevels
#define OK                    0
#define HELP_REQ              1
#define OPEN_ERR              2
//                            3 abnormal termination
#define DISK_FULL             4
#define NO_CONFIG             5
#define CFG_ERROR             6
#define OUT_OF_MEMORY         7
#define READ_ON_DIFF          8
#define CRC_ON_DIFF           9
#define CRC_ON_LIST          10
#define USER_BREAK           11
#define NO_NEW               12   // error renaming new binary files
#define NO_NODELIST          13
#define ERR_TIMEOUT          14   // timeout trying to open binary nodelist
#define ERR_TOO_MANY_FILES   15
#define ERR_UNARCHIVE        16
#define ERR_UPDATE           17   // Updating files (index/data blocks)
#define NOTHING_COMPILED    100
#define MsgApiInitErr       250
#define AreaOpenErr         251
#define AreaLockErr         252
#define AreaCloseErr        253


#define TRUE 1
#define FALSE 0

#define HIGHER 7
#define ZONE   6
#define REGION 5
#define HOST   4
#define HUB    3
#define BOSS   2
#define NODE   1
#define NONE   0

#endif
