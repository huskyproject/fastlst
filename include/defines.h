/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1996       Alberto Pasquale                 */
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

// defines.h

#ifndef _DEFINES_H_
#define _DEFINES_H_


#define __min(a,b)    (((a) < (b)) ? (a) : (b))
#define __max(a,b)    (((a) > (b)) ? (a) : (b))

#define MKushort(a) (*((ushort *)(&(a))))

#ifndef DIRSEP
#ifdef UNIX
#define DIRSEP '/'
#else
#define DIRSEP '\\'
#endif
#endif


#endif
