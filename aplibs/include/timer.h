/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1993-1994  Alberto Pasquale                 */
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
/* How to contact the author:  Alberto Pasquale of 2:332/504.1@fidonet.org   */
/*                             Viale Verdi 106                               */
/*                             41100 Modena                                  */
/*                             Italy                                         */
/*                                                                           */
/*****************************************************************************/

// Timer.h

#ifndef TIMER_H
#define TIMER_H


    APIRET __export Sleep (ULONG interval);     // sleep interval ms


    class __export PTIMER {                         // periodic timer
        ULONG start, interval;

    public:
        VOID Start (ULONG interval);       /* time in s/100 */
        BOOL Elapsed (VOID);
    };


    class __export TIMER {
        ULONG start;

    public:
        VOID Start (VOID);
        ULONG Elapsed (VOID);           /* elapsed in s/100 */
    };

#endif
