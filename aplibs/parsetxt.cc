/*****************************************************************************/
/*                                                                           */
/*                    (C) Copyright 1994 Alberto Pasquale                    */
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

/* ParseTxt.Cpp */

#include "apgenlib.hpp"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


#define __min(a,b)    (((a) < (b)) ? (a) : (b))
#define __max(a,b)    (((a) > (b)) ? (a) : (b))



static const char *skipspace (const char *s)
{
    while (1) {
        switch (*s) {
            case ' ':
            case '\t':
            case '\n':
                break;
            default:
                return s;
        }
        s++;
    }
}


#define OutQuotes   0x00    // for GetName state machine
#define InQuotes    0x01


int GetName (const char *&src,  char *dest, int maxlen, int action, BOOL *HasSpace)
{
    int len = 0;
    const char *s = src;
    char *d = dest;
    char l = 0;
    int status = OutQuotes;
    BOOL Space = FALSE;

    s += strspn (s, " \t\n");    // skip leading space

    if (dest) {    // copy up to maxlen-1 chars, plus terminating NULL
        while (len < (maxlen-1)) {
            switch (*s) {

                case '\t':
                case ' ' :
                    if (status == OutQuotes)
                        goto gn_exit;
                    else {      // InQuotes
                        *(d++) = ' ';  // tab converted to blank
                        s++;
                        len++;
                        Space = TRUE;
                    }
                    break;

                case '\"':
                    s++;
                    if (status == OutQuotes)
                        status = InQuotes;
                    else {
                        if (*s != '"')  // closing quotes found
                            status = OutQuotes;
                        else {    // this is a quoted quotes char
                            *(d++) = *(s++);
                            len ++;
                        }
                    }
                    break;

                case '\n':
                case '\0':
                    goto gn_exit;

                default  :
                    *(d++) = *(s++);
                    len ++;
            }
        }
    }

    while (1) {         // skip rest of src
        switch (*s) {
            case '\t':
            case ' ' :
                if (status == OutQuotes)
                    goto gn_exit;
                else {      // InQuotes
                    l = *(s++);
                    if (!dest) {
                        len++;
                        Space = TRUE;
                    }
                }
                break;

            case '\"':
                s++;
                if (status == OutQuotes)
                    status = InQuotes;
                else {
                    if (*s != '"')  // closing quotes found
                        status = OutQuotes;
                    else {    // this is a quoted quotes char
                        l = *(s++);
                        if (!dest)
                            len ++;
                    }
                }
                break;

            case '\n':
            case '\0':
                goto gn_exit;

            default  :
                l = *(s++);
                if (!dest)
                    len ++;
        }
    }

gn_exit:

    if (HasSpace)
        *HasSpace = Space;

    if (dest) {
        if (!(action & GN_SrcFix))
            src = skipspace (s);   // skip to next non-space character

        if (action & GN_BSlash) {
            if ((len > 0) && (*(d-1) != '\\')) {
                if (len == maxlen - 1) {
                    d--;
                    len--;
                    if (*(d-1) != '\\') {
                        *(d++) = '\\';
                        len++;
                    }
                } else {
                    *(d++) = '\\';
                    len++;
                }
            }
        }

        if (action & GN_UBSlash) {
            if ((len > 0) && (*(d-1) == '\\'))
                d--;
                len--;
        }

        *d = '\0';      // terminate destination

        if (action & GN_MkDir)
            if (mkdirpath (dest) == -1)
                return -1;
    } else {                    // dest == NULL
        if (action & GN_BSlash)
            if ((len > 0) && (l != '\\'))
                len++;

        if (action & GN_UBSlash)
            if ((len > 0) && (l == '\\'))
                len--;
    }

    return len;
}


char *GetStatName (const char *&src, int action, int *len, BOOL *HasSpace)
{
    static char namebuf[PATH_MAX];
    int namelen;

    namelen = GetName (src, namebuf, PATH_MAX, action, HasSpace);
    if (len)
        *len = namelen;
    if (namelen == -1)      // error
        return NULL;
    if ((namelen == 0) && !(action & GN_Empty))
        return NULL;
    return namebuf;
}


char *GetAllocName (const char *&src, int action, int *len, BOOL *HasSpace)
{
    int namelen;
    char *dest;

    namelen = GetName (src, NULL, 0, action, HasSpace);
    if (len)
        *len = namelen;
    if ((namelen == 0) && !(action & GN_Empty))
        return NULL;
    dest = new char[namelen+1];
    if (GetName (src, dest, namelen+1, action) == -1) { // error
        delete[] dest;
        return NULL;
    }
    return dest;
}


void strbslash (char *dir)
{
    int l;

    l=strlen(dir);
    if ((l > 0) && (dir[l-1]!='\\')) {
        dir[l]='\\';
        dir[l+1]=0;
    }
}


void strubslash (char *dir)
{
    int l = strlen (dir);
    if ((l > 0) && (dir[l-1] == '\\'))
        dir[l-1]='\0';
}
 

int GetLn (const char *src, char *dest, int maxlen)
{
    src += strspn (src, " \t\n");   // skip leading space

    if (*src == '\"')      // get string
        return GetName (src, dest, maxlen);

                            // get whole line

    char *e = strchr (src, 0);   // points to terminating NULL

    while (e > src) {       // skip trailing space
        if ((*(e-1) == '\n') || (*(e-1) == '\t') || (*(e-1) == ' '))
            e--;
        else
            break;
    }

    int len = __min (maxlen-1, (int) (e - src));

    if (dest) {
        for (int i = 0; i < len; i++)
            *(dest++) = *(src++);

        *dest = '\0';       // NULL termination
    }

    return len;
}


char *GetStatLn (const char *src, int action, int *len)
{
    static char linebuf[PATH_MAX];
    int linelen;

    linelen = GetLn (src, linebuf, PATH_MAX);
    if (len)
        *len = linelen;
    if ((linelen == 0) && !(action & GL_Empty))
        return NULL;
    return linebuf;
}


char *GetAllocLn (const char *src, int action, int *len)
{
    int linelen;
    char *dest;

    linelen = GetLn (src);
    if (len)
        *len = linelen;
    if ((linelen == 0) && !(action & GL_Empty))
        return NULL;
    dest = new char[linelen+1];
    GetLn (src, dest);
    return dest;
}
