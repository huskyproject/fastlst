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

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "types.hpp"
#include "misc.hpp"
#include "outblk.hpp"
#include "cfgdata.hpp"

#ifdef __QNXNTO__
   #include <strings.h>
#endif // __QNXNTO__

#define vID 8           // version ID of the DAT file (byte)


static BOOL GetDatName (psz DatName, pcsz CfgName)
{
    int cfglen = strlen (CfgName);
    if (cfglen < 5)
        return FALSE;

    if (strcasecmp (CfgName + cfglen - 4, ".cfg") != 0)
        return FALSE;

    strncpy (DatName, CfgName, cfglen - 3);
    strcpy  (DatName + cfglen - 3, "dat");

    return TRUE;
}


static void read_cib (FILE *f, InpSave *is)
{
    fread (is, sizeof (InpSave), 1, f);
}


void read_cob (FILE *f, OutncBlk *cob, OutSave *os = NULL)
{
    time_t NewPwdFileTime = 0;

    if (os) {
        NewPwdFileTime = os->PwdFileTime;    // save newpwdfiletime
        fread (os, sizeof (OutSave), 1, f);     // read cob save blk
    }
                                        // read all cib save blks
    InpncBlk *cib = cob->InpBlk;
    while (cib) {               // if error, Time remains 0
        read_cib (f, cib->s);
        cib = cib->next;
    }

    if (os)
        if (os->PwdFileTime < NewPwdFileTime) {    // if new cob pwd cfg
            cob->InpSavInit ();
            os->PwdFileTime = NewPwdFileTime;
        }
}


void read_data (pcsz CfgName, time_t newcfgtime)
{
    char DatName[PATH_MAX];
    FILE *f;     

    if (!GetDatName (DatName, CfgName)) {
        printf ("\nConfig file name must have a \".cfg\" extension !\n\n");
        myexit (CFG_ERROR);
    }

    f = fopen (DatName, "rb");
    if (!f)
        return;     // file not existent, all fields remain as initialized

    byte versionID = 0;
    fread (&versionID, sizeof (versionID), 1, f);
    if (versionID != vID) {
        fclose (f);     // if different version number don't read
        return;
    }

    time_t cfgtime = 0;
    fread (&cfgtime, sizeof (cfgtime), 1, f);
    if (cfgtime < newcfgtime) {    // Cfg modified ?
        fclose (f);
        return;
    }

    if (nocomp)
        read_cob (f, nocomp);

    OUTBLK *cob = outblk;
    while (cob) {
        read_cob (f, cob, cob->s);
        cob = cob->next;
    }

    fclose (f);
}


int save_cib (FILE *f, InpSave *is)
{
    return (fwrite (is, sizeof (InpSave), 1, f) != 1);
}


int save_cob (FILE *f, OutncBlk *cob, OutSave *os = NULL)
{
    int error = 0;

    if (os)
        error |= (fwrite (os, sizeof (OutSave), 1, f) != 1);

    InpncBlk *cib = cob->InpBlk;
    while (cib) {
        error |= save_cib (f, cib->s);
        cib = cib->next;
    }

    return error;
}


void save_data (pcsz CfgName, time_t cfgtime)
{
    char DatName[PATH_MAX];
    FILE *f;
    int error = 0;

    GetDatName (DatName, CfgName);

    f = fopen (DatName, "wb");
    if (!f) {
        vprintlog ("Cannot save data to \"%s\"\n", DatName);
        return;
    }

    byte versionID = vID;         // version ID of the DAT file
    error |= (fwrite (&versionID, sizeof (versionID), 1, f) != 1);

    error |= (fwrite (&cfgtime, sizeof (cfgtime), 1, f) != 1);

    if (nocomp)
        error |= save_cob (f, nocomp);

    OUTBLK *cob = outblk;
    while (cob) {
        error |= save_cob (f, cob, cob->s);
        cob = cob->next;
    }

    fclose (f);

    if (error)
        vprintlog ("Error writing \"%s\"\n", DatName);
}

