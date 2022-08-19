/* Copyright (c) 2006, 2020, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file
  File containing constants that can be used throughout the server.

  @note This file shall not contain any includes of any kinds.
*/

#ifndef SQL_CONST_INCLUDED
#define SQL_CONST_INCLUDED

#include "my_inttypes.h"

#define LIBLEN FN_REFLEN - FN_LEN /* Max l{ngd p} dev */
/**
  The maximum length of a key in the table definition cache.

  The key consists of the schema name, a '\0' character, the table
  name and a '\0' character. Hence NAME_LEN * 2 + 1 + 1.

  Additionally, the key can be suffixed with either 4 + 4 extra bytes
  for slave tmp tables, or with a single extra byte for tables in a
  secondary storage engine. Add 4 + 4 to account for either of these
  suffixes.
*/
#define MAX_DBKEY_LENGTH (NAME_LEN * 2 + 1 + 1 + 4 + 4)
#define MAX_ALIAS_NAME 256
#define MAX_FIELD_NAME 34 /* Max colum name length +2 */
#define MAX_SYS_VAR_LENGTH 32
#define MAX_KEY MAX_INDEXES  /* Max used keys */
#define MAX_REF_PARTS 16U    /* Max parts used as ref */
#define MAX_KEY_LENGTH 3072U /* max possible key */
#define MAX_REFLENGTH 8      /* Max length for record ref */

#define MAX_MBWIDTH 3 /* Max multibyte sequence */
#define MAX_FIELD_CHARLENGTH 255
#define MAX_FIELD_VARCHARLENGTH 65535
#define MAX_FIELD_BLOBLENGTH UINT_MAX32 /* cf field_blob::get_length() */
/**
  CHAR and VARCHAR fields longer than this number of characters are converted
  to BLOB.
  Non-character fields longer than this number of bytes are converted to BLOB.
  Comparisons should be '>' or '<='.
*/
#define CONVERT_IF_BIGGER_TO_BLOB 512 /* Used for CREATE ... SELECT */

/* Max column width +1 */
#define MAX_FIELD_WIDTH (MAX_FIELD_CHARLENGTH * MAX_MBWIDTH + 1)

#define MAX_BIT_FIELD_LENGTH 64 /* Max length in bits for bit fields */

#define MAX_DATE_WIDTH 10                /* YYYY-MM-DD */
#define MAX_TIME_WIDTH 10                /* -838:59:59 */
#define MAX_TIME_FULL_WIDTH 23           /* -DDDDDD HH:MM:SS.###### */
#define MAX_DATETIME_FULL_WIDTH 29       /* YYYY-MM-DD HH:MM:SS.###### AM */
#define MAX_DATETIME_WIDTH 19            /* YYYY-MM-DD HH:MM:SS */
#define MAX_DATETIME_COMPRESSED_WIDTH 14 /* YYYYMMDDHHMMSS */

#define DATE_INT_DIGITS 8      /* YYYYMMDD       */
#define TIME_INT_DIGITS 7      /* hhhmmss        */
#define DATETIME_INT_DIGITS 14 /* YYYYMMDDhhmmss */

/**
  MAX_TABLES and xxx_TABLE_BIT are used in optimization of table factors and
  expressions, and in join plan generation.
  MAX_TABLES counts the maximum number of tables that can be handled in a
  join operation. It is the number of bits in the table_map, minus the
  number of pseudo table bits (bits that do not represent actual tables, but
  still need to be handled by our algorithms). The pseudo table bits are:
  INNER_TABLE_BIT is set for all expressions that contain a parameter,
  a subquery that accesses tables, or a function that accesses tables.
  An expression that has only INNER_TABLE_BIT is constant for the duration
  of a query expression, but must be evaluated at least once during execution.
  OUTER_REF_TABLE_BIT is set for expressions that contain a column that
  is resolved as an outer reference. Also notice that all subquery items
  between the column reference and the query block where the column is
  resolved, have this bit set. Expressions that are represented by this bit
  are constant for the duration of the subquery they are defined in.
  RAND_TABLE_BIT is set for expressions containing a non-deterministic
  element, such as a random function or a non-deterministic function.
  Expressions containing this bit cannot be evaluated once and then cached,
  they must be evaluated at latest possible point.
  MAX_TABLES_FOR_SIZE adds the pseudo bits and is used for sizing purposes only.
*/
#define MAX_TABLES_FOR_SIZE (sizeof(table_map) * 8)  ///< Use for sizing ONLY
#define MAX_TABLES (MAX_TABLES_FOR_SIZE - 3)         ///< Max tables in join
#define INNER_TABLE_BIT (((table_map)1) << (MAX_TABLES + 0))
#define OUTER_REF_TABLE_BIT (((table_map)1) << (MAX_TABLES + 1))
#define RAND_TABLE_BIT (((table_map)1) << (MAX_TABLES + 2))
#define PSEUDO_TABLE_BITS \
  (INNER_TABLE_BIT | OUTER_REF_TABLE_BIT | RAND_TABLE_BIT)
#define MAX_FIELDS 4096 /* Maximum number of columns */
#define MAX_PARTITIONS 8192

#define MAX_ENUM_VALUES 65535         /* Max number of enumeration values */
#define MAX_INTERVAL_VALUE_LENGTH 255 /* Max length of enum/set values */

#define MAX_SELECT_NESTING (sizeof(nesting_map) * 8 - 1)

#define DEFAULT_SORT_MEMORY (256UL * 1024UL)
#define MIN_SORT_MEMORY (32UL * 1024UL)

/* Some portable defines */

#define STRING_BUFFER_USUAL_SIZE 80

/* Memory allocated when parsing a statement / saving a statement */
#define MEM_ROOT_BLOCK_SIZE 8192
#define MEM_ROOT_PREALLOC 8192
#define TRANS_MEM_ROOT_BLOCK_SIZE 4096
#define TRANS_MEM_ROOT_PREALLOC 4096

#define DEFAULT_ERROR_COUNT 1024
#define EXTRA_RECORDS 10      /* Extra records in sort */
#define SCROLL_EXTRA 5        /* Extra scroll-rows. */
#define FERR -1               /* Error from my_functions */
#define CREATE_MODE 0         /* Default mode on new files */
#define NAMES_SEP_CHAR '\377' /* Char to sep. names */

#define READ_RECORD_BUFFER (uint)(IO_SIZE * 8) /* Pointer_buffer_size */
#define DISK_BUFFER_SIZE (uint)(IO_SIZE * 16)  /* Size of diskbuffer */

/***************************************************************************
  Configuration parameters
****************************************************************************/

#define ACL_CACHE_SIZE 256
#define MAX_PASSWORD_LENGTH 32
#define HOST_CACHE_SIZE 128
#define MAX_ACCEPT_RETRY 10  // Test accept this many times
#define MAX_FIELDS_BEFORE_HASH 32
#define USER_VARS_HASH_SIZE 16
#define TABLE_OPEN_CACHE_MIN 400
#define TABLE_OPEN_CACHE_DEFAULT 4000
static const ulong TABLE_DEF_CACHE_DEFAULT = 400;
static const ulong SCHEMA_DEF_CACHE_DEFAULT = 256;
static const ulong STORED_PROGRAM_DEF_CACHE_DEFAULT = 256;
static const ulong TABLESPACE_DEF_CACHE_DEFAULT = 256;
static const ulong EVENT_DEF_CACHE_DEFAULT = 256;

/**
  Maximum number of connections default value.
  151 is larger than Apache's default max children,
  to avoid "too many connections" error in a common setup.
*/
#define MAX_CONNECTIONS_DEFAULT 151
/**
  We must have room for at least 400 table definitions in the table
  cache, since otherwise there is no chance prepared
  statements that use these many tables can work.
  Prepared statements use table definition cache ids (table_map_id)
  as table version identifiers. If the table definition
  cache size is less than the number of tables used in a statement,
  the contents of the table definition cache is guaranteed to rotate
  between a prepare and execute. This leads to stable validation
  errors. In future we shall use more stable version identifiers,
  for now the only solution is to ensure that the table definition
  cache can contain at least all tables of a given statement.
*/
static const ulong TABLE_DEF_CACHE_MIN = 400;
static const ulong SCHEMA_DEF_CACHE_MIN = 256;
static const ulong STORED_PROGRAM_DEF_CACHE_MIN = 256;
static const ulong TABLESPACE_DEF_CACHE_MIN = 256;
static const ulong EVENT_DEF_CACHE_MIN = 256;

/*
  Stack reservation.
  Feel free to raise this by the smallest amount you can to get the
  "execution_constants" test to pass.
*/
#if defined HAVE_UBSAN && SIZEOF_CHARP == 4
#define STACK_MIN_SIZE 30000  // Abort if less stack during eval.
#else
#define STACK_MIN_SIZE 20000  // Abort if less stack during eval.
#endif

#define STACK_MIN_SIZE_FOR_OPEN 1024 * 80

#if defined(__SUNPRO_CC)
#define STACK_BUFF_ALLOC 352 * 2  ///< For stack overrun checks
#else
#define STACK_BUFF_ALLOC 352  ///< For stack overrun checks
#endif

#ifndef MYSQLD_NET_RETRY_COUNT
#define MYSQLD_NET_RETRY_COUNT 10  ///< Abort read after this many int.
#endif

#define QUERY_ALLOC_BLOCK_SIZE 8192
#define QUERY_ALLOC_PREALLOC_SIZE 8192
#define TRANS_ALLOC_BLOCK_SIZE 4096
#define TRANS_ALLOC_PREALLOC_SIZE 4096
#define RANGE_ALLOC_BLOCK_SIZE 4096
#define ACL_ALLOC_BLOCK_SIZE 1024
#define UDF_ALLOC_BLOCK_SIZE 1024
#define TABLE_ALLOC_BLOCK_SIZE 1024
#define WARN_ALLOC_BLOCK_SIZE 2048

/*
  The following parameters is to decide when to use an extra cache to
  optimise seeks when reading a big table in sorted order
*/
#define MIN_FILE_LENGTH_TO_USE_ROW_CACHE (10L * 1024 * 1024)
#define MIN_ROWS_TO_USE_TABLE_CACHE 100
#define MIN_ROWS_TO_USE_BULK_INSERT 100

/*
  For sequential disk seeks the cost formula is:
    DISK_SEEK_BASE_COST + DISK_SEEK_PROP_COST * #blocks_to_skip

  The cost of average seek
    DISK_SEEK_BASE_COST + DISK_SEEK_PROP_COST*BLOCKS_IN_AVG_SEEK =1.0.
*/
#define DISK_SEEK_BASE_COST (0.9)

#define BLOCKS_IN_AVG_SEEK 128

#define DISK_SEEK_PROP_COST (0.1 / BLOCKS_IN_AVG_SEEK)

/**
  Number of rows in a reference table when refereed through a not unique key.
  This value is only used when we don't know anything about the key
  distribution.
*/
#define MATCHING_ROWS_IN_OTHER_TABLE 10

#define MY_CHARSET_BIN_MB_MAXLEN 1

/** Don't pack string keys shorter than this (if PACK_KEYS=1 isn't used). */
#define KEY_DEFAULT_PACK_LENGTH 8

/** Characters shown for the command in 'show processlist'. */
#define PROCESS_LIST_WIDTH 100
/* Characters shown for the command in 'information_schema.processlist' */
#define PROCESS_LIST_INFO_WIDTH 65535

#define PRECISION_FOR_DOUBLE 53
#define PRECISION_FOR_FLOAT 24

/* -[digits].E+## */
#define MAX_FLOAT_STR_LENGTH (FLT_DIG + 6)
/* -[digits].E+### */
#define MAX_DOUBLE_STR_LENGTH (DBL_DIG + 7)

/*
  Default time to wait before aborting a new client connection
  that does not respond to "initial server greeting" timely
*/
#define CONNECT_TIMEOUT 10

/* The following can also be changed from the command line */
#define DEFAULT_CONCURRENCY 10
#define DELAYED_LIMIT 100 /**< pause after xxx inserts */
#define DELAYED_QUEUE_SIZE 1000
#define DELAYED_WAIT_TIMEOUT 5 * 60 /**< Wait for delayed insert */

#define LONG_TIMEOUT ((ulong)3600L * 24L * 365L)
#define LONG_TIMEOUT_NSEC ((ulonglong)LONG_TIMEOUT * 1000000000ULL)

/**
  Maximum length of time zone name that we support (Time zone name is
  char(64) in db). mysqlbinlog needs it.
*/
#define MAX_TIME_ZONE_NAME_LENGTH (NAME_LEN + 1)

/* Max length for the db-metadata option */
#define DB_METADATA_MAX_LENGTH 1024

#if defined(_WIN32)
#define INTERRUPT_PRIOR -2
#define CONNECT_PRIOR -1
#define WAIT_PRIOR 0
#define QUERY_PRIOR 2
#else
#define INTERRUPT_PRIOR 10
#define CONNECT_PRIOR 9
#define WAIT_PRIOR 8
#define QUERY_PRIOR 6
#endif /* _WIN32 */

/*
  Flags below are set when we perform
  context analysis of the statement and make
  subqueries non-const. It prevents subquery
  evaluation at context analysis stage.
*/

/*
  Don't evaluate this subquery during statement prepare even if
  it's a constant one. The flag is switched off in the end of
  mysqld_stmt_prepare.
*/
#define CONTEXT_ANALYSIS_ONLY_PREPARE 1
/*
  Special SELECT_LEX::prepare mode: changing of query is prohibited.
  When creating a view, we need to just check its syntax omitting
  any optimizations: afterwards definition of the view will be
  reconstructed by means of ::print() methods and written to
  to an .frm file. We need this definition to stay untouched.
*/
#define CONTEXT_ANALYSIS_ONLY_VIEW 2
/*
  Don't evaluate this subquery during derived table prepare even if
  it's a constant one.
*/
#define CONTEXT_ANALYSIS_ONLY_DERIVED 4

/* @@optimizer_switch flags. These must be in sync with optimizer_switch_typelib
 */
#define OPTIMIZER_SWITCH_INDEX_MERGE (1ULL << 0)
#define OPTIMIZER_SWITCH_INDEX_MERGE_UNION (1ULL << 1)
#define OPTIMIZER_SWITCH_INDEX_MERGE_SORT_UNION (1ULL << 2)
#define OPTIMIZER_SWITCH_INDEX_MERGE_INTERSECT (1ULL << 3)
#define OPTIMIZER_SWITCH_ENGINE_CONDITION_PUSHDOWN (1ULL << 4)
#define OPTIMIZER_SWITCH_INDEX_CONDITION_PUSHDOWN (1ULL << 5)
/** If this is off, MRR is never used. */
#define OPTIMIZER_SWITCH_MRR (1ULL << 6)
/**
   If OPTIMIZER_SWITCH_MRR is on and this is on, MRR is used depending on a
   cost-based choice ("automatic"). If OPTIMIZER_SWITCH_MRR is on and this is
   off, MRR is "forced" (i.e. used as long as the storage engine is capable of
   doing it).
*/
#define OPTIMIZER_SWITCH_MRR_COST_BASED (1ULL << 7)
#define OPTIMIZER_SWITCH_BNL (1ULL << 8)
#define OPTIMIZER_SWITCH_BKA (1ULL << 9)
#define OPTIMIZER_SWITCH_MATERIALIZATION (1ULL << 10)
#define OPTIMIZER_SWITCH_SEMIJOIN (1ULL << 11)
#define OPTIMIZER_SWITCH_LOOSE_SCAN (1ULL << 12)
#define OPTIMIZER_SWITCH_FIRSTMATCH (1ULL << 13)
#define OPTIMIZER_SWITCH_DUPSWEEDOUT (1ULL << 14)
#define OPTIMIZER_SWITCH_SUBQ_MAT_COST_BASED (1ULL << 15)
#define OPTIMIZER_SWITCH_USE_INDEX_EXTENSIONS (1ULL << 16)
#define OPTIMIZER_SWITCH_COND_FANOUT_FILTER (1ULL << 17)
#define OPTIMIZER_SWITCH_DERIVED_MERGE (1ULL << 18)
#define OPTIMIZER_SWITCH_USE_INVISIBLE_INDEXES (1ULL << 19)
#define OPTIMIZER_SKIP_SCAN (1ULL << 20)
#define OPTIMIZER_SWITCH_HASH_JOIN (1ULL << 21)
#define OPTIMIZER_SWITCH_PREFER_ORDERING_INDEX (1ULL << 22)
#define OPTIMIZER_SWITCH_GROUP_BY_LIMIT (1ULL << 23)
#define OPTIMIZER_SWITCH_LAST (1ULL << 24)

#define OPTIMIZER_SWITCH_DEFAULT                                          \
  (OPTIMIZER_SWITCH_INDEX_MERGE | OPTIMIZER_SWITCH_INDEX_MERGE_UNION |    \
   OPTIMIZER_SWITCH_INDEX_MERGE_SORT_UNION |                              \
   OPTIMIZER_SWITCH_INDEX_MERGE_INTERSECT |                               \
   OPTIMIZER_SWITCH_ENGINE_CONDITION_PUSHDOWN |                           \
   OPTIMIZER_SWITCH_INDEX_CONDITION_PUSHDOWN | OPTIMIZER_SWITCH_MRR |     \
   OPTIMIZER_SWITCH_MRR_COST_BASED | OPTIMIZER_SWITCH_BNL |               \
   OPTIMIZER_SWITCH_MATERIALIZATION | OPTIMIZER_SWITCH_SEMIJOIN |         \
   OPTIMIZER_SWITCH_LOOSE_SCAN | OPTIMIZER_SWITCH_FIRSTMATCH |            \
   OPTIMIZER_SWITCH_DUPSWEEDOUT | OPTIMIZER_SWITCH_SUBQ_MAT_COST_BASED |  \
   OPTIMIZER_SWITCH_USE_INDEX_EXTENSIONS |                                \
   OPTIMIZER_SWITCH_COND_FANOUT_FILTER | OPTIMIZER_SWITCH_DERIVED_MERGE | \
   OPTIMIZER_SKIP_SCAN | OPTIMIZER_SWITCH_HASH_JOIN |                     \
   OPTIMIZER_SWITCH_PREFER_ORDERING_INDEX)

enum SHOW_COMP_OPTION { SHOW_OPTION_YES, SHOW_OPTION_NO, SHOW_OPTION_DISABLED };

enum enum_mark_columns {
  MARK_COLUMNS_NONE,
  MARK_COLUMNS_READ,
  MARK_COLUMNS_WRITE,
  MARK_COLUMNS_TEMP
};

/*
  Exit code used by mysqld_exit, exit and _exit function
  to indicate successful termination of mysqld.
*/
#define MYSQLD_SUCCESS_EXIT 0
/*
  Exit code used by mysqld_exit, exit and _exit function to
  signify unsuccessful termination of mysqld. The exit
  code signifies the server should NOT BE RESTARTED AUTOMATICALLY
  by init systems like systemd.
*/
#define MYSQLD_ABORT_EXIT 1
/*
  Exit code used by mysqld_exit, exit and _exit function to
  signify unsuccessful termination of mysqld. The exit code
  signifies the server should be RESTARTED AUTOMATICALLY by
  init systems like systemd.
*/
#define MYSQLD_FAILURE_EXIT 2
/*
  Exit code used by mysqld_exit, my_thread_exit function which allows
  for external programs like systemd, mysqld_safe to restart mysqld
  server. The exit code  16 is choosen so it is safe as InnoDB code
  exit directly with values like 3.
*/
#define MYSQLD_RESTART_EXIT 16

#define UUID_LENGTH (8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12)

/*
  This enumeration type is used only by the function find_item_in_list
  to return the info on how an item has been resolved against a list
  of possibly aliased items.
  The item can be resolved:
   - against an alias name of the list's element (RESOLVED_AGAINST_ALIAS)
   - against non-aliased field name of the list  (RESOLVED_WITH_NO_ALIAS)
   - against an aliased field name of the list   (RESOLVED_BEHIND_ALIAS)
   - ignoring the alias name in cases when SQL requires to ignore aliases
     (e.g. when the resolved field reference contains a table name or
     when the resolved item is an expression)   (RESOLVED_IGNORING_ALIAS)
*/
enum enum_resolution_type {
  NOT_RESOLVED = 0,
  RESOLVED_BEHIND_ALIAS,
  RESOLVED_AGAINST_ALIAS,
  RESOLVED_WITH_NO_ALIAS,
  RESOLVED_IGNORING_ALIAS
};

/// Enumeration for {Item,SELECT_LEX[_UNIT],Table_function}::walk
enum class enum_walk {
  PREFIX = 0x01,
  POSTFIX = 0x02,
  SUBQUERY = 0x04,
  SUBQUERY_PREFIX = 0x05,  // Combine prefix and subquery traversal
  SUBQUERY_POSTFIX = 0x06  // Combine postfix and subquery traversal
};

inline enum_walk operator|(enum_walk lhs, enum_walk rhs) {
  return enum_walk(int(lhs) | int(rhs));
}

inline bool operator&(enum_walk lhs, enum_walk rhs) {
  return (int(lhs) & int(rhs)) != 0;
}

class Item;
/// Processor type for {Item,SELECT_LEX[_UNIT],Table_function}::walk
typedef bool (Item::*Item_processor)(uchar *arg);

#endif /* SQL_CONST_INCLUDED */
