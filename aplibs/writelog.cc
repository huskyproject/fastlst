/*****************************************************************************/
/*                                                                           */
/*                 (C)  Copyright 1997      Alberto Pasquale                 */
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

#include "apgenlib.hpp"
#include "apgenlb2.hpp"

LogFile::LogFile ()
{
    f = NULL;
    fname[0] = '\0';
}


void LogFile::setup (pcsz fname, pcsz ID, pcsz logset, word flags)
{
    close ();

    if (fname)
        strzcpy (LogFile::fname, fname, sizeof (LogFile::fname));

    if (ID)
        strzcpy (LogFile::ID, ID, sizeof (LogFile::ID));

    if (logset)
        strzcpy (LogFile::logset, logset, sizeof (LogFile::logset));

    if (flags != LF_NoChange)
        LogFile::flags = flags;
}


LogFile::~LogFile ()
{
    if (f)
        fclose (f);
}


int LogFile::close ()
{
    int ret = 0;

    if (f) {
        ret = fclose (f);
        f = NULL;
    }

    return ret;
}


int LogFile::flush ()
{
    int ret = 0;

    if (f)
        ret = fflush (f);

    return ret;
}


int LogFile::open ()            // private
{
    if (f)
        return 0;

    f = fopen (fname, "at");
    if (!f)
        return -1;

    return 0;
}


int LogFile::va_write (psz date, pcsz strfmt, va_list args)
{
    if (fname[0] == '\0')
        return 0;

    if (*strfmt == '\0')
        return 0;

    time_t timer;

    if (!f)
        if (open ())
            return -1;

    time (&timer);
    strftime (date, LF_DateSize, "%d %b %H:%M:%S", localtime (&timer));

    int err = 0;

    err |= (fprintf (f, "%c %s %4s ", strfmt[0], date, ID) < 0);
    err |= (vfprintf (f, strfmt+1, args) < 0);
    err |= (fputc ('\n', f) == EOF);

    if (flags & LF_CloseAfterWrite)
        err |= close ();
    else if (!(flags & LF_FlushOnRequest))
        err |= flush ();

    return err;
}


int LogFile::vwrite (pcsz strfmt, ...)
{
    va_list args;
    char date[LF_DateSize];

    va_start (args, strfmt);
    int ret = va_write (date, strfmt, args);
    va_end (args);

    return ret;
}


int LogFile::PrintStart (FILE *af, char prty, pcsz date)  // private
{
    int ret = 0;

    if (flags & LF_PrintPrty)
        ret |= (fprintf (af, "%c ", prty) < 0);

    if (flags & LF_PrintDate)
        ret |= (fprintf (af, "%.6s ", date) < 0);

    if (flags & LF_PrintTime)
        ret |= (fprintf (af, "%.8s ", date+7) < 0);

    if (flags & LF_PrintID)
        ret |= (fprintf (af, "%.4s ", ID) < 0);

    return ret;
}


int LogFile::vwrite (FILE *af, pcsz strfmt, ...)
{
    va_list args;
    char date[LF_DateSize];

    int ret = 0;

    va_start (args, strfmt);
    ret |= va_write (date, strfmt, args);
    va_end (args);

    if (af) {
        ret |= PrintStart (af, strfmt[0], date);

        va_start (args, strfmt);
        ret |= (vfprintf (af, strfmt+1, args) < 0);
        va_end (args);

        ret |= (fputc ('\n', af) == EOF);
        ret |= fflush (af);
    }

    return ret;
}
