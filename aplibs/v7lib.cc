/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1991-1996  Alberto Pasquale                 */
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

#include "v7lib.hpp"
#include "v7.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <apgenlib.hpp>



int v7ADRcmp (const v7ADR *key, const v7ADR *desired, int len)
{
   int k;

   k = key->zone - desired->zone;
   if (k)
      return k;

   k = key->net - desired->net;
   if (k)
      return k;

   k = key->node - desired->node;
   if (k)
      return k;

   if (len == 6)
      return (- desired->point);
   else
      return (key->point - desired->point);
}

                                                        // private
int v7lib::get7blk (long nblk)
{
    lseek (ndxh, nblk * sizeof (_ndx), SEEK_SET);

    if (read (ndxh, nblk ? nodeidx : ctlblk, sizeof (_ndx)) != sizeof (_ndx))
        return -1;

    return 0;
}

                                                 // private
int v7lib::get7rec (_vers7 *v7rec)
{
    nextdatofs = nodeidx->LNodeBlk.LeafRef[nentry].KeyVal;
    return getnextdat (v7rec);
}

                                                // private
int v7lib::btree (const v7ADR *desired)    // -1 adr not found
{                                           // -2 read error
    long nblk = ctlblk->CtlBlk.CtlRoot;

    /* Read the first Index node. */

    if (get7blk (nblk))
        return -2;

   // Follow the node tree until we locate the leaf node with the entry.

    while (nodeidx->INodeBlk.IndxFirst != -1) {
       int j;
       for (j = 0; j < nodeidx->INodeBlk.IndxCnt; j++) {
          _IndxRef *ip = (_IndxRef *)&(nodeidx->INodeBlk.IndxRef[j]);
          int k = v7ADRcmp ((v7ADR *) ((byte *)nodeidx + ip->IndxOfs),
                                                        desired, ip->IndxLen);
          if (k > 0)
             break;

          if (k == 0) {
             j++;
             break;
          }
       }

       if (j == 0)
          nblk = nodeidx->INodeBlk.IndxFirst;
       else
          nblk = nodeidx->INodeBlk.IndxRef[--j].IndxPtr;

       if (get7blk (nblk))
          return -2;
    }

    /* We have found the leaf which must contain our entry. */

    for (int j = 0; j < nodeidx->LNodeBlk.IndxCnt; j++) {
       _LeafRef *lp = &(nodeidx->LNodeBlk.LeafRef[j]);
       int k = v7ADRcmp ((v7ADR *) ((byte *)nodeidx + lp->KeyOfs),
                                                     desired, lp->KeyLen);
       if (k > 0)
          break;

       if (k == 0)
          return j;
    }

    return -1;
}



                                // private
int v7lib::getnext ()
{
    if (nentry < 0)
        return -1;

    nentry ++;

    if (nentry < nodeidx->LNodeBlk.IndxCnt)
        return nentry;

    // get next leaf

    long nblk = nodeidx->LNodeBlk.IndxFLink;
    if (!nblk)                  // no more leaves
        return -1;

    if (get7blk (nblk))
        return -2;

    if (nodeidx->LNodeBlk.IndxCnt <= 0)
        return -1;

    nentry = 0;

    return nentry;
}


v7lib::v7lib ()
{
    dath = ndxh = -1;
    v7bsy = NULL;
    nentry = -1;
    nextdatofs = 0L;
    nodeidx = new _ndx;
    ctlblk = new _ndx;
}


v7lib::~v7lib ()
{
    if (v7bsy)
        v7close ();
    delete nodeidx;
    delete ctlblk;
}


int v7lib::v7open (const char *nodex)
{
    if (v7bsy)
        v7close ();

    v7bsy = new Busy (nodex);
    if (v7bsy->Wait (60, BSY_SHARE)) {
        v7close ();
        return -1;
    }

    char fullname[PATH_MAX];

    sprintf (fullname, "%s.DAT", nodex);
    dath = open (fullname, O_RDONLY|O_BINARY);
    if (dath == -1) {
        v7close ();
        return -1;
    }

    sprintf (fullname, "%s.NDX", nodex);
    ndxh = open (fullname, O_RDONLY|O_BINARY);
    if (ndxh == -1) {
        v7close ();
        return -1;
    }
                                // Get CtlBlk
    if (get7blk (0L)) {
        v7close ();
        return -1;
    }

    if (ctlblk->CtlBlk.CtlBlkSize != sizeof (_ndx)) {
        v7close ();
        return -1;
    }

    nentry = -1;
    nextdatofs = 0L;

    return 0;
}


void v7lib::v7close ()
{
    if (dath != -1) {
        close (dath);
        dath = -1;
    }
    if (ndxh != -1) {
        close (ndxh);
        ndxh = -1;
    }
    if (v7bsy) {
        delete v7bsy;
        v7bsy = NULL;
    }
}


int v7lib::get (const v7ADR *adr, _vers7 *v7rec)
{
    if (!v7bsy)
        return -2;

    if (adr) 
        nentry = btree (adr);
    else
        nentry = getnext ();

    if (nentry < 0)
        return nentry;

    if (v7rec)
        get7rec (v7rec);

    return 0;
}
             

int v7lib::getfirst (_vers7 *v7rec)
{
    if (!v7bsy)
        return -2;
                                // load first leaf
    if (get7blk (ctlblk->CtlBlk.CtlLoLeaf))
        return -2;

    if (nodeidx->LNodeBlk.IndxCnt <= 0)
        return -2;

    nentry = 0;

    if (v7rec)
        get7rec (v7rec);

    return 0;
}


int v7lib::getfirstdat (_vers7 *v7rec) // get 1st entry in DAT
{
    nextdatofs = 0L;
    return getnextdat (v7rec);
}


int v7lib::getnextdat (_vers7 *v7rec) // get next entry in DAT
{
    if (lseek (dath, nextdatofs, SEEK_SET) == -1)     /* point at record */
        return -2;

    if (read (dath, v7rec, sizeof (_vers7)) != sizeof (_vers7))
        return -2;

    nextdatofs += sizeof (_vers7) + v7rec->Phone_len + v7rec->Password_len +
                                    v7rec->pack_len;

    return 0;
}
