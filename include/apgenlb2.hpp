/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1997       Alberto Pasquale                 */
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

#ifndef _APGENLB2_HPP_
#define _APGENLB2_HPP_

#include <defines.h>
#include <typedefs.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <limits.h>

//#pragma pack (1)

                    // Implemented in CfgFile.Cpp

// Class to handle a Configuration File

// comments after ';'
// tabs converted to blanks, trailing space removed
// ; and " must be enclosed in quotes (quotes doubled)
// e.g. "example; he said ""what ?"""
// Lines got via fgetln are already "cleaned"
//
// Environment variables are expanded.
// The variable names must be at least two character long.
// A '%' followed by 2 or more characters (different from ' ' and '%')
// indicates the start of the variable name.
// A '%' or a ' ' indicates the end of the variable name.
// To represent the '%' character, "%%" must be used.
// % followed by ONE character is reserved for parameter substitutions.


#define CFGLINESIZE 512         // Size of configuration Line


class CfgFile {
  private:
    FILE *f;
    char *lbuff, *token;
    const char *cleanln;            // First char of cleaned line
    const char *nextc;              // next char to scan
    uint l;                         // line number
    int lsize;                      // Line size, incl. term. null
    BOOL gotln, gottkn;  
    void Init ();
  public:
    CfgFile (int lsize = CFGLINESIZE);
    ~CfgFile ();
    int Open (pcsz filename);       // 0 on success, -1 error, 1 already open
    int Close ();                   // 0 on success
    pcsz GetLn ();                  // First non-space of Full cleaned line,
                                    // NULL on EOF
    pcsz ReGetLn ();                // As previous GetLn; no effect on GetToken
    pcsz GetToken ();               // 0 term. token, NULL if EOL
                                    // If there is a string between commas, the
                                    // entire string is got, quotes excluded.
                                    // Can return empty string.
    pcsz ReGetToken ();             // As previous GetToken
    pcsz TokenPtr ();               // start of next word, NULL if EOL
    pcsz RestOfLine ();             // start of next word, NullStr if EOL
    uint LineN ();                  // Line number
};


                    // Implemented in WriteLog.Cpp

// Class to handle a Log File

// By default the log file is kept open and flushed at each write
// first char of strfmt is "priority", as in logset;
// strfmt must NOT contain \r\n

// If setup not done or fname is empty, no log will be written on *write.


#define LF_NoChange         0x0000

#define LF_CloseAfterWrite  0x0001
#define LF_FlushOnRequest   0x0002

#define LF_PrintPrty        0x0004      // for aux output (usually stdout)
#define LF_PrintDate        0x0008      //
#define LF_PrintTime        0x0010      //
#define LF_PrintID          0x0020      //

#define LF_AllDefaults      0x8000      // To reset flags on 2nd setup


#define LF_LogSetSize       40
#define LF_DateSize         16


class LogFile {
  private:
    FILE *f;
    char fname[PATH_MAX];
    char ID[5];
    char logset[LF_LogSetSize];
    word flags;
    int open ();            // open the log file, 0 on success
    int PrintStart (FILE *f, char prty, pcsz date); // 0 on success
  public:
    LogFile ();
    ~LogFile ();
    void setup (        // can be used again, with open log
                pcsz fname,    // file name; NULL->don't change
                pcsz ID,       // 4 char application ID; NULL->don't change
                pcsz logset,   // messages to be logged, NULL->don't change
                word flags)    // LF_NoChange->don't change
               ;
    int close ();           // Make sure the log is closed, 0 on success
    int flush ();           // Make sure the log is flushed, 0 on success
    int va_write (psz date,         // external date buffer
                  pcsz strfmt,      // format string
                  va_list args)     // args
                 ;                  // 0 on success
    int vwrite (pcsz strfmt, ...);              // 0 on success
    int vwrite (FILE *f,            // stream for additional output, can be NULL
                pcsz strfmt, ...)   // format string and args
               ;                    // 0 on success
};


                    // Implementation in Misc.Cpp

#define SCANFT_BUFSIZE  256         // max length of scanft input

int scanft (pcsz format, time_t timeout, ...);
// as scanf but with timeout (seconds); EOF on timeout

int getcht (time_t timeout);
// as getch but with timeout (seconds); EOF on timeout


#endif

