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
#include "inmem.hpp"
#include "misc.hpp"
#include "data.hpp"

#include <string.h>

static void DelBlocks (DBLK *first)
{
    DBLK *db = first;
    while (db) {
        DBLK *nb = db->next;
        delete db;
        db = nb;
    }
}

InMem::InMem ()
{
    memset (this, 0, sizeof (*this));
}


InMem::~InMem ()
{
    if (np)
        delete[] np;

    if (lp)
        delete[] lp;

    DelBlocks (FirstBlk);
}


void InMem::Open ()
{
    FirstBlk = new DBLK;    // alloc first data block
    FirstBlk->next = NULL;

    CBlk = FirstBlk;
    cofs = 0;
}


void InMem::Write (const EXTADR *adr, dword datofs, dword dtpofs,
                   pcsz SysOp, pcsz Phone)
{
    int sysopsize = strlen (SysOp) + 1;
    int phonelen = strlen (Phone);

    int recsize = sizeof (Node) + sysopsize + phonelen;

    // SysOp and Phone MUST be length limited to remain in 255

    if (cofs + recsize >= DATABLKSIZE) {    // alloc new data block
        CBlk = CBlk->next = new DBLK;       // (one byte for end-mark)
        CBlk->next = NULL;
        cofs = 0;
    }

    NODEP node = (NODEP) (CBlk->data + cofs);

    node->recsize  = byte (recsize);
    node->namesize = byte (sysopsize);
    node->adr      = *adr;
    node->datofs   = datofs;
    node->dtpofs   = dtpofs;
    node->entryn   = NStored++;
    strcpy (node->name, SysOp);
    strcpy (NPhone (node), Phone);

    cofs += recsize;
    CBlk->data[cofs] = 0;   // mark last entry
}


void InMem::Close ()        // build unified block of pointers
{
    if (NStored == 0)
        return;

    np = new NODEP[NStored];
    NODEP *cp = np;

    DBLK *db = FirstBlk;
    while (db) {
        NODEP p = (NODEP) db->data;
        while (p->recsize) {
            *(cp++) = p;
            p = (NODEP) (((byte *) p) + p->recsize);
        }
        db = db->next;
    }
}


static int AdrOfsCmp (NODEP *p1, NODEP *p2)
{
    int result = (*p1)->adr.zone - (*p2)->adr.zone;
    if (result != 0)
        return result;

    result = (*p1)->adr.net - (*p2)->adr.net;
    if (result != 0)
        return result;

    result = (*p1)->adr.node - (*p2)->adr.node;
    if (result != 0)
        return result;

    result = (*p1)->adr.point - (*p2)->adr.point;
    if (result != 0)
        return result;

    if ((*p1)->datofs > (*p2)->datofs)  // lowest ofs will be first
        return 1;
    else if ((*p1)->datofs < (*p2)->datofs)
        return -1;
    else
        return 0;
}


static int AdrCmp (NODEP p1, NODEP p2)
{
    int result = p1->adr.zone - p2->adr.zone;
    if (result != 0)
        return result;

    result = p1->adr.net - p2->adr.net;
    if (result != 0)
        return result;

    result = p1->adr.node - p2->adr.node;
    if (result != 0)
        return result;

    return (p1->adr.point - p2->adr.point);
}


// Compares zone:net/node.point and sets Region/Hub in p1 from p2

#define AVM_Diff 0x00
#define AVM_Dupe 0x01
#define AVM_Inv  0x02


static int AdrValidMatch (NODEP p1, NODEP p2)
{
    if (p1->adr.zone != p2->adr.zone)
        return AVM_Diff;

    if (p1->adr.net != p2->adr.net)
        return AVM_Diff;

    // same zone:net -> match region

    if (p1->adr.region == 0)
        p1->adr.region = p2->adr.region;

    if (p1->adr.node != p2->adr.node)
        return p1->adr.point ? AVM_Inv : AVM_Diff;    // remove points wo boss

    // same zone:net/node -> match hub

    if (p1->adr.hub == 0)
        p1->adr.hub = p2->adr.hub;

    if (p1->adr.point != p2->adr.point)
        return AVM_Diff;

    return AVM_Dupe;
}


dword InMem::RemoveDupes ()
{
    long i,     // current item
         j;     // last valid item

    for (i = 1, j = 0; i < NStored; i++) {
        switch (AdrValidMatch (np[i], np[j])) {  // check item i with last valid
            case AVM_Diff:
                j ++;
            case AVM_Dupe:
                np[j] = np[i];
        }
    }

    return j+1;
}


dword InMem::MkNodeNdx (pcsz NodexNdx)
{
    qsort (np, NStored, sizeof (NODEP), (QSF) AdrOfsCmp);
    TotN = RemoveDupes ();
    mkndx7 (_Adr_Indx, NodexNdx);
    return TotN;
}


#define INode(a) ((_INodeBlk *)a)

int InMem::loadblk (int Select, byte *ndxrec, byte *strbuf, uint *stringlen,
                    long *prevblk, long hiblk, NFILE *inp, NFILE *tmpout)
{                                           // *prevblk == 0 -> leaf block
    static NODEP entry;

    memset (ndxrec, 0, ndx7recsz);

    INode(ndxrec)->IndxFirst = (*prevblk) ? ((*prevblk)++) : -1; // ptr to next lower level
    INode(ndxrec)->IndxBLink = (inp->c == 0) ? 0 : hiblk; // previous link

    bool done = FALSE;
    uint count = 0;
    uint headlen = 16;   /* blk header size */
    *stringlen = 0;
    word *wptr = (word *)(ndxrec + headlen);

    uint entryhdlen = (*prevblk) ? 12 : 8;

    while (1) {  // skip on first entry of 2nd and subsequent blocks of leaf level
        if ((count > 0) || (*prevblk != 0) || (hiblk == 0)) {   
            if (inp->c >= inp->n) {
                if ((count > 0) || (!V7BugFix)) {
                    done = TRUE;
                    break;
                } else
                    (*prevblk) --;
            } else
                entry = inp->bp[inp->c ++];
        }

        byte *entrystring;
        uint entrystlen;

        switch (Select) {
            case _Adr_Indx:
                entrystlen = entry->adr.point ? 8 : 6;
                entrystring = (byte *) &entry->adr;
                break;
            case _Sys_Indx:
                entrystlen = entry->namesize - 1;
                entrystring = (byte *) entry->name;
                break;
            case _Pho_Indx:
                entrystlen = entry->recsize - entry->namesize - sizeof (Node);
                entrystring = (byte *) NPhone (entry);
                break;
        }

        if ((headlen + *stringlen + entryhdlen + entrystlen) > ndx7recsz) {
            tmpout->bp[tmpout->n ++] = entry;
            break;
        }
        *wptr++ = (word) *stringlen;  /* offset of string into block */
        *wptr++ = (word) entrystlen;  /* length of string */
        *((long *)wptr) = entry->datofs; /* Pointer to data block */
        wptr += 2;
        if (*prevblk != 0) {  /* Index level, not leaf */
            *((long *)wptr) = (*prevblk)++;
            wptr += 2;
        }

        memmove (strbuf + *stringlen, entrystring, entrystlen); // copy string
        count++;
        headlen += entryhdlen;
        *stringlen += entrystlen;
    };

    INode(ndxrec)->IndxFLink = done ? 0 : hiblk + 2;  /* next link */
    INode(ndxrec)->IndxCnt = (short) count;

    wptr = (word *) (ndxrec + 16);  // points to offset of first string
    word sshift = (word)(ndx7recsz - *stringlen);

    for (uint i = 0; i < count; i++) {
        *wptr += sshift;
        wptr = (word *) (((byte *)wptr) + entryhdlen);
    }

    INode(ndxrec)->IndxStr = *((word *)(ndxrec + 16));
    
    return done;
}


static void WriteError (FILE *f, pcsz name)
{
    ftrunczero (f);
    vprintlog ("\nError writing \"%s\"\n\n", name);
    myexit (ERR_UPDATE);
}


void InMem::mkndx7 (int Select, pcsz OutName)
{                                       /* varstr == 1 -> key is ASCIIZ */
    NFILE nfil[2];
    NFILE *inp = &nfil[0],
          *tmpout = &nfil[1];
    byte ndxrec[ndx7recsz], strbuf[ndx7recsz];

    FILE *outfile = fopen (OutName, "wb");

    memset (ndxrec, 0, ndx7recsz);
                                                 // write temp control block
    if (fwrite (ndxrec, ndx7recsz, 1, outfile) != 1) 
        DiskFull (outfile);

                                // write leaf nodes, then index nodes
    word idxlev = 0;
    long hiblk = 0;             // blocks total
    long prevblk = 0;           // begin of last index level

    inp->bp = np;  // First of all, take the cleaned (no dupes) array as input
    inp->n  = TotN;

    long levblk;           // this level
    long lastleaf;

    do {
        inp->c = 0;
        tmpout->bp = new NODEP[inp->n/2+1]; // at least 2 entries per block !
        tmpout->n = 0;
        levblk = 0;
        bool done;
        do {
            uint stringlen;
            done = loadblk (Select, ndxrec, strbuf, &stringlen, &prevblk, hiblk, inp, tmpout);
            int error = (fwrite (ndxrec, ndx7recsz - stringlen, 1, outfile) != 1); // write index
            if (stringlen)
                error |= (fwrite (strbuf, stringlen, 1, outfile) != 1);
            if (error)
                DiskFull (outfile);
            hiblk++;
            levblk++;
            if (prevblk == 0)
                lastleaf = hiblk;
        } while (!done);

        if (idxlev > 0)
            delete[] inp->bp;    // delete previous level array
  #ifdef __DOS__
        else
            MinMemFree = __min (MinMemFree, MemFree ());
  #endif

        NFILE *tf = inp;         // switch inp and tmpout
        inp = tmpout;
        tmpout = tf;

        if (prevblk == 0)
            prevblk = 1;

        idxlev++;
    } while (levblk > 1);  /* other index levels to do */

    delete[] inp->bp;

        /* write control record */

    _CtlBlk *ctl = (_CtlBlk *) ndxrec;

    ctl->CtlBlkSize = 0x200;
    ctl->CtlRoot = hiblk;
    ctl->CtlHiBlk = hiblk;
    ctl->CtlLoLeaf = 1;
    ctl->CtlHiLeaf = lastleaf;
    ctl->CtlFree = 0;
    ctl->CtlLvls = idxlev;

    word parity = 0;
    word *ptr = (word *) ctl;
    for (int i = 0; i < (sizeof (_CtlBlk) - 2) / 2; i++, ptr++)
        parity ^= *ptr;

    ctl->CtlParity = parity;

    int error = 0;
    error |= fseek (outfile, 0, SEEK_SET);
    error |= (fwrite (ctl, sizeof (_CtlBlk), 1, outfile) != 1); // write temp control block

    if (error)
        WriteError (outfile, OutName);

    if (fclose (outfile) == EOF)
        DiskFull (outfile);
}


#define lpi(a) (np[a]->entryn)      // np index to lp index


dword InMem::Link (int Select)
{
    if ((TotN == 0) || (!lp))
        return 0;

    dword i;

    bool ingroup = false;
    dword nunique = 1;  // number of unique entries

    for (i = 1; i < TotN; i ++) {

        dword first;        // first entry in same-phone ring
        byte  n;            // number of entry in same-phone ring

        char *p1, *p2;
        byte *pn;
        dword *pofs;
        dword li = lpi (i-1);

        switch (Select) {

            case _Sys_Indx:
                p1   = np[i-1]->name;
                p2   = np[i]->name;
                pn   = &lp[li].dtplnk.A.Sn;
                pofs = &lp[li].dtplnk.A.SOfs;
                break;

            case _Pho_Indx:
                p1   = NPhone (np[i-1]);
                p2   = NPhone (np[i]);
                pn   = &lp[li].dtplnk.A.Pn;
                pofs = &lp[li].dtplnk.A.POfs;
                break;
        }

        if (stricmp (p1, p2) == 0) {

            if (!ingroup) {              // first entry in ring
                ingroup = true;
                first = i-1;
                n = 0;
            }

            *pofs = np[i]->datofs;
            *pn = n++;

        } else {

            nunique ++;

            if (ingroup) {              // last entry in ring
                ingroup = false;
                *pofs = np[first]->datofs;
                *pn = n;
            }
        }

    }


    return nunique;
}



void InMem::DTPLnkOpen ()
{
    lp = new Lnk[NStored];

    memset (lp, 0xff, NStored * sizeof (Lnk));      // unindexed entries will
    for (dword i = 0; i < TotN; i ++) {             // have dtpofs==OfsNoLink
        Lnk *l = &lp[np[i]->entryn];
        l->dtpofs          = np[i]->dtpofs;
        l->dtplnk.A.Region = np[i]->adr.region;
        l->dtplnk.A.Hub    = np[i]->adr.hub;
    }

    dtpTopLnk.ndowns = 0;
    dtpTopLnk.FlOfs = OfsNoLink;
}


static int FidoCmp (NODEP *p1, NODEP *p2)
{
    int result = (*p1)->adr.zone - (*p2)->adr.zone;
    if (result != 0)
        return result;

    result = (*p1)->adr.region - (*p2)->adr.region;
    if (result != 0)
        return result;

    result = (*p1)->adr.net - (*p2)->adr.net;
    if (result != 0)
        return result;

    result = (*p1)->adr.hub - (*p2)->adr.hub;
    if (result != 0)
        return result;

    result = (*p1)->adr.node - (*p2)->adr.node;
    if (result != 0)
        return result;

    return (*p1)->adr.point - (*p2)->adr.point;
}



// for clevel:

#define cl_point    0x01
#define cl_node     0x02
#define cl_hub      0x03
#define cl_host     0x04
#define cl_region   0x05
#define cl_zone     0x06

// for GetSeg

#define _SegNode    0x01
#define _SegHub     0x02
#define _SegNet     0x03
#define _SegRegion  0x04
#define _SegZone    0x05



static bool SameSeg (int SegType, EXTADR *a, EXTADR *b)
{
    switch (SegType) {
        case _SegNode:
            return ((a->zone == b->zone) && (a->net == b->net) && (a->node == b->node));
        case _SegHub:
            return ((a->zone == b->zone) && (a->net == b->net) && (a->hub == b->hub));
        case _SegNet:
            return ((a->zone == b->zone) && (a->net == b->net));
        case _SegRegion:
            return ((a->zone == b->zone) && (a->region == b->region));
        case _SegZone:
            return (a->zone == b->zone);
    }

    return false;       // dummy
}


static int clevel (const EXTADR *adr)
{
    if (adr->point)
        return cl_point;

    if ((adr->zone == adr->net) && (adr->node == 0))
        return cl_zone;

    if ((adr->region == adr->net) && (adr->node == 0))
        return cl_region;

    if (adr->node == 0)
        return cl_host;

    if (adr->node == adr->hub)
        return cl_hub;

    return cl_node;
}



dword InMem::EatDownSeg (dword i)
{
    switch (clevel (&np[i]->adr)) {

        case cl_point:
            i++;
            break;

        case cl_node:
            i = GetSeg (_SegNode, i);
            break;

        case cl_hub:
            i = GetSeg (_SegHub, i);
            break;

        case cl_host:
            i = GetSeg (_SegNet, i);
            break;

        case cl_region:
            i = GetSeg (_SegRegion, i);
            break;

        case cl_zone:
            i = GetSeg (_SegZone, i);
            break;
    }

    return i;
}



dword InMem::GetSeg (int SegType, dword i)
{
    dword iseg = i;              // start of segment
    EXTADR *SegAdr = &np[iseg]->adr;

    if (SegType == _SegNode)
        i ++;
    else
        i = GetSeg (_SegNode, i);       // get Coordinator and his points

    dword lastdown = i - 1;
    word ndowns = word (lastdown - iseg);

    while (i < TotN) {
        if (!SameSeg (SegType, SegAdr, &np[i]->adr))
            break;

        if (ndowns == 0)
            lp[lpi(iseg)].dtplnk.N.FlOfs = np[i]->datofs;
        else
            lp[lpi(lastdown)].dtplnk.A.FeOfs = np[i]->datofs;

        lastdown = i;
        ndowns ++;

        i = EatDownSeg (i);
    }

    lp[lpi(iseg)].dtplnk.N.ndowns = ndowns;

    return i;
}



void InMem::FidoLnk ()
{
    qsort (np, TotN, sizeof (NODEP), (QSF) FidoCmp); // sort by EXTADR

    dword i = 0;
    dword lastdown;

    while (i < TotN) {

        if (dtpTopLnk.ndowns == 0)
            dtpTopLnk.FlOfs = np[i]->datofs;
        else
            lp[lpi(lastdown)].dtplnk.A.FeOfs = np[i]->datofs;

        lastdown = i;
        dtpTopLnk.ndowns ++;

        i = EatDownSeg (i);
    }
}



static int PhoneAdrCmp (NODEP *p1, NODEP *p2)
{
    int result = stricmp (NPhone (*p1), NPhone (*p2));  // index must be NOT case sensitive
    if (result != 0)
        return result;

            /* if equal Phones, use Address */
    return AdrCmp (*p1, *p2);
}



dword InMem::MkPhLst (pcsz NodexPdx)
{
    qsort (np, TotN, sizeof (NODEP), (QSF) PhoneAdrCmp);

    mkndx7 (_Pho_Indx, NodexPdx);

    return Link (_Pho_Indx);
}



static int SysAdrCmp (NODEP *p1, NODEP *p2)
{
    int result = stricmp ((*p1)->name, (*p2)->name);  // index must be NOT case sensitive
    if (result != 0)
        return result;

            /* if equal names, use Address */
    return AdrCmp (*p1, *p2);
}


int InMem::WriteName (NODEP n, FILE *fidouser)
{
    char address[28];

    char *ptr = address;
    sprintf (ptr, "%hd:%hd/%hd", n->adr.zone, n->adr.net, n->adr.node);

    if (n->adr.point) {
        ptr += strlen (ptr);
        sprintf (ptr, ".%hd", n->adr.point);
    }

    int namelen =  59 - strlen (address);

    if (fprintf (fidouser, "%-*.*s %s\n", namelen, namelen,
                                          n->name, address) == EOF)
        return 1;

    return 0;
}


void InMem::WriteFidoUserLst (pcsz FidoUserLst)
{
    FILE *fidouser = fopen (FidoUserLst, "wt");

    for (dword i = 0; i < TotN; i ++)
        if (WriteName (np[i], fidouser))
            DiskFull (fidouser);

    if (fclose (fidouser))
        DiskFull (fidouser);
}



dword InMem::MkSysLst (pcsz NodexSdx, pcsz FidoUserLst)
{
    qsort (np, TotN, sizeof (NODEP), (QSF) SysAdrCmp);

    if (FidoUserLst)
        WriteFidoUserLst (FidoUserLst);

    if (NodexSdx)
        mkndx7 (_Sys_Indx, NodexSdx);

    dword nsysops = 0;

    if (lp)
        nsysops = Link (_Sys_Indx);

    return nsysops;
}


static int DTPUpdateOnDisk (FILE *dtpf,                 // DTP stream
                            const _DTPNodeLnk *DtpTop,  // Top Link
                            const Lnk *lp, dword N      // Other links
                           )
{
    int error = 0;

    error |= fseek (dtpf, sizeof (_DTPCtl), SEEK_SET);
    error |= (fwrite (DtpTop, sizeof (_DTPNodeLnk), 1, dtpf) != 1);

    dword hund = N/100;
    printflush (": ");

    for (int i = 0; i < N; i ++) {

        if (i % hund == 0) {
            printflush ("%02d%%\b\b\b", i * 100 / N);
            fflush (stdout);
        }

        if (lp[i].dtpofs == OfsNoLink)
            continue;

        error |= fseek (dtpf, lp[i].dtpofs, SEEK_SET);  // start of DTP entry

        if (lp[i].dtplnk.N.ndowns == 0xffff)       // point
          error |= (fwrite (&lp[i].dtplnk, sizeof (_DTPAllLnk), 1, dtpf) != 1);
        else      // non-point
          error |= (fwrite (&lp[i].dtplnk, sizeof (_DTPLnk), 1, dtpf) != 1);
    }

    printflush ("\b\b     ");

    return error;
}



// 0 on success, 1 on file error, -1 on out of memory

static int DTPUpdateInMem  (FILE *dtpf,                 // DTP stream
                            const _DTPNodeLnk *DtpTop,  // Top Link
                            const Lnk *lp, dword N      // Other links
                           )
{
    fseek (dtpf, 0, SEEK_END);
    dword fsize = ftell (dtpf);

    byte *buffer = (byte *) malloc (fsize);
    if (!buffer)
        return -1;

    int error = 0;
    int i;

    error |= (fread (buffer, fsize, 1, dtpf) != 1);
    if (error) goto exitpnt;

    ((_DTPHead *)buffer)->lnk = *DtpTop;

    for (i = 0; i < N; i ++) {
        if (lp[i].dtpofs == OfsNoLink)
            continue;
        if (lp[i].dtplnk.N.ndowns == 0xffff)  // point
            *(_DTPAllLnk *)(buffer+lp[i].dtpofs) = lp[i].dtplnk.A;
        else  // non-point
            *(_DTPLnk *)(buffer+lp[i].dtpofs) = lp[i].dtplnk;
    }

    rewind (dtpf);
    error |= (fwrite (buffer, fsize, 1, dtpf) != 1);

 exitpnt:
    free (buffer);

    if (error)
        return 1;
    return 0;
}


void InMem::DTPLnkClose (pcsz NodexDtp, bool OnDisk)
{
    delete[] np;
    np = NULL;
    DelBlocks (FirstBlk);
    FirstBlk = NULL;

    FILE *nodexdtp = fopen (NodexDtp, "rb+");
    if (nodexdtp == NULL)
      {
         perror ("fopen");
         return;  
      }

    int error = 0;

    if (!OnDisk) {
        vwritelog ("Linking DTP in Memory");
        error = DTPUpdateInMem (nodexdtp, &dtpTopLnk, lp, NStored);
        if (error == -1)
            vwritelog ("Insufficient Memory");
    }

    if (OnDisk || (error == -1)) {
        vwritelog ("Linking DTP On Disk\n");
        error = DTPUpdateOnDisk (nodexdtp, &dtpTopLnk, lp, NStored);
        if (error)
          vwritelog ("Error while updating DTP");
    }

    error = fclose (nodexdtp);
    if (error)
      perror ("fclose");   
}
