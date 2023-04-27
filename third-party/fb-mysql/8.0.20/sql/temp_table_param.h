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

#ifndef TEMP_TABLE_PARAM_INCLUDED
#define TEMP_TABLE_PARAM_INCLUDED

#include <sys/types.h>
#include <vector>

#include "my_base.h"
#include "my_inttypes.h"
#include "sql/field.h"
#include "sql/mem_root_allocator.h"
#include "sql/mem_root_array.h"
#include "sql/thr_malloc.h"

class KEY;
class Item;
class Item_copy;
class Window;
struct CHARSET_INFO;
struct MEM_ROOT;

template <typename T>
using Mem_root_vector = std::vector<T, Mem_root_allocator<T>>;

/**
   Helper class for copy_funcs(); represents an Item to copy from table to
   next tmp table.
*/
class Func_ptr {
 public:
  explicit Func_ptr(Item *f) : m_func(f) {}
  Item *func() const { return m_func; }
  void set_override_result_field(Field *f) { m_override_result_field = f; }
  Field *override_result_field() const { return m_override_result_field; }

 private:
  Item *m_func;

  /// If not nullptr, copy_funcs() will save the result of m_func here instead
  /// of in m_func's usual designated result field.
  Field *m_override_result_field = nullptr;
};

/// Used by copy_funcs()
typedef Mem_root_array<Func_ptr> Func_ptr_array;

/**
  Object containing parameters used when creating and using temporary
  tables. Temporary tables created with the help of this object are
  used only internally by the query execution engine.
*/

class Temp_table_param {
 public:
  /**
    Used to store the values of grouped non-column-reference expressions in
    between groups, so they can be retreived when the group changes.

    @see setup_copy_fields
    @see copy_fields
  */
  Mem_root_vector<Item_copy *> grouped_expressions;
  Mem_root_vector<Copy_field> copy_fields;

  uchar *group_buff;
  Func_ptr_array *items_to_copy; /* Fields in tmp table */

  /**
    After temporary table creation, points to an index on the table
    created depending on the purpose of the table - grouping,
    duplicate elimination, etc. There is at most one such index.
  */
  KEY *keyinfo;

  /**
    LIMIT (maximum number of rows) for this temp table, or HA_POS_ERROR
    for no limit. Enforced by MaterializeIterator when writing to the table.
   */
  ha_rows end_write_records{HA_POS_ERROR};

  /**
    Number of normal fields in the query, including those referred to
    from aggregate functions. Hence, "SELECT `field1`,
    SUM(`field2`) from t1" sets this counter to 2.

    @see count_field_types
  */
  uint field_count;
  /**
    Number of fields in the query that have functions. Includes both
    aggregate functions (e.g., SUM) and non-aggregates (e.g., RAND)
    and windowing functions.
    Also counts functions referred to from windowing or aggregate functions,
    i.e., "SELECT SUM(RAND())" sets this counter to 2.

    @see count_field_types
  */
  uint func_count;
  /**
    Number of fields in the query that have aggregate functions. Note
    that the optimizer may choose to optimize away these fields by
    replacing them with constants, in which case sum_func_count will
    need to be updated.

    @see optimize_aggregated_query, count_field_types
  */
  uint sum_func_count;
  uint hidden_field_count;
  uint group_parts, group_length, group_null_parts;
  /**
    Whether we allow running GROUP BY processing into a temporary table,
    i.e., keeping many different aggregations going at once without
    having ordered input. This is usually the case, but is currently not
    supported for aggregation UDFs, aggregates with DISTINCT, or ROLLUP.

    Note that even if this is true, the optimizer may choose to not use
    a temporary table, as it is often more efficient to just read along
    an index.
   */
  bool allow_group_via_temp_table{true};
  /**
    Number of outer_sum_funcs i.e the number of set functions that are
    aggregated in a query block outer to this subquery.

    @see count_field_types
  */
  uint outer_sum_func_count;
  /**
    Enabled when we have atleast one outer_sum_func. Needed when used
    along with distinct.

    @see create_tmp_table
  */
  bool using_outer_summary_function;
  CHARSET_INFO *table_charset;
  bool schema_table;
  /*
    True if GROUP BY and its aggregate functions are already computed
    by a table access method (e.g. by loose index scan). In this case
    query execution should not perform aggregation and should treat
    aggregate functions as normal functions.
  */
  bool precomputed_group_by;
  bool force_copy_fields;
  /**
    true <=> don't actually create table handler when creating the result
    table. This allows range optimizer to add indexes later.
    Used for materialized derived tables/views.
    @see TABLE_LIST::update_derived_keys.
  */
  bool skip_create_table;
  /*
    If true, create_tmp_field called from create_tmp_table will convert
    all BIT fields to 64-bit longs. This is a workaround the limitation
    that MEMORY tables cannot index BIT columns.
  */
  bool bit_fields_as_long;

  /// Whether the UNIQUE index can be promoted to PK
  bool can_use_pk_for_unique;

  /// Whether UNIQUE keys should always be implemented by way of a hidden
  /// hash field, never a unique index. Needed for materialization of mixed
  /// UNION ALL / UNION DISTINCT queries (see comments in
  /// create_result_table()).
  bool force_hash_field_for_unique{false};

  /// (Last) window's tmp file step can be skipped
  bool m_window_short_circuit;

  /// This tmp table is used for a window's frame buffer
  bool m_window_frame_buffer{false};

  /// If this is the out table of a window: the said window
  Window *m_window;

  Temp_table_param(MEM_ROOT *mem_root = *THR_MALLOC)
      : grouped_expressions(Mem_root_allocator<Item_copy *>(mem_root)),
        copy_fields(Mem_root_allocator<Copy_field>(mem_root)),
        group_buff(nullptr),
        items_to_copy(nullptr),
        keyinfo(nullptr),
        field_count(0),
        func_count(0),
        sum_func_count(0),
        hidden_field_count(0),
        group_parts(0),
        group_length(0),
        group_null_parts(0),
        outer_sum_func_count(0),
        using_outer_summary_function(false),
        table_charset(nullptr),
        schema_table(false),
        precomputed_group_by(false),
        force_copy_fields(false),
        skip_create_table(false),
        bit_fields_as_long(false),
        can_use_pk_for_unique(true),
        m_window_short_circuit(false),
        m_window(nullptr) {}

  void cleanup() {
    grouped_expressions.clear();
    copy_fields.clear();
  }
};

#endif  // TEMP_TABLE_PARAM_INCLUDED
