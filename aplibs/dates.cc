/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1995-1997  Alberto Pasquale                 */
/*                 Portions (C) Copyright 1999 Per Lundberg                  */
/*                                                                           */
/*                   A L L   R I G H T S   R E S E R V E D                   */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*   How to contact the author:  Alberto Pasquale of 2:332/504@fidonet       */
/*                               Viale Verdi 106                             */
/*                               41100 Modena                                */
/*                               Italy                                       */
/*                                                                           */
/*****************************************************************************/

#include "apgenlib.hpp"
#include <string.h>


#define YEAR(t)     (((t & 0xFE00) >> 9) + 1980)
#define MONTH(t)    ((t & 0x01E0) >> 5)
#define DAY(t)      (t & 0x001F)
#define HOUR(t)     ((t & 0xF800) >> 11)
#define MINUTE(t)   ((t & 0x07E0) >> 5)
#define SECOND(t)   ((t & 0x001F) << 1)


time_t dosdatimetounix (ushort date, ushort time)
{
    if ((date == 0) && (time == 0))
        return 0;

    struct tm tm;

    tm.tm_sec    = SECOND (time);
    tm.tm_min    = MINUTE (time);
    tm.tm_hour   = HOUR   (time);
    tm.tm_mday   = DAY    (date);
    tm.tm_mon    = MONTH  (date) - 1;
    tm.tm_year   = YEAR   (date) - 1900;
    tm.tm_isdst  = -1;  // compute this field

    return mktime (&tm);
}

/*
time_t DosDatime2Unix (const dosdate_t *ddate, const dostime_t *dtime)
{
    tm tm;

    memset (&tm, 0, sizeof (tm));

    tm.tm_mday = ddate->day;
    tm.tm_mon  = ddate->month - 1;
    tm.tm_year = ddate->year - 1900;
    tm.tm_isdst = -1;

    if (dtime) {
        tm.tm_hour = dtime->hour;
        tm.tm_min  = dtime->minute;
        tm.tm_sec  = dtime->second;
    }

    return mktime (&tm);
}
*/


time_t stamptounix (_stamp *stamp)
{
    return dosdatimetounix (((_dos_st *)stamp)->date, ((_dos_st *)stamp)->time);
}




void unix2dosdatime (time_t utime, ushort *date, ushort *time)
{
    _stamp datime;
    unix2stamp (utime, &datime);
    *date = MKushort (datime.date);
    *time = MKushort (datime.time);
}

/*
void Unix2DosDatime (time_t utime, dosdate_t *ddate, dostime_t *dtime)
{
    tm *tm;
    tm = localtime (&utime);

    if (ddate) {
        ddate->day          = tm.tm_mday;
        ddate->month        = tm.tm_mon + 1;
        ddate->year         = tm.tm_year + 1900;
        ddate->dayofweek    = tm.tm_wday;
    }

    if (dtime) {
        dtime->hour     = tm.tm_hour;
        dtime->minute   = tm.tm_min;
        dtime->second   = tm.tm_sec;
        dtime->hsecond  = 0;
    }

}
*/


void unix2stamp (time_t utime, _stamp *datime)
{
    if (utime == 0) {
        ((_dos_st *)datime)->date = 0;
        ((_dos_st *)datime)->time = 0;
        return;
    }

    struct tm *tm;

    tm = localtime (&utime);
    datime->date.da = (word) tm->tm_mday;
    datime->date.mo = (word) (tm->tm_mon + 1);
    datime->date.yr = (word) (tm->tm_year-80);
    datime->time.ss = (word) (tm->tm_sec/2);
    datime->time.mm = (word) tm->tm_min;
    datime->time.hh = (word) tm->tm_hour;
}



byte DateFormat = DF_USA;

static char *datefs[4] = { "%m-%d-%y", "%d-%m-%y", "%y-%m-%d", "%y%m%d" };


char *unix2dates (time_t utime, char *dates, byte DFovr)
{
    if (DFovr == DF_DEFAULT)
        DFovr = DateFormat;

    static char datestat[9];
    char *d = dates ? dates : datestat;
    if (utime)
        strftime (d, 9, datefs[DFovr], localtime (&utime));
    else
        strcpy (d, "(nodate)");
    return d;
}


char *stamp2dates (_stamp *stamp, char *dates, byte DFovr)
{
    return unix2dates (stamptounix (stamp), dates, DFovr);
}


time_t dates2unix (char *dates, byte DFovr)
{
    if (DFovr == DF_DEFAULT)
        DFovr = DateFormat;

    word tok[3];

    if (DFovr == DF_SCIENT) {
        if (strlen (dates) != 6)
            return 0;
        if (sscanf (dates, "%2hu%2hu%2hu", &tok[0], &tok[1], &tok[2]) != 3)
            return 0;
    } else {
        if (strlen (dates) != 8)
            return 0;
        char sep[3];
        if (sscanf (dates, "%2hu%c%2hu%c%2hu", &tok[0], &sep[0], &tok[1], &sep[1], &tok[2]) != 5)
            return 0;
        sep[2] = '\0';
        if (strspn (sep, ".-/") != 2)   // invalid separation char
            return 0;
    }

    word da, mo, yr;

    switch (DFovr) {
        case DF_USA:
            mo = tok[0];
            da = tok[1];
            yr = tok[2];
            break;
        case DF_EURO:
            da = tok[0];
            mo = tok[1];
            yr = tok[2];
            break;
        case DF_JAPAN:
        case DF_SCIENT:
            yr = tok[0];
            mo = tok[1];
            da = tok[2];
            break;
    }

    if ((mo < 1) || (mo > 12))
        return 0;

    if (da < 1)
        return 0;

    switch (mo) {
        case 2:
            if (da > ((yr % 4) ? 28 : 29))  // valid til 2099
                return 0;
            break;
        case 4:
        case 6:
        case 9:
        case 11:
            if (da > 30)
                return 0;
            break;
        default:
            if (da > 31)
                return 0;
    }

    _stamp st;

    ((_dos_st *)(&st))->time = 0;

    st.date.da = da;
    st.date.mo = mo;
    st.date.yr = (word) ((yr >= 80) ? (yr - 80) : (yr + 20));    // valid till 2079

    return stamptounix (&st);
}
