/*****************************************************************************/
/*                                                                           */
/*                 (C)  Copyright 1993  by  Alberto Pasquale                 */
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

// MUTEX.H

// Class to implement a Mutually exclusive Semaphore

#ifndef MUTEX_H
#define MUTEX_H

#include "myos2.h"

class Mutex {
    private:
        HMTX hmtx;  // Handle for Mutex Semaphore
    public:
        Mutex (void);
        ~Mutex (void);
        void Request (void);
        void Release (void);
};


class AutoMutex {
    private:
        Mutex *MutexSem;
    public:
        AutoMutex (Mutex *MutexSem);
        ~AutoMutex (void);
};

#endif
