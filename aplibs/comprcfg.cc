/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1992-1996  Alberto Pasquale                 */
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
/* How to contact the author:  Alberto Pasquale of 2:332/504@fidonet         */
/*                             Viale Verdi 106                               */
/*                             41100 Modena                                  */
/*                             Italy                                         */
/*                                                                           */
/*****************************************************************************/

// ComprCfg.Cpp

#ifdef __OS2__
  #define INCL_DOS
  #include <os2.h>
#endif
#include "bbsgenlb.hpp"
#include "apgenlib.hpp"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>



#define IDENTBUFSIZE   64
#define CFGLINESIZE    256


#define InString    1   // for CleanLine status
#define OutString   2
#define LineEnd     3


AH_VShow AH_ComprCfg::outf;


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


static char *fgetln (FILE *f, char *lbuff, const char *&tkp)
{
    char *res, *tok;

    while ((res = _fgets (lbuff, CFGLINESIZE, f)) != NULL) {
        CleanLine (lbuff);  /* get rid of tabs, comments and newline */
        tkp = lbuff;
        tok = GetStatName (tkp);
        if (tok != NULL) /* if keyword ok, return */
            break;
    }
    if (res)
        return tok;
    else
        return NULL;
}


void AH_ComprCfg::RcShow (int code, ...)
{
    if (!outf)
        return;
    va_list args;
    va_start (args, code);
    switch (code) {
        case RC_DOING:
            outf (AH_MT_Action, "Executing \"%s\"\n", va_arg (args, char *));
            break;
        case RC_EXIT:
            outf (AH_MT_Info, "Exit code: %d\n", va_arg (args, int));
            break;
        case RC_EMPTY:
            outf (AH_MT_Error, "Error: Empty external command !\n");
            break;
    }
    va_end (args);
}


AH_Archiver::AH_Archiver (const AH_Archiver *aptr)   // constructor
{
    memset (this, 0, sizeof (*this));   // clear all fields
    prev = aptr;
}


AH_ComprCfg::AH_ComprCfg (const char *cfgfile, AH_VShow outf)      // constructor
{
    FILE *f;
    const char *tok, *tkp;

    AH_ComprCfg::outf = outf;

    lastarc = NULL;

    f = fopen (cfgfile, "rt");
    if (!f) {
        if (outf)
            outf (AH_MT_Error, "Error: cannot open \"%s\"\n", cfgfile);
        return;
    }

    if (outf)
        outf (AH_MT_Info, "Including Compress Definition \"%s\"\n", cfgfile);

    BOOL InArchive = FALSE;

    char lbuff[CFGLINESIZE];

    while ((tok = fgetln (f, lbuff, tkp)) != NULL) {

      #ifdef __OS2__
        if (stricmp (tok, "DOS") == 0)  // ignore DOS lines
            continue;

        if (stricmp (tok, "OS2") == 0) {  // skip OS2 prefix
            if ((tok = GetStatName (tkp)) == NULL)
                continue;
        }
      #else // DOS
        if (stricmp (tok, "OS2") == 0)  // ignore OS2 lines
            continue;

        if (stricmp (tok, "DOS") == 0) {  // skip DOS prefix
            if ((tok = GetStatName (tkp)) == NULL)
                continue;
        }
      #endif

        if (!InArchive) {
            if (stricmp (tok, "Archiver") == 0) {
                if ((tok = GetStatName (tkp)) == NULL)
                    continue;
                InArchive = TRUE;
                lastarc = new AH_Archiver (lastarc);
                lastarc->name = newcpy (tok);
                continue;
            }
        } else {        // Inside Archiver definition
            if (stricmp (tok, "End") == 0) {
                InArchive = FALSE;
                continue;
            }

            if (stricmp (tok, "View") == 0) // ignore View command
                continue;

            if (stricmp (tok, "Extension") == 0) {
                if ((tok = GetStatName (tkp)) == NULL)
                    continue;
                lastarc->ext = newcpy (tok);
                continue;
            }

            if (stricmp (tok, "Ident") == 0) {
                if ((tok = GetStatName (tkp)) == NULL)
                    continue;
                ScanIdent (tok);
                continue;
            }

            if (stricmp (tok, "Add") == 0) {
                if (*tkp == '\0')
                    continue;
                lastarc->addcmd = GetAllocLn (tkp, GL_Empty);
                continue;
            }

            if (stricmp (tok, "Extract") == 0) {
                if (*tkp == '\0')
                    continue;
                lastarc->extcmd = GetAllocLn (tkp, GL_Empty);
                continue;
            }
        }

        if (outf)
            outf (AH_MT_Error, "\nUnknown or out of sequence ComprCfg line:\n%s\n\n", lbuff);
    }

    fclose (f);
 
    if (outf)
        outf (AH_MT_Info, "Finished Compress Definition \"%s\"\n", cfgfile);
}


void AH_ComprCfg::ScanIdent (const char *tok)
{
    char hexfig[3] = "00";  // initialized to set hexfig[2] = '\0';
    byte identbuf[IDENTBUFSIZE];
    int identlen = 0;

    char *p = strchr (tok, ',');
    if (!p)
        return;

    p++;    // points to first hex figure

    while (*p) {
        *hexfig = *(p++);
        if (*p == '\0')
            return;
        *(hexfig+1) = *(p++);
        identbuf[identlen++] = (byte) strtol (hexfig, NULL, 16);
    }

    lastarc->identofs = atoi (tok);

    lastarc->identlen = identlen;

    lastarc->identstr = new byte[identlen];
    memcpy (lastarc->identstr, identbuf, identlen);
}


typedef enum { St_Start,        // Look for 1st byte of ID
               St_StrChk        // Looking for next byte
             } State;

struct _Status {
    State state;
    const AH_Archiver *CurArc;    // current arc type in evaluation
    ulong idofs;            // current ofs in ID
};


BOOL AH_ComprCfg::ArcGood (const AH_Archiver *a)
{
    return ((a->identlen > 1) && (a->extcmd != NULL));
}


const AH_Archiver *AH_ComprCfg::SfxNext (const AH_Archiver *a)
{
    if (!a) {
        a = lastarc;
        if (!a)
            return NULL;
        if (ArcGood (a))
            return a;
    }

    do {
        a = a->prev;
        if (!a)
            return NULL;
    } while (!ArcGood (a));

    return a;
}


const AH_Archiver *AH_ComprCfg::ChkSfx (const char *filename)
{
    int namelen = strlen (filename);
    if (namelen < 5)
        return NULL;
    if (stricmp (filename + namelen - 4, ".exe") != 0)
        return NULL;


    FILE *f = fopen (filename, "rb");
    if (!f)
        return NULL;
    setvbuf (f, NULL, _IOFBF, 8192);

    int ch;
    _Status Status = { St_Start, SfxNext (NULL), 0 };
    if (!Status.CurArc)
        return NULL;
    BOOL afound = FALSE;

    while ((ch = fgetc (f)) != EOF) {
        switch (Status.state) {

            case St_Start:      // looking for 1st byte of ID
                if (!Status.CurArc)
                    Status.CurArc = SfxNext (NULL);
                while (Status.CurArc) {
                    if (ch == Status.CurArc->identstr[0]) {
                        Status.state = St_StrChk;
                        Status.idofs = 1;
                        break;
                    }
                    Status.CurArc = SfxNext (Status.CurArc);
                }
                break;

            case St_StrChk:
                if (ch == Status.CurArc->identstr[Status.idofs++]) {
                    if (Status.CurArc->identlen == Status.idofs)
                        afound = TRUE;
                } else {
                    long nback = Status.idofs;
                    Status.state = St_Start;
                    Status.CurArc = SfxNext (Status.CurArc);
                    if (!Status.CurArc)
                        nback --;
                    fseek (f, -nback, SEEK_CUR);
                }
                break;
        }

        if (afound)
            break;
    }

    fclose (f);

    if (!afound)
        return NULL;

    return Status.CurArc;
}


int AH_ComprCfg::UnArc (const char *filename, const char *extract)
{                                      // Unarcs extract from filename
    FILE *f;                           // returns errorlevel
    const AH_Archiver *a;              // returns -1 if cannot execute
    int res;                           // returns -2 on unknown type
    BOOL afound; // Archive type found // returns -3 on file not found


    f = fopen (filename, "rb");
    if (!f) {
        if (outf)
            outf (AH_MT_Error, "Error: cannot open \"%s\"\n", filename);
        return -3;      // cannot open
    }

    afound = FALSE;
    a = lastarc;
    while (a) {
        if ((a->identlen > 0) && (a->extcmd != NULL)) {
            if (a->identofs >= 0)
                res = fseek (f, a->identofs, SEEK_SET);
            else
                res = fseek (f, a->identofs, SEEK_END);
            if (res == 0) {     // seek successful
                afound = TRUE;
                for (int i = 0; i < a->identlen; i ++) {
                    res = fgetc (f);
                    if (res != a->identstr[i]) { // incl. EOF
                        afound = FALSE;
                        break;
                    }
                }
                if (afound)
                    break;
            }
        }
        a = a->prev;
    }

    fclose (f);

    if (!afound) {
        a = ChkSfx (filename);
        afound = (a != NULL);
    }

    if (!afound) {
        if (outf)
            outf (AH_MT_Error, "Error: unknown archive for \"%s\"\n", filename);
        return -2;      // Archive type not identified
    }

    return RunCmd (a->extcmd, RcShow, "af", filename, extract);
}


int AH_ComprCfg::Arc (const AH_Archiver *a, const char *filename, const char *add)
{                                       // Adds add to filename using archiver
    return RunCmd (a->addcmd, RcShow, "af", filename, add);
}


const AH_Archiver *AH_ComprCfg::AddDefined (const char *method) // is method defined in cfg ?
{                                             // return pointer
    const AH_Archiver *a = lastarc;
    while (a) {
        if (stricmp (a->name, method) == 0) {
            if ((a->addcmd != NULL) && (a->ext != NULL))
                return a;
            else
                return NULL;
        }
        a = a->prev;
    }
    return NULL;
}


const AH_Archiver *AH_ComprCfg::ExtDefined (const char *ext)  // is extension defined in compress.cfg ?
{
    const AH_Archiver *a = lastarc;
    while (a) {
        if (stricmp (a->ext, ext) == 0)
            return a;
        a = a->prev;
    }
    return NULL;
}
