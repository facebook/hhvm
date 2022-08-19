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

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <mysql/psi/mysql_rwlock.h>
#include "include/my_inttypes.h"

class index_statistics_row;
struct TABLE;

/*
 * Structure which represents a unique index.
 * This is used as a hash key in our data structures.
 */
struct IndexSpecification {
  std::string table_schema;
  std::string table_name;
  std::string index_name;

  IndexSpecification(const std::string &table_schema,
                     const std::string &table_name,
                     const std::string &index_name);
  bool operator==(const IndexSpecification &) const;
};

/*
 * Structure represents statistics corresponding to one index.
 * As of now, we only track rows_requested.
 */
struct IndexStatiticsElement {
  IndexSpecification index_spec;
  ulonglong rows_requested;
};

/*
 * IndexSpecificationHash is a class that lets IndexSpecification class
 * be used as a key in unordered_map.
 */
class IndexSpecificationHash {
 public:
  size_t operator()(const IndexSpecification &is) const {
    // Simply concatenate and compute std::hash
    return str_hash(is.table_schema + is.table_name + is.index_name);
  }

 private:
  std::hash<std::string> str_hash;
};

/* Global variable to control collecting index statistics */
extern ulong index_stats_control;

/*
 * Read-write lock to make the unordered map storing index statistics,
 * threadsafe.
 */
extern mysql_rwlock_t LOCK_index_statistics;

/*
  aggregate_index_statistics
    Helper to aggregate index statistics accumulated in the THD struct into
    the global data structure.
  Parameters:
    ius           out: std::unordered_map<IndexSpecification,
                                          ulonglong,
                                          IndexSpecificationHash>
                       Reference to the statistics that got accumulated in the
                       THD data structure which would get aggregated into
                       global_index_usage_stats. A reference is accepted to
                       clear the THD data structure.
*/
extern void aggregate_index_statistics(
    std::unordered_map<IndexSpecification, ulonglong, IndexSpecificationHash>
        &ius);

/*
  get_all_index_statistics
    Returns all the rows in index_statistics from the internal data
    structures.
*/
extern std::vector<index_statistics_row> get_all_index_statistics();

/*
  free_index_stats
    Evicts index stats from global_index_usage_stats (defined in the cc file).
*/
extern void free_index_stats();

/*
  get_or_add_index_stats_ptr
    Helper to fetch the index statistics entry corresponding to idx from the
    local THD data structure tracking index usage. If an entry corresponding
    to idx doesn't exist, it is added and returned.
  Parameters:
    ius           out: std::unordered_map<IndexSpecification,
                                          ulonglong,
                                          IndexSpecificationHash>
                       Reference to the data structure that stores statistics
                       in THD for every index that was used. This would
                       eventually get aggregated into global_index_usage_stats.
    tbl           in:  TABLE *
                       Table corresponding to which the index usage entry needs
                       to be fetched.
    idx           in:  uint
                       Ordinal position of the index in table corresponding to
                       tbl. This is the index that gets added to ius or if an
                       entry already exists, a pointer to the existing
                       rows_requested value is returned.
*/
extern ulonglong *get_or_add_index_stats_ptr(
    std::unordered_map<IndexSpecification, ulonglong, IndexSpecificationHash>
        *ius,
    TABLE *tbl, uint idx);
