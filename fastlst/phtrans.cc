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

// PhTrans.Cpp

#include "phtrans.hpp"
#include "parse.hpp"

#ifdef __OS2__
    #define INCL_DOS
    #include <os2.h>
#endif

#include <apgenlib.hpp>


PT::PT ()
{
    n = 0;
    cost = ucost = USHRT_MAX;
}


int PT::Get (pcsz cfg)
{
    pcsz s = cfg;
    char *tok;

    tok = GetToken (&s);
    if (!tok)
        return -1;
    cost = atoi (tok);

    tok = GetToken (&s);
    if (!tok)
        return -1;
    ucost = atoi (tok);

    int len;
    do {
      len = GetName (s, phtr[n], PHTR_SIZE);
      if (len < 0)
        return -1;      // error
      else if (len > 0)
        n ++;
    } while ((len > 0) && (n < PHTR_MAXN));

    if (*s)
        return 1;       // more entries on cfg

    return 0;           // All ok
}


int PT::Apply (word *cost, word *ucost, psz dest, pcsz src, int size)
{
    *cost = PT::cost;
    *ucost = PT::ucost;

    pcsz s = src;
    psz  d = dest;
    int len = 0;
    int ret = 0;

    while (*s) {

        int i = 0;

        while (i < n) {
            if (phtr[i][0] == *s) {
                char *st = &phtr[i][1];
                while (*st) {
                    if (len == size-1) {
                        ret = -1;
                        goto finish;
                    }
                    d[len++] = *(st++);
                }
                break;
            }
            i ++;
        }

        if (i == n) {               // not a trans char
            if (len == size-1) {
                ret = -1;
                goto finish;
            }
            d[len++] = *s;
        }

        s++;
    }

  finish:
    d[len] = '\0';

    return ret;
}


