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

#include <string.h>

#include "apgenlib.hpp"
#include "apgenlb2.hpp"

#define InString    1   // for CleanLine status
#define OutString   2
#define LineEnd     3

const char *tkp = NULL;

static void CleanLine (char *line)    // Get rid of tabs, comments, newline.
{                              // Preserve ';' if enclosed in string.
    int status = OutString;    // Remove trailing space.
    char *c = line;

    while (status != LineEnd) {
        switch (*c) {
            case '\t':      // convert tab to blank
                *c = ' ';
                break;

            case '\n':
            case '\0':
                status = LineEnd;
                continue;

            case '\"':
                if (status == InString) {
                    if (*(c+1) == '\"')
                        c++;
                    else
                        status = OutString;
                } else
                    status = InString;
                break;

            case ';' :
                if (status == OutString) {
                    status = LineEnd;
                    continue;
                }
        }
        c++;
    }

    while (c > line)            // remove trailing space
        if (*(c-1) == ' ')
            c--;
        else
            break;

    *c = '\0';
}


CfgFile::CfgFile (int lsize)
{
    f = NULL;
    lbuff = new char[lsize];
    token = new char[lsize];
    lbuff[0] = '\0';
    nextc = lbuff;
    CfgFile::lsize = lsize;
}


CfgFile::~CfgFile ()
{
    delete[] token;
    delete[] lbuff;
    if (f)
        Close ();
}


int CfgFile::Open (pcsz filename)
{
    if (f)
        return 1;
    f = fopen (filename, "rt");
    if (!f)
        return -1;
    l = 0;
    return 0;
}

int CfgFile::Close ()
{
    if (!f)
        return -1;

    int ret = fclose (f);
    f = NULL;
    return ret;
}

pcsz CfgFile::GetLn ()
{
    do {
        char *res = fgets (lbuff, lsize, f);
        if (!res)
          return NULL;
        l++;
        CleanLine (lbuff);  /* get rid of tabs, comments and newline */
        nextc = lbuff;
        tkp = TokenPtr ();
    } while (!tkp);

    return tkp;
}

pcsz CfgFile::ReGetLn ()                // As previous GetLn; no effect on GetToken
{
  return tkp;
}

pcsz CfgFile::GetToken ()
{
    if (*nextc == '\0')                 // no more tokens !
        return NULL;

    if (GetName (nextc, token, lsize) < 0)
        return NULL;                       // error

    return token;                       // can be empty string !
}

pcsz CfgFile::ReGetToken ()             // As previous GetToken
{
    return token;
}

pcsz CfgFile::TokenPtr ()
{
    while (*nextc == ' ')       // skip space
        nextc++;

    if (*nextc == '\0')
        return NULL;

    return nextc;
}


uint CfgFile::LineN ()
{
    return l;
}

pcsz CfgFile::RestOfLine ()	// start of next word, NullStr if EOL
{
   if (*nextc == 0)
     return NULL;
   return nextc;
}
