/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1995-1996  Alberto Pasquale                 */
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

#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "apgenlib.hpp"

typedef _dir *DIRP;

#define RAWSIZE 2048

typedef int (*QSF) (const void *, const void *);


struct _dirblk {
    byte raw[RAWSIZE];
    byte *np;   // pointer to next entry
    _dirblk *next;
};


static int fnameCmp (DIRP *d1, DIRP *d2)
{
    return stricmp ((*d1)->fname, (*d2)->fname);
}


        // private


void DIRcl::Add (const FFIND *f)
{
    int namelen = strlen (f->name);
    int reclen = sizeof (_dir) + namelen;

    if ((curblk->np - curblk->raw) + reclen + 1 > RAWSIZE) {    // new block
        curblk = curblk->next = new _dirblk;
        curblk->np = curblk->raw;
        curblk->next = NULL;
    }

    DIRP dp = (DIRP) curblk->np;
    dp->namelen = (byte) namelen;
    dp->Got = FALSE;
    dp->fsize = f->size;
    dp->cdate = dosdatimetounix (f->cr_date, f->cr_time);
    dp->mdate = dosdatimetounix (f->wr_date, f->wr_time);
    strcpy (dp->fname, f->name);

    ntot ++;
    curblk->np += reclen;
    *curblk->np = 0;
}


void DIRcl::Read (const char *path)
{
    struct dirent *dirent;
    DIR *dir;
    FFIND ffblk;
    BOOL done = 0;

    dir = opendir (path);   
//the following line seems to be useless
//    if (dirent == NULL) done = 1;
    while (!done) 
      {
         dirent = readdir (dir);
         if (dirent == NULL)
           done = 1;
         else
           {
              strcpy (ffblk.name, dirent->d_name);
              Add (&ffblk);
           }      
    }

    if (ntot == 0)
        return;

    dirp = new DIRP[ntot];

    curblk = dirblkhead;            // build table of pointers
    byte *np = curblk->raw;
    for (uint i = 0; i < ntot; i++) {
        if (*np == 0) {
            curblk = curblk->next;
            np = curblk->raw;
        }
        dirp[i] = (DIRP) np;
        np += *np + sizeof (_dir);
    }

    qsort (dirp, ntot, sizeof (DIRP), (QSF) fnameCmp);  // and sort
}


                    // public

DIRcl::DIRcl (const char *path)
{
    curblk = dirblkhead = new _dirblk;
    curblk->np = curblk->raw;
    *curblk->np = 0;        // mark end of block
    curblk->next = NULL;

    dirp = NULL;
    ntot = 0;

    Read (path);
}


DIRcl::~DIRcl (void)
{
    _dirblk *next;
    _dirblk *db = dirblkhead;
    while (db) {
        next = db->next;
        delete db;
        db = next;
    }

    if (dirp)
        delete[] dirp;
}



_dir *DIRcl::Get (int first)
{
    static uint i;

    if (first)
        i = 0;

    if (i >= ntot)
        return NULL;

    return dirp[i++];
}


static int bsf (char *keyname, DIRP *dp)
{
    return stricmp (keyname, (*dp)->fname);
}


_dir *DIRcl::Get (const char *filename, BOOL NoDupes)
{
    if (ntot == 0)
        return NULL;

    DIRP *p = (DIRP *) bsearch (filename, dirp, ntot, sizeof (DIRP), (QSF) bsf);
    if (!p)
        return NULL;

    if (NoDupes && (*p)->Got)      // already got
        return NULL;

    (*p)->Got = TRUE;

    return *p;
}
