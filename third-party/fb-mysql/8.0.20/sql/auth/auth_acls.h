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

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */
#ifndef AUTH_ACLS_INCLUDED
#define AUTH_ACLS_INCLUDED

#include <string>
#include <unordered_map>
#include <vector>

/* Total Number of ACLs present in mysql.user */
#define NUM_ACLS 31

#define SELECT_ACL (1L << 0)
#define INSERT_ACL (1L << 1)
#define UPDATE_ACL (1L << 2)
#define DELETE_ACL (1L << 3)
#define CREATE_ACL (1L << 4)
#define DROP_ACL (1L << 5)
#define RELOAD_ACL (1L << 6)
#define SHUTDOWN_ACL (1L << 7)
#define PROCESS_ACL (1L << 8)
#define FILE_ACL (1L << 9)
#define GRANT_ACL (1L << 10)
#define REFERENCES_ACL (1L << 11)
#define INDEX_ACL (1L << 12)
#define ALTER_ACL (1L << 13)
#define SHOW_DB_ACL (1L << 14)
#define SUPER_ACL (1L << 15)
#define CREATE_TMP_ACL (1L << 16)
#define LOCK_TABLES_ACL (1L << 17)
#define EXECUTE_ACL (1L << 18)
#define REPL_SLAVE_ACL (1L << 19)
#define REPL_CLIENT_ACL (1L << 20)
#define CREATE_VIEW_ACL (1L << 21)
#define SHOW_VIEW_ACL (1L << 22)
#define CREATE_PROC_ACL (1L << 23)
#define ALTER_PROC_ACL (1L << 24)
#define CREATE_USER_ACL (1L << 25)
#define EVENT_ACL (1L << 26)
#define TRIGGER_ACL (1L << 27)
#define CREATE_TABLESPACE_ACL (1L << 28)
#define CREATE_ROLE_ACL (1L << 29)
#define DROP_ROLE_ACL (1L << 30)
/*
  don't forget to update
  1. static struct show_privileges_st sys_privileges[]
  2. static const char *command_array[] and static uint command_lengths[]
  3. mysql_system_tables.sql and mysql_system_tables_fix.sql
  4. acl_init() or whatever - to define behaviour for old privilege tables
  5. sql_yacc.yy - for GRANT/REVOKE to work
  6. global_privileges map and vector
*/

#define NO_ACCESS (1L << 31)

/**
  Privileges to perform database related operations.
  Use this macro over DB_ACLS unless there is real need to use
  additional privileges present in the DB_ACLS
*/
#define DB_OP_ACLS                                                             \
  (UPDATE_ACL | SELECT_ACL | INSERT_ACL | DELETE_ACL | CREATE_ACL | DROP_ACL | \
   REFERENCES_ACL | INDEX_ACL | ALTER_ACL | CREATE_TMP_ACL | LOCK_TABLES_ACL | \
   EXECUTE_ACL | CREATE_VIEW_ACL | SHOW_VIEW_ACL | CREATE_PROC_ACL |           \
   ALTER_PROC_ACL | EVENT_ACL | TRIGGER_ACL)

/**
  Privileges to perform table related operations.
  Use this macro over TABLE_ACLS unless there is real need to use
  additional privileges present in the DB_ACLS
*/
#define TABLE_OP_ACLS                                                          \
  (SELECT_ACL | INSERT_ACL | UPDATE_ACL | DELETE_ACL | CREATE_ACL | DROP_ACL | \
   REFERENCES_ACL | INDEX_ACL | ALTER_ACL | CREATE_VIEW_ACL | SHOW_VIEW_ACL |  \
   TRIGGER_ACL)

/**
  Privileges to modify or execute stored procedures.
  Use this macro over PROC_ACLS unless there is real need to use
  additional privileges present in the PROC_ACLS
*/
#define PROC_OP_ACLS (ALTER_PROC_ACL | EXECUTE_ACL)

/**
  Represents all privileges which could be granted to users at DB-level. It
  essentially represents all the privileges present in the mysql.db table.
*/
#define DB_ACLS (DB_OP_ACLS | GRANT_ACL)

/**
  Represents all privileges which could be granted to users at table-level. It
  essentially represents all the privileges present in the mysql.tables_priv
  table.
*/
#define TABLE_ACLS (TABLE_OP_ACLS | GRANT_ACL)

/**
  Represents all privileges which could be granted to users at column-level. It
  essentially represents all the privileges present in the columns_priv table.
*/
#define COL_ACLS (SELECT_ACL | INSERT_ACL | UPDATE_ACL | REFERENCES_ACL)

/**
  Represents all privileges which could be granted to users for stored
  procedures. It essentially represents all the privileges present in the
  mysql.procs_priv table.
*/
#define PROC_ACLS (PROC_OP_ACLS | GRANT_ACL)

/**
  Represents all privileges which are required to show the stored procedure.
*/
#define SHOW_PROC_ACLS (PROC_OP_ACLS | CREATE_PROC_ACL)

/**
  Represents all privileges which could be granted to users globally.
  It essentially represents all the privileges present in the mysql.user table
*/
#define GLOBAL_ACLS                                                            \
  (SELECT_ACL | INSERT_ACL | UPDATE_ACL | DELETE_ACL | CREATE_ACL | DROP_ACL | \
   RELOAD_ACL | SHUTDOWN_ACL | PROCESS_ACL | FILE_ACL | GRANT_ACL |            \
   REFERENCES_ACL | INDEX_ACL | ALTER_ACL | SHOW_DB_ACL | SUPER_ACL |          \
   CREATE_TMP_ACL | LOCK_TABLES_ACL | REPL_SLAVE_ACL | REPL_CLIENT_ACL |       \
   EXECUTE_ACL | CREATE_VIEW_ACL | SHOW_VIEW_ACL | CREATE_PROC_ACL |           \
   ALTER_PROC_ACL | CREATE_USER_ACL | EVENT_ACL | TRIGGER_ACL |                \
   CREATE_TABLESPACE_ACL | CREATE_ROLE_ACL | DROP_ROLE_ACL)

#define DEFAULT_CREATE_PROC_ACLS (ALTER_PROC_ACL | EXECUTE_ACL)

/**
  Table-level privileges which are automatically "granted" to everyone on
  existing temporary tables (CREATE_ACL is necessary for ALTER ... RENAME).
*/
#define TMP_TABLE_ACLS                                                         \
  (SELECT_ACL | INSERT_ACL | UPDATE_ACL | DELETE_ACL | CREATE_ACL | DROP_ACL | \
   INDEX_ACL | ALTER_ACL)

/*
  Defines to change the above bits to how things are stored in tables
  This is needed as the 'host' and 'db' table is missing a few privileges
*/

/* Privileges that needs to be reallocated (in continous chunks) */
#define DB_CHUNK0 \
  (SELECT_ACL | INSERT_ACL | UPDATE_ACL | DELETE_ACL | CREATE_ACL | DROP_ACL)
#define DB_CHUNK1 (GRANT_ACL | REFERENCES_ACL | INDEX_ACL | ALTER_ACL)
#define DB_CHUNK2 (CREATE_TMP_ACL | LOCK_TABLES_ACL)
#define DB_CHUNK3 \
  (CREATE_VIEW_ACL | SHOW_VIEW_ACL | CREATE_PROC_ACL | ALTER_PROC_ACL)
#define DB_CHUNK4 (EXECUTE_ACL)
#define DB_CHUNK5 (EVENT_ACL | TRIGGER_ACL)

#define fix_rights_for_db(A)                                               \
  (((A)&DB_CHUNK0) | (((A) << 4) & DB_CHUNK1) | (((A) << 6) & DB_CHUNK2) | \
   (((A) << 9) & DB_CHUNK3) | (((A) << 2) & DB_CHUNK4)) |                  \
      (((A) << 9) & DB_CHUNK5)
#define get_rights_for_db(A)                                           \
  (((A)&DB_CHUNK0) | (((A)&DB_CHUNK1) >> 4) | (((A)&DB_CHUNK2) >> 6) | \
   (((A)&DB_CHUNK3) >> 9) | (((A)&DB_CHUNK4) >> 2)) |                  \
      (((A)&DB_CHUNK5) >> 9)
#define TBL_CHUNK0 DB_CHUNK0
#define TBL_CHUNK1 DB_CHUNK1
#define TBL_CHUNK2 (CREATE_VIEW_ACL | SHOW_VIEW_ACL)
#define TBL_CHUNK3 TRIGGER_ACL
#define fix_rights_for_table(A)                                                \
  (((A)&TBL_CHUNK0) | (((A) << 4) & TBL_CHUNK1) | (((A) << 11) & TBL_CHUNK2) | \
   (((A) << 15) & TBL_CHUNK3))
#define get_rights_for_table(A)                                            \
  (((A)&TBL_CHUNK0) | (((A)&TBL_CHUNK1) >> 4) | (((A)&TBL_CHUNK2) >> 11) | \
   (((A)&TBL_CHUNK3) >> 15))
#define fix_rights_for_column(A) (((A)&7) | (((A) & ~7) << 8))
#define get_rights_for_column(A) (((A)&7) | ((A) >> 8))
#define fix_rights_for_procedure(A)                               \
  ((((A) << 18) & EXECUTE_ACL) | (((A) << 23) & ALTER_PROC_ACL) | \
   (((A) << 8) & GRANT_ACL))
#define get_rights_for_procedure(A)                           \
  ((((A)&EXECUTE_ACL) >> 18) | (((A)&ALTER_PROC_ACL) >> 23) | \
   (((A)&GRANT_ACL) >> 8))

extern const std::vector<std::string> global_acls_vector;
extern const std::unordered_map<std::string, int> global_acls_map;

#endif /* AUTH_ACLS_INCLUDED */
