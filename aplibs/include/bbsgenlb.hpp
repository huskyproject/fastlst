/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1991-1997  Alberto Pasquale                 */
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

// BbsGenLb.Hpp 101

#ifndef BBSGENLB_HPP
#define BBSGENLB_HPP


#include <stdio.h>
#include <time.h>


#if defined (__OS2__)
  #include <os2def.h>
#elif defined (__NT__)
  #include <windows.h>
#else
  #define TRUE 1
  #define FALSE 0
  typedef int BOOL;
#endif

#include <smapi/msgapi.h>

#include <newarea.h>
#include <limits.h>

#pragma pack (1)

typedef NETADDR ADR;


            // Implementation in Fview.Cpp


typedef void (*VShow) (char *strfmt, ...);

int fview (char *filename, VShow vshow);

// filename is an archive full filename
// sc is a function that must be called to output
//    each and every line of the content
// returns 0 on success


        // Implementation in StrLst.Cpp

        // for EchoToss.Log or FileChng.Log
        // Generates a file with a list of strings

        // Use Add for each string to be listed.
        // Use Close when completed.
        // The OutFile will be deleted if Append = FALSE
        // and no Add done.

        // Nothing will be output if no default OutName and no OutNameCfg
        // is found.

class StrLst {
    private:
        char *OutNameCfg;
        char *OutName;  // output file name
        BOOL Append;
        FILE *Outf;
    public:
        StrLst (char *OutNameCfg, BOOL Append = FALSE, char *OutName = NULL);
            // OutNameCfg is the name of the config statement
            //   that specifies the name of the output file.
            // Append: TRUE if the output must be appended.
            //   When Append == FALSE, a '+' sign as the first
            //   character of the outfilename will set
            //   Append = TRUE anyway.
            // OutName specifies a default for OutName, in the case
            //   no OutNameCfg is encountered.
        ~StrLst (void);
        void SetDefCfg (char *OutName);
            // Sets default values, in the case cfg statements are not
            // found. Can be issued any time before Add.
            // Useless if already set in StrLst constructor.
        BOOL ParseCfg (const char *clnline);
            // Parses clnline in search for a cfg statement
            // clnline must be a line with:
            //  - comments removed
            //  - no terminating spaces
            //  - no terminating newline
            //  - no tabs
            // returns TRUE when cfg line recognized
        int Add (const char *tag);
            // Adds tag to the output list
            // returns 0 on success
        int Close (void);
            // returns 0 on success
};


            // Implementation in OutWrap.Cpp

// To output with left/right justification: e.g. for descriptions

// The cfg statement "<WrapCfg> [<indent> [<right>]]" allows to specify
// the indentation for 2nd and up lines and the maximum right column.
// When the statement is not found, wrap is disabled.
// <indent> defaults to 0, <right> to 79.

// Newlines '\n' in source are recognized as line separators.

typedef int (*OWShow) (const char *line, void *prm);

// Is called for each line to output.
// line does not contain trailing newline.
// prm can be used to pass some parameter.
// Must return 0 on success.

#define OWcfgFound     0        // for ParseCfg
#define OWcfgNotFound  1
#define OWcfgError    -1
#define OWcfgDupe     -2


class OutWrap {
    private:
        char *WrapCfg;      // name of the cfg statement for wrap specs
        BOOL CfgDone;       // Cfg statement already found
        int wrapcol,        // blank columns at left (-1 = no wrapping)
            wrapend;        // tot # of columns
        char *LineBuf;      // buffer for line to be output
        int fwrap (const char *src, int begin, OWShow ows, void *prm);
                             // return bytes written from b, EOF on error

    public:
        OutWrap (const char *WrapCfg);
        ~OutWrap ();
        int ParseCfg (const char *clnline);
            // Parses clnline in search for a cfg statement
            // clnline must be a line with:
            //  - comments removed
            //  - no terminating spaces
            //  - no terminating newline
            //  - no tabs
            // returns OWcfgFound on statement found
            //         OWcfgNotFound on statement not found
            //         OWcfgError on error in statement
            //         OWcfgDupe on dupe statement
        int Out (const char *source, OWShow ows, void *prm = NULL);
            // source is the string to be output, no trailing newline
            // ows is a pointer to a function of type OWShow.
            // prm is pointer to optional parameters.
            // Returns 0 on success.
};

                        // Implementation in SqTagPat.Cpp

// Class to convert a Squish TAG to the path and type of area.

struct _SqTagSearch {
    const char *Tag;      // pointer to area TAG in cfg data (input)
    char **Path;    // pointer to path pointer (must be allocated and assigned)
    word *Type;     // pointer to area type in cfg data (to be assigned)
    ADR  *adr;      // pointer to adr in cfg data or NULL
    char **origin;  // pointer to origin pointer or NULL
    dword *attr;    // pointer to attributes or NULL
    _SqTagSearch *next;
};


typedef void (*CharPVoid) (const char *);  // to be invoked for each Tag
                                     // that is not found !

class SqTag2Path {
    private:
        _SqTagSearch *sqhead;
        _SqTagSearch **sqtail;
        ADR *primary;
        int parsesq (char *linebuf, char *echotag, char *path, word *type,
                         ADR *adr);
    public:
        SqTag2Path ();
        ~SqTag2Path ();
        void AddTag (const char *Tag, char **Path, word *Type, ADR *adr = NULL,
                     char **origin = NULL, dword *attr = NULL);
                // Tag: input tag
                // Path: pointer to char * to be assigned;
                //       *Path must be NULL !!!!
                // Type: pointer to word to be assigned;
                //   specifies Squish/SDM and ECHO.
                // Path and Type will be assigned by ParseSquish.
                // *adr is filled with the primary address found in
                //   SquishCfg (including -p<adr> override)
                // *origin is set NULL for netmail areas
                // *attr is set to P for netmail areas;
                //   The setting of the Local flag is up to the application.
        int ParseSquishCfg (char *SquishCfg, CharPVoid sqtnf = NULL);
                // Squish's "Include" Statement supported
                // 0 on SquishCfg successfully accessed.
                // -1 on SquishCfg not found
                // -2 on Error in SquishCfg (no Address)
};
                        

                        // Implementation in Misc.Cpp

                            // for flags, bitwise
#define MSC_SrcMov          0x01    // advance input char ptr
#define MSC_Allow_NoLevel   0x02    // Allow "/<keys>", w no <level>


#define MSC_AcsStrSize      39      // space for <level>/<keys>


int strto4Dadr (const char *&adrs, ADR *adr);

// adrs is a pointer to a 4D address string that must not necessarily
//   be terminated just after the address specification;
//   spaces and tabs are allowed before the address; space, tab, newline,
//   @ are allowed after the address.
// adr is a pointer to a 4D address.
// Returns the number of characters parsed, space included and optionally
// advances adrs to first non-space character (MSC_SrcMov flag).
// Returns -1 on illegal or not full address (*adr unchanged, adrs not moved).


bool eq4Dadr (const ADR *adr1, const ADR *adr2);

// returns true if *adr1==*adr2


int GetLevKey (const char *&src, word *level, dword *keys = NULL, byte flags = 0);

// Gets a <level>[/<keys>] combination.
// src points to a string, with possible leading blanks.
// level will contain the found numerical level.
// keys, if !NULL, will contain the found keys (1..8,A..X),
// case is not significant, keys MUST be !NULL if /<keys> is present.
// Following space is skipped.
// Returns the number of characters parsed, space included and optionally
// advances src to first non-space character (MSC_SrcMov flag).
// If MSC_Allow_NoLevel, "/<keys>" is legal and level assumed USHRT_MAX.
// Returns -1 if no valid entry found (src not moved).

void PrintLevKey (char *buffer, word level, dword keys);

// Prints "<level>[/<keys>]" in buffer.
// Buffer must be MSC_AcsStrSize.


BOOL TagMatch (const char *Tag, const char *WildTag);

// Compares Tag with the WildTag (which can contain OS/2 wildcards).
// Special tags beginning with '<' must get an exact match (no wildcards).


char Ibm2Ascii (char c);

// Changes codes 0-255 to 0-127


                // Implementation in AreaDat.Cpp

        // class to handle Max 3.00 FAREA.DAT

#define FAD_OVR_MAX   100     // max number of command overrides per area
#define FAD_HEAP_SIZE 0x4000  // max size of string  heap
                              // In case of excess, NextArea returns NULL

                              // For NextArea
#define FAD_Normal    0x0000  // Get all entries, BegDiv/EndDiv included
#define FAD_AreasOnly 0x0001  // Skip BegDiv/EndDiv entries


// WARNING: the returned fahp and filesbbs ptr points to "STATIC" areas !
// When a new area is loaded, the previous fahp/filesbbs info is overwritten.


class FAREADAT {
    private:
        char *fareaname;
        FILE *f;
        FAH *fahp;
        FAH *LoadArea ();
        word arean;
        char *filesbbsbuf, *filesbbsptr;
    public:
        FAREADAT ();
        ~FAREADAT ();
        int OpenFAreaDat (const char *FAreaDat);
                                           // prepare to operate on FAreaDat
                                           // return 0 on success
                                           // -1: can't open, -2: short
                                           // -3: ID mismatch
                                           // FAreaDat: .DAT optional
        FAH *NextArea (int act = FAD_Normal);
                                    // return PFAH for next area, NULL if none
        FAH *Area (word num);
                // return PFAH for area #num, NULL if none or division
        FAH *Area (const char *name);
                // return PFAH for area tag "name", NULL if none or division
        word AreaNum ();        // return current areanum
        char *filesbbs ();      // return full name of filesbbs for current area
};


                // Implementation in MaxPrm.Cpp

#include <prm.h>

#define MaxPRM(p,s) (p->prmheap+(p->prm->s))

class MAXPRM {
    private:
        byte *buffer;
    public:
        m_pointers *prm;
        char *prmheap;
        MAXPRM ();
        ~MAXPRM ();
        int Read (char *prmname);   // returns 0 on success
                                    // prmname can have or not .PRM ext
};


            // Implementation in MaxAcs.Cpp

class MAXACS {
    private:
        byte *buffer;           // pointer to allocated array
        char *heap;             // pointer to string heap
        word usn;               // number of user classes
        word ussize;            // size of each class record
        byte *uscl1;            // pointer to first user class record

        int GetGenAcs (char *lks, word *level, dword *keys1, dword *keys0 = NULL);
                                    // Gets level and keys1/keys0 from lks.
                                    // keys0 are the negated keys.
                                    // lks is <level>[/<keys>]
                                    // If keys0 is NULL, error is returned
                                    // in the case of negated '!' keys.
                                    // If keys1 is NULL, keys are not checked.
                                    // Returns 0 on success.
        BOOL TokAcs (char *TokAcs, word level, dword keys);
                                    // TokAcs is a single access string
                                    // (no & | operators).
                                    // returns TRUE if level/keys grants
                                    // access to TokAcs
        BOOL ProdAccess (char *pac, word level, dword keys);
                                    // pac is a series of access strings
                                    // separated by the '&' operator.
                                    // No '|' operator permitted.
        int GetLevel (char *slevel, word *level);
                                    // looks for slevel in access.dat
                                    // and stores the numeric value in level
                                    // returns 0 on success
    public:
        MAXACS ();
        ~MAXACS ();
        int Read (char *acsname);   // returns 0 on success
                                    // acsname can have or not .DAT ext
        int GetAcs (char *ACS, word *level, dword *keys = NULL);
                                    // Gets level and keys for a
                                    // user access string.
                                    // If keys is NULL, keys are not
                                    // checked.
                                    // returns 0 on success
        char *LevName (word level); // returns pointer to class name
                                    // or NULL if level is not equal
                                    // to a defined class level.
        char *LevStr (word level);  // returns pointer to class name
                                    // or to static string with numeric
                                    // level value.
        BOOL HaveAccess (char *ACS, word level, dword keys);
                                    // returns TRUE if level and keys
                                    // give access to an object of ACS.
                                    // "name=<s>" and "alias=<s>"
                                    // do not give access.
                                    // | and & permitted
};


                    // Implementation in ComprCfg.Cpp


class AH_Archiver {
  private:
    int  identofs;      // offset of identity string; -2 = last char
    int  identlen;      // length of identity string
    byte *identstr;     // identity string (can contain 00)
    char *addcmd;       // command to add files (with %a %f)
    char *extcmd;       // command to extract files (with %a %f)
    const class AH_Archiver *prev;  // pointer to previous Archiver definition
  public:
    char *name;         // name of archiver
    char *ext;          // typical extension
    AH_Archiver (const class AH_Archiver *aptr);  // constructor

    friend class AH_ComprCfg;
};

                                // for AH_VShow
#define AH_MT_Action    0
#define AH_MT_Info      1
#define AH_MT_Warning   2
#define AH_MT_Error     3

typedef void (*AH_VShow) (byte msgtype, char *strfmt, ...);
// function type to be used for output


class AH_ComprCfg {
  private:
    static AH_VShow outf;
    static void RcShow (int code, ...); // for RunCmd

    AH_Archiver *lastarc;  // pointer to last Archiver definition
    void ScanIdent (const char *tok);  // Scans Ident string
    BOOL ArcGood (const AH_Archiver *a); // is a good for sfx detection ?
    const AH_Archiver *SfxNext (const AH_Archiver *a); // finds next arc good for sfx
    const AH_Archiver *ChkSfx (const char *filename);  // Checks if Sfx

  public:
    AH_ComprCfg (const char *cfgfile,       // compress.cfg file
                 AH_VShow outf = NULL);     // output function pointer
                                            // is stored as static !
    int UnArc (const char *filename,        // archive name
               const char *extract);        // extract template
                                // returns errorlevel
                                // -1 on error
                                // -2 unknown archiver
                                // -3 file not found
    int Arc (const AH_Archiver *a,          // archiver to be used
             const char *filename,          // archive name
             const char *add);              // template for add
                                // returns errorlevel or -1 on error
    const AH_Archiver *AddDefined (const char *method); // is (compr) method defined in cfg ?
    const AH_Archiver *ExtDefined (const char *ext); // is extension defined in compress.cfg ?
};


            // Implementation in Fbbs.Cpp

// The description is always null terminated and with no terminating '\n'.
// The '\n' character is used to separate multiple lines.
// It's possible that a '\n' is present at the end of the description if
// the description terminates with an empty line.

// Comment line if:
// - empty
// - starting ctrl characters
// - starting space
// - starting '-' and then a space or tab
// - starting high ascii (>127)


typedef BOOL (*SkipFile) (const char *file, void *ptr);

// Function type for function to be used by GetEntry/GetGenEntry to
// establish whether the file entry must be removed or not.


            // FBBS.fbbsflags

#define FBBS_FLAG_NODATESIZE    0x8000  // date and size NOT used
#define FBBS_FLAG_NOCONTSPACE   0x4000  // no space after cont. char

#define FBBS_FLAG_DATEFORMAT    0x0003  // mask for date format
#define FBBS_FLAG_DATE_USA      0x0000
#define FBBS_FLAG_DATE_EURO     0x0001
#define FBBS_FLAG_DATE_JAPAN    0x0002
#define FBBS_FLAG_DATE_SCIENT   0x0003


// ATTENTION: when date&size is used,
// the functions that read the files.bbs use the first two tokens
// after the filename as date and size !!!

// When FBBS_FLAG_NODATESIZE -> date = 0, size = ULONG_MAX.


            // for GetEntry first

#define FBBS_GE_FIRST   TRUE
#define FBBS_GE_NEXT    FALSE

            // for GetDesc action (bitwise)

#define FBBS_REMOVE     0x01

            // flag, besides FB.H

#define FF_SAFE         0x8000      // no Trojan check to be performed
                                    // Input for PutGenEntry/SetDesc


class FBBS {
    private:
        char filesbbs[PATH_MAX];   // full name of file list
        int filesize,               // size of buffer for returned file
            descsize,               // size of buffer for returned description
            linesize;               // size of files.bbs line buffer
        char cont;                  // continuation char
        int cpos;                   // column for cont. char, zero based, -1 for disable
        word fbbsflags;             // flags

        SkipFile sf;                // Function pointer: if !NULL, Get(Gen)Entry
                                    // removes entry if sf is TRUE.
        void *ptr;                  // To be passed to ptr.

        FILE *t;                    // Stream used by GetGenEntry when sf.
                                    // NULL when closed.
        FILE *f;                    // stream for filesbbs, NULL when closed.
        int fileh;                  // write handle for filesbbs, -1 when closed.

        char *line;                 // Pointer to line buffer.

        BOOL repeatln;              // True when line must be got by
                                    // Fgetln again.
        char *lastget;              // result of last Fgetln
        char *Fgetln ();  // Loads line with next line of text (as fgets),
                          // returns line or NULL if EOF.
        int descopy (const char *p = NULL,    // points after filename
                     char *desc = NULL, // where to copy description
                     word *flag = NULL, // where to store flags
                     time_t *date = NULL,  // optional date 
                     dword *size = NULL,   // optional size 
                     FILE *t = NULL     // where to copy the lines
                     );
                  // copies description (even multi-line) for file
                  // present in current line.
                  // returns 0 on success
        int descAddLine (const char *p,       // pointer to begin of desc line
                         char *desc,    // pointer to return desc buffer
                         int desclen);  // current length of desc
                      // Appends a new line, preceded by '\n'
                      // Returns the new length of desc
                      // Does not exceed descsize
        void RdClose (); // close f.
        int WrOpen ();  // prepare fileh for append. 0 on success.


    public:
        FBBS (const char *path, // path must terminate by '\' (files.bbs assumed)
                          // or contain the full filelist name.
              int filesize, // size of buffer for returned file
              int descsize, // size of buffer for returned description
                            // (maybe multi-line) or comment (1 line).
              int linesize = 1024, // size of files.bbs line buffer
              char cont = ' ', // continuation character
              int cpos = 31,   // Column where to put the cont char., 0..78,
                          // if cont == ' ' the range is 1..78
                          // If cpos == -1, the multi-line description
                          // handling is disabled.
                          // While reading, cpos is not used if cont != ' ',
                          // otherwise the continued description line MUST
                          // start at pos (possibly with heading space).
                          // While writing, if cont != ' ' the following is
                          // written starting at cpos: cont, ' ', desc;
                          // otherwise desc is written starting at cpos.
                          // While writing with cpos == -1, only the
                          // first line of a multi-line description
                          // is output, following ones are ignored.
              word fbbsflags = FBBS_FLAG_NODATESIZE, // default flags
              SkipFile sf = NULL,   // optional function pointer to remove
                                    // some entries from files.bbs with
                                    // Get(Gen)Entry.
              void *ptr = NULL      // To be passed to sf.
                        );
        ~FBBS ();           
        int GetDesc (const char *file,    // file to be searched for
                     char *desc = NULL,  // destination buffer
                     word *flag = NULL, // optional flags (/t /b)
                     byte action = 0, //
                     const char *repl = NULL, // Optional additional file(s)
                                        // which description must be removed
                                        // *repl == '\0' is acceptable.
                                        // If equal to file, it's ignored.
                                        // In the case of multiple file names,
                                        // they are separated by space.
                     time_t *date = NULL,  // optional date
                     dword *size = NULL    // optional size 
                     );
                     // the description is returned into desc,
                     // it's always zero terminated
                     // it can contain '\n' in case of multiple lines.
                     // if FBBS_REMOVE, the description for file is
                     // removed from the files.bbs
                     // Return: -2 file list not found
                     //         -1 on error
                     //         0  All Ok
                     //         1  file entry not found
                     //         2  NO repl entry found
                     //         3  both entries not found
                     //
                     // If file not found:
                     // *flag = 0, *date = 0, *size = ULONG_MAX

        int SetDesc (const char *file,          // file to be appended to files.bbs
                     const char *desc,          // description, null terminated
                     word flag = 0,       // optional flags (/t /b)
                     const char *befdesc = NULL,// optional header
                     time_t date = 0,  // optional date for FA_LISTDATE
                     dword size = 0    // optional size for FA_LISTDATE
                     );     // the desc is appended, no check for already
                            // existent entry (you must use GetDesc).
                            // desc can be multi-line ('\n' as separator).
                            // The continuation character and position
                            // specified in the constructor are used.
                            // The appended line does not exceed linesize.
                            // If file is empty or NULL, desc is output
                            // as a comment line.
                            // Returns 0 on success
            // Trojan (control) characters are substituted with space
            // (in the original desc buffer)

        int PutGenEntry (const char *file,
                         const char *desc,
                         word flag = 0,
                         const char *befdesc = NULL,
                         time_t date = 0,
                         dword size = 0);
                         // as SetDesc, but file is not closed.
                         // Suitable for multiple appends.

        int WrClose (); // close fileh. 0 on success.
                        // Not really necessary, but useful to check
                        // for write errors after PutGenEntry.

        int RdOpen ();  // prepare f for reading from begin. 0 on success.
                        // Usually not necessary, but useful to restart
                        // f from begin when you want to use Get(Gen)Entry
                        // with no FBBS_GE_FIRST and f is already open.

        int GetEntry (char *file,           // file name returned
                      char *desc,           // description returned
                      word *flag = NULL,    // optional flags returned
                      BOOL first = FBBS_GE_NEXT,   // get first/next
                            // next is valid also after GetDesc with
                            // repl==NULL and no REMOVE action
                      time_t *date = NULL,  // optional date for FA_LISTDATE
                      dword *size = NULL    // optional size for FA_LISTDATE
                      );        // returns 
                                //        -1 error
                                //         0 while entry is available
                                //         1 no more entries
                                //         3 on entry removed

                      // If file not found:
                      // *flag = 0, *date = 0, *size = ULONG_MAX

                      // If sf is passed to FBBS, GetEntry must be invoked
                      // until no more entries are available, otherwise
                      // the files.bbs remains unchanged.
                      // If GetEntry restarts from first, the status
                      // is reset.
                      // If first = FBBS_GE_NEXT and f is not open,
                      // then a FBBS_GE_FIRST is assumed.

        int GetGenEntry (char *file,   // file name returned, empty if comment
                      char *desc,      // description or comment line returned
                      word *flag = NULL,    // optional flags returned
                      BOOL first = FBBS_GE_NEXT,   // get first/next
                            // next is valid also after GetDesc with
                            // repl==NULL and no REMOVE action
                      time_t *date = NULL,  // optional date for FA_LISTDATE
                      dword *size = NULL    // optional size for FA_LISTDATE
                      );        // returns 
                                //        -1 error
                                //         0 on file entry
                                //         1 no more entries
                                //         2 on comment line
                                //         3 on entry removed
                    // same as GetEntry but also returns comment lines:
                    // 2 as return value, empty file and comment in desc.

        int Trunc ();   // Truncates f, 0 on success.
};


word Max2FbbsDateStyle (    // converts date_style to FBBS_FLAG*
    sword date_style       // as in max.prm
    );


                // Implementation in SqNetScn.Cpp

// Class to handle scan of a a netmail area via Squish MSGAPI.
// In the case of Squish base, if sav is given, a pointer to the last
// scanned message is saved in "area.<sav>" and used to scan new
// messages only.
// The area is Locked.

// It is assumed that the MsgApi are Opened and Closed externally.

#define SQNS_Get_SkipRead   0x01        // Skip msgs marked as read


class SqNetScan {

    private:
        char savfilename[PATH_MAX];    // name of the file for
                                        // stored data (last msg ptr).
                                        // Empty when none applicable
                                        // (SDM or sav not given).
        word areatype;                  // MSGTYPE_SDM or MSGTYPE_SQUISH
        word areazone;                  // Primary (default) zone.
        HAREA harea;                    // Handle of area, NULL when none.
        UMSGID hwmID;                   // ID of last scanned message.
        UMSGID highID;                  // ID of last message to be scanned.
        HMSG hmsg;                      // current msg handle (NULL when none).

    public:

        SqNetScan ();               // Constructor.

        HAREA Open (const char *path, // path of netmail area.
                                    // Backslash terminated when SDM.
                  word type,        // MSGTYPE_SDM or MSGTYPE_SQUISH.
                  word defzone = 0, // Zone to be assumed.
                  const char *sav = NULL);  // Extension (no .) for the
                                    // file where the pointer to the last
                                    // scanned message is stored.
                                    // Return the area Handle,
                                    // NULL on error.

        UMSGID GetNextMsg (XMSG *xmsg,  // Get UMSGID of next available
                           byte flags = 0); // message.
                                        // Load the header into xmsg.
                                        // Optional flags.
                                        // Return 0 when no more.

        void LoadMsgBody (char *body,   // Load message body into body,
                          size_t size); // maximum size bytes, including
                                        // terminating NULL, which is
                                        // guaranteed.

        int MarkMsgRead ();             // Mark current msg as Read and
                                        // leave it closed (no LoadMsgBody
                                        // possible).
                                        // Return 0 on success.

        int KillMsg ();                 // Kill current msg.
                                        // Return 0 on success.

        int Close ();                   // Close the area, save the last
                                        // pointer if necessary.
                                        // Return 0 on success,
                                        // 1 on "sav" save error,
                                        // -1 on Api close error.

        ~SqNetScan ();                  // Destructor.
};



#endif
