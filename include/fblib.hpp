/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1995-1997  Alberto Pasquale                 */
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

#ifndef _FBlib_HPP_
#define _FBlib_HPP_

#include <stdlib.h>
#include <smapi/msgapi.h>
#include <apgenlib.hpp>

//#pragma pack (1)

#define FBBSLSIZE 1024      // size of files.bbs line buffer

class namelist;
class FBD;
class ANumLst;
struct AreaData;
struct FIDX;
struct _area;

typedef void (*DupeF) (FIDX *p, BOOL included);
typedef void (*ShowF) (char *AreaName, char *AreaPath, int Code);

                            // FileBase function, flags parameter

#define _UniqueDmpLine_     0x0001  // Change '\n' to ' ' in .DMP


                            // for ShowF, BuildArea, OpenArea
#define _Error_            -3
#define _BbsPathNotFound_  -2       // for files.*
#define _FilePathNotFound_ -1       // for area files
#define _OK_                0
#define _BegProc_           1    // Processing begun (normally)
#define _ReadOnly_          2
#define _WriteProtected_    3
#define _NoFilesBbs_        4

                // for Build action

#define _FBUpdate_       0x0001     // update main index, otherwise rebuild
#define _FBNoLocBase_    0x0002     // don't write local dat/dmp/idx
#define _FBMarkedOnly_   0x0004     // Build marked areas only, even with Rebuild
#define _FBAutoDate_     0x0008     // Date and Size must be read from dir (by default)
#define _FBNewAsMarked_  0x0010     // Consider areas with FILESBBS newer
                                    // than FILESIDX as marked. RO/CDROM skipped.
#define _FBSkipSlow_     0x0020     // Skip areas of type Slow (CD etc.) and
                                    // build all others !

                // for flag in AreaAddEntry


#define FF_FILE     0x01 /* Entry is a filename */
#define FF_NOTIME   0x10 /* Don't deduct file from user's time limit */
#define FF_NOBYTES  0x20 /* Don't deduce file from user's DL bytes limit */


struct FAH;


class FileBase {
    private:
        namelist *AreaNameList;
        namelist *AreaPathList;
        FBD *fbd;
        AreaData *ad;

        char cont;          // continuation character
        int cpos;           // spaces before cont char
        word flags;         // special flags

        BOOL AreaNameMatch (const char *areaname);
        BOOL AreaPathMatch (const char *areapath);
        int DmpWrite (const char *text, dword *ofs);
            // Writes text to files.dmp, stores offset into ofs.
            // returns 0 on success
        int BuildArea (word areanum, const char *filepath, const char *filesbbs,
                       const char *acs, word fattr, sword date_style,
                       BOOL NoLocBase = FALSE);
            // Builds files.dat .dmp .idx, stores data for maxfiles.idx
            // NoLocBase can be passed along to AreaOpen
            // returns _OK_, _BbsPathNotFound_, _FilePathNotFound_,
            //         _ReadOnly_, _WriteProtected_, _Error_
        BOOL IsBuild (const FAH *fahp, word action);
            // return TRUE if area must be built.

    public:
        FileBase (char cont = ' ',  // cont. character
                  int cpos = 31,    // cont. column, -1 to disable
                  word flags = 0    // Special flags.
                  );
        ~FileBase (void);
        int AreaOpen (word areanum, const char *filesbbs,
                      BOOL NoLocBase = FALSE);
                    // areanum: number of area in area.dat
                    // filesbbs: "<path>\<name>" (can have an extension)
                    // NoLocBase: Do Not Build local DAT/DMP/IDX
                    // Opens files.dat and files.dmp and prepares for addentry
                    // returns _OK_, _BbsPathNotFound_, _ReadOnly_,
                    //         _WriteProtected_, _Error_
        int AreaAddEntry (const char *filename, word flag,
                          const _stamp *cre_date, const _stamp *mod_date,
                          dword size, const char *desc, const char *acs,
                          const char *path = NULL);
                    // Appends files.dat and files.dmp
                    // Stores file data for later index build
                    // cre_date is the date of creation (upload)
                    // mod_date is the date of last write (dir)
                    // path optionally indicates an override path
                    // Returns 0 on success
        int AreaClose (void);
                    // Closes files.dat and files.dmp
                    // Builds the current area index from stored data
                    // Returns 0 on success
        void AddName (const char *areaname, const char *AreaDat = NULL);
                     // area tag to be rebuilt
                    // AreaDat != NULL -> add the upload path of areaname
        void AddPath (const char *areapath);  // area path to be rebuilt
        int Build (const char *AreaDat, word action, const char *FullIdx,
                   const char *NoDupeIdx = NULL, ShowF sf = NULL,
                   DupeF df = NULL);
          // NoDupeIdx is the full pathname for the file-request, no dupes,
          // Index file. In case of dupes, it contains the file in the
          // lowest area number.
          // df is a function called for each Dupe found while building NoDupeIdx
          // sf is a function called for each area processed
          // AreaDat && update  -> Build marked areas, update main index
          // AreaDat && !update -> Build all areas, rebuild main index
          // AreaDat,!update,MarkedOnly -> Build marked areas, rebuild index
          // Stored data is always used in addition to the built areas.
          // !AreaDat -> use stored data only, rebuild/update main index
          // returns 0 on success
};




#endif
