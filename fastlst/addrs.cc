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

// Addrs.Cpp

#ifdef __OS2__
    #define INCL_DOS
    #include <os2.h>
#endif

#include <stdio.h>
#include "addrs.hpp"
#include "misc.hpp"
#include "parse.hpp"


void get_addr_2d (const char *p, ADR *adr)    // leave zone as is, get net/node, set point=0
{
    int n = sscanf (p, "%hd/%hd", &adr->net, &adr->node);
    if (n != 2)
        adr->net = adr->node = 0;
    adr->point = 0;
}


BOOL get_addr (const char **adrs, ADR *adr)  // get 3 or 4D address
{                                      // and advance adrs
    int n;

    *adrs = SkipBlank (*adrs);

    n = sscanf (*adrs, "%hd:%hd/%hd.%hd", &adr->zone, &adr->net, &adr->node, &adr->point);
    if (n < 3) {
        SetAddr (adr, 0,0,0,0);
        return FALSE;
    }
    if (n < 4)
        adr->point = 0;

    *adrs = SkipToken (*adrs);
    return TRUE;
}


BOOL get_next_addr (const char **adrs, ADR *adr, ADR *padr)
{
    int n;
    ADR pAdr;

    pAdr = *padr;   // save assumed address

    if (get_addr (adrs, adr))       // 3 or 4D
        return TRUE;

    adr->point = 0;             // default point
    adr->zone = pAdr.zone;     // assumed zone

    n = sscanf (*adrs, "%hd/%hd.%hd", &adr->net, &adr->node, &adr->point);
    if (n >= 2) {
        *adrs = SkipToken (*adrs);
        return TRUE;
    }

    adr->net = pAdr.net;       // assumed net

    n = sscanf (*adrs, "%hd.%hd", &adr->node, &adr->point);
    if (n >= 1) {
        *adrs = SkipToken (*adrs);
        return TRUE;
    }

    adr->node = pAdr.node;     // assumed node

    n = sscanf (*adrs, ".%hd", &adr->point);
    if (n > 0) {
        *adrs = SkipToken (*adrs);
        return TRUE;
    } else
        return FALSE;
}


ADRLST **get_addr_lst (const char **adrs, ADRLST **tail)     // get list of addresses (starting w 3/4D)
{                                  // and advance adrs
    ADR adr = {0, 0, 0, 0};

    while (get_next_addr (adrs, &adr, &adr)) {
        *tail = new ADRLST;
        (*tail)->adr = adr;
        (*tail)->next = NULL;
        tail = &((*tail)->next);
    }

    return tail;
}


BOOL get_part_addr (const char **adrs, ADR *adr)  // get partial address
{                                           // starting w zone
    int n;                                  // and advance adrs

    *adrs = SkipBlank (*adrs);

    n = sscanf (*adrs, "%hd:%hd/%hd.%hd", &adr->zone, &adr->net, &adr->node, &adr->point);
    switch (n) {
        case EOF:
            n = 0;
        case 0:
            adr->zone = 0xFFFF;
        case 1:
            adr->net = 0xFFFF;
        case 2:
            adr->node = 0xFFFF;
        case 3:
            adr->point = 0xFFFF;
        case 4:
            ;
    }

    if (n == 0)
        return FALSE;

    *adrs = SkipToken (*adrs);
    return TRUE;
}


BOOL get_part_addr (const char *adrs, EXTADR *adr)
{
    adr->region = adr->hub = 0;

    if (!get_part_addr (&adrs, adr))
        return FALSE;

    sscanf (adrs, "%hu %hu", &adr->region, &adr->hub);

    return TRUE;
}


ADRLST **get_part_addr_lst (const char **adrs, ADRLST **tail) // list of partial addresses:
{                                   // CANNOT use abbreviations
    ADR adr;

    while (get_part_addr (adrs, &adr)) {
        *tail = new ADRLST;
        (*tail)->adr = adr;
        (*tail)->next = NULL;
        tail = &((*tail)->next);
    }

    return tail;
}


/* Is this address in the specified ADRLST ? */

BOOL InAdrLst (ADR *adr, ADRLST *al)
{
    while (al) {
        if (adrcmp (&al->adr, adr) == 0)
            return (TRUE);
        al = al->next;
    }

    return (FALSE);
}


BOOL InPartAdrLst (const EXTADR *adr, ADRLST *pal)
{
    while (pal) {
        if (adr->zone == pal->adr.zone) {
            if (pal->adr.net == word(-1))  // zone selected
                return (TRUE);
            if ((adr->region == pal->adr.net) && (pal->adr.node == word(-1))) // region
                return TRUE;
            if (adr->net == pal->adr.net) {
                if (pal->adr.node == word(-1)) // net
                    return (TRUE);
                if ((adr->hub == pal->adr.node) && (pal->adr.point == word(-1))) // hub
                    return TRUE;
                if (adr->node == pal->adr.node) {
                    if (pal->adr.point == word(-1))
                        return TRUE;
                    if (adr->point == pal->adr.point)
                        return TRUE;
                }
            }
        }
        pal = pal->next;
    }
    return (FALSE);
}


void SetAddr (ADR *adr, word zone, word net, word node, word point)
{
    adr->zone  = zone;
    adr->net   = net;
    adr->node  = node;
    adr->point = point;
}
