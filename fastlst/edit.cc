/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1992-1997  Alberto Pasquale                 */
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
/* How to contact the author:  Alberto Pasquale of 2:332/504@fidonet         */
/*                             Viale Verdi 106                               */
/*                             41100 Modena                                  */
/*                             Italy                                         */
/*                                                                           */
/*****************************************************************************/

// Edit.Cpp


#ifdef __OS2__
    #define INCL_DOS
    #include <os2.h>
#endif

#include    <apgenlib.hpp>
#include    <time.h>
#include    <stdio.h>
#include    <string.h>
#include    <ctype.h>
#include    <stdlib.h>
#include    "edit.hpp"
#include    "defines.hpp"
#include    "misc.hpp"
#include    "cfgdata.hpp"
#include    "data.hpp"
#include    "crc16.hpp"
#include    "parse.hpp"


EDIT::EDIT (char *BeforeCmd, char *AfterCmd)
{
    EDIT::BeforeCmd = BeforeCmd;
    EDIT::AfterCmd  = AfterCmd;

    old_name  = new char[PATH_MAX];
    diff_name = new char[PATH_MAX];
    new_name  = new char[PATH_MAX];
}


EDIT::~EDIT (void)
{
    delete[] new_name;
    delete[] diff_name;
    delete[] old_name;
}


int EDIT::DoEdit (void)
{							/* Actually put the new lines in here */
    char line[LINESIZE];
    word official_crc;
    int i, j;

    int ErrorFull = 0, ErrorRead = 0;
    word calc_crc = 0;
    BOOL first_add = TRUE;

    while ((fgets (line, LINESIZE, Diff)) != NULL) {
        if (line[0] == 0x1A)    // ^Z
            break;
        switch (line[0]) {
            case ';':
                continue;

            case 'D':
                j = atoi (line+1);
                for (i = 0; i < j; i++)
                    ErrorRead |= (fgets (line, LINESIZE, Old) == NULL);
                    break;

            case 'A':
                j = atoi (line+1);
                for (i = 0; i < j; i++) {
                    ErrorRead |= (fgets (line, LINESIZE, Diff) == NULL);
                    if (!first_add)
                        calc_crc = crcstr (line, calc_crc);
                    else {
                        first_add = FALSE;
                        char *ptr = strrchr (line, ':');
                        if (!ptr) {
                            vprintlogrsp (MsgLogRsp, "Error: Cannot find new CRC !\n");
                            return -1;
                        }
                        official_crc = (word) atoi (ptr+1);
                    }
                    ErrorFull |= (fputs (line, New) == EOF);
                }
                break;

            case 'C':
                j = atoi (line+1);
                for (i = 0; i < j; i++) {
                    ErrorRead |= (fgets (line, sizeof (line), Old) == NULL);
                    calc_crc = crcstr (line, calc_crc);
                    ErrorFull |= (fputs (line, New) == EOF);
                }
                break;

            default:
                vprintlogrsp (MsgLogRsp, "Spurious line: %.200s", line);
                break;
        }

        if (ErrorRead || ErrorFull || Break)
            break;
    }

    fprintf (New, "%c", '\x1A');        // terminating ^Z

    if (ErrorRead) {
        vprintlogrsp (MsgLogRsp, "Read Error\n");
        errorlevel = READ_ON_DIFF;
        return -1;
    } else
    if (ErrorFull) {
        errorlevel = DISK_FULL;
        return -1;
    } else
    if (Break) {
        errorlevel = USER_BREAK;
        return -1;
    }

    if (calc_crc != official_crc) {
        vprintlogrsp (MsgLogRsp, "\nCRC ERROR: actual/expected CRC: %05hu/%05hu !\n\n", calc_crc, official_crc);
        errorlevel = CRC_ON_DIFF;     /* crc error */
        if (!nocrcexit)
            return -1;
    }

    return 0;     /* ok */
}


int EDIT::Open (void)
{
    char first_line[LINESIZE];
    char *ptr;

    strcpy (old_name, NodeList);
    SetDay3 (old_name, ListDay);

    strcpy (new_name, NodeList);
    SetDay3 (new_name, DiffDay);

    strcpy (diff_name, NodeDiff);
    SetDay3 (diff_name, DiffDay);

    Old = New = Diff = NULL;

    if (BeforeCmd)
        RunCmd (BeforeCmd, RCf, "ld", old_name, diff_name);

    vprintlogrsp (MsgLogRsp, "Applying \"%s\" to \"%s\"\n", diff_name, old_name);

    if ((Old = fopen (old_name, "rb")) == NULL) {
        vprintlogrsp (MsgLogRsp, "Error opening \"%s\" !\n", old_name);
        return -1;
    }
    setvbuf (Old, NULL, _IOFBF, BufSize);

    if (fgets (first_line, LINESIZE, Old) == NULL) {
        vprintlogrsp (MsgLogRsp, "Read Error on Old List file !\n");
        return -1;
    }
    rewind (Old);

    if ((ptr = strrchr (first_line, ':')) == NULL) {
        vprintlogrsp (MsgLogRsp, "Error: Cannot find old day/crc in Old List file !\n");
        return 1;
    }

    word list_old_crc = (word) atoi (ptr+1);  // get crc of old list

    if ((Diff = fopen (diff_name, "rb")) == NULL) {
        vprintlogrsp (MsgLogRsp, "Error opening \"%s\" !\n", diff_name);
        return -1;
    }
    setvbuf (Diff, NULL, _IOFBF, BufSize);

    if (fgets (first_line, LINESIZE, Diff) == NULL) {
        vprintlogrsp (MsgLogRsp, "Read Error on Diff file !\n");
        return -1;
    }

    if ((ptr = strrchr (first_line, ':')) == NULL) {
        vprintlogrsp (MsgLogRsp, "Error: Cannot find old day/crc in Diff file !\n");
        return 1;
    }

    word diff_old_crc = (word) atoi (ptr+1);  // get crc req for old list

    do
        ptr--;
    while ((ptr > first_line) && isspace (*ptr));
    while ((ptr > first_line) && IsDigit (*ptr))
        ptr--;

    int diff_old_day = atoi (ptr);      // get day of old list

    if (diff_old_day != ListDay) {
        vprintlogrsp (MsgLogRsp, "Error: Diff %03d requires List %03d, not existing %03d !\n", DiffDay, diff_old_day, ListDay);
        return 1;
    }

    if (diff_old_crc != list_old_crc) {
        vprintlogrsp (MsgLogRsp, "Error: Diff %03d requires List %03d with crc %05hu, not %05hu !\n", DiffDay, diff_old_day, diff_old_crc, list_old_crc);
        return 1;
    }

    if ((New = fopen (new_name, "wb")) == NULL) {
        vprintlogrsp (MsgLogRsp, "Can't open \"%s\" !\n", new_name);
        return -1;
    }
    setvbuf (New, NULL, _IOFBF, BufSize);

    return 0;
}


void EDIT::Close (BOOL Ok)
{
    time_t difftime;

    if (Old)
        fclose (Old);
    if (Diff) {
        difftime = DosFileTime (fileno (Diff));
        fclose (Diff);
    }
    if (New && Ok) {
        if (fclose (New) == EOF) {
            errorlevel = DISK_FULL;
            Ok = FALSE;
        } else {
            if (!touchf (new_name, difftime))
                vprintlogrsp (MsgLogRsp, "Error setting date and time of \"%s\"\n", new_name);
        }
    }

    if (errorlevel == DISK_FULL)
        vprintlogrsp (MsgLogRsp, "Disk Full\n");

    if ((!Ok) && New) {
        vprintlog ("Erasing \"%s\"\n", new_name);
        KillWriteFile (New, new_name);
        if (Break)
            myexit (USER_BREAK);
    } else
    if (Ok) {
        vprintlog ("Erasing \"%s\" and \"%s\"\n", old_name, diff_name);
        EraseFile (old_name);
        EraseFile (diff_name);
        if (AfterCmd)
            RunCmd (AfterCmd, RCf, "l", new_name);
    }
}


int EDIT::Apply (char *NodeList, int ListDay, char *NodeDiff, int DiffDay)
{
    EDIT::NodeList = NodeList;
    EDIT::NodeDiff = NodeDiff;
    EDIT::ListDay  = ListDay;
    EDIT::DiffDay  = DiffDay;

    int ret;    // 0 = success, -1 = error, 1 = day mismatch

    ret = Open ();

    if (ret == 0)
        ret = DoEdit ();

    Close (ret ? FALSE : TRUE);

    return ret;
}


