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

#include "apgenlib.hpp"
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "crc16.hpp"
#include "data.hpp"
#include "inpblk.hpp"
#include "misc.hpp"
#include "cfgdata.hpp"
#include "parse.hpp"
#include "field.hpp"
#include "parsetyp.hpp"
#include "export.hpp"
#include "inmem.hpp"

const char UnpublishedPhone[] = "-Unpublished-";

InpAll::InpAll (InpAll *ia)
{
    Init (ia);
}

void InpAll::Init (InpAll *ia)
{
    if (ia) {
        *this = *ia;
        ArcMethHead = CopyArcMethod (ia->ArcMethHead);
        ArcDiffMethHead = CopyArcMethod (ia->ArcDiffMethHead);
    } else
        memset (this, 0, sizeof (*this));
}


InpNnc::InpNnc (InpNnc *in)
{
    if (in)
        *this = *in;
    else
        memset (this, 0, sizeof (*this));
}


InpOut::InpOut (void)
{
    memset (this, 0, sizeof (*this));
}


InpLoc::InpLoc (void)
{
    memset (this, 0, sizeof (*this));
}


InpVar::InpVar (void)
{
    memset (this, 0, sizeof (*this));
    crcchk = TRUE;
}


InpSave::InpSave (void)
{
    Init ();
}


void InpSave::Init ()
{
    memset (this, 0, sizeof (*this));
    NodeDay = -1;
}


InpncBlk::InpncBlk (InpAll *ia)
{
    a = new InpAll (ia);
    l = new InpLoc;
    v = new InpVar;
    s = new InpSave;
    next = NULL;
}


void InpncBlk::savinit (void)
{
    s->Init ();
}


INPBLK::INPBLK (InpAll *ia, InpNnc *in, SegAll *isa)      // constructor initializer
{
    a->Init (ia);
    n = new InpNnc (in);
    o = new InpOut;
    sa = new SegAll (isa);
    SegExpHead = NULL;
}


BOOL INPBLK::Process (OUTCUR *ocp, OUTBLK *cob, BOOL *SomeNeeded)
{
    INPCUR inpcur (o);

    Open (&inpcur);

    if (n->BeforeCompile)
        RunCmd (n->BeforeCompile, RCf, "l", l->NodeList);

    BOOL ret = ProcessFile (l->NodeList, &inpcur, ocp, cob, SomeNeeded);
    writersp (MsgLogRsp, "\"%s\" processed%s\r\r", l->NodeList, ret ? "." : " with ERRORS.");

    if (n->AfterCompile)
        RunCmd (n->AfterCompile, RCf, "l", l->NodeList);

    Close (&inpcur);

    return ret;
}


static void TxtHeader (INPCUR *icp)
{
    if (icp->txt_pagenum != 0) {
        if (icp->nodelist_txt)
            fprintf (icp->nodelist_txt, "");
        if (icp->nodelist_prn)
            fprintf (icp->nodelist_prn, "");
    }
    icp->txt_pagenum ++;
    if (icp->nodelist_txt)
        fprintf (icp->nodelist_txt, "%s %-4hd\n\n", icp->header, icp->txt_pagenum);
    if (icp->nodelist_prn)
        fprintf (icp->nodelist_prn, "%s %-4hd\n\n", icp->header, icp->txt_pagenum);
    icp->txt_pg_lines = 2;
}


static void TxtLine (INPCUR *icp, char *text)
{
    icp->txt_pg_lines ++;
    if (icp->nodelist_txt)
        fprintf (icp->nodelist_txt, "%s\n", text);
    if (icp->nodelist_prn)
        fprintf (icp->nodelist_prn, "%s\n", text);
}


static void MkSysopComma (pcsz sysopname, char *sysop_comma)
{
    int l = strlen (sysopname);

    if (l > SYSOPSIZE-2) {
        strcpy (sysop_comma, "Too_Long, Name");
        return;
    }

    if (l == 0) {
        strcpy (sysop_comma, "Empty, Name");
        return;
    }

    pcsz lastname = strrchr (sysopname, '_');
    if (lastname)
        lastname ++;
    else
        lastname = sysopname;

    char *d = fl_stpcpy (sysop_comma, lastname);

    if (sysopname != lastname) {    /* if there is a first name */
        const char *s = sysopname;
        lastname--;     /* skip blank that precede last name */
        *d++ = ',';
        *d++ = ' ';
        while (s < lastname) {
            if (*s == '_') {        // change underscore to blank
                *d++ = ' ';
                s++;
            } else
                *d++ = *s++;
        }
        *d = '\0';
    }
}


static void Pack (word *outp, char *inp, byte *count)
{
                                 /* for name compression */
    static byte unwrk[84] = {
        28,  0,  0,  0,  0,  0, 27,  0,  0,     // ' ( 39) -> / ( 47)
        29, 30, 31, 32, 33, 34, 35, 36, 37, 38, // 0 ( 48) -> 9 ( 57)
         0,  0,  0,  0,  0,  0,  0,             // : ( 58) -> @ ( 64)
         2, 12, 10, 13,  1, 21, 16, 11,         // A ( 65) -> H ( 72)
         8, 23, 18,  9, 14,  3,  5, 17,         // I ( 73) -> P ( 80)
        26,  4,  6,  7, 15, 22, 20, 24,         // Q ( 81) -> X ( 88)
        19, 25,  0,  0,  0,  0,  0, 28,         // Y ( 89) -> ` ( 96)
         2, 12, 10, 13,  1, 21, 16, 11,         // a ( 97) -> h (104)
         8, 23, 18,  9, 14,  3,  5, 17,         // i (105) -> p (112)
        26,  4,  6,  7, 15, 22, 20, 24,         // q (113) -> x (120)
        19, 25 };                               // y (121) -> z (122)


    char c;
    word totcode = 0;
    short j = 0;
    *count = 0;

    while ((c = (char) (*inp++)) != 0) {
        word code;
        if ((c < 39) || (c > 122))
            code = 0;
        else
            code = unwrk[c-39];
        totcode = (word)((word)(totcode * 40) + code);
        j++;
        if (j == 3) {
            *outp++ = totcode;
            (*count) += 2;
            j = 0;
            totcode = 0;
        }
    }

    if (j != 0) {
        for (; j < 3; j++)
            totcode = (word)(totcode * 40);
        *outp = totcode;
        (*count) += 2;
    }

}


void INPBLK::Open (INPCUR *icp)
{
    if (n->FidoTxt || n->FidoPrn) {
        if (n->FidoTxt)
            icp->nodelist_txt = SmartOpen (n->FidoTxt, "");
        if (n->FidoPrn)
            icp->nodelist_prn = SmartOpen (n->FidoPrn, "");
    }
}


void INPBLK::Close (INPCUR *icp)
{

    if (icp->nodelist_prn)
        fclose (icp->nodelist_prn);
    if (icp->nodelist_txt)
        fclose (icp->nodelist_txt);

}


BOOL INPBLK::InSegment (const EXTADR *adr, ADRLST *cobIncAddr, ADRLST *cobExcAddr, ADRLST *IncAddr, ADRLST *ExcAddr)
{
    if (cobIncAddr)
        if (!InPartAdrLst (adr, cobIncAddr))
            return FALSE;

    if (cobExcAddr)
        if (InPartAdrLst (adr, cobExcAddr))
            return FALSE;

    if (IncAddr)
        if (!InPartAdrLst (adr, IncAddr))
            return FALSE;

    if (ExcAddr)
        if (InPartAdrLst (adr, ExcAddr))
            return FALSE;

    return TRUE;
}



static BOOL WantRem (char *MsgRem, char *rem)
{
    if (!MsgRem)
        return FALSE;           // No MsgRem statement
    if (*rem == '\0')           // Empty Comment
        return FALSE;
    if (*MsgRem == '\0')        // MsgRem with no char list: all comments
        return TRUE;
    if (*rem == ' ')            // "; word" comment
        return (strchr (MsgRem, ';') != NULL);
    if (*(rem+1) != ' ')                    // ";word" comment
        return (strchr (MsgRem, ';') != NULL);
    return (strchr (MsgRem, *rem) != NULL);     // ";<l>" comment
}



                // for GetPhoneType


#define PT_PSTN             0x01
#define PT_Unpublished      0x02
#define PT_Verbatim         0x03
#define PT_FIDOIP           0x04


static int GetPhoneType (pcsz phone)
{
    if (*phone == '\0')
        return PT_Unpublished;

    int numlen = strspn (phone, "-0123456789");
    if (phone[numlen] == '\0') {
        if (strncmp (phone, "000-", 4) == 0)
            return PT_FIDOIP;
        return PT_PSTN;
    }

    if (stricmp (phone, UnpublishedPhone) == 0)
        return PT_Unpublished;

    return PT_Verbatim;
}


CO *MkPhone (pcsz orig_phone, psz phone)
{
    CO *co = NULL;
    DL *dl = dl_head;
    while (dl) {
        char *pp = strdcmp (orig_phone, dl->mstr);
        if (pp) {   // first part match (area code)
            bool match;

            if (dl->Exchange_head) {  // exch to match
                match = false;
                StrChain *ple = dl->Exchange_head;
                do {
                    if (strdcmp (pp, ple->text)) { // is Local
                        match = true;
                        break;
                    }
                    ple = ple->next;
                } while (ple);
            } else                  // no exch to match
                match = true;

            if (match) {
                strzcat (phone, PHONESIZE, dl->pre, pp, dl->post, NULL);
                if (dl->co)
                    co = dl->co;
                break;
            }

        }
        dl = dl->next;
    }

    return co;
}



BOOL INPBLK::ProcessFile (char *fname, INPCUR *icp, OUTCUR *ocp, OUTBLK *cob, BOOL *SomeNeeded)
{
    
    char    PackBuf[LINESIZE];
    byte    buffer[sizeof(_vers7) + LINESIZE];
    _vers7  *vers7 = (_vers7 *) buffer;
                                
    word    official_crc, calc_crc;
    FILE    *f;
    word    n_num;
    word    n_cost, u_cost;
    byte    n_modem;
    dword   n_baud;
    int     coord_lev, cur_addr_lev;
    char    def_phone[PHONESIZE];
    byte    def_modem;
    word    def_cost, def_ucost;
    dword   def_baud;
    word    def_flags;
    char    buff[LINESIZE]; char verbuff[LINESIZE]; char *p, *q, *r;
    char    *modifier, *boardname, *cityname,
            *sysopname; /* no comma */
    char    phone[PHONESIZE], phonendx[PHONESIZE];
    char    sysop_comma[SYSOPSIZE];  // SysOp Name: "LastName[, FirstName]"
    int     pnt, hold;
    int     tmplen;
    ADRDATA *ph, *pw, *ad;

    char *origline; // pointer to original nodelist line


    if (SegExpHead) {
        if (SEGEXPORT::OpenAll (SegExpHead, s->NodeTime, s->NodeDay, fname))
            *SomeNeeded = TRUE;
        origline = new char[LINESIZE];
    }

    BOOL SegmentSel = cob->o->IncAddr || cob->o->ExcAddr || o->IncAddr || o->ExcAddr;


           // initialization for "constant" variables

    char *sysop4ndx = (cob->l->v7data.sysopndx || cob->l->FidoUserLst) ?
                      sysop_comma : NullStr;

    char const *phone4ndx = (cob->l->v7data.flags & V7PDX_F) ? phonendx : NullStr;



           //

    ADRDATA *def_pw = NULL;
    *def_phone = '\0';

    cur_addr_lev = NONE;
    if ((f = fopen (fname, "rb")) == NULL) {
        vprintlog ("Could not open Nodelist File \"%s\"\n", fname);
        myexit (NO_NODELIST);
    }
    setvbuf (f, NULL, _IOFBF, BufSize);

    if (!fgets (buff, LINESIZE, f)) {
        vprintlog ("Could not read Nodelist File \"%s\"\n", fname);
        fclose (f);
        myexit (NO_NODELIST);
    }

    if (strncmp (buff, ";A ", 3) == 0)
        p = strrchr (buff, ':');
    else
        p = NULL;

    if (p) {
        if (v->crcchk) {
            official_crc = (word) atoi (p+1);
            calc_crc = 0;
        }
        strzcpy (icp->header, buff + 3, HeaderSize - 8);
        p = strrchr (icp->header, ':');
        if (!p)
            p = icp->header + strlen (icp->header);
        strcpy (p, "- P");

        if(!fgets (buff, LINESIZE, f)) {
            vprintlog ("Empty Nodelist File \"%s\"\n", fname);
            myexit (NO_NODELIST);
        }
    } else {
        v->crcchk = FALSE;
        strcpy (icp->header, "Private List - P");
    }

    TxtHeader (icp);

    vprintlogrsp (MsgLogRsp, "Processing \"%s\"\n", fname);

    if (n->MsgRem)
        writersp (MsgRemRsp, "\r\rComments in \"%s\"\r\r", fname);

                        /* set initial defaults */
    EXTADR adr = l->PartAddr;
    if ((n->flags & GermanPointLst) && (adr.zone == word(-1)))
        adr.zone = 2;

    do {            /* while fgets */
        if (buff[0] == 0x1A)        // ^Z
            break;
        if (v->crcchk)
            calc_crc = crcstr (buff, calc_crc, &tmplen);
        else
            tmplen = strlen (buff);

        if (tmplen < 2)        // skip empty lines
            continue;

        p = buff + tmplen - 2; // remove trailing CR+LF
        if (*p == '\r')
            *p = '\0';

        strcpy (verbuff, buff); // make verbatim copy of nodelist line

        pnt = 0;
        coord_lev = NONE;
        hold = 0;

        p = buff;

        if (*p == ';') {
            p ++;
            if (n->MsgRem)
                if (WantRem (n->MsgRem, p))
                    writersp (MsgRemRsp, "%s\r", p);
            if ((*p == 'A') || (*p == 'F') || (*p == 'U'))
                TxtLine (icp, p+1);
            continue;
        }

        if (SegExpHead)
            strcpy (origline, p);

        /* Modifier (Host, Region, etc.) */
        modifier = nextfield (p);
        strupr (modifier);

        /* Node number (or region or zone) */
        p = nextfield (NULL);
        n_num = (word) atoi (p);

        if (n_num == 0)
            continue;     // if empty line don't overwrite coord !

        /* Did we get anything? */
        if (*modifier) { /* Is it something we need to worry about? */
            if (strcmp (modifier, "ZONE") == 0) {
                ocp->nzones ++;
                coord_lev = ZONE;
            } else if (strcmp (modifier, "REGION") == 0) {
                if (!(n->flags & GermanPointLst))
                    ocp->nregions ++;
                coord_lev = REGION;
            } else if (strcmp (modifier, "HOST") == 0) {
                if (!(n->flags & GermanPointLst))
                    ocp->nnets ++;
                coord_lev = HOST;
            } else if (strcmp (modifier, "HUB") == 0) {
                ocp->nhubs ++;
                coord_lev = HUB;
            } else if (strcmp (modifier, "BOSS") == 0) {
                /* It is a PointList Boss */
                coord_lev = BOSS;
            } else if (strcmp (modifier, "NODE") == 0) {
                /* It is a node address */
                coord_lev = NODE;
            } else if (strcmp (modifier, "POINT") == 0) {
                pnt = 1;
            } else if (strcmp (modifier, "PVT") == 0) {
                /* It is just a private node */
                *modifier = '\0';
            } else if (strcmp (modifier, "HOLD") == 0) {
                *modifier = '\0';
                hold = 1;
            } else if (strcmp (modifier, "DOWN") == 0) {
                ocp->ndown ++;
                continue;
            } else {
                ocp->nunknown ++;
                continue;
            }
        }

        /* Board name */
        boardname = nextfield (NULL);


        if (coord_lev == NONE) {
            if (cur_addr_lev == BOSS)
                pnt = 1;
            if (!pnt) {
                adr.node = n_num;
                adr.point = 0;
            } else
                adr.point = n_num;
        } else {
            cur_addr_lev = coord_lev;
            adr.point = 0;
            switch (coord_lev) {

                case ZONE:
                    adr.zone = n_num;
                    adr.region = 0;
                    goto HostLabel;

                case REGION:
                    if (n->flags & GermanPointLst)
                        continue;
                    adr.region = n_num;

                case HOST:
   HostLabel:       adr.hub = 0;
                    if (n->flags & GermanPointLst) {
                        get_addr_2d (boardname, &adr);  // get net/node from system name
                        cur_addr_lev = BOSS;
                    } else {
                        adr.net = n_num;
                        adr.node = 0;
                    }
                    printflush ("%5hd:%-5hd\r", adr.zone, adr.net);
                    if (n->flags & GermanPointLst)
                        continue;
                    break;

                case HUB:
                    adr.hub = adr.node = n_num;
                    break;

                case BOSS:
                case NODE:

                    strcpy (def_phone, "");
                    def_flags = 0;
                    def_baud = 300;
                    def_modem = 0;
                    def_cost = CostNullPhone.cost;
                    def_ucost = CostNullPhone.ucost;
                    def_pw = NULL;
                    get_addr ((const char **)&p, &adr);
                    adr.region = adr.hub = 0;

                    if (coord_lev == BOSS) {
                        printflush ("%5hd:%-5hd          \r", adr.zone, adr.net);
                        continue;   /* stop here */
                    } else {     // NODE
                        sscanf (p, "%hu %hu", &adr.region, &adr.hub);
                        coord_lev = NONE;
                        break;
                    }
            }
        } 

        if (adr.point) {
            ocp->npoints ++;
            pnt = 1;
        } else {
            ocp->nnodes ++;
            pnt = 0;
        }

        if (SegExpHead)
            SEGEXPORT::WriteAll (SegExpHead, &adr, origline);

        if (SegmentSel)
            if (!InSegment (&adr, cob->o->IncAddr, cob->o->ExcAddr, o->IncAddr, o->ExcAddr) &&
                (coord_lev < n->IncCoord)) {
                ocp->nexcluded ++;
                continue;
            }

        /* Location */
        cityname = nextfield (NULL);

        /* Sysop name */
        sysopname = nextfield (NULL);

        /* Phone number */
        pcsz orig_phone = nextfield (NULL);

        /* Baud rate */
        p = nextfield (NULL);
        n_baud = atol (p);

        /* Flags */
        char *flags = remainder;     // remainder of current text line

        /* Print out the Nodelist.Txt/Prn file */
        if ((icp->nodelist_txt) || (icp->nodelist_prn))
        if ((cur_addr_lev != NODE) && (cur_addr_lev != BOSS)) {
            icp->txt_pg_lines ++;

            if ((coord_lev >= HOST) && (icp->txt_pg_lines >= 40) ||
                (icp->txt_pg_lines >= 60))
                TxtHeader (icp);

            if (icp->nodelist_txt)
                fprintf (icp->nodelist_txt,
                  "%-7.7s%5hd %-19.19s %-20.20s %-19.19s %5lu\n",
                  modifier, n_num, boardname, orig_phone, cityname, n_baud);

            if (icp->nodelist_prn)
                fprintf (icp->nodelist_prn,
                  "%-7.7s%5hd %-19.19s %-20.20s %-19.19s %-20.20s %5lu %-30.30s\n",
                  modifier, n_num, boardname, orig_phone, cityname, sysopname,
                  n_baud, flags);
        }


        PackBuf[0] = '\0';

     // Process Flags

            // Override the Flags if necessary

        ad = GetData (&adr, icp->tab->nf, o->nfn, ocp->tab->nf, cob->o->nfn);
        if (ad)
            flags = ad->txt;

        strupr (flags);     // Convert FLAGS to upper case

        PT *pt = NULL;      // Phone translation (for Verbatim phones)
        byte tdinfo = 0;
                            // Check Modem Type
        n_modem = 0;
        TD *td = td_head;
        while (td) {
            if (FindFlag (flags, td->flag)) {
                n_modem |= td->type;
                if (!BitType) {
                    pt = td->pt;
                    tdinfo = td->info;
                    break;
                }
            }
            td = td->next;
        }


        word n_flags = 0;           // node flags
                            
        switch (coord_lev) {        // coord flags
            case NONE:
                break;
            case ZONE:
                n_flags |= B_zone;
                break;
            case REGION:
                n_flags |= B_region;
                break;
            case HOST:
                n_flags |= B_host;
                break;
            case HUB:
                n_flags |= B_hub;
                break;
        }

        if (pnt)
            n_flags |= B_point;

        if (FindFlag (flags, "CM"))
            n_flags |= B_CM;

        FD *fd = fd_head;       // user flags from node flags
        while (fd) {
            if (FindFlag (flags, fd->flag))
                n_flags |= fd->flag_w;
            fd = fd->next;
        }

                                // User Flags from address
        ad = GetData (&adr, icp->tab->fl, o->fln, ocp->tab->fl, cob->o->fln);
        if (ad)
            n_flags |= ad->w.w1;


     // Find out a password if present

        pw = GetData (&adr, icp->tab->pw, o->pwn, ocp->tab->pw, cob->o->pwn);


    // Process the phone number

            // Override the phone number if necessary

        if ((cur_addr_lev == BOSS) && (n->flags & NoPointLstPhone))
            orig_phone = UnpublishedPhone;

        ph = GetData (&adr, icp->tab->ph, o->phn, ocp->tab->ph, cob->o->phn);
        if (ph)
            orig_phone = ph->txt;

        int phone_type = GetPhoneType (orig_phone);
        bool undialable = (hold && !ph);

        // process the number and related cost

        switch (phone_type) {

          case PT_FIDOIP:

            strzcpy (phonendx, orig_phone+4, PHONESIZE);
            dot_it (phonendx);
            goto VerbPhone;

          case PT_Verbatim:

            strzcpy (phonendx, orig_phone, PHONESIZE);

VerbPhone:
            if (undialable)
                break;

            if (pt)
                pt->Apply (&n_cost, &u_cost, phone, phonendx, PHONESIZE);
            else {
                strzcpy (phone, phonendx, PHONESIZE);
                n_cost = CostVerbatimPhone.cost;
                u_cost = CostVerbatimPhone.ucost;
            }

                                // cost override on address
            ad = GetData (&adr, icp->tab->cs, o->csn, ocp->tab->cs, cob->o->csn);
            if (ad) {
                n_cost = ad->w.w1;
                u_cost = ad->w.w2;
            }

            break;

          case PT_Unpublished:

            strcpy (phonendx, orig_phone);

            break;

          case PT_PSTN:

            CO *co = MkPhone (orig_phone, phone);

            CopyUndash (phone, phonendx);

            if (undialable)
                break;

            if (dash2comma)
                comma_it (phone);

            ad = GetData (&adr, icp->tab->cs, o->csn, ocp->tab->cs, cob->o->csn);
            if (ad) {                   // cost override
                n_cost = ad->w.w1;
                u_cost = ad->w.w2;
                break;
            }

            if (!co) {
                co = co_head;   // separate cost table
                while (co) {
                    if (strdcmp (orig_phone, co->mstr) != NULL)
                        break;
                    co = co->next;
                }
            }

            if (tdinfo & TD_DIGITAL) {
                n_cost = co->costdig;
                u_cost = co->ucostdig;
            } else {
                n_cost = co->cost;
                u_cost = co->ucost;
            }

            break;  // phone_type == PT_PSTN

        }   // switch (phone_type)


        if (undialable ||
            (phone_type == PT_Unpublished)) {

            phone[0] = '\0';

            if (NoRedir || pnt || pw || def_pw ||
               (*def_phone == '\0') || (coord_lev > NONE)) {  // empty phone

                n_flags &= B_admin;
                n_modem = 0;
                n_cost = CostNullPhone.cost;
                u_cost = CostNullPhone.ucost;

                if (n->MsgLog & MsgLogNullPhone)
                    if ((!pnt) || (n->MsgLog & MsgLogPoints))
                        writersp (MsgLogRsp, "%d:%d/%d.%d Null Phone\r", adr.zone, adr.net, adr.node, adr.point);

                ocp->nnullphone ++;

            } else {           // Redirected (only simple nodes)

                strcpy (PackBuf, "-R-");    // prefix the board name

                                    // use defaults for redirected nodes
                strcpy (phone, def_phone);
                n_flags = def_flags;  // There are no admin flags to save
                n_baud = def_baud;
                n_modem = def_modem;
                n_cost = def_cost;
                u_cost = def_ucost;

                if (n->MsgLog & MsgLogRedirected)
                    writersp (MsgLogRsp, "%d:%d/%d.%d Redirected\r", adr.zone, adr.net, adr.node, adr.point);

                ocp->nredirect ++;
            }
        }


        if (!NoRedir) {

            if (coord_lev > NONE) {    // save defaults for redirected nodes
                strcpy (def_phone, phone);
                def_flags = n_flags & ~B_admin;  // saved without coord flags !
                def_baud = n_baud;
                def_modem = n_modem;
                def_cost = n_cost;
                def_ucost = u_cost;
                def_pw = pw;
            }

        }


    // Put the results into the proper files

        if (sysop4ndx == sysop_comma)
            MkSysopComma (sysopname, sysop4ndx);

        ocp->heap->Write (&adr, ocp->datofs, ocp->dtpofs,
                          sysop4ndx, phone4ndx);

        int error = 0; /* check disk full */

        /* Print out the results in NODEX.DAT format */

        vers7->Zone         = adr.zone;
        vers7->Net          = adr.net;
        vers7->Node         = adr.node;
        vers7->HubNode      = (pnt) ? adr.point : adr.hub;
        vers7->CallCost     = n_cost;
        vers7->MsgFee       = u_cost;
        vers7->NodeFlags    = n_flags;
        vers7->ModemType    = n_modem;
        vers7->BaudRate     = (byte) (n_baud / 300);

        q = (char *) (buffer + sizeof (_vers7));
        p = fl_stpcpy (q, phone);
        vers7->Phone_len = (byte) (p - q);

        /* Stick in a password if there is one */
        if (pw != NULL) {
            r = fl_stpcpy (p, pw->txt);
            vers7->Password_len = (byte) (r - p);
        } else {
            r = p;
            vers7->Password_len = 0;
        }

        p = PackBuf + strlen (PackBuf);
        q = fl_stpcpy (p, boardname);
        vers7->Bname_len = (byte) (q - PackBuf);        /* board name */

        p = fl_stpcpy (q, sysopname);
        vers7->Sname_len = (byte) (p - q);           /* SysOp name */

        q = fl_stpcpy (p, cityname);
        vers7->Cname_len = (byte) (q - p);           /* City name */

        if (cob->l->v7data.flags & V7DTP_F) {       // DTP data

            const _DTPLnk DTPFix = {
               {
                 0x0000,     // Region
                 0x0000,     // Hub
                 OfsNoLink,  // DAT pointer for next same-sysop entry
                 OfsNoLink,  // DAT pointer for next same-phone entry
                 OfsNoLink,  // DAT pointer for next same-level entry
                 0xff,       // number of SysOp entry
                 0xff        // number of Phone entry
               },
               {
                 0xffff,     // systems in lower level
                 OfsNoLink   // DAT pointer for lower level
               }
            };
                             // DTP ptr into Pack
            sprintf (q, "%08X", ocp->dtpofs);

            byte DTPFixSize = pnt ? sizeof (_DTPAllLnk) : sizeof (_DTPLnk);

            error |= (fwrite (&DTPFix, DTPFixSize, 1, ocp->nodex_dtp) != 1);

            word dtpflen = (word) (1 + strlen (verbuff));  // raw nodelist line
            error |= (fwrite (&dtpflen, sizeof (dtpflen), 1, ocp->nodex_dtp) != 1);
            error |= (fwrite (verbuff, dtpflen, 1, ocp->nodex_dtp) != 1);

            size_t dtpelen = DTPFixSize + sizeof (dtpflen) + dtpflen;
            ocp->dtpofs += dtpelen;

        }

        Pack ((word *)r, PackBuf, &(vers7->pack_len));

        size_t entrylen = sizeof (_vers7) + vers7->Phone_len +
                                vers7->Password_len + vers7->pack_len;
        error |= (fwrite (buffer, entrylen, 1, ocp->nodex_dat) != 1);

        ocp->datofs += entrylen;

        if (error || Break) {

            ftrunczero (ocp->nodex_dat);

            if (cob->l->v7data.flags & V7DTP_F)
                ftrunczero (ocp->nodex_dtp);

            if (error) {
                vprintlog ("\n\nDisk Full writing nodelist files !\n\n");
                myexit (DISK_FULL);
            } else
                myexit (USER_BREAK);
        }

    } while (fgets (buff, LINESIZE, f));

    fclose (f);

    if (SegExpHead) {
        delete[] origline;
        SEGEXPORT::CloseAll (SegExpHead);
    }

    if (v->crcchk)
        if (official_crc != calc_crc) {
            vprintlog ("\n\nCRC ERROR: actual/expected CRC: %05hu/%05hu !\n\n", calc_crc, official_crc);
            errorlevel = CRC_ON_LIST;
            if (!nocrcexit)
                return FALSE;
        }

    return TRUE;
}


void INPBLK::KillSource (void)
{
    InpncBlk::KillSource ();
    SEGEXPORT::KillSource (SegExpHead);
}


void InpncBlk::KillSource (void)
{
    if (l->ArcList && (l->ArcListKeep > 0) && (!l->NodeDiff || a->ArcMethHead))
        DeleteFile (l->NodeList, CHK_EXIST);

    if (l->ArcDiff && a->ArcDiffMethHead) {
        DAYDIR DayDir (NODEDIFF, this);
        DayDir.KillAll ();
    }
}

