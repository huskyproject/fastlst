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

#include "bbsgenlb.hpp"
#include "daydir.hpp"
#include "cfgdata.hpp"
#include "misc.hpp"
#include "parse.hpp"
#include "filesbbs.hpp"

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <glob.h>
#include <unistd.h>

#define DMOD 367

static int
cmp_dd_data (const dd_data * s1, const dd_data * s2)
{
  if (s1->day < s2->day)
    return -1;
  else if (s1->day == s2->day)
    return 0;
  else
    return +1;
}

			 // for GetExtDay
#define dir_none   -1		// none of the search types
#define dir_arc    -2		//  .arc


int
day3 (int day2, int fday)
{
  int fde = (fday + 10) % DMOD;	// we cannot accept after this
  int fde1 = fde - fde % 100;	// hundreds

  int ret = fde1 + day2;	// try this

  if (day2 > (fde % 100))
    {
      if (fde1 == 0)
	{			// 0?? -> 3??
	  ret += 300;
	  if (ret > 366)
	    ret -= 100;
	}
      else
	ret -= 100;
    }

  return ret;
}


static int
GetExtDay (char *&ext, int fileday = 0)		// returns day or error from extension
{
  if (fileday == 0)
    {				// .<ddd>
      if (IsDigit (*ext) && IsDigit (*(ext + 1)) && IsDigit (*(ext + 2)))
	{
	  int day = atoi (ext);
	  if (day < DMOD)
	    return day;
	}
    }
  else
    {				// .<a><dd>
      if (!IsDigit (*ext) && IsDigit (*(ext + 1)) && IsDigit (*(ext + 2)))
	return day3 (atoi (ext + 1), fileday);
      const AH_Archiver *a;
      if ((a = Compr->ExtDefined (ext)) != NULL)
	{
	  ext = a->ext;
	  return dir_arc;
	}
    }

  return dir_none;
}


void
DAYDIR::CommInit (int Latest)
{
  fname = new char[PATH_MAX];

  switch (Select)
    {
    case NODELIST:
      arc = FALSE;
      break;
    case ARCLIST:
      arc = TRUE;
      break;
    case NODEDIFF:
      arc = FALSE;
      break;
    case ARCDIFF:
      arc = TRUE;
      break;
    }

  firstnew = 0;
  nfound = 0;

  if (!mask)			// name not used in cfg, nfound = 0;
    return;

  char name[PATH_MAX], *q = NULL;
  char path[PATH_MAX];
  char oldpath[PATH_MAX];

  getcwd (oldpath, PATH_MAX);

  q = strrchr (mask, '/');	// mask could contain dir or part of dir
  if (q == NULL)		// point to first char of file name
    strcpy (name, mask);
  else
    strcpy (name, q + 1);
  int
  namelen = strlen (name);	// length of filename (no path)

  strcpy (path, mask);
  path[strlen (path) - namelen] = 0;

  chdir (path);

  // search for ff_name with letters and/or figures in the place of ?

  glob_t g;
  int done;

  int today = dayn (time (NULL));

  dir = new dd_data[DD_SIZE];	// alloc memory for directory data

  int day;
  int fflen;

  done = glob (name, 0, NULL, &g);

  while (!done)
    {
      fflen = strlen (g.gl_pathv[nfound]);

      if (fflen == namelen)
	{			// one char for every '?'
            q = g.gl_pathv[nfound] + fflen - 3;     // point to first char of ext
            day = GetExtDay (q, arc ? dayn (arcfiletime (g.gl_pathv[nfound])) : 0); // if dir_arc, p points to Archive->ext
	  switch (day)
	    {
	    case dir_none:
	      break;
/*	    case dir_arc:	// fixed archive extension
	      {
                      long newtime = arcfiletime (g.gl_pathv[nfound]);
                      if (newtime >= fixtime) { 
                        fixtime = newtime;
                        fixext = q;
                      }
	      }
	      break;*/
	    default:		// day number
	      if (nfound == DD_SIZE)
		{
		  vprintlog ("Error: too many files \"%s\"\n", name);
		  myexit (ERR_TOO_MANY_FILES);
		}
	      if (day > (today + 10) % DMOD)
		day -= DMOD;	// for sorting
	      if (arc)
		dir[nfound].first = q[0];	// get first char of ext
	      dir[nfound].day = (short) day;
	    }
	}
        nfound++;
        if (g.gl_pathc == nfound) done = 1;
    }

  chdir (oldpath);

  if (nfound == 0)
    return;


  qsort (dir, nfound, sizeof (dd_data), (QSF) cmp_dd_data);

  int i;

  if (Latest >= 0)
    {				// find first new
      day = Latest;
      if (day > (today + 10) % DMOD)
	day -= DMOD;
      for (i = 0; (i < nfound) && (dir[i].day <= day); i++);
      firstnew = i;
    }

  for (i = 0; (i < nfound) && (dir[i].day < 0); i++)
    dir[i].day += (short) DMOD;	// restore positive numbers

  if (arc)
    latestime = arcfiletime (Name (nfound - 1));
  else
    latestime = DosFileTime (Name (nfound - 1));
}


DAYDIR::DAYDIR (int Select, char *List, int nKeep)
{
  memset (this, 0, sizeof (*this));
  // VarList, FutDiff, OldTime not applicable
  DAYDIR::Select = Select;
  SrcNodeMissing = TRUE;

  mask = List;
  keep = nKeep;

  CommInit ();
}


DAYDIR::DAYDIR (int Select, InpncBlk * cib, int Latest, int FutDiff)	// Latest = -1: no NodeList avail
{				//          -2: fixed NodeList avail
  memset (this, 0, sizeof (*this));	//       1-366: Var NodeList avail
  // FutDiff != 0 is day of first not applicable nodediff
  DAYDIR::Select = Select;
  DAYDIR::FutDiff = FutDiff;
  VarList = cib->l->VarNodeList;

  SrcNodeMissing = (Latest == -1);

  switch (Select)
    {
    case NODELIST:
      mask = cib->l->NodeList;
      OldTime = cib->s->NodeTime;
      break;
    case ARCLIST:
      keep = cib->l->ArcListKeep;
      mask = cib->l->ArcList;
      OldTime = cib->s->ArcTime;
      break;
    case NODEDIFF:
      mask = cib->l->NodeDiff;
      OldTime = cib->s->NodeTime;
      break;
    case ARCDIFF:
      keep = cib->l->ArcDiffKeep;
      mask = cib->l->ArcDiff;
      OldTime = cib->s->ArcTime;
      break;
    }

  CommInit (Latest);
}


DAYDIR::~DAYDIR (void)
{
  delete[]dir;
  delete[]fname;
}


char *
DAYDIR::Name (int dd_idx)
{
  if (dd_idx == -1)
    {				// return fixed extension !
      if (fixext == NULL)
	return NULL;
      sprintf (fname, "%.*s%s", strlen (mask) - 3, mask, fixext);
    }
  else
    {				// return day extension
      if (dd_idx >= nfound)
	return NULL;
      if (arc)
	sprintf (fname, "%.*s%c%02hd", strlen (mask) - 3, mask,
		 dir[dd_idx].first, dir[dd_idx].day % 100);
      else
	sprintf (fname, "%.*s%03hd", strlen (mask) - 3, mask, dir[dd_idx].day);
    }
  return fname;
}


void
DAYDIR::KillOld (void)
{
  int
  NotKill;			// first not killed

  switch (Select)
    {
    case NODELIST:
      NotKill = nfound - 1;
      break;
    case ARCLIST:
      NotKill = FirstKeep (keep);
      break;
    case NODEDIFF:
      NotKill = firstnew;
      break;
    case ARCDIFF:
      NotKill = FirstKeep (keep);
      break;
    default:
      return;
    }

  for (int i = 0; i < NotKill; i++)
    if (arc)
      KillFile (Name (i));
    else
      DeleteFile (Name (i));
}


void
DAYDIR::KillAll (void)
{
  for (int i = 0; i < nfound; i++)
    if (arc)
      KillFile (Name (i));
    else
      DeleteFile (Name (i));
}


int
DAYDIR::FirstKeep (int keep)
{
  int lastday = -1;
  int i;
  keep++;
  for (i = nfound - 1; i > -1; i--)
    {
      if (lastday != dir[i].day)
	{
	  lastday = dir[i].day;
	  keep--;
	  if (keep == 0)
	    break;
	}
    }
  return i + 1;			// i points to last item to be killed
}


BOOL
DAYDIR::NewAvail (void)
{
  if ((fixtime != 0) || !VarList)
    {
      time_t
      max = __max (fixtime, latestime);
      if (max == 0)
	return FALSE;
      if (SrcNodeMissing)
	return TRUE;
      return (max != OldTime);
    }
  else
    {				// variable extension only
      if (FindNewDay (TRUE) == -1)
	return FALSE;
      else
	return TRUE;
    }
}


char *
DAYDIR::LatestName ()
{
  if ((fixtime == 0) || (latestime > fixtime))	// no fixed extension ARChives or var is latest
    return Name (nfound - 1);
  else				// fixed extension ARChives only
    return Name ();
}


int
DAYDIR::LatestDay (void)
{
  if (nfound == 0)
    return -1;
  return dir[nfound - 1].day;
}


int
DAYDIR::FindNew (BOOL first)
{
  static int
  cur;

  if (first)
    cur = firstnew;

  if (arc)
    while ((cur < (nfound - 1)) && (dir[cur + 1].day == dir[cur].day))
      cur++;			// skip files of same day

  if (cur >= nfound)
    return -1;

  if (first)
    if ((Select == NODEDIFF) || (Select == ARCDIFF))
      if (dir[cur].day == FutDiff)
	return -1;

  return cur++;
}


char *
DAYDIR::FindNewName (BOOL first)
{
  int
  idx = FindNew (first);

  if (idx == -1)
    return NULL;

  return Name (idx);
}


int
DAYDIR::FindNewDay (BOOL first)
{
  int
  idx = FindNew (first);

  if (idx == -1)
    return -1;

  return dir[idx].day;
}
