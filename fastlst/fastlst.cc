
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

#include <signal.h>
#define __USE_GNU
#define _GNU_SOURCE
#include <libio.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <new.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include "apgenlib.hpp"
#include "types.hpp"
#include "defines.hpp"
#include "msgapier.hpp"
#include "misc.hpp"
#include "outblk.hpp"
#include "cfgdata.hpp"
#include "data.hpp"
#include "flstdat.hpp"
#include "parsetyp.hpp"
#include "parsecfg.hpp"

static OUTBLK  *cob;
static BOOL SomeNewNeeded;      // needed for RunBeforeKillSource

void KillSourceFiles (void)
{
    if (nocomp)
        nocomp->KillSource ();

    OUTBLK *ob = outblk;        // compiled blocks
    while (ob) {
        ob->KillSource ();
        ob = ob->next;
    }
}

void PrepareAllNeeded (void)
{
    if (nocomp)
        nocomp->Prepare (IB_NeededOnly);

    OUTBLK *ob = outblk;        // compiled blocks
    while (ob) {
        ob->Prepare (IB_NeededOnly);
        ob = ob->next;
    }
}

void exitfunc (void)
{
    if (BeforeKillSource)
        if (SomeNewNeeded) {
            PrepareAllNeeded ();
            RunCmd (BeforeKillSource, RCf);
        }

    if (killsource)
        KillSourceFiles ();

    switch (errorlevel) {
        case DISK_FULL:
            vprintlogrsp (MsgLogRsp, "\nError: Disk Full !\n");
            break;
        case NOTHING_COMPILED:
            vprintlog ("\nNothing new to compile.\n");
            break;
        case OUT_OF_MEMORY:
            vprintlogrsp (MsgLogRsp, "\nError: Out Of Memory !\n");
            break;
        case USER_BREAK:
            vprintlogrsp (MsgLogRsp, "\n!!! User Break !!!\n");
            break;
    }

    vprintlogrsp (MsgLogRsp, "\nTerminating with Errorlevel %d.\n", errorlevel);

    if (ApiOpened) {
        if (MsgLogRsp)
            closersp (MsgLogRsp, (errorlevel == NOTHING_COMPILED));
        if (MsgLog)
            if (MsgCloseArea (MsgLog))
                wr_mapi_err ();

        if (MsgRemRsp)
            closersp (MsgRemRsp, (errorlevel == NOTHING_COMPILED));
        if (MsgRem)
            if (MsgCloseArea (MsgRem))
                wr_mapi_err ();

        if (MsgCloseApi ())
            vprintlog ("Error closing MsgApi !\n");
    }

    vwritelog ("End, FastLst "VER);

    fcloseall ();

    if ((cob) && (killafter)) {     /* Erase temporary files if existing (aborted) */

        unlink (cob->l->NLname(NL_DAs));
        unlink (cob->l->NLname(NL_NDs));

        if (cob->l->v7data.sysopndx)
            unlink (cob->l->NLname(NL_SDs));

        if (cob->l->v7data.flags & V7DTP_F)
            unlink (cob->l->NLname(NL_DTs));

        if (cob->l->v7data.flags & V7PDX_F)
            unlink (cob->l->NLname(NL_PDs));

    }
}


#pragma off (unreferenced)

void BreakHandler (int sign)
{
    Break = TRUE;
}

#pragma on (unreferenced)


void main (short argc, char *argv[])
{
    char    *s, *config_file = "fastlst.cfg";
    BOOL    PrepOnly;
    short   i;

    StartTime = time (NULL);        // record start time

    fprintf (stderr, "\n\
FastLst ver. "VER"\n\
(C) Copyright 1992-1997 Alberto Pasquale\n\
Portions (C) Copyright 1999 Per Lundberg\n");

//    set_new_handler (new_handler);      // abort on out of memory

    PrepOnly = FALSE;

    if (argc > 1) {
        for (i=1;i<argc;i++) {
            s=argv[i];
            if ((s[0] == '/') || (s[0] == '-')) {
                switch(tolower(s[1])) {
                    case 'h' :
                    case '?' :  printf ("\nAvailable options:\n\n");
                                printf ("-c<config>     Use <config> configuration file\n");
                                printf ("-f             Force compilation even if nothing new\n");
                                printf ("-i             Ignore <FastLst>.DAT (run as 1st time)\n");
                                printf ("-p             Prepare: Unarc and Apply diffs only\n");
                                printf ("-r             Continue Processing on CRC error\n");
                                printf ("-h or -?       This help\n\n");
                                myexit (HELP_REQ);

                    case 'c' :  config_file=s+2;
                                break;

                    case 'f' :  ForceComp = TRUE;
                                break;

                    case 'i' :  IgnoreDat = TRUE;
                                break;

                    case 'p' :  PrepOnly = TRUE;
                                break;

                    case 'r' :  nocrcexit = TRUE;
                                break;

                      default:  vprintlog("Can't understand \"%s\"\n\n", s);
                                break;
                }
            }
        }
    }


    /* Parse configuration file */
    Config *cfg = new Config;
    time_t cfgtime = cfg->parse (config_file);
    delete cfg;

    if (!IgnoreDat)
        read_data (config_file, cfgtime);

    atexit (exitfunc);
    signal (SIGINT, BreakHandler);

    if (MsgLogAreaPath || MsgRemAreaPath) {
        struct _minf minf;
        minf.req_version = 0;
        minf.def_zone = 0;
        if (MsgOpenApi (&minf)) {           /* MSGAPI init */
           vprintlog ("Error initializing MSGAPI !\n");
           myexit (MsgApiInitErr);
        }
        ApiOpened = TRUE;

        if (MsgLogAreaPath) {
            vprintlog ("Reporting to \"%s\"\n", MsgLogAreaPath);
            MsgLog = MsgSOpenArea ((byte *)MsgLogAreaPath, MSGAREA_NORMAL, MsgLogAreaType);
            if (MsgLog == NULL) {
               vprintlog ("Cannot open message area \"%s\"\n", MsgLogAreaPath);
               wr_mapi_err ();
               myexit (AreaOpenErr);
            }
            MsgLogRsp = writerspheader (MsgLog, "FastLst "VER, &MsgFromNode,
                MsgTo, &MsgToNode, "Nodelist Compilation Report",
                (word) MsgAttr, NULL);
        }

        if (MsgRemAreaPath) {
            vprintlog ("Sending Nodelist Comments to \"%s\"\n", MsgRemAreaPath);
            MsgRem = MsgSOpenArea ((byte *)MsgRemAreaPath, MSGAREA_NORMAL, MsgRemAreaType);
            if (MsgRem == NULL) {
               vprintlog ("Cannot open message area \"%s\"\n", MsgRemAreaPath);
               wr_mapi_err ();
               myexit (AreaOpenErr);
            }
            MsgRemRsp = writerspheader (MsgRem, "FastLst "VER, &MsgFromNode,
                MsgTo, &MsgToNode, "Nodelist Comments",
                (word) MsgAttr, NULL);
        }
    }

    SomeNewNeeded = FALSE;      // initialized false

    if (nocomp)
        if (nocomp->Prepare (OB_NewOnly))
            SomeNewNeeded = TRUE;

    BOOL SomethingCompiled = FALSE;
    cob = outblk;
    while (cob) {
        BOOL SomeNew = FALSE;
        if (ForceComp || cob->ChkNew ()) {
            if (cob->Prepare (OB_All, &SomeNew))    // Prepare (Extract) for compilation
                SomeNewNeeded = TRUE;
            if (!PrepOnly) {
                if (SomeNew || ForceComp) {
                    if (cob->Process ())    // Compile
                        SomeNewNeeded = TRUE;
                    SomethingCompiled = TRUE;
                }
            }
        }
        cob = cob->next;
    }

    if ((errorlevel != NO_NEW) && (errorlevel != ERR_TIMEOUT))
        save_data (config_file, cfgtime);

    if ((errorlevel == OK) && (!SomethingCompiled))
        errorlevel = NOTHING_COMPILED;

    exit (errorlevel);
}
