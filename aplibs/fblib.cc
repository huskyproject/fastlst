/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1995-1997  Alberto Pasquale                 */
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
/*   How to contact the author:  Alberto Pasquale of 2:332/504@fidonet       */
/*                               Viale Verdi 106                             */
/*                               41100 Modena                                */
/*                               Italy                                       */
/*                                                                           */
/*****************************************************************************/

// FBlib.Cpp

// This is the main module of FBlib.LIB, which is a library for
// (selectively) building the Maximus (tm) filebase.

#include <bbsgenlb.hpp>
#include <apgenlib.hpp>

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "namelist.hpp"
#include "fbd.hpp"
#include "fb.hpp"
#include "fblib.hpp"


struct AreaData {
    word areanum;
    BOOL LocBase;       // FALSE if the area DAT/DMP/IDX must not be written
    char filesbbs[PATH_MAX];   // complete files.bbs name (with extension)
    FILE *fdat, *fdmp;
    word datpos;
    dword dmpofs;
    dword first;        // number of first idx entry in FBD data
};


char *strnupcpy (char *dest, const char *src, int n)
{
    for (int i = 0; i < n; i++) {
        *(dest ++) = (char) toupper (*src);
        if (*src)
            src ++;
    }
    return dest;
}


                    // for GetEntry flags
#define _GE_NRM_    0
#define _GE_NODIR_  1       // do not read dir to get size and date

static char *GetEntry (const char *filename, _dir *&entry, char *&path,
                       word flags = _GE_NRM_);

// gets data on filename, to be used for files that reside in a different dir.
// filename can include path information.
// entry and path must point to suitable buffers.
// On return path is changed to NULL if no path in filename;
// entry is changed to NULL if filename is offline.
// The return value is the fname (no path) and points to entry->fname,
// even if the file is offline and entry is changed to NULL.
// The return value is NULL in case of error.



static BOOL NewFBBS (const char *filepath, const char *filesbbs)
{
    char files[PATH_MAX];
    if (*filesbbs)
        strcpy (files, filesbbs);
    else
        sprintf (files, "%sfiles.bbs", filepath);

    time_t datebbs, dateidx;

    datebbs = DosFileTime (files);
    setext (files, ".IDX");
    dateidx = DosFileTime (files);
    if (dateidx == 0)
        return FALSE;

    return (datebbs > dateidx);
}


BOOL FileBase::AreaNameMatch (const char *areaname)
{
    char *name = AreaNameList->get (1);
    while (name) {
        if (fnamecmp (areaname, name) == 0)
            return TRUE;
        name = AreaNameList->get ();
    }
    return FALSE;
}


BOOL FileBase::AreaPathMatch (const char *areapath)
{
    char *path = AreaPathList->get (1);
    while (path) {
        if (eqpath (areapath, path))
            return TRUE;
        path = AreaPathList->get ();
    }
    return FALSE;
}


                // -----------------> public


FileBase::FileBase (char cont, int cpos, word flags)
{
    HardErrDisable ();

    this->cont = cont;
    this->cpos = cpos;
    this->flags = flags;

    AreaNameList = new namelist;
    AreaPathList = new namelist;
    fbd = new FBD;
    ad = new AreaData;
}


FileBase::~FileBase (void)
{
    delete ad;
    delete fbd;
    delete AreaPathList;
    delete AreaNameList;
}


int FileBase::AreaOpen (word areanum, const char *filesbbs, BOOL NoLocBase)
{
    char fullpath[PATH_MAX];

    strcpy (fullpath, filesbbs);
    char *p = strrchr (fullpath, DIRSEP); // remove filename
    if (!p)
        p = strchr (fullpath, ':');
    if (p)
        *(p+1) = '\0';
    else
        *fullpath = '\0';

    int fpstat = Exist (fullpath);
    if (!(fpstat & _ExDIR_))        // path for files.* not existent
        return _BbsPathNotFound_;

    ad->LocBase = !NoLocBase && (fpstat & _ExWRITE_); // DAT/DMP/IDX

    fbd->AddArea (areanum);
    ad->areanum = areanum;
    ad->datpos = 0;

    if (!ad->LocBase)
        if (NoLocBase)
            return _OK_;
        else
            return _ReadOnly_;

    strcpy (fullpath, filesbbs);
    setext (fullpath, ".DAT");

    ad->fdat = fopen (fullpath, "wb");
    if (ad->fdat == NULL) {     // must be write protected
        ad->LocBase = FALSE;
        return _WriteProtected_;
    }
    setvbuf (ad->fdat, NULL, _IOFBF, 8192);

    setext (fullpath, ".DMP");
    ad->fdmp = fopen (fullpath, "wb");
    if (ad->fdmp == NULL) {     // this is error
        fclose (ad->fdat);
        ad->LocBase = FALSE;
        return _Error_;
    }
    setvbuf (ad->fdmp, NULL, _IOFBF, 8192);

    if (fwrite ("\0\0\0", 3, 1, ad->fdmp) != 1) {
        fclose (ad->fdat);
        fclose (ad->fdmp);
        ad->LocBase = FALSE;
        return _Error_;
    }

    strcpy (ad->filesbbs, filesbbs);

    ad->dmpofs = 3;
    ad->first = fbd->GetCurn ();

    return _OK_;
}


static int WriteUniqueDmpLine (const char *text, FILE *f)   // 0 on success
{
    const char *p = text;
    while (*p) {
        int linelen = strcspn (p, "\n");     // output line
        if (linelen > 0) {
            if (fwrite (p, linelen, 1, f) != 1)
                return -1;
            p += linelen;
        }
        while (*p == '\n') {                    // change '\n' to ' '
            if (fputc (' ', f) == EOF)
                return -1;
            p ++;
        }
    }

    if (fputc ('\0', f) == EOF)         // NULL termination
        return -1;

    return 0;
}


int FileBase::DmpWrite (const char *text, dword *ofs)
{
    *ofs = ad->dmpofs;
    word dmpreclen = (word) (strlen (text) + 1);    // null terminated !
    if (fwrite (&dmpreclen, sizeof (dmpreclen), 1, ad->fdmp) != 1)
        return -1;
    if (flags & _UniqueDmpLine_) {
        if (WriteUniqueDmpLine (text, ad->fdmp))
            return -1;
    } else {
        if (fwrite (text, dmpreclen, 1, ad->fdmp) != 1)
            return -1;
    }
    ad->dmpofs += (dmpreclen + sizeof (dmpreclen));
    return 0;
}


int FileBase::AreaAddEntry (const char *filename, word flag,
                            const _stamp *cre_date, const _stamp *mod_date,
                            dword size, const char *desc, const char *acs,
                            const char *path)
{
    if (ad->LocBase) {
                        // write .dat
        FDAT fd;
        memset (&fd, 0, sizeof (fd));

        strnupcpy ((char *)fd.name, filename, MAX_FN_LEN);
        fd.struct_len = sizeof (fd);
        fd.flag = flag;
        fd.fdate.msg_st = *mod_date;    // actual filedate
        fd.udate.msg_st = *cre_date;    // upload filedate
        fd.fsize = size;
                                                // .DMP records
        if (DmpWrite (desc, &fd.desc))
            return -1;

        if (path) {
            if (DmpWrite (path, &fd.path))
                return -1;
        }

        if (DmpWrite (acs, &fd.acs))
            return -1;

        if (fwrite (&fd, sizeof (fd), 1, ad->fdat) != 1)    // .DAT record
            return -1;
    }

                    // store data for .idx

    fbd->Store (filename, ad->areanum, ad->datpos);

    ad->datpos ++;

    return 0;
}


int FileBase::AreaClose (void)
{
    int error = 0;

    if (ad->LocBase) {
        error |= fclose (ad->fdmp);
        error |= fclose (ad->fdat);

                        // build files.idx

        char fullpath[PATH_MAX];
        strcpy (fullpath, ad->filesbbs);
        setext (fullpath, ".IDX");
        
        error |= fbd->Build (ad->first, fullpath);
    }

    return error;
}


void FileBase::AddName (const char *areaname, const char *AreaDat)
{
    if (!AreaDat) {
        AreaNameList->add (areaname);
        return;
    }

    // if AreaDat -> add the Upload path !

    FAREADAT fadat;

    if (fadat.OpenFAreaDat (AreaDat))
        return;

    FAH *fap;
    BOOL WildName = (strpbrk (areaname, "?*") != NULL);
    while ((fap = fadat.NextArea (FAD_AreasOnly)) != NULL) {
        if (fnamecmp (PFAS (fap, name), areaname) == 0) {
            if (*PFAS (fap, uppath) != '\0')
                AreaPathList->add (PFAS (fap, uppath));
            if (!WildName)
                break;
        }
    }
}


void FileBase::AddPath (const char *areapath)
{
    AreaPathList->add (areapath);
}


BOOL FileBase::IsBuild (const FAH *fahp, word action)
{
    if (action & _FBSkipSlow_)
        return !(fahp->fa.attribs & FA_SLOW);

    if (!(action & _FBUpdate_) && !(action & _FBMarkedOnly_))
        return TRUE;

    if (action & _FBNewAsMarked_)
        if (!(fahp->fa.attribs & FA_NONEW))
            if (NewFBBS (PFAS (fahp, downpath), PFAS (fahp, filesbbs)))
                return TRUE;

    if (AreaNameMatch (PFAS (fahp, name)) ||
        AreaPathMatch (PFAS (fahp, downpath)))
            return TRUE;

    return FALSE;
}


static word GetAttribs (word attribs, word action)
{
    if ((attribs & (FA_AUTODATE|FA_MANDATE|FA_LISTDATE)) == 0)
        if (action & _FBAutoDate_)
            attribs |= FA_AUTODATE;
        else
            attribs |= FA_MANDATE;

    return attribs;
}


int FileBase::Build (const char *AreaDat, word action, const char *FullIdx,
                     const char *NoDupeIdx, ShowF sf, DupeF df)
{
    int ret = 0;

    if (AreaDat) {
        FAREADAT fadat;
        if (fadat.OpenFAreaDat (AreaDat) != 0)
            return -1;

        FAH *fahp;                              // Build marked areas
        while ((fahp = fadat.NextArea (FAD_AreasOnly)) != NULL) {
            if (fahp->fa.attribs & FA_NOINDEX)
                continue;
            if (IsBuild (fahp, action)) {
                if (sf)
                    sf (PFAS (fahp, name), PFAS (fahp, downpath), _BegProc_);
                word attribs = GetAttribs (fahp->fa.attribs, action);
                int AStat = BuildArea (fadat.AreaNum (),
                                       PFAS (fahp, downpath),
                                       PFAS (fahp, filesbbs),
                                       PFAS (fahp, acs),
                                       attribs,
                                       fahp->fa.date_style,
                                       action & _FBNoLocBase_);
                if (sf)
                    sf (PFAS (fahp, name), PFAS (fahp, downpath), AStat);
                if (AStat == _Error_)
                    ret = -1;
            }
        }
    }

    ret |= fbd->Build ((action & _FBUpdate_) ? FBD_Update : FBD_BuildAll,
                        FullIdx, NoDupeIdx, df);

    return ret;
}


int FileBase::BuildArea (word areanum, const char *filepath,
                         const char *filesbbs, const char *acs,
                         word fattr, sword date_style, BOOL NoLocBase)
{
    if (fattr & FA_AUTODATE)
        if (!(Exist (filepath) & _ExDIR_))        // path not existent
            return _FilePathNotFound_;

    char filbuf[PATH_MAX];
    const char *files;

    if (*filesbbs)
        files = filesbbs;
    else {
        strcpy (filbuf, filepath);
        strcat (filbuf, "FILES.BBS");
        files = filbuf;
    }

    if ((fattr & FA_SLOW) && (*filesbbs == '\0'))
        NoLocBase = TRUE;

    int AStat = AreaOpen (areanum, files, NoLocBase);

    if (AStat == _BbsPathNotFound_)
        return AStat;

    FBBS *fbbs = new FBBS (files, PATH_MAX, DMP_BUF_SIZE, FBBSLSIZE, cont, cpos,
      (fattr & FA_LISTDATE) ? Max2FbbsDateStyle (date_style) : FBBS_FLAG_NODATESIZE);

    word flag;
    time_t date;
    dword size;
    char *filename = new char[PATH_MAX];
    char *descbuf  = new char[DMP_BUF_SIZE];

    int res = fbbs->GetEntry (filename, descbuf, &flag, FBBS_GE_FIRST,
                              &date, &size);
    if (res < 0) {
        delete fbbs;
        delete[] descbuf;
        delete[] filename;
        if (AreaClose ())
            return _Error_;
        else
            return _NoFilesBbs_;
    }

    DIRcl *areadir = NULL;

    if (fattr & FA_AUTODATE)
        areadir = new DIRcl (filepath);      // read directory

    byte *entrybuf = new byte[sizeof (_dir) + PATH_MAX - 1];
    char *linebuf  = new char[DMP_BUF_SIZE];
    char *pathbuf  = new char[PATH_MAX];

    int error = 0;

    while (res == 0) {

        _dir *entry = (_dir *) entrybuf;
        char *path = pathbuf;
        char *fname;        // points to filename, no path

        if (fattr & FA_AUTODATE) {
#ifndef UNIX
            if (strpbrk (filename, "\\:"))       // with path
#else
            if (strpbrk (filename, "/:"))       // with path
#endif
                fname = GetEntry (filename, entry, path);
            else {                              // in current dir
                entry = areadir->Get (filename);  // search in dir
                path = NULL;
                fname = filename;
            }
        } else {        // FA_LISTDATE or FA_MANDATE
            fname = GetEntry (filename, entry, path, _GE_NODIR_);
            if (fattr & FA_LISTDATE) {
                entry->cdate = 0;
                entry->mdate = date;
                entry->fsize = size;
            }
        }

        if (fname) {
            _stamp cre_date, mod_date;
            dword fsize;

            if (entry) {
                unix2stamp (entry->cdate ? entry->cdate : entry->mdate, &cre_date);
                unix2stamp (entry->mdate, &mod_date);
                fsize = entry->fsize;
            } else {            // offline
                *((dword *)(&cre_date)) = 0L;
                *((dword *)(&mod_date)) = 0L;
                fsize = 0;
                flag |= FF_OFFLINE;
            }

            error |= AreaAddEntry (fname, flag, &cre_date, &mod_date,
                                   fsize, descbuf, acs, path);
            if (error)
                break;
        }

        res = fbbs->GetEntry (filename, descbuf, &flag, FBBS_GE_NEXT, &date, &size);
    }

    delete[] pathbuf;
    delete[] linebuf;
    delete[] entrybuf;

    delete fbbs;
    delete[] descbuf;
    delete[] filename;

    if (areadir)
        delete areadir;

    error |= AreaClose ();

    if (error)
        return _Error_;
    else
        return AStat;
}


static char *GetEntry (const char *filename, _dir *&entry, char *&path,
                       word flags) // gets data on filename
{
    #ifndef __QNXNTO__
    char *pathend = strrchr (filename, DIRSEP);
    if (!pathend)
        pathend = strchr (filename, ':');
    #else
    char *pathend = strrchr((char *)filename, DIRSEP);
    if (!pathend)
        pathend = strchr((char *)filename, ':');
    #endif // __QNXNTO__

    const char *fname;
    if (!pathend) {           // no path
        path = NULL;
        fname = filename;
    } else {
        int len = pathend - filename + 1;
        if (len >= PATH_MAX)
            return NULL;
        strncpy (path, filename, len);
        path[len] = '\0';
        fname = filename + len;
    }

    entry->namelen = (byte) strlen (fname);

  #if defined (__DOS__)
    if (entry->namelen >= PATH_MAX)
        return NULL;
  #endif

    strcpy (entry->fname, fname);
    char *ret = entry->fname;

    if (flags & _GE_NODIR_) {
        entry->Got     = TRUE;
        entry->fsize   = 0;
        entry->cdate   = 0;
        entry->mdate   = 0;
    } else {
        _ExistStat statbuf;
        if (!(Exist (filename, &statbuf) & _ExFILE_)) {
            entry = NULL;
            return ret;
        }

        entry->Got     = TRUE;
        entry->fsize   = statbuf.size;
        entry->cdate   = statbuf.ctime;
        entry->mdate   = statbuf.mtime;
    }

    return ret;
}



