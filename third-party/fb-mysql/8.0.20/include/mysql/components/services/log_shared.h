/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef LOG_SHARED_H
#define LOG_SHARED_H

#include <mysql/mysql_lex_string.h>

#include "my_basename.h"
#include "my_inttypes.h"
#include "my_loglevel.h"

/** fallback: includer may not have set this to something sensible. */
#ifndef LOG_SUBSYSTEM_TAG
#define LOG_SUBSYSTEM_TAG NULL
#endif

/**
  The logging sub-system internally uses the log_line structure to pass
  data around. This header primarily the specifics and symbols of that
  structure.

  Within the server, those interfaces may be used, but it is usually
  preferable to use the fluent C++ convenience class LogErr() instead
  where possible (and the variadic convenience function log_message()
  where it is not). (see sql/log.h).

  (The legacy calls sql_print_(error|warning|information) have now
  been defined to use log_message().)

  Finally, this header defines log types (error log etc.) as well
  as log item types (timestamp, message, ...) used by the logging
  components; these are shared between the variadic convenience
  functions (log_message(), sql_print_*()) as well as the lower
  level services using the log_line structure.
*/

/*
  By historical convention in the lex context,
  CSTRING means "constant lex string (char * + size_t)",
  not "C-style string" (char *, \0 terminated)!
*/
typedef struct MYSQL_LEX_CSTRING LEX_CSTRING;

/**
  log_type -- which log to send data to
  check vs enum_log_table_type and LOG_FILE/LOG_TABLE/LOG_NONE
*/
enum enum_log_type {
  LOG_TYPE_UNDEF = 0,
  LOG_TYPE_ERROR = 1,
  LOG_TYPE_GENERAL = 2,
  LOG_TYPE_SLOW = 4,
  LOG_TYPE_AUDIT = 8,
  LOG_TYPE_MISC = 16
};

/**
  item_type -- what to log


  Used by variadic convenience interface
  log_message(), legacy sql_print_*(), legacy my_plugin_log_message().
  Also available via the log_builtins service as message().

  (Wherever possible, use the fluent C++ wrapper LogErr() (see
  log_builtins.h) instead, though.)

  LOG_ITEM_GEN_CSTRING lets the caller submit a \0 terminated,
  C-style string for convenience; it will be converted to
  lex style (char *, size_t) on submission.

  LOG_ITEM_END should not be used in the variadic interface
  as submitting a log line without an actual message is discouraged.
  Instead LOG_ITEM_LOG_MESSAGE or LOG_ITEM_LOG_LOOKUP should be used
  as the last LOG_ITEM_* key-value pairs:

  - LOG_ITEM_LOG_MESSAGE can be used to submit an ad hoc message
    directly while debugging.

  - LOG_ITEM_LOG_LOOKUP asks for the given error number to be
    replaced by the associated error message (i.e. the error message
    for the error number will be looked up, and the LOOKUP item will
    be replaced by a MESSAGE one).

  In variadic submission, both ad hoc and well-known error messages
  are considered printf()-style format strings, and should be followed
  by any arguments/variables required by that format string.

  In some situations, the variadic interface will automatically
  generate some items ("If you cannot afford a timestamp, one will
  be provided for you.").  If an item of the required type already
  exists, the submission interface should not generate another.

  Old-style plug-ins using my_plugin_log_message() will automatically
  be tagged with a default LOG_ITEM_MSC_COMPONENT of their plug-in name.



  Non-variadic interface

  In non-variadic submission (i.e. all functions accepting a log_line),
  LOG_ITEM_LOG_LOOKUP and LOG_ITEM_GEN_CSTRING are not valid item types,
  while LOG_ITEM_LOG_MESSAGE must already be a string literal (i.e. any
  substitutions must already have taken place).
*/
typedef enum enum_log_item_type {
  LOG_ITEM_END = 0,                  /**< end of list, see above */
  LOG_ITEM_LOG_TYPE = 1 << 0,        /**< error log, etc. */
  LOG_ITEM_SQL_ERRCODE = 1 << 1,     /**< mysql error code (numeric) */
  LOG_ITEM_SQL_ERRSYMBOL = 1 << 2,   /**< mysql error code (symbolic) */
  LOG_ITEM_SQL_STATE = 1 << 3,       /**< SQL state */
  LOG_ITEM_SYS_ERRNO = 1 << 4,       /**< OS errno */
  LOG_ITEM_SYS_STRERROR = 1 << 5,    /**< OS strerror() */
  LOG_ITEM_SRC_FILE = 1 << 6,        /**< log called from file ... */
  LOG_ITEM_SRC_LINE = 1 << 7,        /**< log called from line ... */
  LOG_ITEM_SRC_FUNC = 1 << 8,        /**< log called from function ... */
  LOG_ITEM_SRV_SUBSYS = 1 << 9,      /**< log called from subsystem ... */
  LOG_ITEM_SRV_COMPONENT = 1 << 10,  /**< log called from component ... */
  LOG_ITEM_MSC_USER = 1 << 11,       /**< offending thread owned by ... */
  LOG_ITEM_MSC_HOST = 1 << 12,       /**< responsible user on host ... */
  LOG_ITEM_SRV_THREAD = 1 << 13,     /**< connection ID */
  LOG_ITEM_SQL_QUERY_ID = 1 << 14,   /**< query ID */
  LOG_ITEM_SQL_TABLE_NAME = 1 << 15, /**< table name */
  LOG_ITEM_LOG_PRIO = 1 << 16,       /**< log priority (error, warn, ...) */
  LOG_ITEM_LOG_LABEL = 1 << 17,      /**< label, unless auto-derived */
  LOG_ITEM_LOG_VERBATIM = 1 << 18,   /**< the message, no % substitutions */
  LOG_ITEM_LOG_MESSAGE = 1 << 19,    /**< the message, format string */
  LOG_ITEM_LOG_LOOKUP = 1 << 20,     /**< insert message by error-code */
  LOG_ITEM_LOG_TIMESTAMP = 1 << 21,  /**< ISO8601 timestamp */
  LOG_ITEM_LOG_TS = 1 << 22,         /**< millisecs since epoch */
  LOG_ITEM_LOG_BUFFERED = 1 << 23,   /**< integer timestamp if/when buffered */
  LOG_ITEM_LOG_SUPPRESSED = 1 << 24, /**< "and ... more" throttled */
  LOG_ITEM_GEN_FLOAT = 1 << 25,      /**< float not otherwise specified */
  LOG_ITEM_GEN_INTEGER = 1 << 26,    /**< integer not otherwise specified */
  LOG_ITEM_GEN_LEX_STRING = 1 << 27, /**< lex string not otherwise specified */
  LOG_ITEM_GEN_CSTRING = 1 << 28     /**< C-string not otherwise specified */
} log_item_type;

/* some suggested keys for generic items */

/** DIAGNOSTICS: for da->message_text() */
#define LOG_TAG_DIAG "DIAGNOSTICS"

/** AUX: supplementary data not fitting any of the wellknown keys */
#define LOG_TAG_AUX "AUX"

/* data type */
typedef enum enum_log_item_class {
  LOG_UNTYPED = 0,   /**< undefined */
  LOG_CSTRING = 1,   /**< string  (char * + \0; variadic API only) */
  LOG_INTEGER = 2,   /**< integer (long long)  */
  LOG_FLOAT = 3,     /**< float   (double)     */
  LOG_LEX_STRING = 4 /**< string  (char *, size_t) */
} log_item_class;

/* do we need to release any parts of the item after use? */
enum enum_log_item_free {
  LOG_ITEM_FREE_NONE = 0,
  LOG_ITEM_FREE_KEY = 1,
  LOG_ITEM_FREE_VALUE = 2
};

/* union: payload */
typedef union _log_item_data {
  longlong data_integer;
  double data_float;
  LEX_CSTRING data_string;
} log_item_data;

/* item key: for now, a C-string */
typedef const char *log_item_key;

/* log item: key/value */
typedef struct _log_item {
  log_item_type type;
  log_item_class item_class;
  log_item_key key;
  log_item_data data;
  uint32 alloc;
} log_item;

/* service helpers */
typedef enum enum_log_item_error {
  LOG_ITEM_OK = 0,
  LOG_ITEM_TYPE_NOT_FOUND = -1,
  LOG_ITEM_TYPE_RESERVED = -2,
  LOG_ITEM_CLASS_MISMATCH = -3,
  LOG_ITEM_KEY_MISMATCH = -4,
  LOG_ITEM_STRING_NULL = -5,
  LOG_ITEM_KEY_NULL = -6
} log_item_error;

/** a bit mask of log_types. standardizing the width to 64 bit. */
typedef uint64 log_item_type_mask;

/** log line: a collection of log items */
typedef struct _log_line log_line;

/** log iter: an iterator over the collection of log items in a log line */
typedef struct _log_item_iter log_item_iter;

/** advisory. components must not rely on others using the same value. */
#define LOG_BUFF_MAX 8192

/** 26 for regular timestamp, plus 7 (".123456") when using micro-seconds */
static const int iso8601_size = 33;

/**
  index of first server error message: messages with this index or
  higher are intended for the error log; messages below this index
  are intended for the client
*/
#define ER_SERVER_RANGE_START 10000

#endif
