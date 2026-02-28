/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_NESTED_JOIN_INCLUDED
#define SQL_NESTED_JOIN_INCLUDED

#include <sys/types.h>
#include "my_inttypes.h"
#include "my_table_map.h"
#include "sql/handler.h"
#include "sql/sql_list.h"
#include "sql/sql_opt_exec_shared.h"

class Item;
class Item_field;
struct POSITION;
struct TABLE_LIST;

/*
  Used to identify NESTED_JOIN structures within a join (applicable to
  structures representing outer joins that have not been simplified away).
*/
typedef ulonglong nested_join_map;

/**
  Semijoin_mat_optimize collects data used when calculating the cost of
  executing a semijoin operation using a materialization strategy.
  It is used during optimization phase only.
*/

struct Semijoin_mat_optimize {
  /// Optimal join order calculated for inner tables of this semijoin op.
  POSITION *positions{nullptr};
  /// True if data types allow the MaterializeLookup semijoin strategy
  bool lookup_allowed{false};
  /// True if data types allow the MaterializeScan semijoin strategy
  bool scan_allowed{false};
  /// Expected number of rows in the materialized table
  double expected_rowcount{0.0};
  /// Materialization cost - execute sub-join and write rows to temp.table
  Cost_estimate materialization_cost;
  /// Cost to make one lookup in the temptable
  Cost_estimate lookup_cost;
  /// Cost of scanning the materialized table
  Cost_estimate scan_cost;
  /// Array of pointers to fields in the materialized table.
  Item_field **mat_fields{nullptr};
};

/**
  Struct NESTED_JOIN is used to represent how tables are connected through
  outer join operations and semi-join operations to form a query block.
  Out of the parser, inner joins are also represented by NESTED_JOIN
  structs, but these are later flattened out by simplify_joins().
  Some outer join nests are also flattened, when it can be determined that
  they can be processed as inner joins instead of outer joins.
*/
struct NESTED_JOIN {
  NESTED_JOIN() : join_list(*THR_MALLOC) {}

  mem_root_deque<TABLE_LIST *>
      join_list;                /* list of elements in the nested join */
  table_map used_tables{0};     /* bitmap of tables in the nested join */
  table_map not_null_tables{0}; /* tables that rejects nulls           */
  /**
    Used for pointing out the first table in the plan being covered by this
    join nest. It is used exclusively within make_outerjoin_info().
  */
  plan_idx first_nested{0};
  /**
    Set to true when natural join or using information has been processed.
  */
  bool natural_join_processed{false};
  /**
    Number of tables and outer join nests administered by this nested join
    object for the sake of cost analysis. Includes direct member tables as
    well as tables included through semi-join nests, but notice that semi-join
    nests themselves are not counted.
  */
  uint nj_total{0};
  /**
    Used to count tables in the nested join in 2 isolated places:
    1. In make_outerjoin_info().
    2. check_interleaving_with_nj/backout_nj_state (these are called
       by the join optimizer.
    Before each use the counters are zeroed by SELECT_LEX::reset_nj_counters.
  */
  uint nj_counter{0};
  /**
    Bit identifying this nested join. Only nested joins representing the
    outer join structure need this, other nests have bit set to zero.
  */
  nested_join_map nj_map{0};
  /**
    Tables outside the semi-join that are used within the semi-join's
    ON condition (ie. the subquery WHERE clause and optional IN equalities).
    Also contains lateral dependencies from materialized derived tables
    contained inside the semi-join inner tables.
  */
  table_map sj_depends_on{0};
  /**
    Outer non-trivially correlated tables, a true subset of sj_depends_on.
    Also contains lateral dependencies from materialized derived tables
    contained inside the semi-join inner tables.
  */
  table_map sj_corr_tables{0};
  /**
    Query block id if this struct is generated from a subquery transform.
  */
  uint query_block_id{0};

  /// Bitmap of which strategies are enabled for this semi-join nest
  uint sj_enabled_strategies{0};

  /*
    Lists of trivially-correlated expressions from the outer and inner tables
    of the semi-join, respectively.
  */
  List<Item> sj_outer_exprs, sj_inner_exprs;
  Semijoin_mat_optimize sjm;
};

#endif  // SQL_NESTED_JOIN_INCLUDED
