/* Copyright (c) 2005, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_tblmap.h"

#include <stddef.h>
#include <unordered_map>
#include <utility>

#ifdef MYSQL_SERVER
#include "sql/table.h"  // TABLE
#else
#include "sql/log_event.h"  // Table_map_log_event
#endif
#include "lex_string.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "mysql/psi/psi_base.h"
#include "sql/psi_memory_key.h"
#include "sql/thr_malloc.h"
#include "thr_malloc.h"

#ifndef MYSQL_SERVER
#define MAYBE_TABLE_NAME(T) ("")
#else
#define MAYBE_TABLE_NAME(T) ((T) ? (T)->s->table_name.str : "<>")
#endif
#define TABLE_ID_HASH_SIZE 32
#define TABLE_ID_CHUNK 256

#ifndef MYSQL_SERVER
static const PSI_memory_key table_psi_key = PSI_NOT_INSTRUMENTED;
#else
static const PSI_memory_key table_psi_key = key_memory_table_mapping_root;
#endif

table_mapping::table_mapping() : m_free(nullptr), m_table_ids(table_psi_key) {
  /* We don't preallocate any block, this is consistent with m_free=0 above */
  init_alloc_root(table_psi_key, &m_mem_root,
                  TABLE_ID_HASH_SIZE * sizeof(entry), 0);
}

table_mapping::~table_mapping() {
#ifndef MYSQL_SERVER
  clear_tables();
#endif
}

Mapped_table *table_mapping::get_table(ulonglong table_id) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("table_id: %llu", table_id));
  auto it = m_table_ids.find(table_id);
  if (it != m_table_ids.end()) {
    entry *e = it->second;
    DBUG_PRINT("info", ("tid %llu -> table %p (%s)", table_id, e->table,
                        MAYBE_TABLE_NAME(e->table)));
    return e->table;
  }

  DBUG_PRINT("info", ("tid %llu is not mapped!", table_id));
  return nullptr;
}

/*
  Called when we are out of table id entries. Creates TABLE_ID_CHUNK
  new entries, chain them and attach them at the head of the list of free
  (free for use) entries.
*/
int table_mapping::expand() {
  entry *tmp = new (&m_mem_root) entry[TABLE_ID_CHUNK];
  if (tmp == nullptr) return ERR_MEMORY_ALLOCATION;  // Memory allocation failed

  /* Find the end of this fresh new array of free entries */
  entry *e_end = tmp + TABLE_ID_CHUNK - 1;
  for (entry *e = tmp; e < e_end; e++) e->next = e + 1;
  e_end->next = m_free;
  m_free = tmp;
  return 0;
}

int table_mapping::set_table(ulonglong table_id, Mapped_table *table) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("table_id: %llu  table: %p (%s)", table_id, table,
                       MAYBE_TABLE_NAME(table)));
  entry *e;
  auto it = m_table_ids.find(table_id);
  if (it == m_table_ids.end()) {
    if (m_free == nullptr && expand())
      return ERR_MEMORY_ALLOCATION;  // Memory allocation failed
    e = m_free;
    m_free = m_free->next;
  } else {
    e = it->second;
#ifndef MYSQL_SERVER
    delete e->table;
#endif
    m_table_ids.erase(table_id);
  }
  e->table_id = table_id;
  e->table = table;
  m_table_ids.emplace(table_id, e);

  DBUG_PRINT("info", ("tid %llu -> table %p (%s)", table_id, e->table,
                      MAYBE_TABLE_NAME(e->table)));
  return 0;  // All OK
}

int table_mapping::remove_table(ulonglong table_id) {
  auto it = m_table_ids.find(table_id);
  if (it != m_table_ids.end()) {
    /* we add this entry to the chain of free (free for use) entries */
    it->second->next = m_free;
    m_free = it->second;
    m_table_ids.erase(it);
    return 0;  // All OK
  }
  return 1;  // No table to remove
}

/*
  Puts all entries into the list of free-for-use entries (does not free any
  memory), and empties the hash.
*/
void table_mapping::clear_tables() {
  DBUG_TRACE;
  for (const auto &key_and_value : m_table_ids) {
    entry *e = key_and_value.second;
#ifndef MYSQL_SERVER
    delete e->table;
#endif
    e->next = m_free;
    m_free = e;
  }
  m_table_ids.clear();
}
