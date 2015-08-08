// Borrowed from PHP
/*
* Copyright (c) 1989, 1993
*	The Regents of the University of California.  All rights reserved.
*
* This code is derived from software contributed to Berkeley by
* Guido van Rossum.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. All advertising materials mentioning features or use of this software
*    must display the following acknowledgement:
*	This product includes software developed by the University of
*	California, Berkeley and its contributors.
* 4. Neither the name of the University nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*/
#ifndef incl_HPHP_PORTABILITY_GLOB_H_
#define incl_HPHP_PORTABILITY_GLOB_H_

#include <sys/stat.h>

typedef struct {
  int gl_pathc;		/* Count of total paths so far. */
  int gl_matchc;		/* Count of paths matching pattern. */
  int gl_offs;		/* Reserved at beginning of gl_pathv. */
  int gl_flags;		/* Copy of flags parameter to glob. */
  char **gl_pathv;	/* List of paths matching pattern. */
                        /* Copy of errfunc parameter to glob. */
  int(*gl_errfunc)(const char *, int);
} glob_t;

/* Flags */
#define	GLOB_APPEND	0x0001	/* Append to output from previous call. */
#define	GLOB_DOOFFS	0x0002	/* Use gl_offs. */
#define	GLOB_ERR	0x0004	/* Return on error. */
#define	GLOB_MARK	0x0008	/* Append / to matching directories. */
#define	GLOB_NOCHECK	0x0010	/* Return pattern itself if nothing matches. */
#define	GLOB_NOSORT	0x0020	/* Don't sort. */

#ifndef _POSIX_SOURCE
#define	GLOB_BRACE	0x0080	/* Expand braces ala csh. */
#define	GLOB_MAGCHAR	0x0100	/* Pattern had globbing characters. */
#define	GLOB_NOMAGIC	0x0200	/* GLOB_NOCHECK without magic chars (csh). */
#define	GLOB_QUOTE	0x0400	/* Quote special chars with \. */
#define	GLOB_TILDE	0x0800	/* Expand tilde names from the passwd file. */
#define	GLOB_NOESCAPE	0x1000	/* Disable backslash escaping. */
#define GLOB_LIMIT	0x2000	/* Limit pattern match output to ARG_MAX */
#endif

/* Error values returned by glob(3) */
#define	GLOB_NOSPACE	(-1)	/* Malloc call failed. */
#define	GLOB_ABORTED	(-2)	/* Unignored error. */
#define	GLOB_NOMATCH	(-3)	/* No match and GLOB_NOCHECK not set. */
#define	GLOB_NOSYS	(-4)	/* Function not supported. */
#define GLOB_ABEND	GLOB_ABORTED

int glob(const char*, int, int(*)(const char*, int), glob_t*);
void globfree(glob_t*);

#if _MSC_VER < 1800
# define _POSIX_
# include <limits.h>
# undef _POSIX_
#else
/* Visual Studio 2013 removed all the _POSIX_ defines, but we depend on some */
# ifndef ARG_MAX
#  define ARG_MAX 14500
# endif
#endif

#include <sys/stat.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	DOLLAR		'$'
#define	DOT		'.'
#define	EOS		'\0'
#define	LBRACKET	'['
#define	NOT		'!'
#define	QUESTION	'?'
#define	QUOTE		'\\'
#define	RANGE		'-'
#define	RBRACKET	']'
#define	SEP		DEFAULT_SLASH
#define	STAR		'*'
#define	TILDE		'~'
#define	UNDERSCORE	'_'
#define	LBRACE		'{'
#define	RBRACE		'}'
#define	SLASH		'/'
#define	COMMA		','

#define	M_QUOTE		0x8000
#define	M_PROTECT	0x4000
#define	M_MASK		0xffff
#define	M_ASCII		0x00ff
typedef char Char;
#define MAXPATHLEN MAX_PATH


#define	CHAR(c)		((Char)((c)&M_ASCII))
#define	META(c)		((Char)((c)|M_QUOTE))
#define	M_ALL		META('*')
#define	M_END		META(']')
#define	M_NOT		META('!')
#define	M_ONE		META('?')
#define	M_RNG		META('-')
#define	M_SET		META('[')
#define	ismeta(c)	(((c)&M_QUOTE) != 0)

namespace glob_internals {
static int	 compare(const void *, const void *);
static int	 g_Ctoc(const Char *, char *, u_int);
static int	 glob0(const Char *, glob_t *);
static int	 glob1(Char *, Char *, glob_t *, size_t *);
static int	 glob2(Char *, Char *, Char *, Char *, Char *, Char *,
  glob_t *, size_t *);
static int	 glob3(Char *, Char *, Char *, Char *, Char *, Char *,
  Char *, Char *, glob_t *, size_t *);
static int	 globextend(const Char *, glob_t *, size_t *);
static const Char *globtilde(const Char *, Char *, size_t, glob_t *);
static int	 globexp1(const Char *, glob_t *);
static int	 globexp2(const Char *, const Char *, glob_t *, int *);
static int	 match(Char *, Char *, Char *);

/*
* Expand recursively a glob {} pattern. When there is no more expansion
* invoke the standard globbing routine to glob the rest of the magic
* characters
*/
inline static int
globexp1(const Char* pattern, glob_t* pglob)
{
  const Char* ptr = pattern;
  int rv;

  /* Protect a single {}, for find(1), like csh */
  if (pattern[0] == LBRACE && pattern[1] == RBRACE && pattern[2] == EOS)
    return glob0(pattern, pglob);

  while ((ptr = (const Char *)strchr((Char *)ptr, LBRACE)) != NULL)
    if (!globexp2(ptr, pattern, pglob, &rv))
    return rv;

  return glob0(pattern, pglob);
}


/*
* Recursive brace globbing helper. Tries to expand a single brace.
* If it succeeds then it invokes globexp1 with the new pattern.
* If it fails then it tries to glob the rest of the pattern and returns.
*/
inline static int
globexp2(const Char* ptr, const Char* pattern, glob_t* pglob, int* rv)
{
  int     i;
  Char   *lm, *ls;
  const Char *pe, *pm, *pl;
  Char    patbuf[MAXPATHLEN];

  /* copy part up to the brace */
  for (lm = patbuf, pm = pattern; pm != ptr; *lm++ = *pm++)
    ;
  *lm = EOS;
  ls = lm;

  /* Find the balanced brace */
  for (i = 0, pe = ++ptr; *pe; pe++)
    if (*pe == LBRACKET) {
      /* Ignore everything between [] */
      for (pm = pe++; *pe != RBRACKET && *pe != EOS; pe++)
        ;
      if (*pe == EOS) {
        /*
        * We could not find a matching RBRACKET.
        * Ignore and just look for RBRACE
        */
        pe = pm;
      }
    }
    else if (*pe == LBRACE)
      i++;
    else if (*pe == RBRACE) {
      if (i == 0)
        break;
      i--;
    }

    /* Non matching braces; just glob the pattern */
    if (i != 0 || *pe == EOS) {
      *rv = glob0(patbuf, pglob);
      return 0;
    }

    for (i = 0, pl = pm = ptr; pm <= pe; pm++) {
      const Char *pb;

      switch (*pm) {
      case LBRACKET:
        /* Ignore everything between [] */
        for (pb = pm++; *pm != RBRACKET && *pm != EOS; pm++)
          ;
        if (*pm == EOS) {
          /*
          * We could not find a matching RBRACKET.
          * Ignore and just look for RBRACE
          */
          pm = pb;
        }
        break;

      case LBRACE:
        i++;
        break;

      case RBRACE:
        if (i) {
          i--;
          break;
        }
        /* FALLTHROUGH */
      case COMMA:
        if (i && *pm == COMMA)
          break;
        else {
          /* Append the current string */
          for (lm = ls; (pl < pm); *lm++ = *pl++)
            ;

          /*
          * Append the rest of the pattern after the
          * closing brace
          */
          for (pl = pe + 1; (*lm++ = *pl++) != EOS; )
            ;

          /* Expand the current pattern */
          *rv = globexp1(patbuf, pglob);

          /* move after the comma, to the next string */
          pl = pm + 1;
        }
        break;

      default:
        break;
      }
    }
    *rv = 0;
    return 0;
}



/*
* expand tilde from the passwd file.
*/
inline static const Char *
globtilde(const Char* pattern, Char* patbuf, size_t patbuf_len, glob_t* pglob)
{
  char *h;
  const Char *p;
  Char *b, *eb;

  if (*pattern != TILDE || !(pglob->gl_flags & GLOB_TILDE))
    return pattern;

  /* Copy up to the end of the string or / */
  eb = &patbuf[patbuf_len - 1];
  for (p = pattern + 1, h = (char *)patbuf;
  h < (char *)eb && *p && *p != SLASH; *h++ = (char)*p++)
    ;

  *h = EOS;


  if (((char *)patbuf)[0] == EOS) {
    /*
    * handle a plain ~ or ~/ by expanding $HOME
    * first and then trying the password file
    */
    if ((h = getenv("HOME")) == NULL) {
      return pattern;
    }
  }
  else {
    /*
    * Expand a ~user
    */
    return pattern;
  }

  /* Copy the home directory */
  for (b = patbuf; b < eb && *h; *b++ = *h++)
    ;

  /* Append the rest of the pattern */
  while (b < eb && (*b++ = *p++) != EOS)
    ;
  *b = EOS;

  return patbuf;
}


/*
* The main glob() routine: compiles the pattern (optionally processing
* quotes), calls glob1() to do the real pattern matching, and finally
* sorts the list (unless unsorted operation is requested).  Returns 0
* if things went well, nonzero if errors occurred.  It is not an error
* to find no matches.
*/
inline static int
glob0(const Char* pattern, glob_t* pglob)
{
  const Char *qpatnext;
  int c, err, oldpathc;
  Char *bufnext, patbuf[MAXPATHLEN];
  size_t limit = 0;

  qpatnext = globtilde(pattern, patbuf, MAXPATHLEN, pglob);
  oldpathc = pglob->gl_pathc;
  bufnext = patbuf;

  /* We don't need to check for buffer overflow any more. */
  while ((c = *qpatnext++) != EOS) {
    switch (c) {
    case LBRACKET:
      c = *qpatnext;
      if (c == NOT)
        ++qpatnext;
      if (*qpatnext == EOS ||
        strchr((Char *)qpatnext + 1, RBRACKET) == NULL) {
        *bufnext++ = LBRACKET;
        if (c == NOT)
          --qpatnext;
        break;
      }
      *bufnext++ = M_SET;
      if (c == NOT)
        *bufnext++ = M_NOT;
      c = *qpatnext++;
      do {
        *bufnext++ = CHAR(c);
        if (*qpatnext == RANGE &&
          (c = qpatnext[1]) != RBRACKET) {
          *bufnext++ = M_RNG;
          *bufnext++ = CHAR(c);
          qpatnext += 2;
        }
      } while ((c = *qpatnext++) != RBRACKET);
      pglob->gl_flags |= GLOB_MAGCHAR;
      *bufnext++ = M_END;
      break;
    case QUESTION:
      pglob->gl_flags |= GLOB_MAGCHAR;
      *bufnext++ = M_ONE;
      break;
    case STAR:
      pglob->gl_flags |= GLOB_MAGCHAR;
      /* collapse adjacent stars to one,
      * to avoid exponential behavior
      */
      if (bufnext == patbuf || bufnext[-1] != M_ALL)
        *bufnext++ = M_ALL;
      break;
    default:
      *bufnext++ = CHAR(c);
      break;
    }
  }
  *bufnext = EOS;

  if ((err = glob1(patbuf, patbuf + MAXPATHLEN - 1, pglob, &limit)) != 0)
    return(err);

  /*
  * If there was no match we are going to append the pattern
  * if GLOB_NOCHECK was specified or if GLOB_NOMAGIC was specified
  * and the pattern did not contain any magic characters
  * GLOB_NOMAGIC is there just for compatibility with csh.
  */
  if (pglob->gl_pathc == oldpathc) {
    if ((pglob->gl_flags & GLOB_NOCHECK) ||
      ((pglob->gl_flags & GLOB_NOMAGIC) &&
        !(pglob->gl_flags & GLOB_MAGCHAR)))
      return(globextend(pattern, pglob, &limit));
    else
      return(GLOB_NOMATCH);
  }
  if (!(pglob->gl_flags & GLOB_NOSORT))
    qsort(pglob->gl_pathv + pglob->gl_offs + oldpathc,
      pglob->gl_pathc - oldpathc, sizeof(char *), compare);
  return(0);
}

inline static int
compare(const void *p, const void *q)
{
  return(strcmp(*(char **)p, *(char **)q));
}

inline static int
glob1(Char* pattern, Char* pattern_last, glob_t* pglob, size_t* limitp)
{
  Char pathbuf[MAXPATHLEN];

  /* A null pathname is invalid -- POSIX 1003.1 sect. 2.4. */
  if (*pattern == EOS)
    return(0);
  return(glob2(pathbuf, pathbuf + MAXPATHLEN - 1,
    pathbuf, pathbuf + MAXPATHLEN - 1,
    pattern, pattern_last, pglob, limitp));
}

/*
* The functions glob2 and glob3 are mutually recursive; there is one level
* of recursion for each segment in the pattern that contains one or more
* meta characters.
*/
inline static int
glob2(Char* pathbuf, Char* pathbuf_last, Char* pathend, Char* pathend_last, Char* pattern,
  Char* pattern_last, glob_t* pglob, size_t* limitp)
{
  struct stat sb;
  Char *p, *q;
  int anymeta;

  /*
  * Loop over pattern segments until end of pattern or until
  * segment with meta character found.
  */
  for (anymeta = 0;;) {
    if (*pattern == EOS) {		/* End of pattern? */
      *pathend = EOS;
      ++pglob->gl_matchc;
      return(globextend(pathbuf, pglob, limitp));
    }

    /* Find end of next segment, copy tentatively to pathend. */
    q = pathend;
    p = pattern;
    while (*p != EOS && *p != '/' && *p != '\\') {
      if (ismeta(*p))
        anymeta = 1;
      if (q + 1 > pathend_last)
        return (1);
      *q++ = *p++;
    }

    if (!anymeta) {		/* No expansion, do next segment. */
      pathend = q;
      pattern = p;
      while (*pattern == '/' || *pattern == '\\') {
        if (pathend + 1 > pathend_last)
          return (1);
        *pathend++ = *pattern++;
      }
    }
    else
      /* Need expansion, recurse. */
      return(glob3(pathbuf, pathbuf_last, pathend,
        pathend_last, pattern, pattern_last,
        p, pattern_last, pglob, limitp));
  }
  /* NOTREACHED */
}

inline static int
glob3(Char* pathbuf, Char* pathbuf_last, Char* pathend, Char* pathend_last, Char* pattern, Char* pattern_last,
  Char* restpattern, Char* restpattern_last, glob_t* pglob, size_t* limitp)
{
  register struct dirent *dp;
  DIR *dirp;
  int err;
  char buf[MAXPATHLEN];

  if (pathend > pathend_last)
    return (1);
  *pathend = EOS;
  errno = 0;

  if ((dirp = opendir(pathbuf)) == NULL) {
    return(0);
  }

  err = 0;

  /* Search directory for matching names. */
  while ((dp = (*readdir)(dirp))) {
    register u_char *sc;
    register Char *dc;

    /* Initial DOT must be matched literally. */
    if (dp->d_name[0] == DOT && *pattern != DOT)
      continue;
    dc = pathend;
    sc = (u_char *)dp->d_name;
    while (dc < pathend_last && (*dc++ = *sc++) != EOS)
      ;
    if (dc >= pathend_last) {
      *dc = EOS;
      err = 1;
      break;
    }

    if (!match(pathend, pattern, restpattern)) {
      *pathend = EOS;
      continue;
    }
    err = glob2(pathbuf, pathbuf_last, --dc, pathend_last,
      restpattern, restpattern_last, pglob, limitp);
    if (err)
      break;
  }

  closedir(dirp);
  return(err);
}


/*
* Extend the gl_pathv member of a glob_t structure to accommodate a new item,
* add the new item, and update gl_pathc.
*
* This assumes the BSD realloc, which only copies the block when its size
* crosses a power-of-two boundary; for v7 realloc, this would cause quadratic
* behavior.
*
* Return 0 if new item added, error code if memory couldn't be allocated.
*
* Invariant of the glob_t structure:
*	Either gl_pathc is zero and gl_pathv is NULL; or gl_pathc > 0 and
*	gl_pathv points to (gl_offs + gl_pathc + 1) items.
*/
inline static int
globextend(const Char* path, glob_t* pglob, size_t* limitp)
{
  register char **pathv;
  register int i;
  u_int newsize, len;
  char *copy;
  const Char *p;

  newsize = sizeof(*pathv) * (2 + pglob->gl_pathc + pglob->gl_offs);
  pathv = (char**)(pglob->gl_pathv ? realloc((char *)pglob->gl_pathv, newsize) :
    malloc(newsize));
  if (pathv == NULL) {
    if (pglob->gl_pathv) {
      free(pglob->gl_pathv);
      pglob->gl_pathv = NULL;
    }
    return(GLOB_NOSPACE);
  }

  if (pglob->gl_pathv == NULL && pglob->gl_offs > 0) {
    /* first time around -- clear initial gl_offs items */
    pathv += pglob->gl_offs;
    for (i = pglob->gl_offs; --i >= 0; )
      *--pathv = NULL;
  }
  pglob->gl_pathv = pathv;

  for (p = path; *p++;)
    ;
  len = (u_int)(p - path);
  *limitp += len;
  if ((copy = (char*)malloc(len)) != NULL) {
    if (g_Ctoc(path, copy, len)) {
      free(copy);
      return(GLOB_NOSPACE);
    }
    pathv[pglob->gl_offs + pglob->gl_pathc++] = copy;
  }
  pathv[pglob->gl_offs + pglob->gl_pathc] = NULL;

  if ((pglob->gl_flags & GLOB_LIMIT) &&
    newsize + *limitp >= ARG_MAX) {
    errno = 0;
    return(GLOB_NOSPACE);
  }

  return(copy == NULL ? GLOB_NOSPACE : 0);
}


/*
* pattern matching function for filenames.  Each occurrence of the *
* pattern causes a recursion level.
*/
inline static int
match(register Char* name, register Char* pat, register Char* patend)
{
  int ok, negate_range;
  Char c, k;

  while (pat < patend) {
    c = *pat++;
    switch (c & M_MASK) {
    case M_ALL:
      if (pat == patend)
        return(1);
      do
        if (match(name, pat, patend))
        return(1);
      while (*name++ != EOS)
        ;
      return(0);
    case M_ONE:
      if (*name++ == EOS)
        return(0);
      break;
    case M_SET:
      ok = 0;
      if ((k = *name++) == EOS)
        return(0);
      if ((negate_range = ((*pat & M_MASK) == M_NOT)) != EOS)
        ++pat;
      while (((c = *pat++) & M_MASK) != M_END)
        if ((*pat & M_MASK) == M_RNG) {
          if (c <= k && k <= pat[1])
            ok = 1;
          pat += 2;
        }
        else if (c == k)
          ok = 1;
      if (ok == negate_range)
        return(0);
      break;
    default:
      if (*name++ != c)
        return(0);
      break;
    }
  }
  return(*name == EOS);
}

/* Free allocated data belonging to a glob_t structure. */

inline static int
g_Ctoc(register const Char* str, char* buf, u_int len)
{

  while (len--) {
    if ((*buf++ = (char)*str++) == EOS)
      return (0);
  }
  return (1);
}
}

inline int
glob(const char* pattern, int flags, int(*errfunc)(const char *, int), glob_t* pglob)
{
  const u_char *patnext;
  int c;
  Char *bufnext, *bufend, patbuf[MAXPATHLEN];
  /* Force skipping escape sequences on windows
  * due to the ambiguity with path backslashes
  */
  flags |= GLOB_NOESCAPE;

  patnext = (u_char *)pattern;
  if (!(flags & GLOB_APPEND)) {
    pglob->gl_pathc = 0;
    pglob->gl_pathv = NULL;
    if (!(flags & GLOB_DOOFFS))
      pglob->gl_offs = 0;
  }
  pglob->gl_flags = flags & ~GLOB_MAGCHAR;
  pglob->gl_errfunc = errfunc;
  pglob->gl_matchc = 0;

  bufnext = patbuf;
  bufend = bufnext + MAXPATHLEN - 1;
  if (flags & GLOB_NOESCAPE)
    while (bufnext < bufend && (c = *patnext++) != EOS)
    *bufnext++ = c;
  else {
    /* Protect the quoted characters. */
    while (bufnext < bufend && (c = *patnext++) != EOS)
      if (c == QUOTE) {
        if ((c = *patnext++) == EOS) {
          c = QUOTE;
          --patnext;
        }
        *bufnext++ = c | M_PROTECT;
      }
      else
        *bufnext++ = c;
  }
  *bufnext = EOS;

  if (flags & GLOB_BRACE)
    return glob_internals::globexp1(patbuf, pglob);
  else
    return glob_internals::glob0(patbuf, pglob);
}

inline void
globfree(glob_t* pglob)
{
  register int i;
  register char **pp;

  if (pglob->gl_pathv != NULL) {
    pp = pglob->gl_pathv + pglob->gl_offs;
    for (i = pglob->gl_pathc; i--; ++pp)
      if (*pp)
      free(*pp);
    free(pglob->gl_pathv);
    pglob->gl_pathv = NULL;
  }
}
#undef DOLLAR
#undef DOT
#undef EOS
#undef LBRACKET
#undef NOT
#undef QUESTION
#undef QUOTE
#undef RANGE
#undef RBRACKET
#undef SEP
#undef STAR
#undef TILDE
#undef UNDERSCORE
#undef LBRACE
#undef RBRACE
#undef SLASH
#undef COMMA
#undef M_QUOTE
#undef M_PROTECT
#undef M_MASK
#undef M_ASCII
#undef MAXPATHLEN
#undef CHAR
#undef META
#undef M_ALL
#undef M_END
#undef M_NOT
#undef M_ONE
#undef M_RNG
#undef M_SET
#undef ismeta

#endif
