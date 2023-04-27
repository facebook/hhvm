/* Copyright (c) 2006, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/index_statistics.h"

#include <string>
#include <vector>

#include "sql/sql_info.h"
#include "sql/table.h"
#include "storage/perfschema/table_index_statistics.h"

std::unordered_map<IndexSpecification, ulonglong, IndexSpecificationHash>
    global_index_usage_stats;

IndexSpecification::IndexSpecification(const std::string &table_schema,
                                       const std::string &table_name,
                                       const std::string &index_name)
    : table_schema(table_schema),
      table_name(table_name),
      index_name(index_name) {}

// Operator definition for detecting hash collisions.
bool IndexSpecification::operator==(const IndexSpecification &other) const {
  return table_schema.compare(other.table_schema) == 0 &&
         table_name.compare(other.table_name) == 0 &&
         index_name.compare(other.index_name) == 0;
  if (table_schema.compare(other.table_schema) < 0) {
    return true;
  } else if (table_schema.compare(other.table_schema) == 0) {
    if (table_name.compare(other.table_name) < 0) {
      return true;
    } else if (table_name.compare(other.table_name) == 0) {
      return index_name.compare(other.index_name);
    }
  }
  return false;
}

void aggregate_index_statistics(
    std::unordered_map<IndexSpecification, ulonglong, IndexSpecificationHash>
        &ius) {
  if (index_stats_control != SQL_INFO_CONTROL_ON) {
    ius.clear();
    return;
  }
  mysql_rwlock_wrlock(&LOCK_index_statistics);
  for (const auto &ise : ius) {
    if (global_index_usage_stats.find(ise.first) ==
        global_index_usage_stats.end()) {
      global_index_usage_stats[ise.first] = 0;
    }
    global_index_usage_stats[ise.first] += ise.second;
  }
  mysql_rwlock_unlock(&LOCK_index_statistics);

  ius.clear();
}

std::vector<index_statistics_row> get_all_index_statistics() {
  std::vector<index_statistics_row> index_statistics;

  mysql_rwlock_rdlock(&LOCK_index_statistics);
  for (auto iter = global_index_usage_stats.cbegin();
       iter != global_index_usage_stats.cend(); ++iter) {
    index_statistics.emplace_back(iter->first.table_schema,  // TABLE_SCHEMA
                                  iter->first.table_name,    // TABLE_NAME
                                  iter->first.index_name,    // INDEX_NAME
                                  iter->second);             // ROWS_REQUESTED
  }
  mysql_rwlock_unlock(&LOCK_index_statistics);

  return index_statistics;
}

void free_index_stats() {
  mysql_rwlock_wrlock(&LOCK_index_statistics);
  global_index_usage_stats.clear();
  mysql_rwlock_unlock(&LOCK_index_statistics);
}

ulonglong *get_or_add_index_stats_ptr(
    std::unordered_map<IndexSpecification, ulonglong, IndexSpecificationHash>
        *ius,
    TABLE *tbl, uint idx) {
  // Return nullptr if any of the pointer safety checks fail.
  if (!ius || !tbl || !tbl->pos_in_table_list || !tbl->pos_in_table_list->db ||
      !tbl->pos_in_table_list->table_name || !tbl->s || !tbl->s->key_info ||
      idx >= tbl->s->keys || !tbl->s->key_info[idx].name) {
    return nullptr;
  }

  // Return nullptr if index stats are disabled.
  if (index_stats_control != SQL_INFO_CONTROL_ON) {
    ius->clear();
    return nullptr;
  }

  const std::string db_name(tbl->pos_in_table_list->db);
  const std::string table_name(tbl->pos_in_table_list->table_name);
  const std::string index_name(tbl->s->key_info[idx].name);

  // If any of the db, table or index names are empty, return.
  if (db_name.size() == 0 || table_name.size() == 0 || index_name.size() == 0) {
    return nullptr;
  }

  IndexSpecification is(db_name, table_name, index_name);

  // Initialize record, if not present.
  if (ius->find(is) == ius->end()) {
    (*ius)[is] = 0;
  }
  return &(*ius)[is];
}
