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

#ifndef V7_HPP
#define V7_HPP

extern "C" {
typedef struct {
    word Zone;
    word Net;
    word Node;
    word HubNode;
    word CallCost;         /* phone company's charge */
    word MsgFee;           /* Amount charged to user for a message */
    word NodeFlags;        /* set of flags (see below) */
    byte ModemType;        /* RESERVED for modem type */
    byte Phone_len;
    byte Password_len;
    byte Bname_len;
    byte Sname_len;
    byte Cname_len;
    byte pack_len;
    byte BaudRate;         /* baud rate divided by 300 */
} _vers7 __attribute__ ((packed));

/* defines for NodeFlags word */

#define B_hub      0x0001  /* node is a net hub       0000 0000 0000 0001 */
#define B_host     0x0002  /* node is a net host      0000 0000 0000 0010 */
#define B_region   0x0004  /* node is region coord    0000 0000 0000 0100 */
#define B_zone     0x0008  /* node is a zone coord    0000 0000 0000 1000 */
#define B_CM       0x0010  /* runs continuous mail    0000 0000 0001 0000 */
#define B_ores1    0x0020  /* reserved for Opus       0000 0000 0010 0000 */
#define B_ores2    0x0040  /* reserved for Opus       0000 0000 0100 0000 */
#define B_ores3    0x0080  /* reserved for Opus       0000 0000 1000 0000 */
#define B_ores4    0x0100  /* reserved for Opus       0000 0001 0000 0000 */
#define B_ores5    0x0200  /* reserved for Opus       0000 0010 0000 0000 */
#define B_res1     0x0400  /* reserved for non-Opus   0000 0100 0000 0000 */
#define B_res2     0x0800  /* reserved for non-Opus   0000 1000 0000 0000 */
#define B_point    0x1000  /* node is a point         0001 0000 0000 0000 */
#define B_res3     0x2000  /* reserved for non-Opus   0010 0000 0000 0000 */
#define B_res4     0x4000  /* reserved for non-Opus   0100 0000 0000 0000 */
#define B_res5     0x8000  /* reserved for non-Opus   1000 0000 0000 0000 */


#define B_admin    (B_hub|B_host|B_region|B_zone|B_point)


#define ndx7recsz 512       // for mkndx7 functions

typedef struct {   /* structure for temporary NODEXNDX.$$$ */
    ADR adr;
    dword datofs;
} _ndx7s;

/*--------------------------------------------------------------------------*/
/* nodex.ndx                                                                */
/*                                                                          */
/* Version 7 Nodelist Index structure.  This is a 512-byte record, which    */
/* is defined by three structures:  Record 0 is the Control Record, then    */
/* some number of Leaf Node (LNode) Records, then the Index Node (INode)    */
/* Records.  This defines an unbalanced binary tree.                        */
/*                                                                          */
/*--------------------------------------------------------------------------*/

typedef struct {
    word    CtlBlkSize; /* Blocksize of Index Blocks   */
    long    CtlRoot;    /* Block number of Root        */
    long    CtlHiBlk;   /* Block number of last block  */
    long    CtlLoLeaf;  /* Block number of first leaf  */
    long    CtlHiLeaf;  /* Block number of last leaf   */
    long    CtlFree;    /* Head of freelist            */
    word    CtlLvls;    /* Number of index levels      */
    word    CtlParity;  /* XOR of above fields         */
} __attribute__ ((packed)) _CtlBlk;

typedef struct {     /* IndxFirst is -1 in LNodes   */
    long    IndxFirst;  /* Pointer to next lower level */
    long    IndxBLink;  /* Pointer to previous link    */
    long    IndxFLink;  /* Pointer to next link        */
    short   IndxCnt;    /* Count of Items in block     */
    word    IndxStr;    /* Offset in block of 1st str  */
    struct _LeafRef {
        word   KeyOfs;  /* Offset of string into block */
        word   KeyLen;  /* Length of string            */
        long   KeyVal;  /* Pointer to data block       */
    } LeafRef[62];
} __attribute__ ((packed)) _LNodeBlk;

typedef struct {
    long    IndxFirst;  /* Pointer to next lower level */
    long    IndxBLink;  /* Pointer to previous link    */
    long    IndxFLink;  /* Pointer to next link        */
    short   IndxCnt;    /* Count of Items in block     */
    word    IndxStr;    /* Offset in block of 1st str  */
    /* If IndxFirst is NOT -1, this is INode:          */
    struct _IndxRef {
    word   IndxOfs; /* Offset of string into block */
    word   IndxLen; /* Length of string            */
    long   IndxData;/* Record number of string     */
    long   IndxPtr; /* Block number of lower index */
    } IndxRef[41];
} __attribute__ ((packed)) _INodeBlk;


#define OfsNoLink 0xffffffff

                    // this is common for all systems
struct _DTPAllLnk {
    word  Region;
    word  Hub;
    dword SOfs;     // DAT offset of next Same SysOp entry
    dword POfs;     // DAT offset of next Same Phone entry
    dword FeOfs;    // DAT offset of next "Equal Fido Level"
    byte  Sn;       // Number (base 0) of SysOp entry (ADR order)
    byte  Pn;       // Number (base 0) of Phone entry (ADR order)
} __attribute__ ((packed));

                    // nodes only (no points)
struct _DTPNodeLnk {
    word  ndowns;   // number of systems in lower level
    dword FlOfs;    // DAT offset of "Lower Fido Level"
} __attribute__ ((packed));


struct _DTPLnk {
    _DTPAllLnk A;
    _DTPNodeLnk N;
} __attribute__ ((packed));

                    // first record of DTP file
struct _DTPCtl {
    word size;          // Size of this control record
    byte Version;       // Version of DTP file
    byte AllFixSize;    // sizeof (_DTPAllLnk)
    byte AddFixSize;    // sizeof (_DTPNodeLnk)
} __attribute__ ((packed));


struct _DTPHead {
    _DTPCtl ctl;
    _DTPNodeLnk lnk;
} __attribute__ ((packed));
};
 
#endif
