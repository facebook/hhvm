/*
 * Copyright 2001-2004 Brandon Long
 * All Rights Reserved.
 *
 * ClearSilver Templating System
 *
 * This code is made available under the terms of the ClearSilver License.
 * http://www.clearsilver.net/license.hdf
 *
 */

#include "cs_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#include "neo_misc.h"
#include "neo_err.h"
#include "ulist.h"
#include "ulocks.h"

int NERR_PASS = -1;
int NERR_ASSERT = 0;
int NERR_NOT_FOUND = 0;
int NERR_DUPLICATE = 0;
int NERR_NOMEM = 0;
int NERR_PARSE = 0;
int NERR_OUTOFRANGE = 0;
int NERR_SYSTEM = 0;
int NERR_IO = 0;
int NERR_LOCK = 0;
int NERR_DB = 0;
int NERR_EXISTS = 0;
int NERR_MAX_RECURSION = 0;

static NEOERR *FreeList = NULL;
static ULIST *Errors = NULL;
static int Inited = 0;
#ifdef HAVE_PTHREADS
/* In multi-threaded environments, we have to init thread safely */
static pthread_mutex_t InitLock = PTHREAD_MUTEX_INITIALIZER;
#endif

/* Set this to 1 to enable non-thread safe re-use of NEOERR data
 * structures.  This was a premature performance optimization that isn't
 * thread safe, if we want it thread safe we need to add mutex code...
 * which has its own performance penalties...
 */
static int UseFreeList = 0;

static NEOERR *_err_alloc(void)
{
  NEOERR *err;

  if (!UseFreeList || FreeList == NULL)
  {
    err = (NEOERR *)calloc (1, sizeof (NEOERR));
    if (err == NULL)
    {
      ne_warn ("INTERNAL ERROR: Unable to allocate memory for NEOERR");
      return INTERNAL_ERR;
    }
    return err;
  }
  else
  {
    err = FreeList;
    FreeList = FreeList->next;
  }
  err->flags |= NE_IN_USE;
  err->next = NULL;
  return err;
}

static int _err_free (NEOERR *err)
{
  if (err == NULL || err == INTERNAL_ERR)
    return 0;
  if (err->next != NULL)
    _err_free(err->next);
  if (UseFreeList)
  {
    err->next = FreeList;
    FreeList = err;
    err->flags = 0;
    err->desc[0] = '\0';
  }
  else
  {
    free(err);
  }
  return 0;
}

NEOERR *nerr_raisef (const char *func, const char *file, int lineno, int error,
                    const char *fmt, ...)
{
  NEOERR *err;
  va_list ap;

  err = _err_alloc();
  if (err == INTERNAL_ERR)
    return err;

  va_start(ap, fmt);
  vsnprintf (err->desc, sizeof(err->desc), fmt, ap);
  va_end(ap);

  err->error = error;
  err->func = func;
  err->file = file;
  err->lineno = lineno;

  return err;
}

NEOERR *nerr_raise_errnof (const char *func, const char *file, int lineno,
    			   int error, const char *fmt, ...)
{
  NEOERR *err;
  va_list ap;
  int l;

  err = _err_alloc();
  if (err == INTERNAL_ERR)
    return err;

  va_start(ap, fmt);
  vsnprintf (err->desc, sizeof(err->desc), fmt, ap);
  va_end(ap);

  l = strlen(err->desc);
  snprintf (err->desc + l, sizeof(err->desc)-l, ": [%d] %s", errno,
      strerror (errno));

  err->error = error;
  err->func = func;
  err->file = file;
  err->lineno = lineno;

  return err;
}

NEOERR *nerr_passf (const char *func, const char *file, int lineno, NEOERR *err)
{
  NEOERR *nerr;

  if (err == STATUS_OK)
    return err;

  nerr = _err_alloc();
  if (nerr == INTERNAL_ERR)
    return err;

  nerr->error = NERR_PASS;
  nerr->func = func;
  nerr->file = file;
  nerr->lineno = lineno;
  nerr->next = err;

  return nerr;
}

NEOERR *nerr_pass_ctxf (const char *func, const char *file, int lineno,
			NEOERR *err, const char *fmt, ...)
{
  NEOERR *nerr;
  va_list ap;

  if (err == STATUS_OK)
    return err;

  nerr = _err_alloc();
  if (nerr == INTERNAL_ERR)
    return err;

  va_start(ap, fmt);
  vsnprintf (nerr->desc, sizeof(nerr->desc), fmt, ap);
  va_end(ap);

  nerr->error = NERR_PASS;
  nerr->func = func;
  nerr->file = file;
  nerr->lineno = lineno;
  nerr->next = err;

  return nerr;
}

/* In the future, we'll allow someone to register an error handler */
void nerr_log_error (NEOERR *err)
{
  NEOERR *more;
  char buf[1024];
  char *err_name;

  if (err == STATUS_OK)
    return;

  if (err == INTERNAL_ERR)
  {
    ne_warn ("Internal error");
    return;
  }

  more = err;
  fprintf (stderr, "Traceback (innermost last):\n");
  while (more && more != INTERNAL_ERR)
  {
    err = more;
    more = err->next;
    if (err->error != NERR_PASS)
    {
      NEOERR *r;
      if (err->error == 0)
      {
	err_name = buf;
	snprintf (buf, sizeof (buf), "Unknown Error");
      }
      else
      {
	r = uListGet (Errors, err->error - 1, (void **)&err_name);
	if (r != STATUS_OK)
	{
	  err_name = buf;
	  snprintf (buf, sizeof (buf), "Error %d", err->error);
	}
      }

      fprintf (stderr, "  File \"%s\", line %d, in %s()\n%s: %s\n", err->file,
	  err->lineno, err->func, err_name, err->desc);
    }
    else
    {
      fprintf (stderr, "  File \"%s\", line %d, in %s()\n", err->file,
	  err->lineno, err->func);
      if (err->desc[0])
      {
	fprintf (stderr, "    %s\n", err->desc);
      }
    }
  }
}

void nerr_error_string (NEOERR *err, NEOSTRING *str)
{
  NEOERR *more;
  char buf[1024];
  char *err_name;

  if (err == STATUS_OK)
    return;

  if (err == INTERNAL_ERR)
  {
    string_append (str, "Internal error");
    return;
  }

  more = err;
  while (more && more != INTERNAL_ERR)
  {
    err = more;
    more = err->next;
    if (err->error != NERR_PASS)
    {
      NEOERR *r;
      if (err->error == 0)
      {
	err_name = buf;
	snprintf (buf, sizeof (buf), "Unknown Error");
      }
      else
      {
	r = uListGet (Errors, err->error - 1, (void **)&err_name);
	if (r != STATUS_OK)
	{
	  err_name = buf;
	  snprintf (buf, sizeof (buf), "Error %d", err->error);
	}
      }

      string_appendf(str, "%s: %s", err_name, err->desc);
      return;
    }
  }
}

void nerr_error_traceback (NEOERR *err, NEOSTRING *str)
{
  NEOERR *more;
  char buf[1024];
  char buf2[1024];
  char *err_name;

  if (err == STATUS_OK)
    return;

  if (err == INTERNAL_ERR)
  {
    string_append (str, "Internal error");
    return;
  }

  more = err;
  string_append (str, "Traceback (innermost last):\n");
  while (more && more != INTERNAL_ERR)
  {
    err = more;
    more = err->next;
    if (err->error != NERR_PASS)
    {
      NEOERR *r;
      if (err->error == 0)
      {
	err_name = buf;
	snprintf (buf, sizeof (buf), "Unknown Error");
      }
      else
      {
	r = uListGet (Errors, err->error - 1, (void **)&err_name);
	if (r != STATUS_OK)
	{
	  err_name = buf;
	  snprintf (buf, sizeof (buf), "Error %d", err->error);
	}
      }

      snprintf (buf2, sizeof(buf2),
	  "  File \"%s\", line %d, in %s()\n%s: %s\n", err->file,
	  err->lineno, err->func, err_name, err->desc);
      string_append(str, buf2);
    }
    else
    {
      snprintf (buf2, sizeof(buf2), "  File \"%s\", line %d, in %s()\n",
	  err->file, err->lineno, err->func);
      string_append(str, buf2);
      if (err->desc[0])
      {
	snprintf (buf2, sizeof(buf2), "    %s\n", err->desc);
	string_append(str, buf2);
      }
    }
  }
}

void nerr_ignore (NEOERR **err)
{
  _err_free (*err);
  *err = STATUS_OK;
}

int nerr_handle (NEOERR **err, int etype)
{
  NEOERR *walk = *err;

  while (walk != STATUS_OK && walk != INTERNAL_ERR)
  {

    if (walk->error == etype)
    {
      _err_free(*err);
      *err = STATUS_OK;
      return 1;
    }
    walk = walk->next;
  }

  if (walk == STATUS_OK && etype == STATUS_OK_INT)
    return 1;
  if (walk == STATUS_OK)
    return 0;

  if (walk == INTERNAL_ERR && etype == INTERNAL_ERR_INT)
  {
    *err = STATUS_OK;
    return 1;
  }
  if (walk == INTERNAL_ERR)
    return 0;

  return 0;
}

int nerr_match (NEOERR *err, int etype)
{
  while (err != STATUS_OK && err != INTERNAL_ERR)
  {

    if (err->error == etype)
      return 1;
    err = err->next;
  }

  if (err == STATUS_OK && etype == STATUS_OK_INT)
    return 1;
  if (err == STATUS_OK)
    return 0;

  if (err == INTERNAL_ERR && etype == INTERNAL_ERR_INT)
    return 1;
  if (err == INTERNAL_ERR)
    return 0;

  return 0;
}

NEOERR *nerr_register (int *val, const char *name)
{
  NEOERR *err;

  err = uListAppend (Errors, (void *) name);
  if (err != STATUS_OK) return nerr_pass(err);

  *val = uListLength(Errors);
  return STATUS_OK;
}

NEOERR *nerr_init (void)
{
  NEOERR *err;

  if (Inited == 0)
  {
#ifdef HAVE_PTHREADS
    /* In threaded environments, we have to mutex lock to do this init, but
     * we don't want to use a mutex every time to check that it was Inited.
     * So, we only lock if our first test of Inited was false */
    err = mLock(&InitLock);
    if (err != STATUS_OK) return nerr_pass(err);
    if (Inited == 0) {
#endif
    err = uListInit (&Errors, 10, 0);
    if (err != STATUS_OK) return nerr_pass(err);

    err = nerr_register (&NERR_PASS, "InternalPass");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register (&NERR_ASSERT, "AssertError");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register (&NERR_NOT_FOUND, "NotFoundError");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register (&NERR_DUPLICATE, "DuplicateError");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register (&NERR_NOMEM, "MemoryError");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register (&NERR_PARSE, "ParseError");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register (&NERR_OUTOFRANGE, "RangeError");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register (&NERR_SYSTEM, "SystemError");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register (&NERR_IO, "IOError");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register (&NERR_LOCK, "LockError");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register (&NERR_DB, "DBError");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register (&NERR_EXISTS, "ExistsError");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register (&NERR_MAX_RECURSION, "MaxRecursionError");
    if (err != STATUS_OK) return nerr_pass(err);

    Inited = 1;
#ifdef HAVE_PTHREADS
    }
    err = mUnlock(&InitLock);
    if (err != STATUS_OK) return nerr_pass(err);
#endif
  }
  return STATUS_OK;
}
