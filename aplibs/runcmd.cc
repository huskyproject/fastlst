/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1994-1996  Alberto Pasquale                 */
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
#include <stdarg.h>
#include <stdlib.h>
#if !defined (__FreeBSD__) 
  #include <malloc.h>
#endif

static char *StrQTok (char *&c)
{
    char *start;

    c += strspn (c, " \t\n");    // skip blank
    if (*c == '\0')
        return NULL;            // return NULL if no token available

    start = c;

    BOOL InQuotes = FALSE;
    BOOL TokEnd = FALSE;

    while ((*c) && !TokEnd) {
        switch (*c) {
            case ' ':
            case '\t':
            case '\n':
                if (InQuotes)
                    c ++;
                else
                    TokEnd = TRUE;
                break;
            case '"':
                InQuotes = !InQuotes;
                c ++;
                break;
            default:
                c ++;
                break;
        }
    }

    if (*c) {
        *c = '\0';      // terminate token
        c++;
        c += strspn (c, " \t\n");  // advance c to next token or terminating NULL
    }

    return start;
}


#define _Quotes_Delimited_ TRUE


// Builds command line, one space between tokens, double trailing zero.
// Triple trailing zero in case of empty args string.
// Just to be sure, a triple zero is always appended.

static void BuildCmd (char *cmdline, uint cmdlinesize, int nargs,
                      char const *args[], BOOL delimited = FALSE)
{
    char *p = cmdline;
    *p = '\0';              // init empty command line string

    if (nargs == 0)         // nothing to do
        return;

    if (args[0][0] == '\0')     // empty command name
        return;

    cmdlinesize -= 3;           // leave space for trailing zeros

    if (strlen (args[0]) > cmdlinesize)    // too long command name
        return;

    p = stpcpy (p, args[0]);        // command name

    for (int i = 1; i < nargs; i++) {
        if (((p - cmdline) + 1 + strlen (args[i]) + (delimited ? 2 : 0)) > cmdlinesize)
            break;

        *p++ = ' ';

        if (delimited)
            *p++ = '"';
        p = stpcpy (p, args[i]);
        if (delimited)
            *p++ = '"';
    }

    *p++ = '\0';        // assure double trailing zero in any case
    *p++ = '\0';        // double trailing zero
    *p++ = '\0';
}

int RunCmd (const char *cmd, RCShow rcs, const char *prmlst, ...)
{
    #define MAXARGS 21
    #define MAXSUBS 10

    char const *subs[MAXSUBS];
    char const *args[MAXARGS];
    int j;

    if (!cmd)
        return -1;

    int nsubs = __min (strlen (prmlst), MAXSUBS);
                            
    va_list vargs;
    va_start (vargs, prmlst);
    for (j = 0; j < nsubs; j++)    // init prm substitutions
        subs[j] = va_arg (vargs, char *);
    va_end (vargs);

    char *workline = newcpy (cmd);
    char *c = workline;

    int i = 0;
    args[i++] = StrQTok (c);  // Prg name

    if (args[0] == NULL) {
        if (rcs)
            rcs (RC_EMPTY);
        delete[] workline;
        return -1;
    }
         
    while (i < MAXARGS-1) {
        char *arg = StrQTok (c);
        if (!arg)
            break;
        if ((strlen (arg) == 2) && (arg[0] == '%')) {    // can be a substitution parameter
            BOOL found = FALSE;
            for (j = 0; j < nsubs; j ++)
                if (arg[1] == prmlst[j]) {
                    args[i++] = subs[j];
                    found = TRUE;
                    break;
                }
            if (found)
                continue;
        }
        args[i++] = arg;
    }

    int nargs = i;
    args[i] = NULL;  // necessary for spawn function

    char *rescmdline = new char[4 * PATH_MAX];
    BuildCmd (rescmdline, 4 * PATH_MAX, nargs, args);
    if (rcs)
        rcs (RC_DOING, rescmdline);

    int code = system (rescmdline);

    delete[] rescmdline;
    delete[] workline;

    if (rcs)
        rcs (RC_EXIT, code);

    return code;

    #undef MAXSUBS
    #undef MAXARGS
}


