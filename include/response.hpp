/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1991-1997  Alberto Pasquale                 */
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

/* RESPONSE.Hpp */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <time.h>
#include <stdarg.h>
#include "types.hpp"
extern "C" {
#include <msgapi.h>
};

class _RSP {
public:
    HAREA msg;                /* Area handle */
    XMSG rsphd;              /* MSG header for response */
    char *rsptxt;            /* text (ASCIIZ); text + tear + Origin (MsgSize+160) */
    char *origin;           // Origin
    uint rsplen;             /* Lenght of rsptxt */
    uint part;               /* Part # of message response */
    uint partpos;            /* "Part #" ofset in subject */
    long unix4ascii;        // Unix time for ASCII field, to avoid false DUPES with Squish
    _RSP (void);
    ~_RSP (void);
};


// if origin == NULL -> no origin

_RSP *writerspheader (HAREA msg, char *from, ADR *fmadr, char *to, ADR *toadr,
                      char *subject, word flags, char *origin);

void vwritersp (_RSP *rsp, char *strfmt, va_list args);
void writersp (_RSP *rsp, char *strfmt,...);
// if rsp == NULL, the function returns immediately, with no error

void closersp (_RSP *rsp, BOOL thrash = FALSE);

void getmsgdate(struct _stamp *date, time_t timer);

dword uid (void);

void SetOrigin (char *buffer, char *origin, ADR *adr);


#endif
