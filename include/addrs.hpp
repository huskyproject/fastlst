/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1992-1997  Alberto Pasquale                 */
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

// Addrs.h


#ifndef ADDRS_H
#define ADDRS_H

#include "defines.hpp"

extern "C" {
#include <msgapi.h>
};

typedef NETADDR ADR;


struct EXTADR : public ADR {
    word region;
    word hub;
};


struct ADRLST {
   ADR adr;
   ADRLST *next;
};


void get_addr_2d (const char *p, ADR *adr);
// leave zone as is, get net/node, set point=0


BOOL get_addr (const char **adrs, ADR *adr);

// Skip blank, get 3D or 4D address, advance adrs to next token, return TRUE
// If no 3D/4D address, adr = 0:0/0.0, advance adrs to first token, return FALSE


BOOL get_next_addr (const char **adrs, ADR *adr, ADR *padr);

// Skip blank, get 3D or 4D address assuming zone, net, node from padr if
// necessary, advance adrs to next token, return TRUE
// If no address, advance adrs to first token, return FALSE.
// padr can point to the same object as adr !

ADRLST **get_addr_lst (const char **adrs, ADRLST **tail);

// get_next_addr while address available and store in ADRLST
// chain appending to tail, advance adrs to first non-address token,
// return the pointer to the tail.
// If no address, advance adrs to first token and return tail.


BOOL get_part_addr (const char **adrs, ADR *adr);

// Skip blank, get partial address (z, z:n, z:n/f, z:n/f.p, where n can be
// net or region), advance adrs to next token, return TRUE
// If no address, advance adrs to first token, return FALSE;
// All fields not stored are set to -1

BOOL get_part_addr (const char *adrs, EXTADR *adr);

// Skip blank, get partial address optionally followed by region and hub,
// return TRUE. If no address, return FALSE.
// zone/net/node/point not stored are set to -1.
// region/hub not stored are set to 0.
// e.g.: 2:332/505 33 500; 2:332/500 33; 2:332 33

ADRLST **get_part_addr_lst (const char **adrs, ADRLST **tail);

// get_part_addr while address available and append to ADRLST chain,
// advance adrs to next token, return pointer to the tail.
// If no address, advance adrs to first token, return tail.


BOOL InAdrLst (ADR *adr, ADRLST *al);

// Look for an address in a list, return TRUE if found, FALSE if not.


BOOL InPartAdrLst (const EXTADR *adr, ADRLST *pal);

// Look for an adress/region/hub in a partial-address list.


void SetAddr (ADR *adr, word zone, word net, word node, word point);

// Set Addr to the specified fields

#endif
