/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1996       Alberto Pasquale                 */
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

#include "bbsgenlb.hpp"
#include <apgenlib.hpp>
#include <string.h>
#include <unistd.h>

SqNetScan::SqNetScan ()
{
    harea = NULL;
    hmsg = NULL;
}


SqNetScan::~SqNetScan ()
{
    Close ();       // in the case it has not been done.
}


  // try Open for 30 s

static HAREA MsgTOpenArea (byte *name, word mode, word type)
{
    int i = 0;

    HAREA ret = MsgOpenArea (name, mode, type);
    while (!ret && (i < 30)) {
        sleep (1);
        i++;
        ret = MsgOpenArea (name, mode, type);
    };

    return ret;
}


// tryes MsgLock 6 times (30 s)


static sword MsgTLock (HAREA ha)
{
    sword ret = 0;

    for (int i = 0; i < 6; i ++) {
        ret = MsgLock (ha);
        if (ret == 0)
            break;
    }

    return ret;
}


static UMSGID GetHwmID (const char *filename)
{
    UMSGID hwmID = 0;

    FILE *f = fopen (filename, "rb");
    if (f) {
        if (fread (&hwmID, sizeof (hwmID), 1, f) != 1)
            hwmID = 0;
        fclose (f);
    }

    return hwmID;
}


static int SetHwmID (const char *filename, UMSGID hwmID)
{
    FILE *f = fopen (filename, "wb");
    if (!f)
        return -1;
    int ret = 0;
    ret |= (fwrite (&hwmID, sizeof (hwmID), 1, f) != 1);
    ret |= fclose (f);

    return ret;
}


HAREA SqNetScan::Open (const char *path, word type, word defzone, const char *sav)
{
    if (Close ())
        return NULL;

    if ((type == MSGTYPE_SQUISH) && sav)
        sprintf (savfilename, "%s.%s", path, sav);
    else
        savfilename[0] = '\0';

    areatype = type;
    areazone = defzone;
                                        // Area Open tries 30s

    harea = MsgTOpenArea ((byte *)path, MSGAREA_NORMAL, type);
    if (!harea)
        return NULL;

    if (MsgTLock (harea)) {      // MsgLock tries 30s
       MsgCloseArea (harea);
       harea = NULL;
       return NULL;
    }

    if (savfilename[0])
        hwmID = GetHwmID (savfilename);
    else
        hwmID = 0;

    highID = MsgMsgnToUid (harea, MsgGetHighMsg (harea));

    if (highID < hwmID)   // if base has been rebuilt -> rescan
        hwmID = 0;

    return harea;
}



UMSGID SqNetScan::GetNextMsg (XMSG *xmsg, byte flags)
{
    if (!harea)
        return 0;

    BOOL Found = FALSE;

    do {
        if (hmsg) {
            MsgCloseMsg (hmsg);
            hmsg = NULL;
        }

        UMSGID msgID = hwmID + 1;

        if (msgID > highID)
            return 0;

        dword msgn = MsgUidToMsgn (harea, msgID, UID_NEXT);
        if (msgn == 0)
            return 0;
        msgID = MsgMsgnToUid (harea, msgn);
        if (msgID <= hwmID)  // not really necessary, but useful with new MSGAPI
           return 0;         // if the final msg has been deleted in the meantime
        if (msgID > highID)  // In the case highID has been deleted and we are
            return 0;        // pointing to a newly created message.

        hmsg = MsgOpenMsg (harea, MOPEN_READ, msgn);
        if (!hmsg)
            return 0;

        dword readlen = MsgReadMsg (hmsg, xmsg, 0, 0, NULL, 0, NULL);
        if (readlen == (dword)-1)
            memset (xmsg, 0, sizeof (XMSG));    // in case of corrupted message
        else {
            if (xmsg->orig.zone == 0)           // default zone when necessary
                xmsg->orig.zone = areazone;
            if (xmsg->dest.zone == 0)
                xmsg->dest.zone = areazone;
        }

        hwmID = msgID;

        Found = TRUE;

        if ((flags & SQNS_Get_SkipRead) && (xmsg->attr & MSGREAD))
            Found = FALSE;

    } while (!Found);


    return hwmID;
}


void SqNetScan::LoadMsgBody (char *body, size_t size)
{
    if (!hmsg)
        return;
    if (size < 1)
        return;
    dword readlen = MsgReadMsg (hmsg, NULL, 0, size-1, (byte *)body, 0, NULL);
    if (readlen == (dword)-1)   // corrupted message !
        body[0] = '\0';
    else
        body[(int)readlen] = '\0'; /* assure a NULL terminated string */
}


int SqNetScan::MarkMsgRead ()
{
    if (!hmsg)
        return 1;
    MsgCloseMsg (hmsg);
                            // time critical if not locked

    hmsg = MsgOpenMsg (harea, MOPEN_RW, MsgUidToMsgn (harea, hwmID, UID_EXACT));
    if (!hmsg)
        return 1;

    int err = 0;

    XMSG xmsg;
    // explicit typecast of -1 to unsigned to make smapi happy
    // see smapi/sq_read.c
    // IMHO this should be changed in smapi to sword
    err |= (MsgReadMsg (hmsg, &xmsg, 0, 0, NULL, 0, NULL) == (dword)-1);
    if (!err) {
        xmsg.attr |= MSGREAD;
        err |= (MsgWriteMsg (hmsg, 0, &xmsg, NULL, 0, 0, 0, NULL) == -1);
    }
    err |= MsgCloseMsg (hmsg);
    hmsg = NULL;

    return err;
}


int SqNetScan::KillMsg ()
{
    if (!hmsg)
        return 1;
    MsgCloseMsg (hmsg);
    hmsg = NULL;

    if (MsgKillMsg (harea, MsgUidToMsgn (harea, hwmID, UID_EXACT)))
        return 1;

    return 0;
}


int SqNetScan::Close ()
{
    if (hmsg) {
        MsgCloseMsg (hmsg);
        hmsg = NULL;
    }

    int ret = 0;

    if (harea) {
        if (savfilename[0])
            if (SetHwmID (savfilename, hwmID))
                ret = 1;
        MsgUnlock (harea);
        if (MsgCloseArea (harea))
            ret = -1;
        harea = NULL;
    }

    return ret;
}
