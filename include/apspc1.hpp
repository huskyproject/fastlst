/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1996-1998  Alberto Pasquale                 */
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

#ifndef _APSPC1_HPP_
#define _APSPC1_HPP_

#include <typedefs.h>

//#pragma pack (1)


            // implementation in Feature.Cpp

#ifdef __OS2__

        // class to add support for a Feature DLL

//        The ParseCfg function waits for:
//
//        FeatureLoad <dllrootname>
//        Feature <cfgline>
//
//        where <ddlrootname> is the name (w/o .Dll ext) of the
//        DLL to be loaded;
//        <cfgline> is a configuraton line to be passed to the DLL.


#define FeatureFound     0        // for ParseCfg
#define FeatureNotFound  1
#define FeatureError    -1


                // for dest of FeatOut, bitwise

#define FO_LOG  0x01 // send output to the log
#define FO_CRT  0x02 // send output to the screen
#define FO_BOTH 0x03 // send output to both log and screen

typedef VOID (* APIENTRY FeatOut) (UCHAR dest, CHAR prty, PCSZ format, ...);

// Function to write output to screen and/or log.
// prty is the log priority char, usually one of "!*#: ".
// prty is ignored for output to screen.
// format and ... are the formatting string and parameters as in printf.
// Heading and trailing \n are ignored for log entry.


#define _MAXFEATDLL_    25      // Maximum number of DLL that can be loaded.
#define _FEATFUNCSIZE_  30      // size of DLL function names.


class Feature {
    private:
        FeatOut prnf;       // function for log output
        HMODULE hmod[_MAXFEATDLL_];
        int hmodn;          // number of Modules loaded
        int hmodi;          // For use by GetFunc
        char gfname[_FEATFUNCSIZE_];   // for use by GetFunc
    public:
        Feature (FeatOut featprintf // Function for log output
                 );
        ~Feature ();
        int ParseCfg (const char *clnline);
            // Parses clnline in search for a cfg statement
            // clnline must be a line with:
            //  - comments removed
            //  - no terminating spaces
            //  - no terminating newline
            //  - no tabs
            // returns FeatureFound on statement found
            //         FeatureNotFound on statement not found
            //         FeatureError on error in statement
        void *GetFunc (const char *funcname = NULL);
            // returns a pointer to a function of name "funcname"
            // in the loaded modules.
            // Returns NULL when no more functions are available.
            // Please note that for each name there can be one function
            // per module.
            // The first time the funcname must be supplied,
            // then it must be NULL.
};


#endif  // __OS2__


            // implemented in Misc.Cpp

struct dosdate_t *Easter (struct dosdate_t *date);

// date->year must contain the year number as input
// The structure will be filled in with the date of Easter
// On success, a pointer to date is returned.
// On error, NULL is returned.
// Valid years: 1583->9005
// Easter is between 22 March -> 25 Apr


            // implemented in File.Cpp


// filename CANNOT have <drive:><path>

int MkFATname (psz filename);  // changes filename so that it's FAT legal

#ifdef __OS2__
  int MkHPFSname (psz filename); // changes filename so that it's HPFS legal
#endif

// 0: filename unchanged



        // copyfile2

// srcname and destname are filename specifications with no wildcards.
// destination path tree must exist.


// Under OS/2:
//
// If destname is on FAT:
// 1 - dest dir is read, considering .LONGNAME extended attributes
// 2 - if a file of same name exists, it is overwritten, otherwise
// 3 - a suitable filename is found; it is:
// 4   - adjusted (underscore in place of illegal characters)
// 5   - truncated (to 8.3)
// 6   - made unique (if exists, last two chars become 00, 01, 02 ... FF
// 7 - the file is copied
// 8 - if the name was modified, the original is stored in .LONGNAME
//
// If destname is on LAN w EA support:
// same as on FAT but with additional step after 2.
// 2.1 - copy is attempted: if copy successful, done; otherwise:
//
// When the destination file is modified, destname is changed accordingly.
//
// WARNING: destname must be _MAX_PATH long.


#define CF2_FL_NONE        0x00
#define CF2_FL_MOVE        0x01       // move the file (kill source)
#define CF2_FL_EXISTFAIL   0x02       // fail if destination exists


          // normal conditions and non-fatal errors (bit wise)

#define CF2_OK          0x0000  // OK (normal)
#define CF2_TRUNCATED   0x0100  // OK, destname contains truncated filename

#define CF2_SRCKILLERR  0x0200  // can't kill source
#define CF2_EAERROR     0x0400  // error dealing with EAs

             // fatal errors (exclusive), make result < 0

#define CF2_FATAL       0x00FF
#define CF2_NOSRC       0x0001 // Error: no srcname found
#define CF2_SRCEQDEST   0x0002 // srcname==destname
#define CF2_NONAMEAVAIL 0x0003 // no more available short names
#define CF2_NODESTPATH  0x0004 // destination drive/path not found
#define CF2_DESTEXISTS  0x0005 // destination already exists
#define CF2_ERROR       0x0006 // Other fatal error



int copyfile2 (pcsz srcname,     // source filename (no wildcards)
               psz destname,     // destination filename (no wildcards)
               byte flags = CF2_FL_NONE // flags
              );


#endif      // _APSPC1_HPP_
