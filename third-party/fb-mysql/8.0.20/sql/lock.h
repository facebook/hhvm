/* Copyright (c) 2010, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef LOCK_INCLUDED
#define LOCK_INCLUDED

#include <stddef.h>
#include <sys/types.h>
#include <string>

#include "map_helpers.h"
#include "sql/mdl.h"

class THD;
// Forward declarations
struct TABLE;
struct THR_LOCK_DATA;

struct MYSQL_LOCK {
  TABLE **table;
  uint table_count, lock_count;
  THR_LOCK_DATA **locks;
};

MYSQL_LOCK *mysql_lock_tables(THD *thd, TABLE **table, size_t count,
                              uint flags);
void mysql_unlock_tables(THD *thd, MYSQL_LOCK *sql_lock);
void mysql_unlock_read_tables(THD *thd, MYSQL_LOCK *sql_lock);
void mysql_unlock_some_tables(THD *thd, TABLE **table, uint count);
void mysql_lock_remove(THD *thd, MYSQL_LOCK *locked, TABLE *table);
void mysql_lock_abort_for_thread(THD *thd, TABLE *table);
MYSQL_LOCK *mysql_lock_merge(MYSQL_LOCK *a, MYSQL_LOCK *b);
/* Lock based on name */
bool lock_schema_name(THD *thd, const char *db);

// Hash set to hold set of tablespace names.
typedef malloc_unordered_set<std::string> Tablespace_hash_set;

// Lock tablespace names.
bool lock_tablespace_names_nsec(THD *thd, Tablespace_hash_set *tablespace_set,
                                ulonglong lock_wait_timeout_nsec);

/* Lock based on stored routine name */
bool lock_object_name(THD *thd, MDL_key::enum_mdl_namespace mdl_type,
                      const char *db, const char *name);

/* Acquire protection against the global read lock. */
bool acquire_shared_global_read_lock_nsec(THD *thd,
                                          ulonglong lock_wait_timeout_nsec);

#endif /* LOCK_INCLUDED */
