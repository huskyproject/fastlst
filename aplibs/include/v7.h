/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1991-1997  Alberto Pasquale                 */
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
/*   How to contact the author:  Alberto Pasquale of 2:332/504@fidonet       */
/*                               Viale Verdi 106                             */
/*                               41100 Modena                                */
/*                               Italy                                       */
/*                                                                           */
/*****************************************************************************/

// V7.h

#include <typedefs.h>


struct _CtlBlk {
    word    CtlBlkSize; /* Blocksize of Index Blocks   */
    long    CtlRoot;    /* Block number of Root        */
    long    CtlHiBlk;   /* Block number of last block  */
    long    CtlLoLeaf;  /* Block number of first leaf  */
    long    CtlHiLeaf;  /* Block number of last leaf   */
    long    CtlFree;    /* Head of freelist            */
    word    CtlLvls;    /* Number of index levels      */
    word    CtlParity;  /* XOR of above fields         */
};


struct _IndxRef {
    word   IndxOfs; /* Offset of string into block */
    word   IndxLen; /* Length of string            */
    long   IndxData;/* Record number of string     */
    long   IndxPtr; /* Block number of lower index */
};


struct _INodeBlk {
    long    IndxFirst;  /* Pointer to next lower level */
    long    IndxBLink;  /* Pointer to previous link    */
    long    IndxFLink;  /* Pointer to next link        */
    short   IndxCnt;    /* Count of Items in block     */
    word    IndxStr;    /* Offset in block of 1st str  */
    /* If IndxFirst is NOT -1, this is INode:          */
    _IndxRef IndxRef[1];
};


struct _LeafRef {
    word   KeyOfs;  /* Offset of string into block */
    word   KeyLen;  /* Length of string            */
    long   KeyVal;  /* Pointer to data block       */
};


struct _LNodeBlk {
    /* IndxFirst is -1 in LNodes   */
    long    IndxFirst;  /* Pointer to next lower level */
    long    IndxBLink;  /* Pointer to previous link    */
    long    IndxFLink;  /* Pointer to next link        */
    short   IndxCnt;    /* Count of Items in block     */
    word    IndxStr;    /* Offset in block of 1st str  */
    _LeafRef LeafRef[1];
};


struct _ndx {
    union {
        _CtlBlk CtlBlk;
        _INodeBlk INodeBlk;
        _LNodeBlk LNodeBlk;
        byte RawNdx[512];
    };
};



