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

// Response.Cpp


#ifdef __OS2__
    #define INCL_DOS
    #include <os2.h>
#endif

#include <apgenlib.hpp>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "response.hpp"
#include "misc.hpp"
#include "msgapier.hpp"

void closekeeprsp (_RSP *rsp);


_RSP::_RSP (void)
{
    rsptxt = new char[MsgSize + 160];
}

_RSP::~_RSP (void)
{
    delete[] rsptxt;
}


_RSP *writerspheader (HAREA msg, char *from, ADR *fmadr, char *to, ADR *toadr, char *subject, word flags, char *origin)
{
    time_t timer;
    _RSP *rsp;
    XMSG *rsphd;

    rsp = new _RSP;
    rsp->msg = msg;
    rsphd = &rsp->rsphd;

    memset (rsphd, 0, sizeof (XMSG));
    rsphd->attr = flags | MSGLOCAL;
    strzcpy ((char *)rsphd->from, from, XMSG_FROM_SIZE);
    rsphd->orig = *fmadr;
    strzcpy ((char *)rsphd->to, to, XMSG_TO_SIZE);
    rsphd->dest = *toadr;
    strzcpy ((char *)rsphd->subj, subject, XMSG_SUBJ_SIZE);

    timer = time (NULL);
    rsp->unix4ascii = timer;   // save time for ASCII, to avoid dupes with Squish
    UnixToFTime ((char *)rsphd->__ftsc_date, timer);  // FTS-0001 Datime
    getmsgdate (&(rsphd->date_written), timer);
    rsphd->date_arrived = rsphd->date_written;

    rsp->rsplen = 0;
    rsp->part = 0;
    rsp->partpos = strlen ((char *)rsphd->subj);
    rsp->origin = origin;

    return rsp;
}


void vwritersp (_RSP *rsp, char *strfmt, va_list args)
{
    if (!rsp)
        return;

    char format[100];

    char *c = strfmt;     // convert \n to \r
    char *d = format;
    while (*c) {
        if (*c == '\n')
            *d = '\r';
        else
            *d = *c;
        c ++;
        d ++;
    }
    *d = '\0';

    char *rspline = new char[2048];

    vsprintf (rspline, format, args);

    uint linelen = strlen (rspline);

    if ((rsp->rsplen + linelen) > MsgSize) { /* assure correct termination */
        if (rsp->part == 0)
            rsp->part = 1;
        closekeeprsp (rsp);
        rsp->rsplen = 0;
    }
    strcpy (rsp->rsptxt+rsp->rsplen, rspline);
    rsp->rsplen += linelen;

    if ((rsp->rsplen > (MsgSize - 80)) &&   /* keep the full line together */
        (rspline[linelen-1] == '\r')) {
		if (rsp->part == 0)
			rsp->part = 1;
		closekeeprsp (rsp);
        rsp->rsplen = 0;
    }

    delete[] rspline;
}


void writersp (_RSP *rsp, char *strfmt,...)
{
    if (!rsp)
        return;

    va_list args;

    va_start (args, strfmt);
    vwritersp (rsp, strfmt, args);
    va_end (args);
}


void closekeeprsp (_RSP *rsp)
{
    char parts[30];
    char msgids[80];   /* ^A Kludge (ASCIIZ) for ^AMSGID */
    char buff[81];     /* Tear line or origin */
    HMSG msgh;

    if (rsp->part != 0) {
        sprintf (parts, " (Part #%u)", rsp->part++);
        strcpy ((char *)rsp->rsphd.subj + __min (rsp->partpos, XMSG_SUBJ_SIZE - 1 - strlen (parts)), parts);
        UnixToFTime ((char *)rsp->rsphd.__ftsc_date, rsp->unix4ascii - 1 + rsp->part);  // Unique ASCII time to avoid false dupes
    }

    sprintf (msgids, "MSGID: %s %08lX", addrs (&rsp->rsphd.orig), uid ());

    sprintf (buff, "\r\r--- FastLst "VER"\r");
    strcpy (rsp->rsptxt+rsp->rsplen, buff);
    rsp->rsplen += strlen (buff);
    if (rsp->origin) {
        SetOrigin (buff, rsp->origin, &rsp->rsphd.orig);
        strcpy (rsp->rsptxt+rsp->rsplen, buff);
        rsp->rsplen += strlen (buff);
    }

    msgh = MsgOpenMsg (rsp->msg, MOPEN_CREATE, 0);
    if (msgh == NULL) {
        wr_mapi_err ();
        return;
    }

    if (MsgWriteMsg (msgh, 0, &rsp->rsphd, (byte *)rsp->rsptxt, rsp->rsplen + 1, rsp->rsplen + 1, strlen (msgids) + 1, (byte *)msgids))
        wr_mapi_err ();

    if (MsgCloseMsg (msgh))
        wr_mapi_err ();
}


void closersp (_RSP *rsp, BOOL thrash)
{
    if (!thrash)
        closekeeprsp (rsp);
    delete rsp;
}


void getmsgdate(struct _stamp *date, time_t timer)
{
    struct tm *timeptr;

    timeptr=localtime(&timer);

    date->date.da = (word) timeptr->tm_mday;
    date->date.mo = (word) (timeptr->tm_mon + 1);
    date->date.yr = (word) (timeptr->tm_year - 80);
    date->time.ss = (word) (timeptr->tm_sec/2);
    date->time.mm = (word) (timeptr->tm_min);
    date->time.hh = (word) (timeptr->tm_hour);
}


dword uid (void)
{
    static dword lastid = 0;
    dword timeid;

    timeid = time (NULL) << 4;
    while (timeid <= lastid)
        timeid++;

    lastid = timeid;

    return (timeid);
}


void SetOrigin (char *buffer, char *origin, ADR *adr)
{
    char *c;

    c = addrs (adr);            // max 79chars in origin
    sprintf (buffer, " * Origin: %.*s (%s)\r", 65 - strlen (c), origin, c);
}



