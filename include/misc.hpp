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

#ifndef MISC_HPP
#define MISC_HPP

#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>
#include <apgenlib.hpp>
#include <apgenlb2.hpp>
#include "cfgdata.hpp"
#include "defines.hpp"
#include "addrs.hpp"
#include "types.hpp"
#include <smapi/stamp.h>
#include "response.hpp"


#define __min(a,b)    (((a) < (b)) ? (a) : (b))
#define __max(a,b)    (((a) > (b)) ? (a) : (b))

#define CHK_EXIST TRUE

void vpwlog (byte msgtype, char *strfmt, ...);

void DeleteFile (char *name, BOOL ChkExist = FALSE);
void ftrunczero (FILE *f);
void KillWriteFile (FILE *f, char *name);
char *FindFlag (char *flags, char *flag);

//void new_handler (void);

BOOL IsFullPath (const char *path);
char *MkName (const char *path, const char *fname);

void BuildName (char *filename, const char *path, const char *fname);
// filename must be _MAX_PATH size

void SetDay3 (char *filename, int day);

time_t arcfiletime (const char *path);   // Unix File Time, 0 if not found.
                                         // The date used depends on ArcDate.
                                      // If Creation and not available,
                                      // Write is used.
time_t filetounix (const FFIND *ffblk);
BOOL toucharcfile (const char *fname, time_t ftime);

void MoveName (char *fullname, char *filename);
int adrcmp (ADR *ptr1, ADR *ptr2);


#define Normal  0x0000
#define Slash   0x0001      // bit oriented !!!
#define UnSlash 0x0002      // Slash and UnSlash are mutually exclusive !
#define MkDir   0x0004
#define Build   0x0008

psz getpath (pcsz tkp, psz path, int action = Normal, pcsz HeadPath = NULL);

// From tkp: skip blank, copy pathname (simple token or string ("string")
// to path (max _MAX_PATH chars, always null terminated), return pointer
// to next token. If no token available, path and returned pointer point
// to null strings.
// Slash: a slash is appended if necessary.
// UnSlash: terminating slash removed if present.
// MkDir: the dir is created if missing.
// Build: HeadPath is prepended if necessary.
//        If HeadPath is omitted, the fullpath is acquired.

psz getallocpath (pcsz tkp, psz *path, int action = Normal, pcsz HeadPath = NULL, int extralen = 0);

// As getpath, but allocates memory for path (with new).
// extralen specifies how many bytes in excess must be allocated.


void strslash(char *directory);
void strunslash (char *directory);

char *strzcat (char *dest, size_t maxlen, char *src, ...);
void strupr (char *);

void comma_it (char *str);
void dot_it (psz str);
void CopyUndash (pcsz fms, psz tos);

void vprintlog(char *strfmt,...);
void vwritelog(char *strfmt,...);
void vwritelogrsp (_RSP *rsp, char *strfmt,...);
void vprintlogrsp (_RSP *rsp, char *strfmt,...);

void EraseFile (const char *filename, int timeout = 0);
// Deletes filename and logs errors, optional timeout in seconds

BOOL fexist (char *filename);
// Checks for existance of file

char *newcpy (char *text);
// allocs (with new) and copies text

int dayn (time_t timer);
// converts unix time to day of year: 1..366

char *strdcmp (const char *s1, const char *s2);
// compares s1 to s2.
// If s2 is not equal to the beginning of s1 returns NULL
// If s2 is equal to the beginning of s2, returns a pointer to the first
//    character of s1 after the s2 string.
// Dashes '-' are ignored in comparison.

void myexit (int status);
// sets global var errorlevel = status before terminating

void UnixToFTime (char *FTime, time_t unixtime);

char *addrs (ADR *adr);

time_t dayntounix (int day);
// from dayn (1->366) to unix time

char *UnixToDate (time_t unixtime);
// unixtime is converted to date: e.g. "Jan 01, 1995"

char *UnixToLDate (time_t unixtime);
// unixtime is converted to local date

FILE *SmartOpen (char *filename, char *SepString = NULL);

// Opens filename in text mode, SH_DENYWR.
// If filename already exists and has been created during the current
// session, it is appended, otherwise it is overwritten.
// SepString is written before appending.

char *StrQTok (char *&c);

// Return a token (string delimited by ' ', '\t', '\n') considering
// quoted strings as a unique word.
// The string pointed to by c is changed (NULLs set after tokens).
// NULL is returned when no more tokens available.
// The c pointer is advanced to next token, to the empty string when no
// more tokens available.

int str2uint (const char *str, uint *res);

// converts str to unsigned int in res.
// returns 0 on success, -1 on error

void RCf (int code, ...);
// For RunCmd output

FILE* MyOpen (pcsz filename, pcsz mode, int shflags, int timeout);
// returns the stream pointer or exits with OPEN_ERR

void DiskFull (FILE *f);
// Truncates f, writes "Disk Full", exits

int printflush (const char *format, ...);
// As printf but with flush

void CfgError (CfgFile &cf);

#ifdef __DOS__

dword MemFree (void);

// Returns bytes of allocable memory

#endif

#endif      // Misc.hpp
