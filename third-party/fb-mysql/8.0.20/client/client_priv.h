/*
   Copyright (c) 2001, 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef CLIENT_PRIV_INCLUDED
#define CLIENT_PRIV_INCLUDED

/* Common defines for all clients */

#include <mysql.h>

#include "errmsg.h"
#include "m_string.h"
#include "my_getopt.h"
#include "my_sys.h"

#ifndef WEXITSTATUS
#ifdef _WIN32
#define WEXITSTATUS(stat_val) (stat_val)
#else
#define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#endif

enum options_client {
  OPT_CHARSETS_DIR = 256,
  OPT_DEFAULT_CHARSET,
  OPT_PAGER,
  OPT_TEE,
  OPT_LOW_PRIORITY,
  OPT_AUTO_REPAIR,
  OPT_COMPRESS,
  OPT_DROP,
  OPT_LOCKS,
  OPT_KEYWORDS,
  OPT_OPTIMIZE,
  OPT_FTB,
  OPT_LTB,
  OPT_ENC,
  OPT_O_ENC,
  OPT_ESC,
  OPT_TABLES,
  OPT_MASTER_DATA,
  OPT_AUTOCOMMIT,
  OPT_AUTO_REHASH,
  OPT_LINE_NUMBERS,
  OPT_COLUMN_NAMES,
  OPT_CONNECT_TIMEOUT,
  OPT_MAX_ALLOWED_PACKET,
  OPT_NET_BUFFER_LENGTH,
  OPT_SELECT_LIMIT,
  OPT_MAX_JOIN_SIZE,
  OPT_SSL_SSL,
  OPT_SSL_KEY,
  OPT_SSL_CERT,
  OPT_SSL_CA,
  OPT_SSL_CAPATH,
  OPT_SSL_CIPHER,
  OPT_SHUTDOWN_TIMEOUT,
  OPT_LOCAL_INFILE,
  OPT_DELETE_MASTER_LOGS,
  OPT_COMPACT,
  OPT_PROMPT,
  OPT_IGN_LINES,
  OPT_TRANSACTION,
  OPT_MYSQL_PROTOCOL,
  OPT_SHARED_MEMORY_BASE_NAME,
  OPT_FRM,
  OPT_SKIP_OPTIMIZATION,
  OPT_COMPATIBLE,
  OPT_RECONNECT,
  OPT_DELIMITER,
  OPT_OPEN_FILES_LIMIT,
  OPT_SET_CHARSET,
  OPT_SET_GTID_PURGED,
  OPT_STOP_POSITION,
  OPT_START_DATETIME,
  OPT_STOP_DATETIME,
  OPT_SIGINT_IGNORE,
  OPT_HEXBLOB,
  OPT_ORDER_BY_PRIMARY,
  OPT_COUNT,
  OPT_TRIGGERS,
  OPT_MYSQL_ONLY_PRINT,
  OPT_MYSQL_LOCK_DIRECTORY,
  OPT_USE_THREADS,
  OPT_IMPORT_USE_THREADS,
  OPT_MYSQL_NUMBER_OF_QUERY,
  OPT_IGNORE_TABLE,
  OPT_INSERT_IGNORE,
  OPT_SHOW_WARNINGS,
  OPT_DROP_DATABASE,
  OPT_TZ_UTC,
  OPT_CREATE_SLAP_SCHEMA,
  OPT_MYSQLDUMP_SLAVE_APPLY,
  OPT_MYSQLDUMP_SLAVE_DATA,
  OPT_MYSQLDUMP_INCLUDE_MASTER_HOST_PORT,
  OPT_MYSQLDUMP_IGNORE_ERROR,
  OPT_SLAP_CSV,
  OPT_SLAP_CREATE_STRING,
  OPT_SLAP_AUTO_GENERATE_SQL_LOAD_TYPE,
  OPT_SLAP_AUTO_GENERATE_WRITE_NUM,
  OPT_SLAP_AUTO_GENERATE_ADD_AUTO,
  OPT_SLAP_AUTO_GENERATE_GUID_PRIMARY,
  OPT_SLAP_AUTO_GENERATE_EXECUTE_QUERIES,
  OPT_SLAP_AUTO_GENERATE_SECONDARY_INDEXES,
  OPT_SLAP_AUTO_GENERATE_UNIQUE_WRITE_NUM,
  OPT_SLAP_AUTO_GENERATE_UNIQUE_QUERY_NUM,
  OPT_SLAP_PRE_QUERY,
  OPT_SLAP_POST_QUERY,
  OPT_SLAP_PRE_SYSTEM,
  OPT_SLAP_POST_SYSTEM,
  OPT_SLAP_COMMIT,
  OPT_SLAP_DETACH,
  OPT_SLAP_NO_DROP,
  OPT_MYSQL_REPLACE_INTO,
  OPT_BASE64_OUTPUT_MODE,
  OPT_SERVER_ID,
  OPT_FIX_TABLE_NAMES,
  OPT_FIX_DB_NAMES,
  OPT_SSL_VERIFY_SERVER_CERT,
  OPT_AUTO_VERTICAL_OUTPUT,
  OPT_DEBUG_INFO,
  OPT_DEBUG_CHECK,
  OPT_COLUMN_TYPES,
  OPT_ERROR_LOG_FILE,
  OPT_WRITE_BINLOG,
  OPT_DUMP_DATE,
  OPT_INIT_COMMAND,
  OPT_PLUGIN_DIR,
  OPT_DEFAULT_AUTH,
  OPT_DEFAULT_PLUGIN,
  OPT_RAW_OUTPUT,
  OPT_WAIT_SERVER_ID,
  OPT_STOP_NEVER,
  OPT_BINLOG_ROWS_EVENT_MAX_SIZE,
  OPT_HISTIGNORE,
  OPT_BINARY_MODE,
  OPT_SSL_CRL,
  OPT_SSL_CRLPATH,
  OPT_MYSQLBINLOG_SKIP_GTIDS,
  OPT_MYSQLBINLOG_INCLUDE_GTIDS,
  OPT_MYSQLBINLOG_INCLUDE_GTIDS_FROM_FILE,
  OPT_MYSQLBINLOG_EXCLUDE_GTIDS,
  OPT_MYSQLBINLOG_EXCLUDE_GTIDS_FROM_FILE,
  OPT_REMOTE_PROTO,
  OPT_CONFIG_ALL,
  OPT_REWRITE_DB,
  OPT_SERVER_PUBLIC_KEY,
  OPT_ENABLE_CLEARTEXT_PLUGIN,
  OPT_CONNECTION_SERVER_ID,
  OPT_TLS_VERSION,
  OPT_SSL_MODE,
  OPT_PRINT_TABLE_METADATA,
  OPT_SSL_FIPS_MODE,
  OPT_TLS_CIPHERSUITES,
  OPT_MYSQL_BINARY_AS_HEX,
  OPT_PRINT_ORDERING_KEY,
  OPT_LRA_SIZE,
  OPT_LRA_SLEEP,
  OPT_LRA_PAGES_BEFORE_SLEEP,
  OPT_TIMEOUT,
  OPT_RECEIVE_BUFFER_SIZE,
  OPT_LONG_QUERY_TIME,
  OPT_IGNORE_VIEWS,
  OPT_START_GTID,
  OPT_STOP_GTID,
  OPT_FIND_GTID_POSITION,
  OPT_INDEX_FILE,
  OPT_ORDER_BY_PRIMARY_DESC,
  OPT_USE_ROCKSDB,
  OPT_INNODB_STATS_ON_METADATA,
  OPT_DUMP_FBOBJ_STATS,
  OPT_MYSQLBINLOG_SKIP_EMPTY_TRANS,
  OPT_PRINT_GTIDS,
  OPT_MINIMUM_HLC,
  OPT_CHECKSUM,
  OPT_THREAD_PRIORITY,
  /* Add new option above this */
  OPT_MAX_CLIENT_OPTION,
  OPT_COMPRESS_DATA,
  OPT_COMPRESS_DATA_CHUNK_SIZE
};

/**
  First mysql version supporting the information schema.
*/
#define FIRST_INFORMATION_SCHEMA_VERSION 50003

/**
  Name of the information schema database.
*/
#define INFORMATION_SCHEMA_DB_NAME "information_schema"

/**
  First mysql version supporting the performance schema.
*/
#define FIRST_PERFORMANCE_SCHEMA_VERSION 50503

/**
  Name of the performance schema database.
*/
#define PERFORMANCE_SCHEMA_DB_NAME_MACRO "performance_schema"

/**
  First mysql version supporting the sys schema.
*/
#define FIRST_SYS_SCHEMA_VERSION 50707

/**
  Name of the sys schema database.
*/
#define SYS_SCHEMA_DB_NAME "sys"

/**
  Client deprecation warnings
*/
#define CLIENT_WARN_DEPRECATED_NO_REPLACEMENT_MSG(opt) \
  opt " is deprecated and will be removed in a future version\n"

#define CLIENT_WARN_DEPRECATED_MSG(opt, new_opt)                 \
  opt " is deprecated and will be removed in a future version. " \
      "Use " new_opt " instead.\n"

#define CLIENT_WARN_DEPRECATED_NO_REPLACEMENT(opt) \
  printf("WARNING: " CLIENT_WARN_DEPRECATED_NO_REPLACEMENT_MSG(opt))

#define CLIENT_WARN_DEPRECATED(opt, new_opt) \
  printf("WARNING: " CLIENT_WARN_DEPRECATED_MSG(opt, new_opt))
#endif
