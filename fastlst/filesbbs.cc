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

// FilesBbs.Cpp

#ifdef __OS2__
    #define INCL_DOS
    #include <os2.h>
#endif

#include <bbsgenlb.hpp>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <apgenlib.hpp>
#include "misc.hpp"
#include "data.hpp"
#include "filesbbs.hpp"



void KillFile (char *fullname)
{
    DeleteFile (fullname);

    char filename[NAME_MAX];
    char filesbbs[PATH_MAX];
    strcpy (filesbbs, fullname);
    MoveName (filesbbs, filename);
                                                 
    FBBS *fbbs = new FBBS (filesbbs, 0, 0, 1024, fb_cont, fb_cpos);
    if (fbbs->GetDesc (filename, NULL, NULL, FBBS_REMOVE) == -1)
        vprintlog ("Error deleting description for \"%s\"\n", fullname);
    delete fbbs;
}


#define FBBSLSIZE 1024


void SetDesc (char *fullname, char *desc, int day, char *arcname)
{
    char path[PATH_MAX];        // path only
    char file[NAME_MAX];     // name only

    strcpy (path, fullname);
    MoveName (path, file);

    char days[4];
    sprintf (days, "%03d", day);

    // build complete description

    char *fulldesc = new char[FBBSLSIZE];

    char *c = desc;
    char *d = fulldesc;

    while (*c) {
        if (*c == '%') {        // escape char
            c ++;
            if (*c == '\0')
                break;

            if (day != -1)
                switch (*c) {
                    case 'D':       // %D -> Date
                        c ++;
                        d = stpcpy (d, UnixToDate (dayntounix (day)));
                        continue;
                    case 'L':       // %L -> Locale Date
                        c ++;
                        d = stpcpy (d, UnixToLDate (dayntounix (day)));
                        continue;
                    case 'd':       // %d -> day
                        c ++;
                        d = stpcpy (d, days);
                        continue;
                }

            switch (*c) {
                case 'a':       // %a -> archiver name
                    c ++;
                    d = stpcpy (d, arcname);
                    continue;
            }
        }                       // default: copy as is (%% -> %)
        *(d++) = *(c++);
    }
    *d = '\0';

                                      
    FBBS *fbbs = new FBBS (path, 0, 0, FBBSLSIZE, fb_cont, fb_cpos);
    fbbs->GetDesc (file, NULL, NULL, FBBS_REMOVE);  // remove old description if present
    fbbs->SetDesc (file, fulldesc);         // append new description
    delete fbbs;

    delete[] fulldesc;
}

