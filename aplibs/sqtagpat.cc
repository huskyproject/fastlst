/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1995-1996  Alberto Pasquale                 */
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

#include "bbsgenlb.hpp"
#include "apgenlib.hpp"
#include <string.h>

#define TAGSIZE     256
#define SQLNSIZE    1024

// returns 0 on "EchoArea or NetArea"
// returns -1 on Address not found before "Echo/NetArea"
// returns 1 on "not Echo/NetArea"
// returns 2 on Include: path contains the include filename.


int SqTag2Path::parsesq (char *linebuf, char *echotag, char *path, word *type,
                         ADR *adr)
{
    const char *c;
    uint toklen;

    c = linebuf;
    c += strspn (c, " \t");     /* skip blank */
    toklen = strcspn (c, " \t"); /* token length */

    *type = 0;

    switch (toklen) {
        case 8:
            if (strncasecmp (c, "EchoArea", toklen) == 0) {   /* is echo */
                *type |= MSGTYPE_ECHO;
                break;
            }
            return 1;
        case 7:
            if (strncasecmp (c, "NetArea", toklen) == 0) {  /* is netmail */
                break;
            }
            if (strncasecmp (c, "Include", toklen) == 0) { // Include
                c += toklen;
                GetName (c, path, PATH_MAX);
                return 2;
            }
            if (!primary && (strncasecmp (c, "Address", toklen) == 0)) {
                c += toklen;
                primary = new ADR;
                strto4Dadr (c, primary);
            }
            return 1;
        default:
            return 1;
    }

    if (!primary)       // Address must have already been found
        return -1;

    c += toklen;    /* skip token */

    c += strspn (c, " \t"); /* skip blank */
    toklen = strcspn (c, " \t"); /* Tag length */
    if (toklen >= TAGSIZE)
        return 1;       // Tag too long
    strncpy (echotag, c, toklen);
    echotag[toklen] = '\0';    /* echotag copied */
    c += toklen;         /* skip echotag */


    c += strspn (c, " \t"); /* skip blank */
    toklen = strcspn (c, " \t"); /* path length */
    if (toklen >= PATH_MAX)    // Path too long
        return 1;
    strncpy (path, c, toklen);
    path[toklen] = '\0';    /* path copied */
    c += toklen;         /* skip path */

    *adr  = *primary;

    c += strspn (c, " \t"); /* skip blank */
    while (*c == '-') {
        c++;
        switch (*c) {
            case '$': // Squish Format
                *type |= MSGTYPE_SQUISH;
                break;
            case 'p':
            case 'P':
                c ++;
                strto4Dadr (c, adr);
                break;
        }
        c += strcspn (c, " \t"); /* skip flag */
        c += strspn (c, " \t");  /* skip blank */
    }

    if (!(*type & MSGTYPE_SQUISH))
        *type |= MSGTYPE_SDM;

    return 0;
}


SqTag2Path::SqTag2Path ()
{
    sqhead = NULL;
    sqtail = &sqhead;
    primary = NULL;
}


SqTag2Path::~SqTag2Path ()
{
    if (primary)
        delete primary;

    _SqTagSearch *sq = sqhead,
                 *next;
    while (sq) {
        next = sq->next;
        delete sq;
        sq = next;
    }
}


void SqTag2Path::AddTag (const char *Tag, char **Path, word *Type, ADR *adr,
                         char **origin, dword *attr)
{
    _SqTagSearch *sqt = *sqtail = new _SqTagSearch;
    sqt->Tag    = Tag;
    sqt->Path   = Path;
    sqt->Type   = Type;
    sqt->adr    = adr;
    sqt->origin = origin;
    sqt->attr   = attr;
    sqt->next   = NULL;
    sqtail = &sqt->next;
}


int SqTag2Path::ParseSquishCfg (char *SquishCfg, CharPVoid sqtnf)
{
    FILE *sqf = fopen (SquishCfg, "rt");
    if (!sqf)
        return -1;
    setvbuf (sqf, NULL, _IOFBF, 8192);
    char *linebuf = new char[SQLNSIZE],
         *echotag = new char[TAGSIZE],
         *path    = new char[PATH_MAX];
    word type;
    ADR  adr;
    BOOL Done = FALSE;
    int error = 0;
    while (_fgets (linebuf, SQLNSIZE, sqf)) {
        int pret = parsesq (linebuf, echotag, path, &type, &adr);
        if (pret < 0) {
            error = -2;
            break;
        }

        if (pret == 2) {     // Include
            error = ParseSquishCfg (path);
            if (error)
                break;
            continue;
        }

        if (pret == 0) {
            _SqTagSearch *sq = sqhead;
            Done = TRUE;
            while (sq) {
                if (*sq->Path == NULL) {
                    if (stricmp (sq->Tag, echotag) == 0) {
                        *sq->Path = newcpy (path);
                        *sq->Type = type;
                        if (sq->adr)
                            *sq->adr = adr;
                        if (!(type & MSGTYPE_ECHO)) {
                            if (sq->origin)
                                *sq->origin = NULL;
                            if (sq->attr)
                                *sq->attr = MSGPRIVATE;
                        }
                    } else
                        Done = FALSE;
                }
                sq = sq->next;
            }
            if (Done)
                break;
        }
    }
    delete[] path;
    delete[] echotag;
    delete[] linebuf;
    fclose (sqf);

    if (error)
        return error;

    if (sqtnf)
    if (!Done) {        // report tags not found
        _SqTagSearch *sq = sqhead;
        while (sq) {
            if (*sq->Path == NULL)
                sqtnf (sq->Tag);
            sq = sq->next;
        }
    }

    return 0;
}





