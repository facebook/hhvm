// Borrowed from Musl Libc, under the following license:
/*
 * Copyright ? 2005-2014 Rich Felker, et al.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
// Note that this file has had moderate changes to make it compile as C++, and
// under MSVC.
#include "hphp/util/portability/glob.h"

#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>

#include <folly/CPortability.h>
#include <folly/FilePortability.h>
#include <folly/ScopeGuard.h>

#include "hphp/util/portability/fnmatch.h"
#include "hphp/util/portability.h"

struct match {
  struct match *next;
  char name[1];
};

static int is_literal(const char *p, int useesc) {
  int bracket = 0;
  for (; *p; p++) {
    switch (*p) {
      case '\\':
        if (!useesc) break;
      case '?':
      case '*':
        return 0;
      case '[':
        bracket = 1;
        break;
      case ']':
        if (bracket) return 0;
        break;
    }
  }
  return 1;
}

static int append(struct match **tail, const char *name, size_t len, int mark) {
  struct match *ne = (match*)malloc(sizeof(struct match) + len + 1);
  if (!ne) return -1;
  (*tail)->next = ne;
  ne->next = nullptr;
  strcpy(ne->name, name);
  if (mark) strcat(ne->name, "/");
  *tail = ne;
  return 0;
}

// Never inline, to ensure that the alloca of the pattern buffer is always safe.
NEVER_INLINE
static int match_in_dir(const char *d,
                        const char *p,
                        int flags,
                        int(*errfunc)(const char *path, int err),
                        struct match **tail) {
  DIR *dir;
  struct dirent de_buf, *de;
  char* pat = (char*)alloca(sizeof(char) * (strlen(p) + 1));
  const char *p2;
  size_t l = strlen(d);
  int literal;
  int fnm_flags = ((flags & GLOB_NOESCAPE) ? FNM_NOESCAPE : 0)
    | ((!(flags & GLOB_PERIOD)) ? FNM_PERIOD : 0);
  int error;

  if ((p2 = strchr(p, '/'))) {
    strcpy(pat, p);
    pat[p2 - p] = 0;
    for (; *p2 == '/'; p2++);
    p = pat;
  }
  literal = is_literal(p, !(flags & GLOB_NOESCAPE));
  if (*d == '/' && !*(d + 1)) l = 0;

  /* rely on opendir failing for nondirectory objects */
  dir = opendir(*d ? d : ".");
  error = errno;
  if (!dir) {
    /* this is not an error -- we let opendir call stat for us */
    if (error == ENOTDIR) return 0;
    if (error == EACCES && !*p) {
      struct stat st;
      if (!stat(d, &st) && S_ISDIR(st.st_mode)) {
        if (append(tail, d, l, l))
          return GLOB_NOSPACE;
        return 0;
      }
    }
    if (errfunc(d, error) || (flags & GLOB_ERR))
      return GLOB_ABORTED;
    return 0;
  }
  if (!*p) {
    error = append(tail, d, l, l) ? GLOB_NOSPACE : 0;
    closedir(dir);
    return error;
  }
  while (!(error = readdir_r(dir, &de_buf, &de)) && de) {
    char* namebuf = (char*)malloc(sizeof(char) * (l + strlen(de->d_name) + 2));
    SCOPE_EXIT {
      free(namebuf);
    };
    char* name = namebuf;
    if (!literal && fnmatch(p, de->d_name, fnm_flags))
      continue;
    if (literal && strcmp(p, de->d_name))
      continue;
    if (p2 && de->d_type && !S_ISDIR(de->d_type << 12)
        && !S_ISLNK(de->d_type << 12))
      continue;
    if (*d) {
      memcpy(name, d, l);
      name[l] = '/';
      strcpy(name + l + 1, de->d_name);
    } else {
      name = de->d_name;
    }
    if (p2) {
      if ((error = match_in_dir(name, p2, flags, errfunc, tail))) {
        closedir(dir);
        return error;
      }
    } else {
      int mark = 0;
      if (flags & GLOB_MARK) {
        if (de->d_type && !S_ISLNK(de->d_type << 12))
          mark = S_ISDIR(de->d_type << 12);
        else {
          struct stat st;
          stat(name, &st);
          mark = S_ISDIR(st.st_mode);
        }
      }
      if (append(tail, name, l + strlen(de->d_name) + 1, mark)) {
        closedir(dir);
        return GLOB_NOSPACE;
      }
    }
  }
  closedir(dir);
  if (error && (errfunc(d, error) || (flags & GLOB_ERR)))
    return GLOB_ABORTED;
  return 0;
}

static int ignore_err(const char *path, int err) {
  return 0;
}

static void freelist(struct match *head) {
  struct match *match, *next;
  for (match = head->next; match; match = next) {
    next = match->next;
    free(match);
  }
}

static int sort(const void *a, const void *b) {
  return strcmp(*(const char **)a, *(const char **)b);
}

extern "C" int glob(const char *__restrict pat,
                    int flags,
                    int(*errfunc)(const char *path, int err),
                    glob_t *__restrict g) {
  const char *p = pat, *d;
  struct match head = { nullptr }, *tail = &head;
  size_t cnt, i;
  size_t offs = (flags & GLOB_DOOFFS) ? g->gl_offs : 0;
  int error = 0;

  if (*p == '/') {
    for (; *p == '/'; p++);
    d = "/";
  } else {
    d = "";
  }

  if (strlen(p) > PATH_MAX) return GLOB_NOSPACE;

  if (!errfunc) errfunc = ignore_err;

  if (!(flags & GLOB_APPEND)) {
    g->gl_offs = offs;
    g->gl_pathc = 0;
    g->gl_pathv = nullptr;
  }

  if (*p) error = match_in_dir(d, p, flags, errfunc, &tail);
  if (error == GLOB_NOSPACE) {
    freelist(&head);
    return error;
  }

  for (cnt = 0, tail = head.next; tail; tail = tail->next, cnt++);
  if (!cnt) {
    if (flags & GLOB_NOCHECK) {
      tail = &head;
      if (append(&tail, pat, strlen(pat), 0))
        return GLOB_NOSPACE;
      cnt++;
    } else
      return GLOB_NOMATCH;
  }

  if (flags & GLOB_APPEND) {
    char **pathv = (char**)realloc(g->gl_pathv,
                           (offs + g->gl_pathc + cnt + 1) * sizeof(char*));
    if (!pathv) {
      freelist(&head);
      return GLOB_NOSPACE;
    }
    g->gl_pathv = pathv;
    offs += g->gl_pathc;
  } else {
    g->gl_pathv = (char**)malloc((offs + cnt + 1) * sizeof(char *));
    if (!g->gl_pathv) {
      freelist(&head);
      return GLOB_NOSPACE;
    }
    for (i = 0; i<offs; i++)
      g->gl_pathv[i] = nullptr;
  }
  for (i = 0, tail = head.next; i<cnt; tail = tail->next, i++)
    g->gl_pathv[offs + i] = tail->name;
  g->gl_pathv[offs + i] = nullptr;
  g->gl_pathc += cnt;

  if (!(flags & GLOB_NOSORT))
    qsort(g->gl_pathv + offs, cnt, sizeof(char *), sort);

  return error;
}

extern "C" void globfree(glob_t *g) {
  size_t i;
  for (i = 0; i<g->gl_pathc; i++)
    free(g->gl_pathv[g->gl_offs + i] - offsetof(struct match, name));
  free(g->gl_pathv);
  g->gl_pathc = 0;
  g->gl_pathv = nullptr;
}
