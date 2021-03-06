/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1993-1997  Alberto Pasquale                 */
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

#include <stdio.h>
#include <time.h>
#include "apctrl.hpp"

#if defined(__CYGWIN32__)
#include <unistd.h>
#endif

int Sleep (ulong interval)   /* wait interval ms (rounded to clock ticks) */
{
  int ret = 0;
#if defined(__CYGWIN32__)
   usleep( interval * 1000);
#else
   timespec t;
   t.tv_sec = 0;
   t.tv_nsec = interval * 1000000;
   ret = nanosleep  (&t, &t);
#endif
  return(ret);
}

void PTIMER::Start (ulong interval) {          /* time in s/1000 */
    start = clock ();
    PTIMER::interval = interval;
}


bool PTIMER::Elapsed () {
    ulong now = clock ();
    if ((now - start) >= interval) {
        start = now;
        return true;
    } else
        return false;
}


void TIMER::Start () {
    start = clock ();
}

ulong TIMER::Elapsed () {                   /* time in s/1000 */
    return (clock () - start);
}

