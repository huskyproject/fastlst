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

#include "types.hpp"
#include "cfgdata.hpp"

//#pragma pack (1)    // necessary for (packed) data blocks !

#define _Adr_Indx 1     // for mkndx7 and loadblk
#define _Sys_Indx 2
#define _Pho_Indx 3

#define DATABLKSIZE 100000

#define NPhone(a) ((a)->name+(a)->namesize)

struct Node {
    byte recsize;
    byte namesize; // length of SysOp Name including terminating NULL
    EXTADR adr;
    dword datofs;
    dword dtpofs;
    dword entryn;   // number of entry, 0 based
    char name[1];
};

typedef Node * NODEP;

struct DBLK {
    DBLK *next;
    byte data[DATABLKSIZE];
};


struct NFILE {
    NODEP *bp;  // pointer to first element in array
    dword n,    // total number of elements
          c;    // index to current element
};


struct Lnk {       // To Link SysOps, Phones, fido structure
    dword dtpofs;        // Offset of DTP to be written
    _DTPLnk dtplnk;      // syslnk to be written at dtpofs
};


class InMem {
    private:

        dword NStored;  // Total number of stored entries.
        dword TotN;     // total number of unique entries (after MkNodeNdx)
        DBLK *FirstBlk; // pointer to first data block
        DBLK *CBlk;     // pointer to current data block
        dword cofs;     // current offset in current data block

        NODEP *np;      // Points to array of pointers to nodes
        Lnk *lp;        // Points to array of Link-info blocks
        _DTPNodeLnk dtpTopLnk;  // Top level link-info

        void WriteFidoUserLst (pcsz FidoUserLst);
        void mkndx7 (int Select, pcsz OutName);
        int  loadblk (int Select, byte *ndxrec, byte *strbuf, uint *stringlen,
                      long *prevblk, long hiblk, NFILE *inp, NFILE *tmpout);
        int WriteName (NODEP *n, FILE *fidouser); // 0 on success
        dword RemoveDupes ();
        int WriteName (NODEP n, FILE *fidouser);
        dword EatDownSeg (dword i);
        dword GetSeg (int SegType, dword i);
        dword Link (int Select);


    public:

        InMem ();
        ~InMem ();

        void Open ();

        void Write (const EXTADR *adr,
                    dword datofs,
                    dword dtpofs,
                    pcsz SysOp,      // Empty string if not used
                    pcsz Phone);     // Empty string if not used

        void Close ();               // Build list of pointers


        dword MkNodeNdx (pcsz NodexNdx); // removes duplicates and
                                        // points with no Boss.
                                        // Creates nodex.ndx index.
                                        // Returns the number
                                        // of unique entries.

                                        // MkSysLst, MkPhLst and FidoLnk
                                        // MUST be preceded by MkNodeNdx.

        void DTPLnkOpen ();

        void FidoLnk ();                 // Set Fido Links

        dword MkPhLst (pcsz NodexPdx);      // Make Phone Index,
                                            // Set Phone Links,
                                            // Return Unique Phones

        dword MkSysLst (pcsz NodexSdx,      // NULL if not to be used
                        pcsz FidoUserLst    // NULL if not to be used
                       );                   // Make SysOp Index(es),
                                            // Set SysOp Links,
                                            // Return Unique SysOps.

        void DTPLnkClose (pcsz NodexDtp,    // Update DTP with stored links
                          bool OnDisk);     // Must work on Disk ?
};

