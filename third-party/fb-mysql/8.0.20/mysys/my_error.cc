/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/my_error.cc
*/

#include <errno.h>
#include <stdarg.h>
#ifdef __linux__
#include <features.h>
#endif
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "m_ctype.h"
#include "m_string.h"
#include "my_base.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysql/service_mysql_alloc.h"
#include "mysys/my_handler_errors.h"
#include "mysys/mysys_priv.h"
#include "mysys_err.h"
#include "strings/mb_wc.h"
#include "template_utils.h"

/* Max length of a error message. Should be kept in sync with MYSQL_ERRMSG_SIZE.
 */
#define ERRMSGSIZE (512)

/* Define some external variables for error handling */

/*
  WARNING!
  my_error family functions have to be used according following rules:
  - if message has no parameters, use my_message(ER_CODE, ER(ER_CODE), MYF(N))
  - if message has parameters and is registered: my_error(ER_CODE, MYF(N), ...)
  - for free-form messages use my_printf_error(ER_CODE, format, MYF(N), ...)

  These three send their messages using error_handler_hook, which normally
  means we'll send them to the client if we have one, or to error-log / stderr
  otherwise.
*/

/*
  Message texts are registered into a linked list of 'my_err_head' structs.
  Each struct contains
  (1.) a pointer to a function that returns C character strings with '\0'
       termination
  (2.) the error number for the first message in the array (array index 0)
  (3.) the error number for the last message in the array
       (array index (last - first)).
  The function may return NULL pointers and pointers to empty strings.
  Both kinds will be translated to "Unknown error %d.", if my_error()
  is called with a respective error number.
  The list of header structs is sorted in increasing order of error numbers.
  Negative error numbers are allowed. Overlap of error numbers is not allowed.
  Not registered error numbers will be translated to "Unknown error %d.".
*/
static struct my_err_head {
  struct my_err_head *meh_next;   /* chain link */
  const char *(*get_errmsg)(int); /* returns error message format */
  int meh_first;                  /* error number matching array slot 0 */
  int meh_last;                   /* error number matching last slot */
} my_errmsgs_globerrs = {nullptr, get_global_errmsg, EE_ERROR_FIRST,
                         EE_ERROR_LAST};

static struct my_err_head *my_errmsgs_list = &my_errmsgs_globerrs;

/**
  Get a string describing a system or handler error. thread-safe.

  @param  buf  a buffer in which to return the error message
  @param  len  the size of the aforementioned buffer
  @param  nr   the error number

  @retval buf  always buf. for signature compatibility with strerror(3).
*/

char *my_strerror(char *buf, size_t len, int nr) {
  const char *msg = nullptr;

  buf[0] = '\0'; /* failsafe */

  /*
    These (handler-) error messages are shared by perror, as required
    by the principle of least surprise.
  */
  if ((nr >= HA_ERR_FIRST) && (nr <= HA_ERR_LAST))
    msg = handler_error_messages[nr - HA_ERR_FIRST];

  if (msg != nullptr)
    strmake(buf, msg, len - 1);
  else {
    /*
      On Windows, do things the Windows way. On a system that supports both
      the GNU and the XSI variant, use whichever was configured (GNU); if
      this choice is not advertised, use the default (POSIX/XSI).  Testing
      for __GNUC__ is not sufficient to determine whether this choice exists.
    */
#if defined(_WIN32)
    strerror_s(buf, len, nr);
    if (thr_winerr() != 0) {
      /*
        If error code is EINVAL, and Windows Error code has been set, we append
        the Windows error code to the message.
      */
      if (nr == EINVAL) {
        char tmp_buff[256];

        snprintf(tmp_buff, sizeof(tmp_buff), " [OS Error Code : 0x%x]",
                 thr_winerr());

        strcat_s(buf, len, tmp_buff);
      }

      set_thr_winerr(0);
    }
#elif ((defined _POSIX_C_SOURCE && (_POSIX_C_SOURCE >= 200112L)) || \
       (defined _XOPEN_SOURCE && (_XOPEN_SOURCE >= 600))) &&        \
    !defined _GNU_SOURCE
    strerror_r(nr, buf, len); /* I can build with or without GNU */
#elif defined(__GLIBC__) && defined(_GNU_SOURCE)
    char *r = strerror_r(nr, buf, len);
    if (r != buf)               /* Want to help, GNU? */
      strmake(buf, r, len - 1); /* Then don't. */
#else
    strerror_r(nr, buf, len);
#endif
  }

  /*
    strerror() return values are implementation-dependent, so let's
    be pragmatic.
  */
  if (!buf[0] || !strcmp(buf, "No error information"))
    strmake(buf, "Unknown error", len - 1);

  return buf;
}

/**
  @brief Get an error format string from one of the my_error_register()ed sets

  @note
    NULL values are possible even within a registered range.

  @param nr Errno

  @retval NULL  if no message is registered for this error number
  @retval str   C-string
*/

const char *my_get_err_msg(int nr) {
  const char *format;
  struct my_err_head *meh_p;

  /* Search for the range this error is in. */
  for (meh_p = my_errmsgs_list; meh_p; meh_p = meh_p->meh_next)
    if (nr <= meh_p->meh_last) break;

  /*
    If we found the range this error number is in, get the format string.
    If the string is empty, or a NULL pointer, or if we're out of ranges,
    we return NULL.
  */
  if (!(format = (meh_p && (nr >= meh_p->meh_first)) ? meh_p->get_errmsg(nr)
                                                     : nullptr) ||
      !*format)
    return nullptr;

  return format;
}

/**
  Fill in and print a previously registered error message.

  @note
    Goes through the (sole) function registered in error_handler_hook

  @param nr        error number
  @param MyFlags   Flags
  @param ...       variable list matching that error format string
*/

void my_error(int nr, myf MyFlags, ...) {
  const char *format;
  char ebuff[ERRMSGSIZE];
  DBUG_TRACE;
  DBUG_PRINT("my", ("nr: %d  MyFlags: %d  errno: %d", nr, MyFlags, errno));

  if (!(format = my_get_err_msg(nr)))
    (void)snprintf(ebuff, sizeof(ebuff), "Unknown error %d", nr);
  else {
    va_list args;
    va_start(args, MyFlags);
    (void)vsnprintf(ebuff, sizeof(ebuff), format, args);
    va_end(args);
  }

  /*
    Since this function is an error function, it will frequently be given
    values that are too long (and thus truncated on byte boundaries,
    not code point or grapheme boundaries), values that are binary, etc..
    Go through and replace every malformed UTF-8 byte with a question mark,
    so that the result is safe to send to the client and makes sense to read
    for the user.
  */
  for (char *ptr = ebuff, *end = ebuff + strlen(ebuff); ptr != end;) {
    my_wc_t ignored;
    int len = my_mb_wc_utf8mb4(&ignored, pointer_cast<const uchar *>(ptr),
                               pointer_cast<const uchar *>(end));
    if (len > 0) {
      ptr += len;
    } else {
      *ptr++ = '?';
    }
  }

  (*error_handler_hook)(nr, ebuff, MyFlags);
}

/**
  Print an error message.

  @note
    Goes through the (sole) function registered in error_handler_hook

  @param error     error number
  @param format    format string
  @param MyFlags   Flags
  @param ...       variable list matching that error format string
*/

void my_printf_error(uint error, const char *format, myf MyFlags, ...) {
  va_list args;
  char ebuff[ERRMSGSIZE];
  DBUG_TRACE;
  DBUG_PRINT("my", ("nr: %d  MyFlags: %d  errno: %d  Format: %s", error,
                    MyFlags, errno, format));

  va_start(args, MyFlags);
  (void)vsnprintf(ebuff, sizeof(ebuff), format, args);
  va_end(args);
  (*error_handler_hook)(error, ebuff, MyFlags);
}

/**
  Print an error message.

  @note
    Goes through the (sole) function registered in error_handler_hook

  @param error     error number
  @param format    format string
  @param MyFlags   Flags
  @param ap        variable list matching that error format string
*/

void my_printv_error(uint error, const char *format, myf MyFlags, va_list ap) {
  char ebuff[ERRMSGSIZE];
  DBUG_TRACE;
  DBUG_PRINT("my", ("nr: %d  MyFlags: %d  errno: %d  format: %s", error,
                    MyFlags, errno, format));

  (void)vsnprintf(ebuff, sizeof(ebuff), format, ap);
  (*error_handler_hook)(error, ebuff, MyFlags);
}

/**
  Print an error message.

  @note
    Goes through the (sole) function registered in error_handler_hook

  @param error     error number
  @param str       error message
  @param MyFlags   Flags
*/

void my_message(uint error, const char *str, myf MyFlags) {
  (*error_handler_hook)(error, str, MyFlags);
}

/**
  Register error messages for use with my_error().

    The function is expected to return addresses to NUL-terminated
    C character strings.
    NULL pointers and empty strings ("") are allowed. These will be mapped to
    "Unknown error" when my_error() is called with a matching error number.
    This function registers the error numbers 'first' to 'last'.
    No overlapping with previously registered error numbers is allowed.

  @param   get_errmsg  function that returns error messages
  @param   first       error number of first message in the array
  @param   last        error number of last message in the array

  @retval  0        OK
  @retval  != 0     Error
*/

int my_error_register(const char *(*get_errmsg)(int), int first, int last) {
  struct my_err_head *meh_p;
  struct my_err_head **search_meh_pp;

  /* Allocate a new header structure. */
  if (!(meh_p = (struct my_err_head *)my_malloc(
            key_memory_my_err_head, sizeof(struct my_err_head), MYF(MY_WME))))
    return 1;
  meh_p->get_errmsg = get_errmsg;
  meh_p->meh_first = first;
  meh_p->meh_last = last;

  /* Search for the right position in the list. */
  for (search_meh_pp = &my_errmsgs_list; *search_meh_pp;
       search_meh_pp = &(*search_meh_pp)->meh_next) {
    if ((*search_meh_pp)->meh_last > first) break;
  }

  /* Error numbers must be unique. No overlapping is allowed. */
  if (*search_meh_pp && ((*search_meh_pp)->meh_first <= last)) {
    my_free(meh_p);
    return 1;
  }

  /* Insert header into the chain. */
  meh_p->meh_next = *search_meh_pp;
  *search_meh_pp = meh_p;
  return 0;
}

/**
  Unregister formerly registered error messages.

    This function unregisters the error numbers 'first' to 'last'.
    These must have been previously registered by my_error_register().
    'first' and 'last' must exactly match the registration.
    If a matching registration is present, the header is removed from the
    list.

  @param   first     error number of first message
  @param   last      error number of last message

  @retval  true      Error, no such number range registered.
  @retval  false     OK
*/

bool my_error_unregister(int first, int last) {
  struct my_err_head *meh_p;
  struct my_err_head **search_meh_pp;

  /* Search for the registration in the list. */
  for (search_meh_pp = &my_errmsgs_list; *search_meh_pp;
       search_meh_pp = &(*search_meh_pp)->meh_next) {
    if (((*search_meh_pp)->meh_first == first) &&
        ((*search_meh_pp)->meh_last == last))
      break;
  }
  if (!*search_meh_pp) return true;

  /* Remove header from the chain. */
  meh_p = *search_meh_pp;
  *search_meh_pp = meh_p->meh_next;

  /* Free the header. */
  my_free(meh_p);

  return false;
}

/**
  Unregister all formerly registered error messages.

    This function unregisters all error numbers that previously have
    been previously registered by my_error_register().
    All headers are removed from the list; the messages themselves are
    not released here as they may be static.
*/

void my_error_unregister_all(void) {
  struct my_err_head *cursor, *saved_next;

  for (cursor = my_errmsgs_globerrs.meh_next; cursor != nullptr;
       cursor = saved_next) {
    /* We need this ptr, but we're about to free its container, so save it. */
    saved_next = cursor->meh_next;

    my_free(cursor);
  }
  my_errmsgs_globerrs.meh_next = nullptr; /* Freed in first iteration above. */

  my_errmsgs_list = &my_errmsgs_globerrs;
}

/**
  Issue a message locally (i.e. on the same host the program is
  running on, don't transmit to a client).

  This is the default value for local_message_hook, and therefore
  the default printer for my_message_local(). mysys users should
  not call this directly, but go through my_message_local() instead.

  This printer prepends an Error/Warning/Note label to the string,
  then prints it to stderr using my_message_stderr().
  Since my_message_stderr() appends a '\n', the format string
  should not end in a newline.

  @param ll      log level: (ERROR|WARNING|INFORMATION)_LEVEL
                 the printer may use these to filter for verbosity
  @param ecode   Error code of a error message.
  @param args    parameters to go with the error message.
*/
void my_message_local_stderr(enum loglevel ll, uint ecode, va_list args) {
  char buff[1024];
  size_t len;
  DBUG_TRACE;

  len = snprintf(
      buff, sizeof(buff), "[%s] ",
      (ll == ERROR_LEVEL ? "ERROR" : ll == WARNING_LEVEL ? "Warning" : "Note"));
  vsnprintf(buff + len, sizeof(buff) - len, EE(ecode), args);

  my_message_stderr(0, buff, MYF(0));
}

/**
  Issue a message locally (i.e. on the same host the program is
  running on, don't transmit to a client).

  This goes through local_message_hook, i.e. by default, it calls
  my_message_local_stderr() which prepends an Error/Warning/Note
  label to the string, then prints it to stderr using my_message_stderr().
  More advanced programs can use their own printers; mysqld for instance
  uses its own error log facilities which prepend an ISO 8601 / RFC 3339
  compliant timestamp etc.

  @param ll      log level: (ERROR|WARNING|INFORMATION)_LEVEL
                 the printer may use these to filter for verbosity
  @param ecode   Error code of a error message.
  @param ...     parameters to go with the error message.
*/
void my_message_local(enum loglevel ll, uint ecode, ...) {
  va_list args;
  DBUG_TRACE;

  va_start(args, ecode);
  (*local_message_hook)(ll, ecode, args);
  va_end(args);
}
