/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1995-1997  Alberto Pasquale                 */
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


/* DosFind.Cpp */

#include <stdlib.h>
#include <string.h>


#ifdef __OS2__
    #define INCL_DOS
    #include <os2.h>
#endif

#include "apgenlib.hpp"

#define EA_LONGNAME ".LONGNAME"


#ifdef __OS2__

    #pragma pack(1)

    struct FILEFINDBUF3NONAME {
        ULONG   oNextEntryOffset;            /* new field */
        FDATE   fdateCreation;
        FTIME   ftimeCreation;
        FDATE   fdateLastAccess;
        FTIME   ftimeLastAccess;
        FDATE   fdateLastWrite;
        FTIME   ftimeLastWrite;
        ULONG   cbFile;
        ULONG   cbFileAlloc;
        ULONG   attrFile;                    /* widened field */
    };

    struct FILEFINDEABUF {
        EAOP2 eaop2;
        FILEFINDBUF3NONAME ffb3;
        FEA2LIST fea2list;      // followed by name and value of EA;
        byte buffer[3 * _MAX_PATH]; // On dword boundary, cbName (byte) and achName
    };


                    // for ffind.flags
  #define FF_LONGNAME   0x01


  #define DwordBound(p)     pvoid ((dword(p)+3) & 0xFFFFFFFC)

#endif


void Dos2My (FFIND *buf)
{

#if defined (__OS2__)

    FILEFINDBUF3NONAME *ffb3nn = (buf->flags & FF_LONGNAME) ?
                                 &buf->ffeab->ffb3 :
                                 (FILEFINDBUF3NONAME *)buf->ffb3;

    buf->attrib  = (char) ffb3nn->attrFile;
    buf->cr_time = MKushort (ffb3nn->ftimeCreation);
    buf->cr_date = MKushort (ffb3nn->fdateCreation);
    buf->ac_time = MKushort (ffb3nn->ftimeLastAccess);
    buf->ac_date = MKushort (ffb3nn->fdateLastAccess);
    buf->wr_time = MKushort (ffb3nn->ftimeLastWrite);
    buf->wr_date = MKushort (ffb3nn->fdateLastWrite);
    buf->size    = ffb3nn->cbFile;

    if (buf->flags & FF_LONGNAME) {

        PFEA2 fea2 = &buf->ffeab->fea2list.list[0];

        byte *p = (byte *) DwordBound (
                    fea2->szName +      // end of FEA2
                    fea2->cbName +      // length of EA name
                    1 +                 // NULL terminator
                    fea2->cbValue       // length of EA value
                  );        // points to cbName of file

        int filenamelen = *p;
        buf->name = (char *)(p+1);
        buf->name[filenamelen] = '\0'; // just make sure...

        buf->longname = NULL;
        if (fea2->cbValue != 0) {
            byte *p = (byte *)fea2->szName + fea2->cbName + 1;
            word eatype = *(word *)p;
            if (eatype == EAT_ASCII) {
                p += 2;
                word ealen = *(word *)p;
                if ((4 + ealen) <= fea2->cbValue) {
                    p += 2;
                    buf->longname = (char *)p;
                    buf->longname[ealen] = '\0'; // may overwrite cbName
                }
            }
        }

    } else {
        buf->name = buf->ffb3->achName;
        buf->longname = NULL;
    }

#elif defined (__NT__)

    PWIN32_FIND_DATA ff = buf->ff;

    buf->attrib  = (byte)ff->dwFileAttributes;

    FiletimeUtc2Dosdatime (&ff->ftCreationTime, &buf->cr_date, &buf->cr_time);
    FiletimeUtc2Dosdatime (&ff->ftLastAccessTime, &buf->ac_date, &buf->ac_time);
    FiletimeUtc2Dosdatime (&ff->ftLastWriteTime, &buf->wr_date, &buf->wr_time);

    buf->size    = ff->nFileSizeLow;
    buf->name    = ff->cFileName;

#elif defined (__linux__)
#else

    find_t *ff = buf->ff;
    buf->attrib  = ff->attrib;
    buf->cr_time = 0;
    buf->cr_date = 0;
    buf->ac_time = 0;
    buf->ac_date = 0;
    buf->wr_time = ff->wr_time;
    buf->wr_date = ff->wr_date;
    buf->size    = ff->size;
    buf->name    = ff->name;

#endif

}


static bool OkAttribute (byte fileattr, byte required, byte forbidden)
{
            // check file attributes

    if ((fileattr & required) != required)
        return false;

    if ((fileattr & forbidden) != 0)
        return false;

    return true;
}


void SetAttrMask (unsigned attributes, FFIND *buffer)
{
    buffer->flags = 0;

  #ifdef __OS2__
    if (attributes & _DFF_A_LONGNAME)
        buffer->flags |= FF_LONGNAME;
  #endif

    byte forbidden = _DFF_FILE_SYSTEM | _DFF_FILE_HIDDEN | _DFF_FILE_DIRECTORY;
    byte required   = _DFF_FILE_NORMAL;

                    // set required

    if (attributes & _DFF_A_REQ_HIDDEN)
        required |= _DFF_FILE_HIDDEN;

    if (attributes & _DFF_A_REQ_SYSTEM)
        required |= _DFF_FILE_SYSTEM;

    if (attributes & _DFF_A_REQ_DIRECTORY)
        required |= _DFF_FILE_DIRECTORY;

    if (attributes & _DFF_A_REQ_ARCHIVED)
        required |= _DFF_FILE_ARCHIVED;

    if (attributes & _DFF_A_REQ_READONLY)
        required |= _DFF_FILE_READONLY;

    forbidden &= ~required;            //  remove required from forbidden

                    // remove may from forbidden

    if (attributes & _DFF_A_MAY_HIDDEN)
        forbidden &= ~_DFF_FILE_HIDDEN;

    if (attributes & _DFF_A_MAY_SYSTEM)
        forbidden &= ~_DFF_FILE_SYSTEM;

    if (attributes & _DFF_A_MAY_DIRECTORY)
        forbidden &= ~_DFF_FILE_DIRECTORY;

                    // add forbidden

    if (attributes & _DFF_A_NON_ARCHIVED)
        forbidden |= _DFF_FILE_ARCHIVED;

    if (attributes & _DFF_A_NON_READONLY)
        forbidden |= _DFF_FILE_READONLY;

    buffer->required_attributes = required;
    buffer->forbidden_attributes = forbidden;
}


unsigned _DosFindFirst (pcsz path, unsigned attributes, FFIND *buffer)
{
    SetAttrMask (attributes, buffer);

#if defined (__OS2__)

    if (buffer->flags & FF_LONGNAME) {
        int fst = QueryFS (path);
        if ((fst != QFS_FAT) && (fst != QFS_NET_EA))
            buffer->flags &= ~FF_LONGNAME;
    }

    if (buffer->flags & FF_LONGNAME) {
        buffer->bufsize = sizeof (FILEFINDEABUF);
        buffer->infolevel = FIL_QUERYEASFROMLIST;
        buffer->ffeab = new FILEFINDEABUF;
    } else {
        buffer->bufsize = sizeof (FILEFINDBUF3);
        buffer->infolevel = FIL_STANDARD;
        buffer->ffb3 = new FILEFINDBUF3;
    }

    if (buffer->flags & FF_LONGNAME) {       // setup the EAOP2
        PEAOP2 eaop2 = &buffer->ffeab->eaop2;
        int eanamelen = strlen (EA_LONGNAME);
        int gea2listsize = sizeof (GEA2LIST) + eanamelen;
        eaop2->fpGEA2List = (PGEA2LIST) new byte[gea2listsize];
        eaop2->fpFEA2List = NULL;
        eaop2->fpGEA2List->cbList = gea2listsize;
        eaop2->fpGEA2List->list[0].oNextEntryOffset = 0;
        eaop2->fpGEA2List->list[0].cbName = eanamelen;
        strcpy (eaop2->fpGEA2List->list[0].szName, EA_LONGNAME);
    }

    ULONG FindCount = 1;
    buffer->DirHandle = HDIR_CREATE;
    if (DosFindFirst (path, &buffer->DirHandle,
        FILE_ARCHIVED|FILE_DIRECTORY|FILE_SYSTEM|FILE_HIDDEN|FILE_READONLY,
        buffer->ff, buffer->bufsize, &FindCount, buffer->infolevel))

#elif defined (__NT__)

    buffer->ff = new WIN32_FIND_DATA;
    buffer->DirHandle = FindFirstFile (path, buffer->ff);
    if (buffer->DirHandle == INVALID_HANDLE_VALUE)

#elif defined (__linux__)
#else
    buffer->ff = new find_t;
    if (_dos_findfirst (path, _A_NORMAL | _A_HIDDEN | _A_SYSTEM | _A_SUBDIR,
                        buffer->ff))
#endif
        return 1;

    Dos2My (buffer);

    if (!OkAttribute (buffer->attrib,
                 buffer->required_attributes, buffer->forbidden_attributes))
        return _DosFindNext (buffer);

    return 0;
}


unsigned _DosFindNext (FFIND *buffer)
{

   do {
#if defined (__OS2__)

    ULONG SearchCount = 1;
    if (DosFindNext (buffer->DirHandle, buffer->ff, buffer->bufsize,
                     &SearchCount))

#elif defined (__NT__)
    if (!FindNextFile (buffer->DirHandle, buffer->ff))
#elif defined (__linux__)
#else
    if (_dos_findnext (buffer->ff))
#endif
        return 1;

    Dos2My (buffer);

   } while (!OkAttribute (buffer->attrib,
            buffer->required_attributes, buffer->forbidden_attributes));

    return 0;
}



unsigned _DosFindClose (FFIND *buffer)
{
#if defined (__OS2__)

    DosFindClose (buffer->DirHandle);

    if (buffer->flags & FF_LONGNAME)
        delete buffer->ffeab->eaop2.fpGEA2List;

#elif defined (__NT__)

    FindClose (buffer->DirHandle);

#elif defined (__linux__)
#else

    _dos_findclose (buffer->ff);

#endif

    delete buffer->ff;

    return 0;
}

