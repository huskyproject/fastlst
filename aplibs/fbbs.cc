/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1995-1997  Alberto Pasquale                 */
/*                  Portions (C) Copyright 1999 Per Lundberg                 */
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
#include <apgenlib.hpp>
#include <string.h>
#include <fb.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

word Max2FbbsDateStyle (sword date_style)
{
    switch (date_style) {                       // as in Max.Prm date_style
        case 0:
            return FBBS_FLAG_DATE_USA;
        case 1:
            return FBBS_FLAG_DATE_EURO;
        case 2:
            return FBBS_FLAG_DATE_JAPAN;
        case 3:
            return FBBS_FLAG_DATE_SCIENT;
    }

    return FBBS_FLAG_NODATESIZE;
}


static BOOL isComment (const char *line)
{
    if ((line[0] <= ' ') || (line[0] > '~'))  // empty, ctrl, space, high ascii
        return TRUE;
    if ((line[0] == '-') && ((line[1] == ' ') || (line[1] == '\t'))) // comment
        return TRUE;
    return FALSE;
}


int FBBS::RdOpen ()
{
    repeatln = FALSE;

    if (f) {
        rewind (f);
        return 0;
    }

    if (WrClose ())
        return -1;

    f = fopen (filesbbs, "rt");
    if (!f)
        return -1;
    setvbuf (f, NULL, _IOFBF, 8192);

    return 0;
}


void FBBS::RdClose ()
{
    if (f) {
        fclose (f);
        f = NULL;
    }
}


int FBBS::WrOpen ()
{
    if (fileh != -1)
        return 0;

    RdClose ();
    fileh = open (filesbbs, O_WRONLY | O_APPEND);
    if (fileh == -1)
        return -1;

    return 0;
}


int FBBS::WrClose ()
{
    int err = 0;
    if (fileh != -1) {
        err |= (close (fileh) == -1);
        fileh = -1;
    }
    return err;
}


int FBBS::Trunc ()
{
    RdClose ();
    if (WrClose ())
        return -1;

    FILE *ft = fopen (filesbbs, "wt");
    if (!ft)
        return -1;
    if (fclose (ft))
        return -1;

    return 0;
}


char *FBBS::Fgetln ()
{
    if (repeatln) {
        repeatln = FALSE;
        return lastget;
    }

    lastget = _fgets (line, linesize, f);
    return lastget;
}



FBBS::FBBS (const char *path, int filesize, int descsize, int linesize,
            char cont, int cpos, word fbbsflags, SkipFile sf, void *ptr)
{
    strcpy (filesbbs, path);

    if (*filefrompath (path) == '\0')
        strcat (filesbbs, "files.bbs");

    this->filesize = filesize;
    this->descsize = descsize;
    this->linesize = linesize;
    this->cont = cont;
    this->cpos = cpos;
    this->fbbsflags = fbbsflags;
    this->sf = sf;
    this->ptr = ptr;
    t = NULL;

    line = new char[linesize];

    repeatln = FALSE;
    lastget = NULL;
    f = NULL;
    fileh = -1;
}


FBBS::~FBBS ()
{
    delete[] line;
    WrClose ();
    RdClose ();
    if (t)
        fclose (t);
}


static bool IsInTokLst (const char *line, size_t toklen, const char *toklst)
{
    WordStore ws ((char *)toklst);

    char *tok = ws.GetFirst ();
    while (tok) {
        if (strlen (tok) == toklen)
            if (strncasecmp (line, tok, toklen) == 0)
                return true;
        tok = ws.GetNext ();
    }

    return false;
}


int FBBS::GetDesc (const char *file, char *desc, word *flag, byte action,
                   const char *repl, time_t *date, dword *size)
{
    if (flag)
        *flag = 0;
    if (date)                   	// default inits
        *date = 0;
    if (size)
        *size = ULONG_MAX;

    FILE *tmp = NULL;
    char tmpname[PATH_MAX];
    
    if (!repl)          		// make sure the repl pointer is not NULL
        repl = "";
    if (access (filesbbs, 0)) {		// if files.bbs does not exist
    	strcpy (tmpname, filesbbs);
    	tmp = fopen(tmpname, "wt"); 	// create empty file with correct umask
	if (!tmp)
	  return -1;
	if (fclose(tmp) == EOF)
	  return -1;
        return -2;
    }  
    if (RdOpen ())
        return -1;
    
    if ((action & FBBS_REMOVE) || (*repl)) {    // tmp file must be used
        strcpy (tmpname, filesbbs);
        setext (tmpname, ".$$$");
        tmp = fopen (tmpname, "wt");
        if (!tmp)
            return -1;
        setvbuf (tmp, NULL, _IOFBF, 8192);
    }

    BOOL filedone = FALSE,
         repldone = FALSE;
    int filelen = strlen (file);

    int res = 0;
    while (Fgetln ()) {
        if (!isComment (line)) {
            int thislen = strcspn (line, " \t");
            if (thislen == filelen)
                if (strncasecmp (line, file, filelen) == 0) {
                    char *p = line+thislen;
                    p += strspn (p, " \t");
                    res |= descopy (p, desc, flag, date, size,
                                    (action & FBBS_REMOVE) ? NULL : tmp);
                    if (res)
                        break;
                    filedone = TRUE;
                    continue;
                }
            if (*repl)
            if (IsInTokLst (line, thislen, repl)) {
                res |= descopy ();
                if (res)
                    break;
                repldone = TRUE;
                continue;
            }
        }
        if (tmp)
            res |= (_fputs (line, tmp) == EOF);
        if (res)
            break;
    }

    if (tmp) {
        res |= (fclose (tmp) == EOF);
        tmp = NULL;

        if ((res == 0) &&
            (repldone || (filedone && (action & FBBS_REMOVE)))) {
            RdClose ();
            tunlink (filesbbs, 20);
            res |= rename (tmpname, filesbbs);
        } else
            unlink (tmpname);
    }

    if (res)
        return -1;

    res = 0;
    if (!filedone)
        res |= 1;
    if (*repl && !repldone)
        res |= 2;

    return res;
}


int FBBS::GetGenEntry (char *file, char *desc, word *flag, BOOL first,
                       time_t *date, dword *size)
{
    if (flag)
        *flag = 0;
    if (date)                   // default inits
        *date = 0;
    if (size)
        *size = ULONG_MAX;

    if (first || !f) {

        if (RdOpen ())
            return -1;

        if (t) {
            fclose (t);
            t = NULL;
        }
        if (sf) {       // open temporary file
            char filetmp[PATH_MAX];
            strcpy (filetmp, filesbbs);
            setext (filetmp, "$$0");
            t = fopen (filetmp, "wt");
            if (!t)
                return -1;
        }
    }

    BOOL EntryRemoved = FALSE;

    int res = 0;
    BOOL found = (Fgetln () != NULL);
    BOOL comment = FALSE;
    if (found) {
        comment = isComment (line);
        if (comment) {
            *file = '\0';
            strzcpy (desc, line, descsize);
            if (t)
                res |= (_fputs (line, t) == EOF);
        } else {
            const char *p = line;
            GetName (p, file, filesize);  // get file name
            if (sf)
                EntryRemoved = sf (file, ptr);
            res |= descopy (p, desc, flag, date, size, EntryRemoved ? NULL : t);
        }
    }

    if (res || !found) {
        if (t) {
            res |= fclose (t);
            t = NULL;
            char filetmp[PATH_MAX];
            strcpy (filetmp, filesbbs);
            setext (filetmp, "$$0");
            if (!res) {         // rename new bbs on old one
                RdClose ();
                tunlink (filesbbs, 20);
                res |= rename (filetmp, filesbbs);
            } else
                unlink (filetmp);
        }
        if (res)
            return -1;
        else
            return 1;
    }

    if (EntryRemoved)
        return 3;
    if (comment)
        return 2;
    return 0;
}


int FBBS::GetEntry (char *file, char *desc, word *flag, BOOL first,
                    time_t *date, dword *size)
{
    int ret;
    do {
        ret = GetGenEntry (file, desc, flag, first, date, size);
        first = FBBS_GE_NEXT;
    } while (ret == 2);       // skip comment lines
    return ret;
}


static word GetFileFlags (const char *&desc)
{
    char flags[10];
    word flag = FF_FILE;            // get flags if available
    while (*desc == '/') {
        GetName (desc, flags, sizeof (flags)); // get flag string
        char *f = flags;
        while (*f) {
            switch (tolower (*f)) {
                case 't':
                    flag |= FF_NOTIME;
                    break;
                case 'b':
                    flag |= FF_NOBYTES;
                    break;
            }
            f ++;
        }
    }
    return flag;
}


static byte Flags2Df (word fbbsflags)
{
    if (fbbsflags & FBBS_FLAG_NODATESIZE)
        return DF_DEFAULT;

    switch (fbbsflags & FBBS_FLAG_DATEFORMAT) {
        case FBBS_FLAG_DATE_USA:
            return DF_USA;
        case FBBS_FLAG_DATE_EURO:
            return DF_EURO;
        case FBBS_FLAG_DATE_JAPAN:
            return DF_JAPAN;
        case FBBS_FLAG_DATE_SCIENT:
            return DF_SCIENT;
    }

    return DF_DEFAULT;
}


static void GetDateSize (const char *&desc, time_t *datep, dword *sizep, word fbbsflags)
{
    if (fbbsflags & FBBS_FLAG_NODATESIZE) {
        if (datep)
            *datep = 0;
        if (sizep)
            *sizep = ULONG_MAX;
        return;
    }

    byte date_format = Flags2Df (fbbsflags);

    const int toksize = 10;
    char tok[2][toksize];
    const char *p = desc;

    GetName (p, tok[0], toksize);
    GetName (p, tok[1], toksize);

    time_t date;
    dword size;
    int isize = 1; // let's suppose tok[1] contains size

    date = dates2unix (tok[0], date_format);

    if (date == 0) {    // if first item is not date, let's try with second one
        isize = 0;
        date = dates2unix (tok[1], date_format);
    }

    if (date) {
        char *endptr;
        size = strtoul (tok[isize], &endptr, 10);
        if ((*endptr != '\0') || (size == ULONG_MAX)) { // token is not length only
            date = 0;
            size = 0;
        }
    } else
        size = 0;

    if (date)
        desc = p;

    if (datep)
        *datep = date;
    if (sizep)
        *sizep = size;
}



int FBBS::descopy (const char *p, char *desc, word *flag, time_t *date, dword *size,
                   FILE *t)
{
    if (!p && (desc || flag || date || size))
        return -1;

    int desclen = 0;

    if (p) {
        GetDateSize (p, date, size, fbbsflags);
        word flags = GetFileFlags (p);
        if (flag)   
            *flag = flags;
        if (desc)
            desclen = descAddLine (p, desc, 0);
    }

    int res = 0;
    if (t)
        res |= (_fputs (line, t) == EOF);

    if (cpos != -1) {
        while (Fgetln ()) {                  // get continuation lines (no '\n')
            int spacelen = strspn (line, " \t");
            char *firstchar = line+spacelen;
            if (cont != ' ') {
                if ((spacelen == 0) && (cpos != 0))
                    break;
                if (*firstchar != cont)       // not a continuation line
                    break;
                else {
                    firstchar ++;       // skip cont char
                    if ((*firstchar == ' ') && !(fbbsflags & FBBS_FLAG_NOCONTSPACE))
                        firstchar ++;     // skip space following cont
                }
            } else {
                if ((spacelen < cpos) && *firstchar) // not a continuation line
                    break;
                if (spacelen > cpos)
                    firstchar = line + cpos;
            }
                        // this is a continuation (description) line
            if (desc)
                desclen = descAddLine (firstchar, desc, desclen);
            if (t)
                res |= (_fputs (line, t) == EOF);
        }

        repeatln = TRUE;        // next Fgetln will repeat last result
    }

    if (res)
        return -1;
    return 0;
}


int FBBS::descAddLine (const char *p, char *desc, int desclen)
{
    int remsize = descsize - desclen;
    if (remsize < 3)       // space for '\n', at least one char, '\0'
        return desclen;

    if (desclen > 0) {
        desc[desclen++] = '\n';   // add separator
        remsize --;
    }

    char *tn = stpzcpy (desc+desclen, p, remsize);
    return int (tn - desc);
}


int FBBS::PutGenEntry (const char *file, const char *desc, word flag,
                       const char *befdesc, time_t date, dword size)
{
    if (WrOpen ())
        return -1;

    if (!(flag & FF_SAFE)) {
        char *p = (char *)desc;
        while (*p) {          // remove trojan control chars
            if ((*p < ' ') && (*p != '\n'))
                *p = ' ';
            p++;
        }
    }

    BOOL isComment = FALSE;     // is this a comment ?
    if (!file)
        isComment = TRUE;
    else if (*file == '\0')
        isComment = TRUE;

    int err = 0;

    if (!isComment) {
        const char *b = desc;                     // start of 1st line
        const char *e = b + strcspn (b, "\n");      // end of 1st line
        int linelen = int (e - b);          // lenght of 1st line

        int befdesclen = 0;
        if (befdesc)
            befdesclen = strlen (befdesc);

        int filelen = strlen (file);
        err |= (write (fileh, file, filelen) == -1);  // filename
        int spacelen = __max (13 - filelen, 1);
        err |= (write (fileh, "             ", spacelen) == -1);  // blanks

        if (!(fbbsflags & FBBS_FLAG_NODATESIZE)) {
            char sizes[13]; char dates[10];
            sprintf (sizes, "%7lu ", size);
            err |= (write (fileh, sizes, strlen (sizes)) == -1);  // size
            unix2dates (date, dates, Flags2Df (fbbsflags));
            strcat (dates, " ");
            err |= (write (fileh, dates, strlen (dates)) == -1);  // date
        }

        int flaglen = 0;                            // flags
        if (flag & (FF_NOTIME|FF_NOBYTES)) {
            char flags[5];
            flags[flaglen++] = '/';
            if (flag & FF_NOTIME)
                flags[flaglen++] = 't';
            if (flag & FF_NOBYTES)
                flags[flaglen++] = 'b';
            flags[flaglen++] = ' ';
            flags[flaglen] = '\0';
            err |= (write (fileh, flags, flaglen) == -1);
        }

        if (befdesclen)
            err |= (write (fileh, befdesc, befdesclen) == -1);
        int writelen = __min (linelen, linesize - filelen - spacelen - flaglen - befdesclen - 2);
        if (writelen)
            err |= (write (fileh, desc, writelen) == -1);
        err |= (write (fileh, "\n", 1) == -1);

        if (cpos != -1) {     // output continuation lines
                                          // init start of continuation lines
            char blank[] = "                                                                                ";
            if ((size_t)cpos > sizeof (blank) - 3) // space for cont, ' ', '\0'
                cpos = sizeof (blank) - 3;
            int contlen = cpos;

            if (cont != ' ') {
                blank[cpos] = cont;
                if (fbbsflags & FBBS_FLAG_NOCONTSPACE)
                    contlen += 1;
                else
                    contlen += 2;       // include cont and space
            }

            while (*e) {            // while lines available
                b = e + 1;              // begin on char following '\n'
                e = b + strcspn (b, "\n");   // end of line
                linelen = int (e - b);  // length of line
                err |= (write (fileh, blank, contlen) == -1);  // blanks and cont
                if (linelen)
                    err |= (write (fileh, b, __min (linelen, linesize - contlen - 2)) == -1);  // desc line
                err |= (write (fileh, "\n", 1) == -1);
            }
        }

    } else {            // isComment
        int writelen = strlen (desc);
        if (writelen)
            err |= (write (fileh, desc, writelen) == -1);
        err |= (write (fileh, "\n", 1) == -1);
    }

    return err;
}


int FBBS::SetDesc (const char *file, const char *desc, word flag, const char *befdesc,
                   time_t date, dword size)
{
    int err = 0;

    err |= PutGenEntry (file, desc, flag, befdesc, date, size);
    err |= WrClose ();

    return err;
}



