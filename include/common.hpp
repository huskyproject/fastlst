// Common.Hpp

#include <apgenlib.hpp>


int IsIpfTag (const char *Tag);

// Tag points to the heading colon ':'.
// Returns 0 if Tag is not a IPF Tag.
//         1 if ipf info terminates with '.'
//         2 if ipf info is full line.


                        // return values for IsCWord

#define _None_          0
#define _Remove_        1       // remove entire line
#define _FileImport_    2
#define _Center_        3
#define _Define_        4       // define symbol
#define _IfDef_         5

int IsCWord (const char *CWord);
char *GetStdSymbol (const char *str, int len);
int GetNameIt (const char *line, const char *&s, int &slen, const char *&t, int &tlen);


struct heapobj {
    word size;          // total size
    word symsize;       // total size (incl null) of symbol name
    byte buffer[1];     // first byte
};

class HeapStrStore: protected HeapStore {
    private:
        heapobj *objbuf;
        BOOL sorted;
    public:
        HeapStrStore ();
        ~HeapStrStore ();
        void StrStore (const char *sym, int symlen, const char *text, int textlen);
        char *GetSymbol (const char *str, int len); // NULL when not found
};

extern HeapStrStore *hs;

class ExpLine {
    private:
        char buf[2][512];
        int bufi;
    public:
        ExpLine ();
        char *Expand (const char *s);
};


