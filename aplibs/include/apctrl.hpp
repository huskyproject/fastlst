/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1993-1998  Alberto Pasquale                 */
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

// ApCtrl.Hpp Ver. 1.00

#ifndef _APCTRL_HPP_
#define _APCTRL_HPP_


#ifdef __OS2__
  #define INCL_DOSSEMAPHORES
  #define INCL_DOSDATETIME
  #include <os2.h>
#endif

#include <typedefs.h>


#pragma pack (1)

                // Implemented in TIMERS.CPP


int Sleep (ulong interval);     // sleep interval ms (rounded to clock ticks)
                                // Returns 0 on success


        // The following class allows to know whether a specified
        // interval has elapsed since last time.

        // Start allows to set the interval and start the timer.

        // Elapsed returns TRUE when interval has elapsed
        // since Start or last time Elapsed returned TRUE.

        // Start can be used many times with different values.


class PTIMER {                         // periodic timer
    ulong start, interval;

public:
    void Start (ulong interval);       /* time in s/1000 */
    bool Elapsed ();                // True if interval has elapsed
};


        // The following class is useful to know the time
        // elapsed (in s/1000) from last call to the Start.


class TIMER {
    ulong start;

public:
    void Start ();
    ulong Elapsed ();             /* elapsed in s/1000 */
};



#ifdef __OS2__
                // Implemented in MUTEX.CPP


    // Class to implement a Mutually exclusive Semaphore

// Request to request access to the exclusive resource, blocking.
// Release to release the resource.


class Mutex {
    private:
        HMTX hmtx;  // Handle for Mutex Semaphore
    public:
        Mutex ();
        ~Mutex ();
        void Request ();
        void Release ();
};


// Automatic Request/Release of the Mutex object passed to creator

class AutoMutex {
    private:
        Mutex *MutexSem;
    public:
        AutoMutex (Mutex *MutexSem);
        ~AutoMutex ();
};

#endif


#endif
