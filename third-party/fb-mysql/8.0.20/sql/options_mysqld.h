/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef OPTIONS_MYSQLD_INCLUDED
#define OPTIONS_MYSQLD_INCLUDED

/**
  only options that need special treatment in get_one_option() deserve
  to be listed below
*/
enum options_mysqld {
  OPT_to_set_the_start_number = 256,
  OPT_BIND_ADDRESS,
  OPT_BINLOG_CHECKSUM,
  OPT_BINLOG_DO_DB,
  OPT_BINLOG_FORMAT,
  OPT_BINLOG_IGNORE_DB,
  OPT_BINLOG_MAX_FLUSH_QUEUE_TIME,
  OPT_EXPIRE_LOGS_DAYS,
  OPT_BINLOG_EXPIRE_LOGS_SECONDS,
  OPT_BIN_LOG,
  OPT_BOOTSTRAP,
  OPT_CONSOLE,
  OPT_DEBUG_SYNC_TIMEOUT,
  OPT_DELAY_KEY_WRITE_ALL,
  OPT_ISAM_LOG,
  OPT_IGNORE_DB_DIRECTORY,
  OPT_KEY_BUFFER_SIZE,
  OPT_KEY_CACHE_AGE_THRESHOLD,
  OPT_KEY_CACHE_BLOCK_SIZE,
  OPT_KEY_CACHE_DIVISION_LIMIT,
  OPT_LC_MESSAGES_DIRECTORY,
  OPT_LOWER_CASE_TABLE_NAMES,
  OPT_MASTER_RETRY_COUNT,
  OPT_MASTER_VERIFY_CHECKSUM,
  OPT_POOL_OF_THREADS,
  OPT_REPLICATE_DO_DB,
  OPT_REPLICATE_DO_TABLE,
  OPT_REPLICATE_IGNORE_DB,
  OPT_REPLICATE_IGNORE_TABLE,
  OPT_REPLICATE_REWRITE_DB,
  OPT_REPLICATE_WILD_DO_TABLE,
  OPT_REPLICATE_WILD_IGNORE_TABLE,
  OPT_SERVER_ID,
  OPT_SKIP_HOST_CACHE,
  OPT_SKIP_LOCK,
  OPT_SKIP_NEW,
  OPT_SKIP_RESOLVE,
  OPT_SKIP_STACK_TRACE,
  OPT_SKIP_SYMLINKS,
  OPT_SLAVE_SQL_VERIFY_CHECKSUM,
  OPT_SSL_CA,
  OPT_SSL_CAPATH,
  OPT_SSL_CERT,
  OPT_SSL_CIPHER,
  OPT_TLS_CIPHERSUITES,
  OPT_TLS_VERSION,
  OPT_SSL_KEY,
  OPT_UPDATE_LOG,
  OPT_WANT_CORE,
  OPT_LOG_ERROR,
  OPT_MAX_LONG_DATA_SIZE,
  OPT_PLUGIN_LOAD,
  OPT_PLUGIN_LOAD_ADD,
  OPT_SSL_CRL,
  OPT_SSL_CRLPATH,
  OPT_PFS_INSTRUMENT,
  OPT_DEFAULT_AUTH,
  OPT_THREAD_CACHE_SIZE,
  OPT_HOST_CACHE_SIZE,
  OPT_TABLE_DEFINITION_CACHE,
  OPT_SKIP_INNODB,
  OPT_AVOID_TEMPORAL_UPGRADE,
  OPT_SHOW_OLD_TEMPORALS,
  OPT_ENFORCE_GTID_CONSISTENCY,
  OPT_INSTALL_SERVER,
  OPT_EARLY_PLUGIN_LOAD,
  OPT_SSL_FIPS_MODE,
  OPT_KEYRING_MIGRATION_SOURCE,
  OPT_KEYRING_MIGRATION_DESTINATION,
  OPT_KEYRING_MIGRATION_USER,
  OPT_KEYRING_MIGRATION_HOST,
  OPT_KEYRING_MIGRATION_PASSWORD,
  OPT_KEYRING_MIGRATION_SOCKET,
  OPT_KEYRING_MIGRATION_PORT,
  OPT_LOG_SLAVE_UPDATES,
  OPT_SLAVE_PRESERVE_COMMIT_ORDER,
  OPT_LOG_SLOW_EXTRA,
  OPT_NAMED_PIPE_FULL_ACCESS_GROUP,
  OPT_RELAY_LOG_INFO_FILE,
  OPT_MASTER_INFO_FILE,
  OPT_LOG_BIN_USE_V1_ROW_EVENTS,
  OPT_SLAVE_ROWS_SEARCH_ALGORITHMS,
  OPT_TRIM_BINLOG_TO_RECOVER,
};

#endif  // OPTIONS_MYSQLD_INCLUDED
