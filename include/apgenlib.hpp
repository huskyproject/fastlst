/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1994-1997  Alberto Pasquale                 */
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

#include <dirent.h>

#ifndef _APGENLIB_HPP_
#define _APGENLIB_HPP_

#include <limits.h>
#include <stddef.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "defines.h"
#include "typedefs.h"
#include <smapi/stamp.h>
#include "bbsgenlb.hpp"

//#pragma pack (1)

typedef int (*CMPF) (const void *, const void *);
// Usually the two pointers are double pointers.



                // Implementation in MDIR.CPP


int mkdirpath (const char *path);

// Tries to build path, if not existent.
// Returns 0 on path created, 1 on path already existent, -1 on error.


int cdd (const char *newdir);

// Changes drive and directory.
// newdir is a full path specification (can have drive) and
// can optionally have trailing backslash.
// Returns 0 on success.


                // Implementation in DirStack.Cpp

// Class to handle Push and Pop of directory.
// The drive is also changed whenever appropriate.

struct _DirStackEl;

class DirStack {
    private:
        _DirStackEl *Top;
    public:
        DirStack ();
        ~DirStack ();
        int Push (const char *newdir);
                // newdir is a full path specification (can have drive)
                // and can optionally have a terminating backslash.
                // Returns 0 on success.
        int Pop ();
                // Returns 0 on success.
                // Returns 1 if stack is empty.
                // Returns -1 in case of error.
};

                // Implementation in PARSETXT.CPP


#define GN_None    0x0000
#define GN_Empty   0x0001       // Return empty string instead of NULL
#define GN_SrcFix  0x0002
#define GN_BSlash  0x0004       // Assure backslash termination
#define GN_UBSlash 0x0008       // Assure no backslash termination
#define GN_MkDir   0x0010       // Make the dir path if missing


int GetName (const char *&src, char *dest=NULL, int maxlen=INT_MAX, int action=GN_None, BOOL *HasSpace=NULL);

/* Copies a "name" from src to dest.
 *
 * The "name" can be: - A word (sequence of chars not containing
 *                      blanks ' ' or quotes '"').
 *                    - A multi-word between quotes ("word word word").
 *                      If you need to include the quotes character '"'
 *                      in the "name", you must write it twice.
 *                      Tabs are changed to blanks, if found inside
 *                      the quoted string.
 *                      E.G. """one""""two""" becomes '"one""two"'.
 *                           "one two" becomes 'one two'
 *
 * src:    A reference to the pointer to the buffer to be analyzed, there
 *         can be space before the real name.
 *         On return, the src pointer points to the first non space character
 *         that follows the "name", unless dest is NULL or action is GN_SrcFix
 *         (in which case it is not moved).
 *         It will point to the terminating NULL if no other non-space
 *         characters are present in the string.
 *
 * dest:   A pointer to the buffer where the name will be copied.
 *         The enclosing quotes (if present) will NOT be copied.
 *         If no name is found, only the terminating NULL is copied to
 *         dest, so that it will contain a valid empty string.
 *         You must assure there is enough space to hold the "name".
 *         If omitted, nothing is stored but the length of the name
 *         is evaluated and src not moved.
 *
 * maxlen: An optional specification for maximum copy length.
 *         If used, no more than maxlen bytes will be copied to
 *         dest, including the terminating NULL (always included).
 *         When dest==NULL, maxlen is ignored.
 *
 * action: Specifies some optional action.
 *         GN_MkDir is ignored if dest == NULL
 *
 * HasSpace: is set TRUE if the name has spaces.
 *
 * return: The length of dest, NULL excluded, -1 on error
 *         When no more "names", returns 0 !
 */


char *GetStatName (const char *&src, int action = GN_None, int *len=NULL, BOOL *HasSpace=NULL);

/* Copies a "name" from src to a static buffer  !!! No Multithread !!!
 *
 * As GetName, but returns a pointer to a static buffer.
 * If the name lenght exceeds that of the buffer (PATH_MAX), it is truncated.
 * If action != GN_Empty, returns NULL on empty string.
 * The optional len pointer allows to return the length of the string,
 * NULL excluded.
 * In case of error (GN_MkDir) return NULL.
 * !!!!!!! No provision for MultiThreading !!!!!!!!
 */


char *GetAllocName (const char *&src, int action=GN_None, int *len=NULL, BOOL *HasSpace=NULL);

/* Copies a "name" from src to an allocated dest.
 *
 * The same as GetStatName, but this function allocates space
 * for dest and returns the resulting pointer.
 * You can free the allocated memory using operator delete.
 * Instead you are not allowed to use the free function.
 * There is no limit on the length of "name".
 */


void strbslash (char *dir);

// Assure dir is backslash terminated.
// Directory MUST have space for the added character

void strubslash (char *dir);

// Assure dir is not backslash terminated.
// The terminating backslash is overwritten with '\0'.


#define GL_None    0x00
#define GL_Empty   0x01

int GetLn (const char *src, char *dest=NULL, int maxlen=INT_MAX);
char *GetStatLn (const char *src, int action=GL_None, int *len=NULL);
char *GetAllocLn (const char *src, int action=GL_None, int *len=NULL);

// Get rest of line.
// Similar to GetName/GetStatName/GetAllocName, but:
// - src is not moved
// - The whole line is copied to destination, skipping both heading and
//   trailing space.
// - If the first non space character is a quotes '"', only that string
//   is copied (subsequent strings, if present, are ignored).


#if defined (__OS2__)

                // Implementation in QueryFs.Cpp


#define QFS_ERROR         -1    // Usually when unit not ready or undefined
#define QFS_OTH            0
#define QFS_FAT            1
#define QFS_HPFS           2
#define QFS_NET_EA         3    // network drive, EAs supported
#define QFS_NET_NOEA       4    // network drive, EAs not supported
#define QFS_DEV            5    // device
#define QFS_CDFS           7    // CDROM
#define QFS_NET_DEV        8    // device


int QueryFS (const char *filename);

// Returns the type of files system where filename resides.
// If filename is NULL or does not have a drive specification, current
// drive is used.
// UNC names are recognized.
// If filename specifies a dir or file, this may not exist.


            // Implementation in System.Cpp

void SetPriority (LONG Delta, ULONG Class = PRTYC_NOCHANGE, ULONG Scope = PRTYS_PROCESSTREE, ULONG ID = 0);

// Changes priority of process or thread.
// Delta: -31 (PRTYD_MINIMUM) -> 31 (PRTYD_MAXIMUM)
//        relative to current (0 if class changed also)
// Class: PRTYC_IDLETIME, PRTYC_REGULAR, PRTYC_FOREGROUNDSERVER, PRTYC_TIMECRITICAL
// Scope: PRTYS_PROCESS (all threads of any process)
//        PRTYS_PROCESSTREE (all threads of a process and any descendants)
//        PRTYS_THREAD (a single thread of the current process)
// Priority of other processes is changed only if they were not set explicitly.
// ID: Process or thread identifier


            // Implementation in OS2_extended_attributes.cpp


#define EAGS_NULL            2      // EA is not present
#define EAGS_MULTI           1      // EA is MultiValue
#define EAGS_OK              0
#define EAGS_SMALL_BUFFER   -1
#define EAGS_OPEN_ERR       -2
#define EAGS_CLOSE_ERR      -3
#define EAGS_ERROR         -99


struct EAsingle {      // structure for single EA
    word type;
    word length;
    pvoid value;
};

// EAsingleGet, input: type ignored
//                     length is size of value
//                     value points to a buffer of length byte size
//             output: type is EAT_* (BSEDOS.H)
//                     length is length of ea stored in value
//                     value contains the EA of length size
//
// EAsingleSet       : type is EAT_* of value
//                     length is length of EA stored in value
//                     value contains the EA of size length
//
//                     If ea==NULL, the EA is deleted
//

#define EAsingleHeadSize    4


int EAsingleGet (pcsz filename,
                 pcsz eaname,
                 EAsingle *ea
                );        // Returns:
                          // EAGS_OK: success
                          // EAGS_SMALL_BUFFER: easize insufficient
                          // EAGS_OPEN_ERR: cannot open filename
                          // EAGS_ERROR: other error
                          // EAsingle.length == 0 means EA not found

int EAsingleGet (int handle,      // must be open for read access, deny-write
                 pcsz eaname,
                 EAsingle *ea
                );


int EAsingleSet (pcsz filename,
                 pcsz eaname,
                 const EAsingle *ea
                );         // EAGS_OK: success
                           // EAGS_OPEN_ERR: no filename
                           // EAGS_CLOSE_ERR: closing filename
                           // EAGS_ERROR: other error


int EAsingleSet (int handle,      // must be open for write, deny-both
                 pcsz eaname,
                 const EAsingle *ea
                );

                                // Sets longname as .LONGNAME EA
int SetLongName (int handle,    // open for write, deny=both
                 pcsz longname  // no path
                );
int SetLongName (pcsz filename, // w path
                 pcsz longname  // no path
                );
                  // Gets longname from .LONGNAME, empty string if none
int GetLongName (int handle,    // open for read, deny-write
                 psz longname   // PATH_MAX size required
                );
int GetLongName (pcsz filename, // w path
                 psz longname   // PATH_MAX size required
                );

#endif


            // Implementation in Dates.Cpp

time_t dosdatimetounix (ushort date, ushort time);
// converts dos date and time to unix timestamp
// if date and time == 0, returns 0

//time_t DosDatime2Unix (const dosdate_t *ddate, const dostime_t *dtime = NULL);
// the dayofweek in *ddate is not relevant.

time_t stamptounix (_stamp *stamp);
// converts a datime "stamp" to unix timestamp
// If stamp is zeroed, returns 0


//void unix2dosdatime (time_t utime, ushort *date, ushort *time);
// same as unix2stamp but without _stamp structure

//void Unix2DosDatime (time_t utime, dosdate_t *ddate = NULL, dostime_t *dtime = NULL);

void unix2stamp (time_t utime, _stamp *datime);
// converts unix datime to dos stamp
// If time == 0, stamp is zeroed

#ifdef __NT__
  void FiletimeUtc2Dosdatime (CONST LPFILETIME lpft, ushort *date, ushort *time);
#endif


extern byte DateFormat;

#define DF_DEFAULT  0xFF        // use DateFormat (no override)

// WARNING WARNING: these defines MUST NOT be changed !!!!!!

#define DF_USA      0x00        // mm-dd-yy, default for DateFormat
#define DF_EURO     0x01        // dd-mm-yy
#define DF_JAPAN    0x02        // yy-mm-dd
#define DF_SCIENT   0x03        // yymmdd

char *unix2dates (time_t utime, char *dates = NULL, byte DFovr = DF_DEFAULT);
char *stamp2dates (_stamp *stamp, char *dates = NULL, byte DFovr = DF_DEFAULT);

// Convert unix timestamp or "dos" timestamp to a string of 8 or 6 characters.
// dates must have enough space. If NULL, static buffer is returned.
// Returns pointer to dates.
// The format of dates depends on global variable DateFormat.
// DFovr overrides DateFormat (necessary for multithreading)

time_t dates2unix (char *dates, byte DFovr = DF_DEFAULT);
// converts to unix a date in the form specified by DateFormat or DFovr
// returns 0 if dates not valid


        // Implementation in DosFind.Cpp


// attributes (input):

#define _DFF_A_NORMAL           0x00000000
#define _DFF_A_REQ_HIDDEN       0x00000001
#define _DFF_A_REQ_SYSTEM       0x00000002
#define _DFF_A_REQ_DIRECTORY    0x00000004
#define _DFF_A_REQ_ARCHIVED     0x00000008
#define _DFF_A_REQ_READONLY     0x00000010
#define _DFF_A_MAY_HIDDEN       0x00000020
#define _DFF_A_MAY_SYSTEM       0x00000040
#define _DFF_A_MAY_DIRECTORY    0x00000080
#define _DFF_A_NON_ARCHIVED     0x00000100
#define _DFF_A_NON_READONLY     0x00000200

#ifdef __OS2__
  #define _DFF_A_LONGNAME    0x80000000 // get .LONGNAME (FAT, NET w EAs)
#endif

// _DFF_A_NORMAL: nothing required, SYSTEM,HIDDEN,DIRECTORY prohibited
//                READONLY,ARCHIVED permitted


// FFIND.attrib:
//
#define _DFF_FILE_ARCHIVED      0x20
#define _DFF_FILE_DIRECTORY     0x10
#define _DFF_FILE_SYSTEM        0x04
#define _DFF_FILE_HIDDEN        0x02
#define _DFF_FILE_READONLY      0x01
#define _DFF_FILE_NORMAL        0x00


struct FILEFINDEABUF;

struct FFIND {

  private:
#if defined (__OS2__)
    HDIR DirHandle;
    ULONG bufsize;
    ULONG infolevel;
    union {
      FILEFINDBUF3  *ffb3;
      FILEFINDEABUF *ffeab;
      pvoid         *ff;
    };
#elif defined (__NT__)
    HANDLE DirHandle;
    PWIN32_FIND_DATA ff;
#elif defined (__linux__) || defined(UNIX)
    dirent *ff;   
#else   // __DOS__
    find_t *ff;
#endif

    byte flags;
    byte required_attributes;
    byte forbidden_attributes;

  public:
    byte attrib;
    unsigned short cr_time;     // 0 when not available (DOS FAT)
    unsigned short cr_date;     // 0
    unsigned short ac_time;     // 0
    unsigned short ac_date;     // 0
    unsigned short wr_time;
    unsigned short wr_date;
    unsigned long  size;
    char *name;

    friend void Dos2My (FFIND *);
    friend void SetAttrMask (unsigned, FFIND *);
    friend unsigned _DosFindFirst (pcsz, unsigned, FFIND *);
    friend unsigned _DosFindNext (FFIND *);
    friend unsigned _DosFindClose (FFIND *);

};


unsigned _DosFindFirst (pcsz path, unsigned attributes, FFIND *buffer);
unsigned _DosFindNext (FFIND *buffer);
unsigned _DosFindClose (FFIND *buffer);



        // Implementation in Dir.Cpp


struct _dir {
    byte namelen;
    BOOL Got;       // Already Got via Get (filename) method
    long fsize;
    time_t cdate;   // creation date, 0 under DOS or FAT
    time_t mdate;   // modification date
    char fname[1];  // first char of name
};

struct _dirblk;

class DIRcl {
    private:
        _dirblk *dirblkhead;
        _dirblk *curblk;
        _dir **dirp;
        uint ntot;

        void Add (const FFIND *f);
        void Read (const char *path);

    public:
        DIRcl (const char *path);
        ~DIRcl(void);
        _dir *Get (int first = 0);
        _dir *Get (const char *filename, BOOL NoDupes = FALSE);
};

// DIRcl (path): reads the directory pointed to by path, normal files only.
// Get (1): returns the first file (alphabetically).
// Get (): returns next file.
// Get (filename): returns the _dir struct for filename, NULL if none.
//      Sets Got=TRUE.
//      filename must be a simple filename (no wildcards).
//      If NoDupes = TRUE and Got = TRUE, returns NULL.


            // Implementation in file.cpp

struct _ExistStat {
    time_t ctime;  // HPFS Creation time, 0 under Dos or FAT
    time_t atime;  // HPFS Last Access time, 0 under Dos or FAT
    time_t mtime;  // only field valid under Dos or FAT
    ulong  size;
};
                    // bit-wise for Exist
#define _ExFILE_    1      // is a file
#define _ExDIR_     2      // is a directory
#define _ExWRITE_   4      // has write permission
#define _ExDEVICE_  8      // is a character device

int Exist (const char *path, _ExistStat *st = NULL);
// path can have a trailing backslash or not in the case it's a directory
// If st, *st is filled in.

BOOL IsShareViol (void);
// Returns TRUE if last error was due to a Share violation.

int stopen (char *path, int access, int shflag, int mode, int timeout);
// sopen with timeout in seconds

FILE *_fstopen (const char *filename, const char *mode, int shflag, int timeout);
// _fsopen with timeout in seconds

BOOL getftime (int handle, time_t *mtime, time_t *ctime = NULL, time_t *atime = NULL);
// get file time, 0 if not existent (in FAT and Dos only mtime can be retrieved)
// Return TRUE on success.

BOOL getftime (const char *filename, time_t *mtime, time_t *ctime = NULL, time_t *atime = NULL);


                     // bit wise definitions for touchf and copy/movefile
#define _CF_notouch_    0x00
#define _CF_mtouch_     0x01
#define _CF_ctouch_     0x02
#define _CF_touchflags_ 0x03

                                // for copy/movefile only,
                                // ignored by touchf.
#define _CF_ExistFail_  0x80    // Fail if destination exists

                    // return values for copy/movefile

#define _CF_OK_             0
#define _CF_ERROR_         -1
#define _CF_NOSRC_         -2       // source does not exist
#define _CF_TOUCHERR_       1
#define _CF_SRCNOTDELETED_  2
#define _CF_SRCEQDEST_      3
#define _CF_DESTEXISTS_     4


                            // for touchf mtime
#define _TF_Auto     1UL    // If ctime not valid -> set mtime

BOOL touchf (int handle, time_t mtime, time_t ctime = 0, time_t *nowp = NULL);
BOOL touchf (pcsz filename, time_t mtime, time_t ctime = 0, time_t *nowp = NULL);
BOOL touchf (int handle, byte touchflag = _CF_mtouch_ | _CF_ctouch_, time_t *nowp = NULL);
BOOL touchf (pcsz filename, byte touchflag = _CF_mtouch_ | _CF_ctouch_, time_t *nowp = NULL);
// Sets filename time to ?time,
// handle MUST be open for writing with SH_DENYRW
// if ?time is 0 it is not changed,
// if ?time is ULONG_MAX the current time is used (touch).
// If ctime=ULONG_MAX is specified and old ctime==0 (FAT, DOS), mtime is touched.
// If nowp != NULL, the current time is stored in *nowp.
// Returns TRUE on success.
// The last 2 forms allow touch only, full by default.

                // copyfile & movefile

// Dos: _CF_ctouch_ is equivalent to _CF_mtouch.
// OS/2: if _CF_ctouch specified and creation datime not available,
// _CF_mtouch is done.


int copyfile (pcsz sourcefile, pcsz destfile, byte flags = _CF_notouch_);
// Copies sourcefile to destfile, optionally resets its datime.
// Source and destination cannot contain wildcards.
// In the case of source==dest, touch is done anyway

int movefile (pcsz sourcefile, pcsz destfile, byte flags = _CF_notouch_);
// as copyfile but deletes source (uses rename when possible).


// for tunlink, bitmapped
#define _TUNLINK_NRM_  0        // normal unlink
#define _TUNLINK_ANY_  1        // reset HSR attributes before deleting

// for tunlink results

#define _TUNLINK_OK_            0
#define _TUNLINK_NOTFILE_       1
#define _TUNLINK_UNDELETABLE_   2
#define _TUNLINK_TIMEOUT_       3
#define _TUNLINK_ERROR_        -1

int tunlink (const char *path, int timeout, int flags = _TUNLINK_NRM_);
// as unlink, but retries for timeout seconds before giving up

char *pathnobs (char *path);
// Removes trailing backslash from path, if present.
// Does not remove from "c:\" or "\".
// Returns path.

char *pathwbs (char *path);
// Appends a trailing backslash to path, if not present.
// Does not add to "c:" or "".
// Returns path.

char *fullpath (char *buffer, const char *path, size_t size = PATH_MAX,
                int *len = NULL);
// Finds the full path specification for path and stores it in buffer.
// Returns buffer on success, NULL on error.
// If len != NULL, the length of the returned path is stored.
// path can be a file or a directory, with or without a terminating '\'.
// The returned fullpath will have a terminating '\' if path has it.
// Multiple '.' are supported: "..." means two levels up in the tree.
// If path is NULL or points to an empty string, returns the current
// working directory.
// If buffer is NULL, allocates a buffer of PATH_MAX that can be
// deallocated with free.
// If size is omitted, PATH_MAX is assumed.
// If size is not enough, error is returned.
// After error, errno contains:
// ENOENT: erroneous path.
// ENOMEM: the buffer could not be allocated.
// ERANGE: the buffer was too small.


BOOL eqpath (const char *p1, const char *p2);
// compares p1 to p2 after converting them to full path.
// if p1 or p2 is NULL, returns FALSE.
// p1 and p2 can have terminating '\' and use multiple '.'.
// returns TRUE if they are equal.


int fnamecmp (const char *fname, const char *wildname);
// compares fname to wildname (? and * accepted, OS/2 way)
// return 0 if equal.


char *hasext (pcsz filename);
// NULL: filename (w possible path) has no extension
// !NULL: points to '.' starting the extension
// May also be used with dir names, if not terminated by backslash.

void remext (char *filename);
// removes extension from filename, if available

void setext (char *filename, pcsz newext);
// substitues current extension (if any) with newext in filename.
// If filename does not have an extension, newext is appended.
// newext must include the heading '.'

void addext (char *filename, pcsz ext);
// if filename has no extension, ext is appended; otherwise NOP

char *filefrompath (const char *path);
// returns a pointer to the filename contained in path.
// Points to the terminating '\0' if path terminates with '\'.

char *fullname2path (const char *fullname, char *path, const char **fname = NULL);
// copies from fullname the path, including terminating '\', to path.
// path must be PATH_MAX long.
// The optional fname, on return, will point to the filename in fullname.
// Returns path.

time_t DosFileTime (const char *filename);
time_t DosFileTime (int handle);
// returns the unix modification time of filename or handle.
// Returns 0 if file not found.





            // Implementation in HardErr.Cpp


void HardErrDisable (void);
// Disables Hard error pop ups


            // Implementation in RunCmd.Cpp

typedef void (*RCShow) (int code, ...);
// called by RunCmd when something must be output.
// code values and respective following parameters

#define RC_DOING 1  // resolved command line string
#define RC_EXIT  2  // int exit code
#define RC_EMPTY 3  // none: empty command line error

int RunCmd (const char *cmd, RCShow rcs = NULL, const char *prmlst = "", ...);
// - Runs the command contained in cmd.
// - cmd can be an executable, batch, or any command line command.
//   If the command is not found, .COM and .EXE extensions are attempted.
//   If cmd cannot be executed directly, the command processor is
//   loaded and executed.
//   A maximum of 20 tokens is allowed.
// - rcs is a function to be called for outputting status
// - prmlst is a string containing the list of characters for
//   parameter substitution. E.g. prmlst = "ad": tokens %a and %d in
//   cmd will be substituted by the first and second string listed after
//   prmlst.
//   Under OS/2, when the command processor must be invoked,
//   each substituted parameter is enclosed in quotes, so that
//   special characters (command separator, escape etc.) do not
//   cause trouble.
// - The total size of the expanded command line must not exceed
//   4*PATH_MAX.
// - ... there must be as many strings as many characters are contained
//   in prmlst (max 10).
// - returns the program exit code (errorlevel)
//   returns -1 on RC_EMPTY error or (cmd == NULL)
// - If you go out of limits with tokens, parameters, expanded length
//   nothing bad will happen (all limits checked internally).


        // implementation in Misc.Cpp


char *newcpy (pcsz text);
// allocates with new the space for text and copies it therein.
// returns the allocated and filled string.

char *newncpy (pcsz text, size_t len);
// allocates with new and copies (up to) len chars therein, NULL appended.
// returns the allocated and filled string.

char *strzcpy(char *dest, const char *src, size_t maxlen);
// copies src to dest up to a maximum of maxlen-1 characters,
// always appends a NULL, returns dest.
// maxlen MUST be at least 1.

char *stpcpy (char *dest, const char *src);
// copies src to dest, returns pointer to terminating null

char *stpzcpy (char *dest, const char *src, size_t maxlen);
// copies src to dest up to a maximum of maxlen-1 characters,
// always appends a NULL, returns pointer to terminating null
// maxlen MUST be at least 1.

// char *strlcat (char *dest, const char *src, size_t totsize);
// Appends src (w terminating null) to dest, returns dest.
// If totsize is not sufficient, returns NULL

char *stristr (const char *str, const char *substr);
// As strstr but case insensitive.

char *_fgets (char *buf, size_t n, FILE *fp, int *buflen = NULL);
// As standard fgets, but removes trailing \n and skips remainder
// if line exceeds n.
// Optionally, the string length (\n removed) is stored in buflen.

char *StrChg (char *src, char from, char to);
// Changes 'from' to 'to' in string src.
// Returns src;

int _fputs (const char *buf, FILE *fp);
// As standard fputs, but adds trailing \n

// class Busy to get exclusive access to some resource.
// The .BSY flag is named after refname: the path must be valid,
// the extension is changed to or appended as .BSY.
// No problem if .BSY is not deleted after power failure etc.

#define BSY_EXCL  0     // exclusive access
#define BSY_SHARE 1     // shared (read) access

class Busy {
    private:
        int bsyh;
        char *bsyname;
    public:
        Busy (pcsz refname);       // Establish flag name.
        int Wait (int timeout = 0,  // timeout in seconds.
                  byte shflg = BSY_EXCL); // sharing flags
                        // returns 0 on success (bsy created),
                        // -1 on timeout (bsy already open)
        ~Busy ();                   // deletes the bsy flag
};


                    // implementation in Pipe.Cpp

// class to deal with two pipes (one bidirectional pipe)

                    // The Pipe must always have enough space
                    // for the items that are written
class Pipe {
    private:
        int hpread,     // handle for reading from pipe
            hpwrite;    // handle for writing into pipe
    public:
        Pipe (int hpread, int hpwrite); // handles for read and write
        ~Pipe ();       // closes the pipe
        void SendByte (byte b);    // send byte
        void SendString (const char *text);   // send zero terminated string
        void SendBlock (const void *data, long count);  // send data
                                            // Zero length string or block
                                            // is acceptable.
        byte WaitByte (byte *b = NULL); // if b != NULL,
                                        // the returned byte is stored in *b.
        char *WaitString (char *buffer); // buffer must be long enough
                                        // returns buffer
        void *WaitBlock (void *data);    // returns data
                                        // data must be capable enough
};


                    // Implementation in HeapStor.Cpp

struct HeapBlk;

typedef void (*HSPF) (void *obj, void *prm);

// When the CMPF is used to compare a key to an element of the heap,
// the first argument is the pointer to the key,
// the second one is the (double) pointer to the element in the heap.


class HeapStore {
    private:
        size_t blksize;
        HeapBlk *HeapHead,      // head of HeapBlk list
                *HeapTail,      // current HeapBlk, for store
                *HeapCur;       // current HeapBlk, for GetFirst/Next
        uint tailofs,   // offset in current HeapBlk.buf, for store
             curofs;    // offset in current HeapBlk.buf, for GetFirst/Next
        uint ni;        // total number of items stored
        CMPF cmpf;      // last sort function used

        void **ptr;     // pointer to array of pointers to stored objects

        void LoadArray ();  // Load array of indexes
    public:
        HeapStore (size_t blksize = 1024);
                                    // blksize MUST be large enough to get
                                    // an adequate number of entries.
                                    // Entries > blksize are skipped
        ~HeapStore ();
        void Store (const void *p); // object to be stored,
                                    // total size in first word.
        uint NTot ();               // Returns number of stored objects
        void Sort (CMPF cmpf);            // function for sort and compare
        const void *Retrieve (uint n);   // Get nth item, 0 based
        const void *Retrieve (const void *key, // key to be searched.
                              uint *n = NULL, // index for returned item.
                              CMPF bscmpf = NULL // Optional cmp function
                              );         // if not specified, the Sort one
                                        // is used and key must point to
                                        // a pointer to a full object !
                        // binary search, NULL if none
                        // MUST be preceded by Sort
        const void *LRetrieve (const void *key, // key to be searched.
                               CMPF cmpf,       // Compare function.
                               uint *n = NULL   // index for returned item.
                               );
                        // Linear search.
                        // NULL if not found.
                        // n is in order of storing, not affected
                        // by Sort.
        void Process (uint n,       // Item to be modified, 0 based
                      HSPF hspf,    // function to modify.
                      void *prm = NULL);    // parameters for hspf.
                      // the size of the item MUST NOT change !
        const void *GetFirst ();    // Get first/next item
        const void *GetNext ();     // in order of storing
};


                // Implementation in WordStore.Cpp

// Class to handle storage and retrieval of list of words in a string.
// Words are tokens that do not contain space or are enclosed in
// quotes.
// The returned words cannot exceed PATH_MAX size.

#define     WST_NoClear     0x0001      // Don't clear buffer.


class WordStore {
    private:
        char *storebuf;             // storage string
        char retbuf[PATH_MAX];     // Return buffer
        int  storesize,      // size of storage string
             usedsize;       // Currently used storage
        const char *nextp,   // Pointer to next item to return
                   *lastp;   // Pointer to last item returned
        int  sflags;
        void Init ();       // Internal use
    public:
        WordStore (char *buf,       // Buffer to be used
                   int size = 0,    // Size of string: 0   -> No Store.
                   int flags = 0);  //               : !=0 -> clear buffer.
        void Clear ();
        int Store (const char *tok); // returns 0 on success
        char *GetFirst ();          // NULL when not found
        char *GetNext ();           // Get Next; at start as GetFirst.
        void RemoveLast ();         // Removes last returned entry.
};


int AddWord (const char *src, char *dest, size_t totsize);
// Appends src to dest, adding a heading space is dest is not empty.
// Returns 1 if src is empty
//        -1 if totsize is not sufficient
//         0 on success

int filelength (int);


#endif

