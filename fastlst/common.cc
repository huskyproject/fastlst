
#include <string.h>
#include "common.hpp"


HeapStrStore *hs;


struct _tags {
    char *tag;
    int ret;
};
            // 1 ipf only till '.';  2 ipf full line

_tags taglist[] = { { "acviewport", 2 },
                    { "artlink", 2 },
                    { "artwork", 2 },
                    { "caution", 1 },
                    { "cgraphic", 2 },
                    { "color", 1 },
                    { "ctrl", 2 },
                    { "ctrldef", 2 },
                    { "ddf", 2 },
                    { "dl", 2 },
                    { "docprof", 2 },
                    { "eartlink", 1 },
                    { "ecaution", 1 },
                    { "ecgraphic", 1 },
                    { "ectrldef", 1 },
                    { "edl", 1 },
                    { "efig", 1 },
                    { "efn", 1 },
                    { "ehide", 1 },
                    { "ehp1", 1 },
                    { "ehp2", 1 },
                    { "ehp3", 1 },
                    { "ehp4", 1 },
                    { "ehp5", 1 },
                    { "ehp6", 1 },
                    { "ehp7", 1 },
                    { "ehp8", 1 },
                    { "ehp9", 1 },
                    { "elines", 1 },
                    { "elink", 1 },
                    { "ent", 1 },
                    { "eol", 1 },
                    { "eparml", 1 },
                    { "esl", 1 },
                    { "etable", 1 },
                    { "eul", 1 },
                    { "euserdoc", 1 },
                    { "ewarning", 1 },
                    { "exmp", 1 },
                    { "fig", 2 },
                    { "figcap", 2 },
                    { "font", 1 },
                    { "fn", 1 },
                    { "h1", 2 },
                    { "h2", 2 },
                    { "h3", 2 },
                    { "h4", 2 },
                    { "h5", 2 },
                    { "h6", 2 },
                    { "hide", 1 },
                    { "hp1", 1 },
                    { "hp2", 1 },
                    { "hp3", 1 },
                    { "hp4", 1 },
                    { "hp5", 1 },
                    { "hp6", 1 },
                    { "hp7", 1 },
                    { "hp8", 1 },
                    { "hp9", 1 },
                    { "i1", 2 },
                    { "i2", 2 },
                    { "icmd", 2 },
                    { "isyn", 2 },
                    { "li", 2 },
                    { "lines", 1 },
                    { "link", 1 },
                    { "lm", 1 },
                    { "lp", 1 },
                    { "note", 1 },
                    { "nt", 1 },
                    { "ol", 1 },
                    { "p", 1 },
                    { "parml", 1 },
                    { "pbutton", 2 },
                    { "pd", 1 },
                    { "pt", 1 },
                    { "rm", 1 },
                    { "sl", 1 },
                    { "table", 1 },
                    { "title", 1 },
                    { "ul", 1 },
                    { "userdoc", 1 },
                    { "warning", 1 },
                    { "xmp", 1 },
                    { "" , 0 }
                  };

int IsIpfTag (const char *Tag)
{
    int i = 0;
    int TagLen = strcspn (Tag+1, " .");
    while (taglist[i].tag[0]) {
        int taglistlen = strlen (taglist[i].tag);
        if (TagLen == taglistlen)
            if (strncasecmp (taglist[i].tag, Tag+1, TagLen) == 0)
                return taglist[i].ret;
        i ++;
    }
    return 0;
}


struct _cwords {
    char *cword;
    int ret;
};


_cwords cwordlist[] = { { "im",     _FileImport_ },
                        { "br",     _Remove_ },
                        { "ce",     _Center_ },
                        { "*",      _Remove_ },
                        { "nameit", _Define_ },
                        { "ifdef",  _IfDef_  },
                        { "" , 0 }
                      };

int IsCWord (const char *CWord)
{
    int i = 0;
    int CWordLen = strcspn (CWord+1, " ");
    while (cwordlist[i].cword[0]) {
        int cwordlistlen = strlen (cwordlist[i].cword);
        if (CWordLen == cwordlistlen)
            if (strncasecmp (cwordlist[i].cword, CWord+1, CWordLen) == 0)
                return cwordlist[i].ret;
        i ++;
    }
    return _None_;
}


struct SymLst {
    char symbol[10];
    char text[10];
};


char *GetStdSymbol (const char *str, int len)
{
    static SymLst symtbl[] = { { "osq", "'"  },
                               { ""   , "" }
                             };

    SymLst *s = symtbl;
    while (*s->symbol) {
        if (strncmp (str, s->symbol, len) == 0)
            if (s->symbol[len] == '\0')
                return s->text;
        s ++;
    }
    return NULL;
}

// ================ HeapStrStore ================

#define bufsize 256


int ocmpf (const heapobj **obj1, const heapobj **obj2)
{
    return strcasecmp ((char *)(*obj1)->buffer, (char *)(*obj2)->buffer);
}


HeapStrStore::HeapStrStore ()
{
    objbuf = (heapobj *) new byte[sizeof (heapobj) + bufsize];
    sorted = FALSE;
}

HeapStrStore::~HeapStrStore ()
{
    delete objbuf;
}


void HeapStrStore::StrStore (const char *sym, int symlen, const char *text, int textlen)
{
    int totstrlen = symlen + 1 + textlen;
    if (totstrlen > bufsize)
        return;
    objbuf->size = (word) (totstrlen + sizeof (heapobj));
    objbuf->symsize = word (symlen + 1);

    strncpy ((char *)objbuf->buffer, sym, symlen);  // symbol name
    objbuf->buffer[symlen] = '\0';

    strncpy ((char *)objbuf->buffer+objbuf->symsize, text, textlen); // text
    objbuf->buffer[totstrlen] = '\0';

    Store (objbuf);
    sorted = FALSE;
}


char *HeapStrStore::GetSymbol (const char *str, int len)
{
    if (!sorted) {
        Sort (CMPF (ocmpf));
        sorted = TRUE;
    }

    len = __min (len, bufsize);
    strncpy ((char *)objbuf->buffer, str, len);
    objbuf->buffer[len] = '\0';

    heapobj *obj = (heapobj *) Retrieve (&objbuf);

    if (!obj)
        return NULL;

    return (char *)obj->buffer+obj->symsize;
}


// ================ Expline ================


ExpLine::ExpLine ()
{
    bufi = 0;           // use first buffer for destination
}


char *ExpLine::Expand (const char *s)
{
    BOOL ExpDone = FALSE;
    char *d = buf[bufi];

    while (*s) {
        if (*s == '&') {
            int len = strcspn (s, ".");  // length of possible symbol name
            if (s[len] == '.') {
                len ++;
                if (len > 2) {
                    char *text = hs->GetSymbol (s+1, len-2);
                    if (!text)
                        text = GetStdSymbol (s+1, len-2);
                    if (text) {
                        d = fl_stpcpy (d, text);
                        s += len;
                        ExpDone = TRUE;
                        continue;
                    }
                }
            }
        }
        *(d++) = *(s++);
    }
    *d = '\0';

    d = buf[bufi];      // reset string pointer

    if (ExpDone) {
        bufi = (bufi + 1) % 2;      // new destination buffer
        return Expand (d);
    }

    return d;
}

// ============== Function to process .nameit =================

int GetNameIt (const char *line, const char *&s, int &slen, const char *&t, int &tlen)
{                                                   // 0 on success
    s = line;
    s += strcspn (s, " \t\n"); // skip nameit
    s += strspn (s, " \t\n"); // skip space
    slen = strcspn (s, " \t\n"); // length of symbol name
    if (slen == 0)
        return -1;
    t = s + slen;          // skip symbol name
    t += strspn (t, " \t\n");   // skip space
    if (*t != '\'')
        return -1;
    t ++;           // skip heading '
    char *endofstr = strrchr (t, '\'');
    if (!endofstr)
        return -1;
    tlen = int (endofstr - t); // length of text
    return 0;
}



