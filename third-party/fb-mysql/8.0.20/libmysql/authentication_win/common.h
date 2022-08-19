/* Copyright (c) 2011, 2017, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef COMMON_H
#define COMMON_H

#include <mysql/plugin_auth_common.h>  // for MYSQL_PLUGIN_VIO
#include <sspi.h>                      // for CtxtHandle
#include <windows.h>

#include "my_dbug.h"

/// Maximum length of the target service name.
#define MAX_SERVICE_NAME_LENGTH 1024

/** Debugging and error reporting infrastructure ***************************/

/*
  Note: We use plugin local logging and error reporting mechanisms until
  WL#2940 (plugin service: error reporting) is available.
*/

#undef INFO
#undef WARNING
#undef ERROR

struct error_log_level {
  typedef enum { INFO, WARNING, ERROR } type;
};

extern "C" int opt_auth_win_log_level;
unsigned int get_log_level(void);
void set_log_level(unsigned int);

/*
  If DEBUG_ERROR_LOG is defined then error logging happens only
  in debug-copiled code. Otherwise ERROR_LOG() expands to
  error_log_print() even in production code.

  Note: Macro ERROR_LOG() can use printf-like format string like this:

    ERROR_LOG(Level, ("format string", args));

  The implementation should handle it correctly. Currently it is passed
  to fprintf() (see error_log_vprint() function).
*/

#if defined(DEBUG_ERROR_LOG) && defined(DBUG_OFF)
#define ERROR_LOG(Level, Msg) \
  do {                        \
  } while (0)
#else
#define ERROR_LOG(Level, Msg) error_log_print<error_log_level::Level> Msg
#endif

void error_log_vprint(error_log_level::type level, const char *fmt,
                      va_list args);

template <error_log_level::type Level>
void error_log_print(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  error_log_vprint(Level, fmt, args);
  va_end(args);
}

typedef char Error_message_buf[1024];
const char *get_last_error_message(Error_message_buf);

/*
  Internal implementation of debug message printing which does not use
  dbug library. This is invoked via macro:

    DBUG_PRINT_DO(Keyword, ("format string", args));

  This is supposed to be used as an implementation of DBUG_PRINT() macro,
  unless the dbug library implementation is used or debug messages are disabled.
*/

#ifndef DBUG_OFF

#define DBUG_PRINT_DO(Keyword, Msg)            \
  do {                                         \
    if (4 > get_log_level()) break;            \
    fprintf(stderr, "winauth: %s: ", Keyword); \
    debug_msg Msg;                             \
  } while (0)

inline void debug_msg(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  fputc('\n', stderr);
  fflush(stderr);
  va_end(args);
}

#else
#define DBUG_PRINT_DO(K, M) \
  do {                      \
  } while (0)
#endif

#ifndef WINAUTH_USE_DBUG_LIB

#undef DBUG_PRINT
#define DBUG_PRINT(Keyword, Msg) DBUG_PRINT_DO(Keyword, Msg)

/*
  Redefine few more debug macros to make sure that no symbols from
  dbug library are used.
*/

#undef DBUG_ENTER
#define DBUG_ENTER(X) \
  do {                \
  } while (0)

#undef DBUG_RETURN
#define DBUG_RETURN(X) return (X)

#undef DBUG_ASSERT
#ifndef DBUG_OFF
#define DBUG_ASSERT(X) assert(X)
#else
#define DBUG_ASSERT(X) \
  do {                 \
  } while (0)
#endif

#undef DBUG_DUMP
#define DBUG_DUMP(A, B, C) \
  do {                     \
  } while (0)

#endif

/** Blob class *************************************************************/

typedef unsigned char byte;

/**
  Class representing a region of memory (e.g., a string or binary buffer).

  @note This class does not allocate memory. It merely describes a region
  of memory which must be allocated externally (if it is dynamic memory).
*/

class Blob {
  byte *m_ptr;   ///< Pointer to the first byte of the memory region.
  size_t m_len;  ///< Length of the memory region.

 public:
  Blob() : m_ptr(NULL), m_len(0) {}

  Blob(const byte *ptr, const size_t len)
      : m_ptr(const_cast<byte *>(ptr)), m_len(len) {}

  Blob(const char *str) : m_ptr((byte *)str) { m_len = strlen(str); }

  byte *ptr() const { return m_ptr; }

  size_t len() const { return m_len; }

  byte &operator[](unsigned pos) const {
    static byte out_of_range = 0;  // alas, no exceptions...
    return pos < len() ? m_ptr[pos] : out_of_range;
  }

  bool is_null() const { return m_ptr == NULL; }

  void trim(size_t l) { m_len = l; }
};

/** Connection class *******************************************************/

/**
  Convenience wrapper around MYSQL_PLUGIN_VIO object providing basic
  read/write operations.
*/

class Connection {
  MYSQL_PLUGIN_VIO *m_vio;  ///< Pointer to @c MYSQL_PLUGIN_VIO structure.

  /**
    If non-zero, indicates that connection is broken. If this has happened
    because of failed operation, stores non-zero error code from that failure.
  */
  int m_error;

 public:
  Connection(MYSQL_PLUGIN_VIO *vio);
  int write(const Blob &);
  Blob read();

  int error() const { return m_error; }
};

/** Sid class **************************************************************/

/**
  Class for storing and manipulating Windows security identifiers (SIDs).
*/

class Sid {
  TOKEN_USER *m_data;   ///< Pointer to structure holding identifier's data.
  SID_NAME_USE m_type;  ///< Type of identified entity.

 public:
  Sid(const wchar_t *);
  Sid(HANDLE sec_token);
  ~Sid();

  bool is_valid(void) const;

  bool is_group(void) const {
    return m_type == SidTypeGroup || m_type == SidTypeWellKnownGroup ||
           m_type == SidTypeAlias;
  }

  bool is_user(void) const { return m_type == SidTypeUser; }

  bool operator==(const Sid &);

  operator PSID() const { return (PSID)m_data->User.Sid; }

#ifndef DBUG_OFF

 private:
  char *m_as_string;  ///< Cached string representation of the SID.
 public:
  const char *as_string();

#endif
};

/** UPN class **************************************************************/

/**
  An object of this class obtains and stores User Principal Name of the
  account under which current process is running.
*/

class UPN {
  char *m_buf;   ///< Pointer to UPN in utf8 representation.
  size_t m_len;  ///< Length of the name.

 public:
  UPN();
  ~UPN();

  bool is_valid() const { return m_len > 0; }

  const Blob as_blob() const {
    return m_len ? Blob((byte *)m_buf, m_len) : Blob();
  }

  const char *as_string() const { return (const char *)m_buf; }
};

char *wchar_to_utf8(const wchar_t *, size_t *);
wchar_t *utf8_to_wchar(const char *, size_t *);

#endif
