/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1995-1997  Alberto Pasquale                 */
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

#include "apgenlib.hpp"
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <utime.h>
#include <stdio.h>

int Exist (const char *path, _ExistStat *st)
{
  int plen = strlen (path);
  if (plen == 0)
    return 0;

  char mypath[PATH_MAX];
  BOOL DirReq = FALSE,
    RootReq = FALSE;

  if (path[plen-1] == '/') {     // Dir Required
    DirReq = TRUE;
    RootReq = TRUE;
    if (plen > 1)
      if (path[plen-2] != ':') {      // remove trailing backslash
	RootReq = FALSE;
	strcpy (mypath, path);
	mypath[plen-1] = '\0';
	path = mypath;
      }
  }

  struct stat buf;

  if (stat (path, &buf))
    return 0;

  if (S_ISCHR (buf.st_mode))
    return _ExDEVICE_;

  int ret = ((S_ISDIR (buf.st_mode) | RootReq) ? _ExDIR_  : _ExFILE_) |
    ((buf.st_mode & S_IWUSR) ? _ExWRITE_ : 0);

  if (DirReq && !(ret & _ExDIR_))
    return 0;

  if (st) {
    st->ctime = 0;
    st->atime = 0;
    st->mtime = buf.st_mtime;
    st->size  = buf.st_size;
  }

  return ret;
}


BOOL getftime (int handle, time_t *mtime, time_t *ctime, time_t *atime)
{
  struct stat buf;

  if (fstat (handle, &buf))
    return FALSE;
  if (mtime)
    *mtime = buf.st_mtime;
  if (ctime)
    *ctime = buf.st_ctime;
  if (atime)
    *atime = buf.st_atime;
  return TRUE;
}


BOOL getftime (const char *filename, time_t *mtime, time_t *ctime, time_t *atime)
{
  struct stat buf;

  if (stat (filename, &buf))
    return FALSE;
  if (mtime)
    *mtime = buf.st_mtime;
  if (ctime)
    *ctime = buf.st_ctime;
  if (atime)
    *atime = buf.st_atime;

  return TRUE;
}

// BOOL touchf (int handle, time_t mtime, time_t ctime, time_t *nowp)
BOOL my_touchf (const char *fname, time_t mtime, time_t ctime, time_t *nowp)
{
  time_t now = time (NULL);
  struct utimbuf tvp;

  if (nowp)
    *nowp = now;

  BOOL FTouch = FALSE;        // must touch mtime when ctime not valid

  if (mtime == _TF_Auto) {
    mtime = 0;
    FTouch = TRUE;
  }

  if (ctime == ULONG_MAX) {
    ctime = now;
    FTouch = TRUE;
  }

  if (mtime == ULONG_MAX)
    mtime = now;

  ushort date, time;
  if (FTouch)
    mtime = ctime;
  if (mtime) {
    tvp.actime = ctime;
    tvp.modtime = mtime;
    if (utime (fname, &tvp))
      return FALSE;
  }

  return TRUE;
}


BOOL touchf (pcsz filename, time_t mtime, time_t ctime, time_t *nowp)
{
  if (!my_touchf (filename, mtime, ctime, nowp))
    return FALSE;
  return TRUE;
}

/*
BOOL touchf (int handle, byte touchflag, time_t *nowp)
{
  return touchf (handle, (touchflag & _CF_mtouch_) ? ULONG_MAX : 0,
		 (touchflag & _CF_ctouch_) ? ULONG_MAX : 0, nowp);
}
*/

BOOL touchf (pcsz filename, byte touchflag, time_t *nowp)
{
  return touchf (filename, (touchflag & _CF_mtouch_) ? ULONG_MAX : 0,
		 (touchflag & _CF_ctouch_) ? ULONG_MAX : 0, nowp);
}


#define COPYBUFSIZE 0x8000


// Just copies the file (no EAs), handles Modification date only
// returns 0 on success

// Used under DOS and with OS/2 when destination is NFS with no EAs.


static int CopyFile (pcsz sourcefile, pcsz destfile, byte flags)
{
  int source, destination;
  unsigned bread;
  ushort srcfdate, srcftime;

  byte *bufp = new byte[COPYBUFSIZE];

  if ((source = open (sourcefile, O_RDONLY | O_BINARY)) == -1) {
    delete[] bufp;
    return -1;
  }

  if ((destination = open (destfile,
			   ((flags & _CF_ExistFail_) ? O_EXCL : O_TRUNC) |
			   O_WRONLY | O_CREAT | O_BINARY)) == -1) {
    delete[] bufp;
    close (source);
    return -1;
  }

  //    _dos_getftime (source, &srcfdate, &srcftime);

  while ((bread = read (source, bufp, COPYBUFSIZE)) > 0) {
    if (bread == (unsigned) -1)
      break;
    if (write (destination, bufp, bread) != bread) {
      delete[] bufp;
      close (source);
      close (destination);
      unlink (destfile);
      return -1;
    }
  }

  if (bread != 0) {
    delete[] bufp;
    close (source);
    close (destination);
    unlink (destfile);
    return -1;
  }

  //    _dos_setftime (destination, srcfdate, srcftime);

  delete[] bufp;
  close (source);
  if (close (destination))
    return -1;

  return 0;
}


int copyfile (pcsz sourcefile, pcsz destfile, byte flags)
{
  if (!(Exist (sourcefile) & _ExFILE_))
    return _CF_NOSRC_;

  if (eqpath (sourcefile, destfile)) {
    if (flags & _CF_touchflags_)
      touchf (destfile, flags);
    return _CF_SRCEQDEST_;
  }

  bool copyerr;

  copyerr = (CopyFile (sourcefile, destfile, flags) != 0);

  if (copyerr) {

    if (flags & _CF_ExistFail_) {
      if (Exist (destfile) & _ExFILE_)
	return _CF_DESTEXISTS_;
    }

    return _CF_ERROR_;
  }

  if (flags & _CF_touchflags_)
    if (!touchf (destfile, flags))
      return _CF_TOUCHERR_;

  return _CF_OK_;
}


int movefile (pcsz sourcefile, pcsz destfile, byte flags)
{
  if (!(Exist (sourcefile) & _ExFILE_))
    return _CF_NOSRC_;

  if (eqpath (sourcefile, destfile)) {
    if (flags & _CF_touchflags_)
      touchf (destfile, flags);
    return _CF_SRCEQDEST_;
  }

  if (Exist (destfile) & _ExFILE_)
    if (flags & _CF_ExistFail_)
      return _CF_DESTEXISTS_;
    else
      unlink (destfile);

  if (rename (sourcefile, destfile) == 0) {       // try rename
    if (flags & _CF_touchflags_)
      if (!touchf (destfile, flags))
	return 1;
    return _CF_OK_;
  }

  int err = copyfile (sourcefile, destfile, flags);  // otherwise copy
  if ((err != _CF_OK_) && (err != _CF_TOUCHERR_))
    return err;

  if (unlink (sourcefile))      // erase source
    return _CF_SRCNOTDELETED_;

  return err;
}


char *pathnobs (char *path) // remove trailing backslash
{
  int len = strlen (path);
  if (len < 2)            // at least 2 chars needed e.g. "A\"
    return path;
  if ((path[1] == ':') && (len < 4))  // at least 4 chars e.g. "c:a\"
    return path;
  if (path[len-1] == '/')
    path[len-1] = '\0';     // remove trailing backslash
  return path;
}


char *pathwbs (char *path)  // append trailing backslash
{
  int len = strlen (path);
  if (len < 1)            // at leat 1 char needed e.g. "A" -> "A\"
    return path;
  if ((path[1] == ':') && (len < 3)) // at least 3 chars e.g. "c:a"
    return path;
  if (path[len-1] != '/') {
    path[len] = '/';
    path[len+1] = '\0';
  }
  return path;
}

#ifdef UNIX

char *fullpath(char *buffer, const char *path, size_t size, int *len)
{
  if (!buffer)
    buffer = (char *)malloc(PATH_MAX);

  realpath(path, buffer);
  return buffer;
}

#else
char *fullpath (char *buffer, const char *path, size_t size, int *len)
{
        char work[PATH_MAX];
	int wlen;

	if (!path) {
        getcwd (work, PATH_MAX);
        wlen = strlen (work);
        goto fpret;
	}
	if (*path == '\0') {
        getcwd (work, PATH_MAX);
        wlen = strlen (work);
        goto fpret;
	}

	if ((path[0] == '\\') && (path[1] == '\\')) {   // UNC names
        strcpy (work, path);
        wlen = strlen (work);
        goto fpret;
	}

	{
    const char *p = path;
    char *w = work;
    BOOL cwdgot = FALSE;

    if (p[1] == ':') {      // drive specified
        work[0] = p[0];
        work[1] = ':';
        w += 2;
        p += 2;
    } else {
        getcwd (work, PATH_MAX);
        cwdgot = TRUE;
        w += 2;
    }

    if (*p == '\\') {        // root specified
        *(w++) = '\\';
        p ++;
    } else {     // root not specified
        if (!cwdgot) {
            unsigned curdrv;
            _dos_getdrive (&curdrv);
            unsigned wrkdrv = (toupper (work[0]) - 'A') + 1;
            if (curdrv != wrkdrv) {
                unsigned actdrv, total;
                _dos_setdrive (wrkdrv, &total);
                _dos_getdrive (&actdrv);
                if (actdrv != wrkdrv) {
                    errno = ENOENT;
                    return NULL;
                }
                getcwd (work, PATH_MAX);
                _dos_setdrive (curdrv, &total);
            } else {
                getcwd (work, PATH_MAX);
            }
        }
        w = strchr (w, '\0');  // skip dir
    }

    while (*p) {
        if (*p == '\\') {
            errno = ENOENT;
            return NULL;
        }

        if (*p == '.') {        // special token
            p ++;
            while (*p && (*p != '\\')) {
                if (*p != '.') {  // check all '.'
                    errno = ENOENT;
                    return NULL;
                }
                w --;   // points to last char
                if (*w == '\\') {    // root: too many '.'
                    errno = ENOENT;
                    return NULL;
                }
                while (*w != '\\')   // remove last token
                    w --;
                if (*(w-1) == ':')
                    w ++;
                p ++;
            }
        } else {    // normal token: copy
            if (*(w-1) != '\\')
                *(w++) = '\\';
            while (*p && (*p != '\\'))
                *(w++) = *(p++);
        }

        if (*p == '\\')
            p ++;
    }

    if (*(w-1) != '\\') {  // if path is '\' terminated, terminate fullpath
        if (*(p-1) == '\\')
            *(w++) = '\\';
    }

    *w = '\0';

    wlen = w - work;
    }

    fpret:
    if (!buffer) {
    size = PATH_MAX;
    buffer = (char *) malloc (size);
    if (!buffer) {
    errno = ENOMEM;
    return NULL;
    }
    }
    if (size < PATH_MAX) {
    if (size <= wlen) {
    errno = ERANGE;
    return NULL;
    }
    }
    strcpy (buffer, work);
    if (len)
    *len = wlen;
  return buffer;
}
#endif

BOOL eqpath (const char *p1, const char *p2)
{
  if (!p1 || !p2)
    return FALSE;

  char p1f[PATH_MAX], p2f[PATH_MAX];
  int len1, len2;

  if (!fullpath (p1f, p1, PATH_MAX, &len1))
    return FALSE;
  if (!fullpath (p2f, p2, PATH_MAX, &len2))
    return FALSE;

  if (len1 > 4) {
    if (p1f[len1-1] == DIRSEP) {
      p1f[len1-1] = '\0'; // remove trailing backslash if present
      len1 --;
    }
  }

  if (len2 > 4) {
    if (p2f[len2-1] == DIRSEP) {
      p2f[len2-1] = '\0'; // remove trailing backslash if present
      len2 --;
    }
  }

  return (stricmp (p1f, p2f) == 0);
}


int fnamecmp (const char *fname, const char *wildname)
{
  const char *f, *w;

  f = fname;
  w = wildname;

  while (*w && *f) {
    if (*w == '?') {
      f++;
      w++;
      continue;
    }
    if (*w == '*') {
      while (((*w == '?') || (*w == '*')) && (*f)) {
	if (*w == '?')
	  f++;
	w++;
      }
      while ((toupper (*f) != toupper (*w)) && (*f))
	f++;
      continue;
    }

    if (toupper (*f) != toupper (*w))
      return 1;
    f++;
    w++;
  }

  if (*w == '.')      // skip trailing ".*" if ok till now
    w++;

  while ((*w == '?') || (*w == '*'))  // skip trailing ? if ok till now
    w++;

  if (*w || *f)
    return 1;

  return 0;
}


char *hasext (pcsz filename)
{
  pcsz p = strrchr (filename, '.');
  if (!p)
    return NULL;
  if (strchr (p+1, DIRSEP))
    return NULL;
  return (char *)p;
}


void remext (char *filename)
{
  char *p = hasext (filename);
  if (p)
    *p = '\0';
}


void setext (char *filename, pcsz newext)
{
  remext (filename);
  strcat (filename, newext);
}


void addext (char *filename, pcsz ext)
{
  if (!hasext (filename))
    strcat (filename, ext);
}


char *filefrompath (const char *path)
{
  char *bs = strrchr (path, DIRSEP);
  if (!bs)
    bs = strchr (path, ':');
  if (!bs)
    return (psz) path;
  return (bs + 1);
}


char *fullname2path (const char *fullname, char *path, const char **fname)
{
  const char *name = filefrompath (fullname);
  int pathlen = int (name - fullname);
  if (pathlen)
    strncpy (path, fullname, pathlen);
  path[pathlen] = '\0';
  if (fname)
    *fname = name;
  return path;
}

time_t DosFileTime (const char *filename)
{
  time_t mtime;

  if (!getftime (filename, &mtime))
    return 0;
  return mtime;
}

time_t DosFileTime (int handle)
{
  time_t mtime;

  if (!getftime (handle, &mtime))
    return 0;
  return mtime;
}

static int myunlink (const char *path, int flags)
{
  return unlink (path);
}

int tunlink (const char *path, int timeout, int flags)
{
  if (!(Exist (path) & _ExFILE_))
    return _TUNLINK_NOTFILE_;

  /*    if (!(flags & _TUNLINK_ANY_)) {
        unsigned attribs;
        if (_dos_getfileattr (path, &attribs))
	return _TUNLINK_ERROR_;
        if (attribs & (_A_HIDDEN|_A_SYSTEM|_A_RDONLY))
	return _TUNLINK_UNDELETABLE_;
	}*/

  int i = 0;
  int ret = myunlink (path, flags);
  while ((ret != 0) && (i < timeout)) {
    sleep (1);
    i++;
    ret = myunlink (path, flags);
  }

  if (ret)
    return _TUNLINK_TIMEOUT_;

  return _TUNLINK_OK_;
}
