/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1995       Alberto Pasquale                 */
/*                 Portions (C) Copyright 1999 Per Lundberg                  */
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

// MaxAcs.Cpp

#include "bbsgenlb.hpp"
#include "uclass.h"
#include <apgenlib.hpp>
#include <string.h>
#include <ctype.h>

#define _LEVELSIZE 80       // max length of access level name
#define _TOKACSSIZE 160     // max length of "<=Normal/a!b"
#define _PRODACSSIZE 512    // max length of TOKACS separated by '&'

// returns 0 on success.  keys0 are the keys preceded by '!'

static int GetKeys (char *ks, dword *keys1, dword *keys0 = NULL)
{
	char *c;
	int u, nbit;
    long bit;

    BOOL flag0 = FALSE;
    *keys1 = 0L;
    if (keys0)
        *keys0 = 0L;

    for (c = ks; (u = toupper(*c)) != 0; c++)
	{
        if ((keys0) && (u == '!')) {
            flag0 = TRUE;
            continue;
        }

        if ((u < '1') || (u > 'X') || (u > '8') && (u < 'A'))
            return -1;

        if (isdigit (u))
            nbit = u - '1';
        else
            nbit = u - 'A' + 8;

		bit = 1L << nbit;

        if (flag0) {
            *keys0 |= bit;
            flag0 = FALSE;
        } else
            *keys1 |= bit;
	}
    return 0;
}


MAXACS::MAXACS ()
{
    buffer = NULL;
}


MAXACS::~MAXACS ()
{
    if (buffer)
        delete[] buffer;
}


int MAXACS::Read (char *acsname)
{
    char fullname[PATH_MAX];
    strcpy (fullname, acsname);
    addext (fullname, ".DAT");

    FILE *f = fopen (fullname, "rb");
    if (!f)
        return -1;
    long fsize = filelength (fileno (f)); 
    if ((fsize == -1) || (fsize < (long)sizeof (CLSHDR))) {
        fclose (f);
        return -1;
    }

    buffer = new byte[fsize];

    if (fread (buffer, (size_t)fsize, 1, f) != 1) {
        fclose (f);
        return -1;
    }

    fclose (f);

    CLSHDR *chp = (CLSHDR *) buffer;
    if (chp->ulclhid != CLS_ID)
        return -1;

    usn = chp->usn;
    ussize = chp->ussize;

    int heapofs = chp->usclfirst + usn * ussize;

    if (fsize < (heapofs + chp->usstr))
        return -1;

    heap = (char *) (buffer + heapofs);
    uscl1 = buffer + chp->usclfirst;

    return 0;
}


int MAXACS::GetLevel (char *slevel, word *level)
{
    byte *b;
    int i;
    for (i = 0, b = uscl1; i < usn; i ++, b += ussize) {
        CLSREC *crp = (CLSREC *) b;
        if ((stricmp (slevel, heap + crp->zAbbrev) == 0) ||
            (stricmp (slevel, heap + crp->zAlias)  == 0)) {
            *level = crp->usLevel;
            return 0;
        }
    }
    return -1;
}


int MAXACS::GetGenAcs (char *lks, word *level, dword *keys1, dword *keys0)
{
    char slevel[_LEVELSIZE];

    char *p = strchr (lks, '/');
    if (p) {                        // there are keys
        strzcpy (slevel, lks, __min (p - lks + 1, _LEVELSIZE));
        if (keys1)
            if (GetKeys (p+1, keys1, keys0))
                return -1;
    } else {
        strzcpy (slevel, lks, _LEVELSIZE);
        if (keys1) {
            *keys1 = 0L;
            if (keys0)
                *keys0 = 0L;
        }
    }

    if (isdigit (slevel[0])) {      // numeric access level
        char *endptr;
        ulong res = strtoul (slevel, &endptr, 10);
        if ((res > 65535) || (*endptr != '\0'))
            return -1;
        *level = (word) res;
    } else {                // string access level: look up access.dat
        if (GetLevel (slevel, level))
            return -1;
    }

    return 0;
}


int MAXACS::GetAcs (char *ACS, word *level, dword *keys)
{
    return GetGenAcs (ACS, level, keys);
}


BOOL MAXACS::TokAcs (char *TokAcs, word level, dword keys)
{
    char *p = TokAcs;

    if (stricmp (p, "NoAccess") == 0)
        return FALSE;
    if (strncasecmp (p, "name=", 5) == 0)
        return FALSE;
    if (strncasecmp (p, "alias=", 6) == 0)
        return FALSE;

    _privcmp op;

    switch (strspn (p, "=><!")) {       // evaluate comparison operator
        case 0:
            op = privGE;
            break;
        case 1:
            if (*p == '=')
                op = privEQ;
            else if (*p == '>')
                op = privGT;
            else if (*p == '<')
                op = privLT;
            else
                return FALSE;
            p ++;
            break;
        case 2:
            if ((*p == '>') && (*(p+1) == '='))
                op = privGE;
            else if ((*p == '<') && (*(p+1) == '='))
                op = privLE;
            else if ((*p == '<') && (*(p+1) == '>'))
                op = privNE;
            else if ((*p == '!') && (*(p+1) == '='))
                op = privNE;
            else
                return FALSE;
            p += 2;
            break;
        default:
            return FALSE;
    }

        // now evaluate the level/keys

    word reqlevel;
    dword reqkeys1, reqkeys0;

    if (GetGenAcs (p, &reqlevel, &reqkeys1, &reqkeys0))
        return FALSE;

    switch (op) {           // compare level
        case privGE:
            if (!(level >= reqlevel))
                return FALSE;
            break;
        case privLE:
            if (!(level <= reqlevel))
                return FALSE;
            break;
        case privGT:
            if (!(level > reqlevel))
                return FALSE;
            break;
        case privLT:
            if (!(level < reqlevel))
                return FALSE;
            break;
        case privEQ:
            if (!(level == reqlevel))
                return FALSE;
            break;
        case privNE:
            if (!(level != reqlevel))
                return FALSE;
            break;
        default:
            return FALSE;
    }

                    // and now let's check keys

    if ((reqkeys1 & keys) != reqkeys1)  // missing some required key
        return FALSE;

    if ((reqkeys0 & keys) != 0)     // has some key that must be absent
        return FALSE;

    return TRUE;
}


BOOL MAXACS::ProdAccess (char *pac, word level, dword keys)
{
    char tokacs[_TOKACSSIZE];

    char *p = pac;
    while (*p) {
        int toklen = strcspn (p, "&");
        if (toklen == 0)
            return FALSE;
        strzcpy (tokacs, p, __min (toklen + 1, _TOKACSSIZE));
        if (!TokAcs (tokacs, level, keys))
            return FALSE;
        p += toklen;
        if (*p == '&')
            p ++;
    }
    return TRUE;
}


BOOL MAXACS::HaveAccess (char *ACS, word level, dword keys)
{
    char prodacs[_PRODACSSIZE];

    char *p = ACS;
    while (*p) {
        int toklen = strcspn (p, "|");
        if (toklen == 0)
            return FALSE;
        strzcpy (prodacs, p, __min (toklen + 1, _PRODACSSIZE));
        if (ProdAccess (prodacs, level, keys))
            return TRUE;
        p += toklen;
        if (*p == '|')
            p ++;
    }
    return FALSE;
}


char *MAXACS::LevName (word level)
{
    byte *b;
    int i;
    for (i = 0, b = uscl1; i < usn; i ++, b += ussize) {
        CLSREC *crp = (CLSREC *) b;
        if (level == crp->usLevel)
            return heap + crp->zAbbrev;
    }
    return NULL;
}


char *MAXACS::LevStr (word level)
{
    char *p = LevName (level);
    if (p)
        return p;

    static char buf[6];
    sprintf (buf, "%hu", level);
    return buf;
}


