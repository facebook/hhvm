/* Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef QUERY_OPTIONS_INCLUDED
#define QUERY_OPTIONS_INCLUDED

/**
  @file

  @details
  This file is used in the server, and the mysqlbinlog client.
*/

/*
   This is included in the server and in the client.
   Options for select set by the yacc parser (stored in lex->options).

   NOTE
   log_event.h defines OPTIONS_WRITTEN_TO_BIN_LOG to specify what THD
   options list are written into binlog. These options can NOT change their
   values, or it will break replication between version.

   context is encoded as following:
   SELECT - SELECT_LEX::options
   THD    - THD::options
   intern - neither. used only as
            func(..., select_node->options | thd->options | OPTION_XXX, ...)

   TODO: separate three contexts above, move them to separate bitfields.
*/

#define SELECT_DISTINCT (1ULL << 0)       // SELECT, user
#define SELECT_STRAIGHT_JOIN (1ULL << 1)  // SELECT, user
// Free slot, used to be SELECT_DESCRIBE: (1ULL << 2)
#define SELECT_SMALL_RESULT (1ULL << 3)  // SELECT, user
#define SELECT_BIG_RESULT (1ULL << 4)    // SELECT, user
#define OPTION_FOUND_ROWS \
  (1ULL << 5)                             // SELECT, user
                                          // 1ULL << 6 is free
#define SELECT_NO_JOIN_CACHE (1ULL << 7)  // intern
/** always the opposite of OPTION_NOT_AUTOCOMMIT except when in fix_autocommit()
 */
#define OPTION_AUTOCOMMIT (1ULL << 8)          // THD, user
#define OPTION_BIG_SELECTS (1ULL << 9)         // THD, user
#define OPTION_LOG_OFF (1ULL << 10)            // THD, user
#define OPTION_QUOTE_SHOW_CREATE (1ULL << 11)  // THD, user, unused
#define TMP_TABLE_ALL_COLUMNS (1ULL << 12)     // SELECT, intern
#define OPTION_WARNINGS (1ULL << 13)           // THD, user
#define OPTION_AUTO_IS_NULL (1ULL << 14)       // THD, user, binlog
#define OPTION_FOUND_COMMENT (1ULL << 15)      // DEPRECATED
#define OPTION_SAFE_UPDATES (1ULL << 16)       // THD, user
#define OPTION_BUFFER_RESULT (1ULL << 17)      // SELECT, user
#define OPTION_BIN_LOG (1ULL << 18)            // THD, user
#define OPTION_NOT_AUTOCOMMIT (1ULL << 19)     // THD, user
#define OPTION_BEGIN (1ULL << 20)              // THD, intern
#define OPTION_TABLE_LOCK (1ULL << 21)         // THD, intern
#define OPTION_QUICK (1ULL << 22)              // SELECT (for DELETE)
#define OPTION_NO_CONST_TABLES (1ULL << 23)    // No const tables, intern

/* The following is used to detect a conflict with DISTINCT */
#define SELECT_ALL (1ULL << 24)           // SELECT, user, parser
#define SELECT_NO_SEMI_JOIN (1ULL << 25)  // SELECT, intern
/** The following can be set when importing tables in a 'wrong order'
   to suppress foreign key checks */
#define OPTION_NO_FOREIGN_KEY_CHECKS (1ULL << 26)  // THD, user, binlog
/** The following speeds up inserts to InnoDB tables by suppressing unique
   key checks in some cases */
#define OPTION_RELAXED_UNIQUE_CHECKS (1ULL << 27)  // THD, user, binlog
#define SELECT_NO_UNLOCK (1ULL << 28)              // SELECT, intern
#define OPTION_SCHEMA_TABLE (1ULL << 29)           // SELECT, intern
/** Flag set if setup_tables already done */
#define OPTION_SETUP_TABLES_DONE (1ULL << 30)  // intern
/** If not set then the thread will ignore all warnings with level notes. */
#define OPTION_SQL_NOTES (1ULL << 31)  // THD, user

/** (1ULL << 32) is not used after removing TMP_TABLE_FORCE_MYISAM option */

#define OPTION_PROFILING (1ULL << 33)
/**
  Indicates that this is a HIGH_PRIORITY SELECT.
  Currently used only for printing of such selects.
  Type of locks to be acquired is specified directly.
*/
#define SELECT_HIGH_PRIORITY (1ULL << 34)  // SELECT, user
/**
  Is set in slave SQL thread when there was an
  error on master, which, when is not reproducible
  on slave (i.e. the query succeeds on slave),
  is not terminal to the state of replication,
  and should be ignored. The slave SQL thread,
  however, needs to rollback the effects of the
  succeeded statement to keep replication consistent.
*/
#define OPTION_MASTER_SQL_ERROR (1ULL << 35)

/*
  Dont report errors for individual rows,
  But just report error on commit (or read, of course)
  Note! Reserved for use in MySQL Cluster
*/
#define OPTION_ALLOW_BATCH (1ULL << 36)  // THD, intern (slave)

#define OPTION_SELECT_FOR_SHOW (1ULL << 37)  // SELECT for SHOW over DD.

// Is set while thread is updating the data dictionary tables.
#define OPTION_DD_UPDATE_CONTEXT (1ULL << 38)  // intern

/**
  If this option is set, subqueries should not be evaluated during
  optimization, even if they are known to produce a constant result.
*/
#define OPTION_NO_SUBQUERY_DURING_OPTIMIZATION (1ULL << 39)  // intern

#endif /* QUERY_OPTIONS_INCLUDED */
