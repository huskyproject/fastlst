/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1992-1997  Alberto Pasquale                 */
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

#include <bbsgenlb.hpp>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#if !defined (__FreeBSD__)
  #include <malloc.h>
#endif
#include <unistd.h>

#include "types.hpp"
#include "misc.hpp"
#include "parse.hpp"
#include "apgenlib.hpp"
#include "data.hpp"
#include <smapi/stamp.h>

#define YEAR(t)     (((t & 0xFE00) >> 9) + 1980)
#define MONTH(t)    ((t & 0x01E0) >> 5)
#define DAY(t)      (t & 0x001F)
#define HOUR(t)     ((t & 0xF800) >> 11)
#define MINUTE(t)   ((t & 0x07E0) >> 5)
#define SECOND(t)   ((t & 0x001F) << 1)


void DeleteFile (char *name, BOOL CheckExist)
{
    if (CheckExist)
        if (!fexist (name))
            return;
    if (unlink (name))
        vprintlog ("Cannot erase \"%s\"\n", name);
    else
        vprintlog ("Erased \"%s\"\n", name);
}


void ftrunczero (FILE *f)
{
  ftruncate (fileno (f), 0);
}


void KillWriteFile (FILE *f, char *name)
{
    ftrunczero (f);
    fclose (f);
    unlink (name);
}


char *FindFlag (char *flags, char *flag)
{
    char *p1 = flags,
         *p2,
         *fp;

    while (*p1) {           // while flag string not exausted
        fp = p1;            // points to current flag
        p2 = flag;
        while (*p2) {       // compare loop
            if (*p1 != *p2)
                break;
            p1 ++;
            p2 ++;
        }
        if ((*p2 == '\0') && ((*p1 == '\0') || (*p1 == ',')))
            return fp;      // flag found, return pointer
        while ((*p1) && (*p1 != ','))  // advance to next flag
            p1 ++;
        if (*p1 == ',')
            p1 ++;
    }
    return NULL;            // flag not found
}


void new_handler (void)
{
    myexit (OUT_OF_MEMORY);
}


BOOL IsFullPath (const char *path)
{
    if (*path == '\0')
        return FALSE;
    if (*path == '/')       
        return TRUE;
    return FALSE;
}


char *MkName (const char *path, const char *fname)
{
    static char filename[PATH_MAX];

    if (path)
        BuildName (filename, path, fname);
    else
      strcpy (filename, fname);
//        _fullpath (filename, fname, PATH_MAX);

    return filename;
}


void BuildName (char *filename, const char *path, const char *fname)
{
   strcpy (filename, path);
   strslash (filename);
   strcat (filename, fname);      
}


void SetDay3 (char *filename, int day)
{
    char *p = filename + strlen (filename) - 3;
    sprintf (p, "%03d", day);
}


int dayn (time_t timer)
{
    return (localtime (&timer)->tm_yday + 1);
}


time_t arcfiletime (const char *path)
{
    time_t mtime, ctime;

    if (!getftime (path, &mtime, &ctime))
        return 0;

    return mtime;
}


time_t filetounix (const FFIND *ffblk)
{
    return dosdatimetounix (ffblk->wr_date, ffblk->wr_time);
}


BOOL toucharcfile (const char *fname, time_t ftime)
{
    return touchf (fname, ftime);
}


void MoveName (char *fullname, char *filename)
{
#ifdef UNIX
    char *p = strrchr (fullname, '/');
#else
    char *p = strrchr (fullname, '\\');
#endif
    if (!p)
        p = fullname;
    else
        p ++;

    strcpy (filename, p);
    *p = '\0';
}


int adrcmp (ADR *ptr1, ADR *ptr2)
{
    register int result;

    result = ptr1->zone - ptr2->zone;
    if (result != 0)
        return (result);

    result = ptr1->net - ptr2->net;
    if (result != 0)
        return (result);

    result = ptr1->node - ptr2->node;
    if (result != 0)
        return (result);

    return (ptr1->point - ptr2->point);
}


psz getpath (pcsz p, psz path, int action, pcsz HeadPath)
{
    pcsz d;

    p = SkipBlank (p);
    if (*p == '"') {
        p++;
        d = p + strcspn (p, "\"");
    } else
        d = p + strcspn (p, " ");

    int pathlen = __min ((int) (d - p), PATH_MAX - 1);
    strncpy (path, p, pathlen);
    path[pathlen] = '\0';

    d = SkipBlank (d);
   
    if (action & Build) 
    {
        pcsz fullname = MkName (HeadPath, path);
        strcpy (path, fullname);
    }

    if (action & Slash)
        strslash (path);
    if (action & UnSlash)
        strunslash (path);
    if (action & MkDir)
        mkdirpath (path);

    return (char *)d;
}


psz getallocpath (pcsz p, psz *path, int action, pcsz HeadPath, int extralen)
{
    char pathbuff[PATH_MAX];

    p = getpath (p, pathbuff, action, HeadPath);

    *path = new char[strlen (pathbuff) + extralen + 1];
    strcpy (*path, pathbuff);

    return (char *)p;
}


void strslash (char *directory)
{
    int l;

    l=strlen(directory);
    if ((l > 0) && (directory[l-1]!='/')) {
        directory[l]='/';
        directory[l+1]=0;
	}
}


void strunslash (char *directory)
{
    int l = strlen (directory);
    if ((l > 0) && (directory[l-1] == '/'))
        directory[l-1]='\0';
}


char *strzcat (char *dest, size_t maxlen, char *src, ...)
{
    va_list args;
	char *s, *d;
    size_t count;

    count = 1;

	d = dest;
	s = src;

	while ((*s) && (count < maxlen)) {
		*d++ = *s++;
        count++;
    }

    va_start (args, src);
    while ((s = va_arg (args, char *)) != NULL) {
        while ((*s) && (count < maxlen)) {
			*d++ = *s++;
            count++;
        }
    }
    va_end (args);

	*d = '\0';

	return (dest);
}


void comma_it (char *str)    /* Change dashes to commas */
{
   char *p;

   p = str;
   while (*p)
      {
	  if (*p == '-')
		  *p = ',';
      ++p;
      }
}


void dot_it (psz str)    // Change dashes to dots
{
   psz p = str;
   while (*p) {
	  if (*p == '-')
          *p = '.';
      ++p;
   }
}

                            // Copy removing dashes
void CopyUndash (pcsz fms, psz tos)
{
   const char *f = fms;
   char *t = tos;
   while (*f) {
      if (*f != '-')
         *t++ = *f;
      f++;
   }
   *t = '\0';
}


void wtlog (char *strfmt, va_list args)
{
    char date[16];
    char format[100];
    time_t timer;
    char *c, *d;

    if (!logfile)
        return;

    c = strfmt;     // skip LF CR TAB
    while ((*c == '\n') || (*c == '\r') || (*c == '\t'))
        c++;

    d = c;
    while ((*d) && (*d != '\n'))
        d++;
    strncpy (format, c, (int) (d-c));
    format[(int)(d-c)] = '\0';

    time (&timer);
    strftime (date, 16, "%d %b %H:%M:%S", localtime (&timer));

    fprintf (logfile,"  %s FLST ", date);

    vfprintf (logfile, format, args);

    fputc ('\n', logfile);

    fflush (logfile);
}


void vwritelog (char *strfmt,...)
{
    va_list args;

    va_start (args, strfmt);
    wtlog (strfmt, args);
    va_end (args);
}


void vprintlog (char *strfmt,...)
{
    va_list args;

    va_start (args, strfmt);
    vprintf (strfmt, args);
    va_end (args);

    va_start (args, strfmt);
    wtlog (strfmt, args);
    va_end (args);
}


void vpwlog (byte msgtype, char *strfmt, ...)
{
    va_list args;

    if (msgtype == AH_MT_Action) {
        va_start (args, strfmt);
        vprintf (strfmt, args);
        va_end (args);
    }

    va_start (args, strfmt);
    wtlog (strfmt, args);
    va_end (args);
}


#pragma off (unreferenced)

void vwritelogrsp (_RSP *rsp, char *strfmt,...)
{
    va_list args;

    va_start (args, strfmt);
    wtlog (strfmt, args);
    va_end (args);

    va_start (args, strfmt);
    vwritersp (rsp, strfmt, args);
    va_end (args);

}

void vprintlogrsp (_RSP *rsp, char *strfmt,...)
{
    va_list args;

    va_start (args, strfmt);
    vprintf (strfmt, args);
    va_end (args);

    va_start (args, strfmt);
    wtlog (strfmt, args);
    va_end (args);

    va_start (args, strfmt);
    vwritersp (rsp, strfmt, args);
    va_end (args);

}
#pragma on (unreferenced)


void EraseFile (const char *filename, int timeout)
{
    int ret;
    int i;

    i = 0;
    ret = unlink (filename);
    while ((ret != 0) && (i < timeout)) {
        sleep (1);
        i++;
        ret = unlink (filename);
    }

    if (ret)
        vprintlog ("Cannot erase \"%s\"\n", filename);
}


BOOL fexist (char *filename)
{
    return (access (filename, F_OK) == 0);
}


void RCf (int code, ...)
{
    va_list args;
    va_start (args, code);
    switch (code) {
        case RC_DOING:
            vprintlog ("Executing \"%s\"\n", va_arg (args, char *));
            break;
        case RC_EXIT:
            vprintlog ("Exit code: %d\n", va_arg (args, int));
            break;
        case RC_EMPTY:
            vprintlog ("Error: Empty external command !\n");
            break;
    }
    va_end (args);
}


char *newcpy (char *text)
{
    char *tmp = new char[strlen (text) + 1];
    strcpy (tmp, text);
    return tmp;
}


char *strdcmp (const char *s1, const char *s2)
{
    while (1) {
        while (*s1 == '-')
            s1 ++;
        while (*s2 == '-')
            s2 ++;
        if (*s2 == '\0')
            return (char *)s1;
        if (*(s1++) != *(s2++))
            return NULL;
    }
}


void myexit (int status)
{
    errorlevel = status;
    exit (status);
}


void UnixToFTime (char *FTime, time_t unixtime)
{
    strftime (FTime, 20, "%d %b %y  %H:%M:%S", localtime (&unixtime));
}


char *addrs (ADR *adr)
{
    static char addrstr[30];

    sprintf (addrstr, "%hd:%hd/%hd.%hd", adr->zone, adr->net, adr->node, adr->point);
    return addrstr;
}


time_t dayntounix (int day)     // from dayn (1->366) to unix time
{
    time_t now = time (NULL);
    struct tm *tm = localtime (&now);

    int today = tm->tm_yday + 1;
    int year = tm->tm_year + 1900;

    int maxday = (today + 10) % 367;
    if (maxday < today)
        year ++;
    if (day > maxday)
        year --;

    tm->tm_sec = 0;
    tm->tm_min = 0;
    tm->tm_hour = 0;
    tm->tm_mon = 0;
    tm->tm_mday = 1;
    tm->tm_year = year - 1900;
    tm->tm_wday = 0;
    tm->tm_yday = 0;
    tm->tm_isdst = 0;

    time_t jan1 = mktime (tm);      // 1st of january

    return (jan1 + (day - 1) * 86400L);
}


char *UnixToDate (time_t unixtime)
{
    static char date[13];
    strftime (date, sizeof (date), "%b %d, %Y", localtime (&unixtime));
    return date;
}


char *UnixToLDate (time_t unixtime)
{
    static char date[30];
    strftime (date, sizeof (date), "%x", localtime (&unixtime));
    return date;
}


FILE *SmartOpen (char *filename, char *SepString)
{
    FILE *f = fopen (filename, "at");

    long futime = DosFileTime (fileno (f));

    if (futime < StartTime)   // old file
        ftrunczero (f);
    else
        if (SepString)
            fprintf (f, SepString);

    return f;
}


char *StrQTok (char *&c)
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


int str2uint (const char *str, uint *res)
{
    *res = strtoul (str, NULL, 10);
    return 0;
}


FILE* MyOpen (pcsz filename, pcsz mode, int shflags, int timeout)
{
    FILE *f = fopen (filename, mode);

    if (!f) {
        vprintlog ("Error opening \"%s\"\n", filename);
        myexit (OPEN_ERR);
    }

    setvbuf (f, NULL, _IOFBF, BufSize);

    return f;
}


void DiskFull (FILE *f)
{
    ftrunczero (f);
    vprintlog ("\nDisk Full\n\n");
    myexit (DISK_FULL);
}


int printflush (const char *format, ...)
{
    va_list args;

    va_start (args, format);
    int ret = vprintf (format, args);
    va_end (args);

    fflush (stdout);

    return ret;
}


void CfgError (CfgFile &cf)
{
    vprintlog ("Error in line %d: \"%s\"\n", cf.LineN (), cf.ReGetLn ());
    myexit (CFG_ERROR);
}
