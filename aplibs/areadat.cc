/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1995-1996  Alberto Pasquale                 */
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

#include "apgenlib.hpp"
#include <string.h>
#include "bbsgenlb.hpp"


FAREADAT::FAREADAT ()
{
    fareaname = NULL;
    f = NULL;
    fahp = NULL;
    arean = (word) -1;
    filesbbsbuf = new char[PATH_MAX];
    filesbbsptr = NULL;
}


FAREADAT::~FAREADAT ()
{
    delete[] filesbbsbuf;
    if (fahp) {
        delete[] fahp->pov;
        delete[] fahp->heap;
        delete fahp;
    }
    if (f)
        fclose (f);
    if (fareaname)
        delete[] fareaname;
}


int FAREADAT::OpenFAreaDat (const char *FAreaDat)
{
    char fareadatname[PATH_MAX];
    long areaid;

    fareaname = newcpy (FAreaDat);

    strcpy (fareadatname, FAreaDat);
    addext (fareadatname, ".DAT");

    f = fopen (fareadatname, "rb");
    if (!f)
        return -1;
    setvbuf (f, NULL, _IOFBF, 8192);
    if (fread (&areaid, sizeof (areaid), 1, f) != 1)
        return -2;
    if (areaid != FAREA_ID)
        return -3;

    fahp = new FAH;
    fahp->pov = new OVERRIDE[FAD_OVR_MAX];
    fahp->heap = new char[FAD_HEAP_SIZE];

    return 0;
}


FAH *FAREADAT::LoadArea ()
{
    if (fread (&fahp->fa, sizeof (FAREA), 1, f) != 1)
        return NULL;
    if (fahp->fa.cbArea != sizeof (FAREA)) {
        if (fahp->fa.cbArea < sizeof (FAREA))
            return NULL;
        if (fseek (f, fahp->fa.cbArea - sizeof (FAREA), SEEK_CUR))
            return NULL;
    }
    if (fahp->fa.num_override > FAD_OVR_MAX)
        return NULL;
    if (fahp->fa.cbHeap > FAD_HEAP_SIZE)
        return NULL;
    if (fread (fahp->pov, sizeof (OVERRIDE), fahp->fa.num_override, f) != fahp->fa.num_override)
        return NULL;
    if (fread (fahp->heap, fahp->fa.cbHeap, 1, f) != 1)
        return NULL;
    fahp->heap_size = fahp->fa.cbHeap;
    fahp->bi.use_barpriv = 0;

    if (*PFAS (fahp, filesbbs) == '\0') {
        strcpy (stpcpy (filesbbsbuf, PFAS (fahp, downpath)), "files.bbs");
        filesbbsptr = filesbbsbuf;
    } else
        filesbbsptr = PFAS (fahp, filesbbs);

    return fahp;
}


FAH *FAREADAT::NextArea (int act)
{
    FAH *fahp;

    while (1) {
        arean ++;
        fahp = LoadArea ();
        if (!fahp)
            break;
        if (act == FAD_Normal)
            break;
        if  (!(fahp->fa.attribs & (FA_DIVBEGIN | FA_DIVEND)))
            break;
    }

    return fahp;
}


FAH *FAREADAT::Area (word num)
{
    char idxname[PATH_MAX];
    strcpy (idxname, fareaname);
    setext (idxname, ".IDX");

    FILE *fi = fopen (idxname, "rb");
    if (!fi)
        return NULL;
    if (fseek (fi, num * sizeof (MFIDX), SEEK_SET)) {
        fclose (fi);
        return NULL;
    }
    MFIDX mfidx;
    if (fread (&mfidx, sizeof (mfidx), 1, fi) != 1) {
        fclose (fi);
        return NULL;
    }
    fclose (fi);

    if (fseek (f, mfidx.ofs, SEEK_SET))
        return NULL;

    arean = num;

    FAH *fahp = LoadArea ();
    if (!fahp)
        return NULL;
    if  (fahp->fa.attribs & (FA_DIVBEGIN | FA_DIVEND))
        return NULL;

    return fahp;
}


FAH *FAREADAT::Area (const char *name)
{
    char idxname[PATH_MAX];
    strcpy (idxname, fareaname);
    setext (idxname, ".IDX");

    FILE *fi = fopen (idxname, "rb");
    if (!fi)
        return NULL;

    MFIDX mfidx;
    arean = (word) -1;
    FAH *fahp = NULL;

    while (fread (&mfidx, sizeof (mfidx), 1, fi) == 1) {
        arean ++;
        if (strncasecmp (name, mfidx.name, 15) == 0) {
            if (fseek (f, mfidx.ofs, SEEK_SET)) {
                fclose (fi);
                return NULL;
            }
            fahp = LoadArea ();
            if (!fahp) {
                fclose (fi);
                return NULL;
            }
            if (stricmp (name, PFAS (fahp, name)) == 0) {
                fclose (fi);
                if  (fahp->fa.attribs & (FA_DIVBEGIN | FA_DIVEND))
                    return NULL;
                return fahp;
            }
        }
    }

    fclose (fi);
    return NULL;
}


word FAREADAT::AreaNum ()
{
    return arean;
}


char *FAREADAT::filesbbs ()
{
    return filesbbsptr;
}
