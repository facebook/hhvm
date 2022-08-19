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

/**
  @file

  @brief
  Create plan for a single select.


  @defgroup Query_Planner  Query Planner
  @{
*/

#include "sql/sql_planner.h"

#include "my_config.h"

#include <float.h>
#include <limits.h>
#include <string.h>
#include <algorithm>
#include <atomic>

#include "my_base.h"  // key_part_map
#include "my_bit.h"   // my_count_bits
#include "my_bitmap.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_macros.h"
#include "sql/enum_query_type.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/item_cmpfunc.h"
#include "sql/key.h"
#include "sql/merge_sort.h"  // merge_sort
#include "sql/nested_join.h"
#include "sql/opt_costmodel.h"
#include "sql/opt_hints.h"  // hint_table_state
#include "sql/opt_range.h"  // QUICK_SELECT_I
#include "sql/opt_trace.h"  // Opt_trace_object
#include "sql/opt_trace_context.h"
#include "sql/query_options.h"
#include "sql/query_result.h"
#include "sql/sql_bitmap.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_opt_exec_shared.h"
#include "sql/sql_optimizer.h"  // JOIN
#include "sql/sql_select.h"     // JOIN_TAB
#include "sql/sql_test.h"       // print_plan
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/window.h"
#include "sql_string.h"

using std::max;
using std::min;

static double prev_record_reads(JOIN *join, uint idx, table_map found_ref);
static void trace_plan_prefix(JOIN *join, uint idx, table_map excluded_tables);

static uint max_part_bit(key_part_map bits) {
  uint found;
  for (found = 0; bits & 1; found++, bits >>= 1)
    ;
  return found;
}

static uint cache_record_length(JOIN *join, uint idx) {
  uint length = 0;
  JOIN_TAB **pos, **end;

  for (pos = join->best_ref + join->const_tables, end = join->best_ref + idx;
       pos != end; pos++) {
    JOIN_TAB *join_tab = *pos;
    if (!join_tab->used_fieldlength)  // Not calculated yet
    {
      uint used_fields, used_blobs;
      bool used_null_fields, used_uneven_bit_fields;
      /*
        (1) needs_rowid: we don't know if Duplicate Weedout may be
        used, length will thus be inaccurate, this is acceptable.
      */
      calc_used_field_length(join_tab->table(),
                             false,  // (1)
                             &used_fields, &join_tab->used_fieldlength,
                             &used_blobs, &used_null_fields,
                             &used_uneven_bit_fields);
    }
    length += join_tab->used_fieldlength;
  }
  return length;
}

Optimize_table_order::Optimize_table_order(THD *thd_arg, JOIN *join_arg,
                                           TABLE_LIST *sjm_nest_arg)
    : thd(thd_arg),
      join(join_arg),
      search_depth(determine_search_depth(thd->variables.optimizer_search_depth,
                                          join->tables - join->const_tables)),
      prune_level(thd->variables.optimizer_prune_level),
      cur_embedding_map(0),
      emb_sjm_nest(sjm_nest_arg),
      excluded_tables(
          (emb_sjm_nest ? (join->all_table_map & ~emb_sjm_nest->sj_inner_tables)
                        : 0) |
          (join->allow_outer_refs ? 0 : OUTER_REF_TABLE_BIT)),
      has_sj(!(join->select_lex->sj_nests.empty() || emb_sjm_nest)),
      test_all_ref_keys(false),
      found_plan_with_allowed_sj(false),
      got_final_plan(false) {}

/**
  Find the best index to do 'ref' access on for a table.

  The best index chosen using the following priority list
  1) A clustered primary key with equality predicates on all keyparts is
     always chosen.
  2) A non nullable unique index with equality predicates on
     all keyparts is preferred over a non-unique index,
     nullable unique index or unique index where there are some
     keyparts without equality predicates.
  3) Otherwise, the index with best cost estimate is chosen.

  As a side-effect, bound_keyparts/read_cost/fanout is set for the first
  Key_use of every considered key.

  @param tab                        the table to be joined by the function
  @param remaining_tables           set of tables not included in the
                                    partial plan yet.
  @param idx                        the index in join->position[] where 'tab'
                                    is added to the partial plan.
  @param prefix_rowcount            estimate for the number of records returned
                                    by the partial plan
  @param [out] found_condition      whether or not there exists a condition
                                    that filters away rows for this table.
                                    Always true when the function finds a
                                    usable 'ref' access, but also if it finds
                                    a condition that is not usable by 'ref'
                                    access, e.g. is there is an index covering
                                    (a,b) and there is a condition only on 'b'.
                                    Note that all dependent tables for the
                                    condition in question must be in the plan
                                    prefix for this to be 'true'. Unmodified
                                    if no relevant condition is found.
  @param [out] ref_depend_map       tables the best ref access depends on.
                                    Unmodified if no 'ref' access is found.
  @param [out] used_key_parts       Number of keyparts 'ref' access uses.
                                    Unmodified if no 'ref' access is found.

  @return pointer to Key_use for the index with best 'ref' access, NULL if
          no 'ref' access method is found.
*/
Key_use *Optimize_table_order::find_best_ref(
    const JOIN_TAB *tab, const table_map remaining_tables, const uint idx,
    const double prefix_rowcount, bool *found_condition,
    table_map *ref_depend_map, uint *used_key_parts) {
  // Skip finding best_ref if quick object is forced by hint.
  if (tab->quick() && tab->quick()->forced_by_hint) return nullptr;

  // Return value - will point to Key_use of the index with cheapest ref access
  Key_use *best_ref = nullptr;

  /*
    Cost of using best_ref; used to determine if ref access on another
    index is cheaper. Calculated as follows:

    (cost_ref_for_one_value + row_evaluate_cost(fanout_for_ref)) *
    prefix_rowcount
  */
  double best_ref_cost = DBL_MAX;

  // Index type, note that code below relies on this element definition order
  enum idx_type { CLUSTERED_PK, UNIQUE, NOT_UNIQUE, FULLTEXT };
  enum idx_type best_found_keytype = NOT_UNIQUE;

  TABLE *const table = tab->table();
  Opt_trace_context *const trace = &thd->opt_trace;

  /*
    Guessing the number of distinct values in the table; used to
    make "rec_per_key"-like estimates when no statistics is
    available.
  */
  ha_rows distinct_keys_est = tab->records() / MATCHING_ROWS_IN_OTHER_TABLE;

  // Test how we can use keys
  for (Key_use *keyuse = tab->keyuse(); keyuse->table_ref == tab->table_ref;) {
    // keyparts that are usable for this index given the current partial plan
    key_part_map found_part = 0;
    // Bitmap of keyparts where the ref access is over 'keypart=const'
    key_part_map const_part = 0;
    // Keyparts where ref access will not match on NULL values.
    // Used for unique indexes on nullable columns to decide whether
    // a specific key may match on (multiple) NULL valued rows.
    key_part_map null_rejecting_part = 0;
    /*
      Cost of ref access on current index. Calculated as follows:
      cost_ref_for_one_value * prefix_rowcount
    */
    double cur_read_cost;
    // Fanout for ref access using this index
    double cur_fanout;
    uint cur_used_keyparts = 0;  // number of used keyparts
    // tables 'ref' access on this index depends on
    table_map table_deps = 0;
    const uint key = keyuse->key;
    const KEY *const keyinfo = table->key_info + key;
    /*
      Bitmap of keyparts in this index that have a condition

        "WHERE col=... OR col IS NULL"

      If 'ref' access is to be used in such cases, the JT_REF_OR_NULL
      type will be used.
    */
    key_part_map ref_or_null_part = 0;

    DBUG_PRINT("info", ("Considering ref access on key %s", keyinfo->name));
    Opt_trace_object trace_access_idx(trace);

    enum idx_type cur_keytype =
        (keyuse->keypart == FT_KEYPART) ? FULLTEXT : NOT_UNIQUE;

    // Calculate how many key segments of the current key we can use
    Key_use *const start_key = keyuse;
    start_key->bound_keyparts = 0;  // Initially, no ref access is possible

    // For each keypart
    while (keyuse->table_ref == tab->table_ref && keyuse->key == key) {
      const uint keypart = keyuse->keypart;
      // tables the current keypart depends on
      table_map cur_keypart_table_deps = 0;
      double best_distinct_prefix_rowcount = DBL_MAX;

      /*
        Check all ways to access the keypart. There is one keyuse
        object for each equality predicate for the keypart, and this
        loop estimates which equality predicate is best. Example that
        would have two keyuse objects for a keypart covering
        t1.col_x: "WHERE t1.col_x=4 AND t1.col_x=t2.col_y"
      */
      for (; keyuse->table_ref == tab->table_ref && keyuse->key == key &&
             keyuse->keypart == keypart;
           ++keyuse) {
        /*
          This keyuse cannot be used if
          1) it is a key reference between a table inside a semijoin
             nest and one outside of it. The same applices to
             materialized subqueries
          2) it is a key reference to a table that is not in the plan
             prefix (i.e., a table that will be later in the join
             sequence)
          3) there will be two ref_or_null keyparts
             ("WHERE col=... OR col IS NULL"). Thus if
             a) the condition for an earlier keypart is of type
                ref_or_null, and
             b) the condition for the current keypart is ref_or_null
        */
        if ((excluded_tables & keyuse->used_tables) ||        // 1)
            (remaining_tables & keyuse->used_tables) ||       // 2)
            (ref_or_null_part &&                              // 3a)
             (keyuse->optimize & KEY_OPTIMIZE_REF_OR_NULL)))  // 3b)
          continue;

        if (keypart != FT_KEYPART) {
          const bool keyinfo_maybe_null =
              keyinfo->key_part[keypart].field->is_nullable() ||
              tab->table()->is_nullable();
          if (keyuse->null_rejecting || !keyuse->val->maybe_null ||
              !keyinfo_maybe_null)
            null_rejecting_part |= keyuse->keypart_map;
        }
        found_part |= keyuse->keypart_map;
        if (!(keyuse->used_tables & ~join->const_table_map))
          const_part |= keyuse->keypart_map;

        const double cur_distinct_prefix_rowcount =
            prev_record_reads(join, idx, (table_deps | keyuse->used_tables));
        if (cur_distinct_prefix_rowcount < best_distinct_prefix_rowcount) {
          /*
            We estimate that the currently considered usage of the
            keypart will have to lookup fewer distinct key
            combinations from the prefix tables.
          */
          cur_keypart_table_deps = keyuse->used_tables & ~join->const_table_map;
          best_distinct_prefix_rowcount = cur_distinct_prefix_rowcount;
        }
        if (distinct_keys_est > keyuse->ref_table_rows)
          distinct_keys_est = keyuse->ref_table_rows;
        /*
          If there is one 'key_column IS NULL' expression, we can
          use this ref_or_null optimisation of this field
        */
        if (keyuse->optimize & KEY_OPTIMIZE_REF_OR_NULL)
          ref_or_null_part |= keyuse->keypart_map;
      }
      table_deps |= cur_keypart_table_deps;
    }

    if (distinct_keys_est < MATCHING_ROWS_IN_OTHER_TABLE) {
      // Fix for small tables
      distinct_keys_est = MATCHING_ROWS_IN_OTHER_TABLE;
      if (tab->records() && tab->records() < distinct_keys_est)
        distinct_keys_est = tab->records();
    }

    // fulltext indexes require special treatment
    if (cur_keytype != FULLTEXT) {
      *found_condition |= found_part;

      const bool all_key_parts_covered =
          (found_part == LOWER_BITS(key_part_map, actual_key_parts(keyinfo)));
      const bool all_key_parts_non_null =
          (ref_or_null_part == 0 &&
           null_rejecting_part ==
               LOWER_BITS(key_part_map, actual_key_parts(keyinfo)));
      /*
        check for the current key type.
        If we find a key with all the keyparts having equality predicates and
        --> if it is a clustered primary key, current key type is set to
            CLUSTERED_PK.
        --> if it is non-nullable unique key, it is set as UNIQUE.
        --> If none of the specified key parts may result in NULL value(s)
            being matched, it is set as UNIQUE.
        --> otherwise its a NOT_UNIQUE keytype.
      */
      if (all_key_parts_covered && (keyinfo->flags & HA_NOSAME)) {
        if (key == table->s->primary_key &&
            table->file->primary_key_is_clustered())
          cur_keytype = CLUSTERED_PK;
        else if ((keyinfo->flags & HA_NULL_PART_KEY) == 0)
          cur_keytype = UNIQUE;
        else if (all_key_parts_non_null)
          cur_keytype = UNIQUE;
      }

      if (cur_keytype == UNIQUE || cur_keytype == CLUSTERED_PK)
        trace_access_idx.add_alnum("access_type", "eq_ref");
      else
        trace_access_idx.add_alnum("access_type", "ref");

      trace_access_idx.add_utf8("index", keyinfo->name);

      if (cur_keytype > best_found_keytype) {
        trace_access_idx.add("chosen", false)
            .add_alnum("cause", "heuristic_eqref_already_found");
        if (unlikely(!test_all_ref_keys))
          continue;
        else {
          /*
            key will be rejected further down, after we compute its
            bound_keyparts/read_cost/fanout.
          */
        }
      }

      // Check if we found full key
      if (all_key_parts_covered && !ref_or_null_part) /* use eq key */
      {
        cur_used_keyparts = (uint)~0;
        if (keyinfo->flags & HA_NOSAME &&
            ((keyinfo->flags & HA_NULL_PART_KEY) == 0 ||
             all_key_parts_non_null)) {
          cur_read_cost = prev_record_reads(join, idx, table_deps) *
                          table->cost_model()->page_read_cost(1.0);
          cur_fanout = 1.0;
        } else {
          if (!table_deps) { /* We found a const key */
            /*
              ReuseRangeEstimateForRef-1:
              We get here if we've found a ref(const) (c_i are constants):
              "(keypart1=c1) AND ... AND (keypartN=cN)"   [ref_const_cond]

              If range optimizer was able to construct a "range"
              access on this index, then its condition "quick_cond" was
              eqivalent to ref_const_cond (*), and we can re-use E(#rows)
              from the range optimizer.

              Proof of (*): By properties of range and ref optimizers
              quick_cond will be equal or tighter than ref_const_cond.
              ref_const_cond already covers "smallest" possible interval -
              a singlepoint interval over all keyparts. Therefore,
              quick_cond is equivalent to ref_const_cond (if it was an
              empty interval we wouldn't have got here).
            */
            if (table->quick_keys.is_set(key))
              cur_fanout = (double)table->quick_rows[key];
            else {
              // quick_range couldn't use key
              cur_fanout = (double)tab->records() / distinct_keys_est;
            }
          } else {
            // Use records per key statistics if available
            if (keyinfo->has_records_per_key(actual_key_parts(keyinfo) - 1)) {
              cur_fanout =
                  keyinfo->records_per_key(actual_key_parts(keyinfo) - 1);
            } else { /* Prefer longer keys */
              DBUG_ASSERT(table->s->max_key_length > 0);
              cur_fanout =
                  ((double)tab->records() / (double)distinct_keys_est *
                   (1.0 +
                    ((double)(table->s->max_key_length - keyinfo->key_length) /
                     (double)table->s->max_key_length)));
              if (cur_fanout < 2.0)
                cur_fanout = 2.0; /* Can't be as good as a unique */
            }

            /*
              ReuseRangeEstimateForRef-2:  We get here if we could not reuse
              E(#rows) from range optimizer. Make another try:

              If range optimizer produced E(#rows) for a prefix of the ref
              access we're considering, and that E(#rows) is lower then our
              current estimate, make an adjustment. The criteria of when we
              can make an adjustment is a special case of the criteria used
              in ReuseRangeEstimateForRef-3.
            */
            if (table->quick_keys.is_set(key) &&
                (const_part &
                 (((key_part_map)1 << table->quick_key_parts[key]) - 1)) ==
                    (((key_part_map)1 << table->quick_key_parts[key]) - 1) &&
                table->quick_n_ranges[key] == 1 &&
                cur_fanout > (double)table->quick_rows[key]) {
              cur_fanout = (double)table->quick_rows[key];
            }
          }
          // Limit the number of matched rows
          const double tmp_fanout =
              min(cur_fanout, (double)thd->variables.max_seeks_for_key);
          if (table->covering_keys.is_set(key)) {
            // We can use only index tree
            const Cost_estimate index_read_cost =
                table->file->index_scan_cost(key, 1, tmp_fanout);
            cur_read_cost = prefix_rowcount * index_read_cost.total_cost();
          } else if (key == table->s->primary_key &&
                     table->file->primary_key_is_clustered()) {
            const Cost_estimate table_read_cost =
                table->file->read_cost(key, 1, tmp_fanout);
            cur_read_cost = prefix_rowcount * table_read_cost.total_cost();
          } else
            cur_read_cost = prefix_rowcount *
                            min(table->cost_model()->page_read_cost(tmp_fanout),
                                tab->worst_seeks);
        }
      } else if ((found_part & 1) &&
                 (!(table->file->index_flags(key, 0, false) &
                    HA_ONLY_WHOLE_INDEX) ||
                  all_key_parts_covered)) {
        /*
          Use as many key-parts as possible and a unique key is better
          than a not unique key.
          Set cur_fanout to (previous record count) * (records / combination)
        */

        cur_used_keyparts = max_part_bit(found_part);
        /*
          ReuseRangeEstimateForRef-3:
          We're now considering a ref[or_null] access via
          (t.keypart1=e1 AND ... AND t.keypartK=eK) [ OR
          (same-as-above but with one cond replaced
          with "t.keypart_i IS NULL")]  (**)

          Try re-using E(#rows) from "range" optimizer:
          We can do so if "range" optimizer used the same intervals as
          in (**). The intervals used by range optimizer may be not
          available at this point (as "range" access might have choosen to
          create quick select over another index), so we can't compare
          them to (**). We'll make indirect judgements instead.
          The sufficient conditions for re-use are:
          (C1) All e_i in (**) are constants, i.e. table_deps==false. (if
          this is not satisfied we have no way to know which ranges
          will be actually scanned by 'ref' until we execute the
          join)
          (C2) max #key parts in 'range' access == K == max_key_part (this
          is apparently a necessary requirement)

          We also have a property that "range optimizer produces equal or
          tighter set of scan intervals than ref(const) optimizer". Each
          of the intervals in (**) are "tightest possible" intervals when
          one limits itself to using keyparts 1..K (which we do in #2).

          From here it follows that range access uses either one or
          both of the (I1) and (I2) intervals:

          (t.keypart1=c1 AND ... AND t.keypartK=eK)  (I1)
          (same-as-above but with one cond replaced
          with "t.keypart_i IS NULL")               (I2)

          The remaining part is to exclude the situation where range
          optimizer used one interval while we're considering
          ref-or-null and looking for estimate for two intervals. This
          is done by last limitation:

          (C3) "range optimizer used (have ref_or_null?2:1) intervals"
        */
        double tmp_fanout = 0.0;
        if (table->quick_keys.is_set(key) && !table_deps &&      //(C1)
            table->quick_key_parts[key] == cur_used_keyparts &&  //(C2)
            table->quick_n_ranges[key] ==
                1 + (ref_or_null_part ? 1 : 0))  //(C3)
        {
          tmp_fanout = cur_fanout = (double)table->quick_rows[key];
        } else {
          // Check if we have statistic about the distribution
          if (keyinfo->has_records_per_key(cur_used_keyparts - 1)) {
            cur_fanout = keyinfo->records_per_key(cur_used_keyparts - 1);

            /*
              Fix for the case where the index statistics is too
              optimistic:
              If
              (1) We're considering ref(const) and there is quick select
              on the same index,
              (2) and that quick select uses more keyparts (i.e. it will
              scan equal/smaller interval then this ref(const))
              Then use E(#rows) from quick select.

              One observation is that when there are multiple
              indexes with a common prefix (eg (b) and (b, c)) we
              are not always selecting (b, c) even when this can
              use more keyparts. Inaccuracies in statistics from
              the storage engines can cause the record estimate
              for the quick object for (b) to be lower than the
              record estimate for the quick object for (b,c).

              Q: Why do we choose to use 'ref'? Won't quick select be
              cheaper in some cases ?
              TODO: figure this out and adjust the plan choice if needed.
            */
            if (!table_deps && table->quick_keys.is_set(key) &&   // (1)
                table->quick_key_parts[key] > cur_used_keyparts)  // (2)
            {
              trace_access_idx.add("chosen", false)
                  .add_alnum("cause", "range_uses_more_keyparts");
              continue;
            }

            tmp_fanout = cur_fanout;
          } else {
            /*
              Assume that the first key part matches 1% of the file
              and that the whole key matches 10 (duplicates) or 1
              (unique) records.
              Assume also that more key matches proportionally more
              records
              This gives the formula:
              records = (x * (b-a) + a*c-b)/(c-1)

              b = records matched by whole key
              a = records matched by first key part (1% of all records?)
              c = number of key parts in key
              x = used key parts (1 <= x <= c)
            */
            rec_per_key_t rec_per_key;
            if (keyinfo->has_records_per_key(keyinfo->user_defined_key_parts -
                                             1))
              rec_per_key =
                  keyinfo->records_per_key(keyinfo->user_defined_key_parts - 1);
            else
              rec_per_key =
                  rec_per_key_t(tab->records()) / distinct_keys_est + 1;

            if (tab->records() == 0)
              tmp_fanout = 0.0;
            else if (rec_per_key / tab->records() >= 0.01)
              tmp_fanout = rec_per_key;
            else {
              const double a = tab->records() * 0.01;
              if (keyinfo->user_defined_key_parts > 1)
                tmp_fanout =
                    (cur_used_keyparts * (rec_per_key - a) +
                     a * keyinfo->user_defined_key_parts - rec_per_key) /
                    (keyinfo->user_defined_key_parts - 1);
              else
                tmp_fanout = a;
              tmp_fanout = std::max(tmp_fanout, 1.0);
            }
            cur_fanout = (ulong)tmp_fanout;
          }

          if (ref_or_null_part) {
            // We need to do two key searches to find key
            tmp_fanout *= 2.0;
            cur_fanout *= 2.0;
          }

          /*
            ReuseRangeEstimateForRef-4:  We get here if we could not reuse
            E(#rows) from range optimizer. Make another try:

            If range optimizer produced E(#rows) for a prefix of the ref
            access we're considering, and that E(#rows) is lower then our
            current estimate, make the adjustment.

            The decision whether we can re-use the estimate from the range
            optimizer is the same as in ReuseRangeEstimateForRef-3,
            applied to first table->quick_key_parts[key] key parts.
          */
          if (table->quick_keys.is_set(key) &&
              table->quick_key_parts[key] <= cur_used_keyparts &&
              const_part & ((key_part_map)1 << table->quick_key_parts[key]) &&
              table->quick_n_ranges[key] ==
                  1 + ((ref_or_null_part & const_part) ? 1 : 0) &&
              cur_fanout > (double)table->quick_rows[key]) {
            tmp_fanout = cur_fanout = (double)table->quick_rows[key];
          }
        }

        // Limit the number of matched rows
        tmp_fanout =
            std::min(tmp_fanout, double(thd->variables.max_seeks_for_key));
        if (table->covering_keys.is_set(key)) {
          // We can use only index tree
          const Cost_estimate index_read_cost =
              table->file->index_scan_cost(key, 1, tmp_fanout);
          cur_read_cost = prefix_rowcount * index_read_cost.total_cost();
        } else if (key == table->s->primary_key &&
                   table->file->primary_key_is_clustered()) {
          const Cost_estimate table_read_cost =
              table->file->read_cost(key, 1, tmp_fanout);
          cur_read_cost = prefix_rowcount * table_read_cost.total_cost();
        } else
          cur_read_cost = prefix_rowcount *
                          min(table->cost_model()->page_read_cost(tmp_fanout),
                              tab->worst_seeks);
      } else {
        // No useful predicates on the first keypart; cannot use key
        trace_access_idx.add("usable", false).add("chosen", false);
        continue;
      }
    } else {
      // This is a full-text index

      trace_access_idx.add_alnum("access_type", "fulltext")
          .add_utf8("index", keyinfo->name);

      if (best_found_keytype < NOT_UNIQUE) {
        trace_access_idx.add("chosen", false)
            .add_alnum("cause", "heuristic_eqref_already_found");
        // Ignore test_all_ref_keys, semijoin loosescan never uses fulltext
        continue;
      }
      // Actually it should be cur_fanout=0.0 (yes!) but 1.0 is probably safer
      cur_read_cost = prev_record_reads(join, idx, table_deps) *
                      table->cost_model()->page_read_cost(1.0);
      cur_fanout = 1.0;
    }

    start_key->bound_keyparts = found_part;
    start_key->fanout = cur_fanout;
    start_key->read_cost = cur_read_cost;

    const double cur_ref_cost =
        cur_read_cost +
        prefix_rowcount * join->cost_model()->row_evaluate_cost(cur_fanout);
    trace_access_idx.add("rows", cur_fanout).add("cost", cur_ref_cost);

    /*
      The current index usage is better than the best index usage found
      so far if:

       1) The access type for the best index and the current index is
          FULLTEXT or REF, and the current index has a lower cost
       2) The access type is the same for the best index and the
          current index, and the current index has a lower cost
          (ie, both indexes are UNIQUE)
       3) The access type of the current index is better than
          that of the best index (EQ_REF better than REF, Clustered PK
          better than EQ_REF etc)
    */
    bool new_candidate = false;

    if (best_found_keytype >= NOT_UNIQUE && cur_keytype >= NOT_UNIQUE)
      new_candidate = cur_ref_cost < best_ref_cost;  // 1
    else if (best_found_keytype == cur_keytype)
      new_candidate = cur_ref_cost < best_ref_cost;  // 2
    else if (best_found_keytype > cur_keytype)
      new_candidate = true;  // 3

    if (new_candidate) {
      *ref_depend_map = table_deps;
      *used_key_parts = cur_used_keyparts;
      best_ref = start_key;
      best_ref_cost = cur_ref_cost;
      best_found_keytype = cur_keytype;
    }

    trace_access_idx.add("chosen", best_ref == start_key);

    if (best_found_keytype == CLUSTERED_PK) {
      trace_access_idx.add_alnum("cause", "clustered_pk_chosen_by_heuristics");
      if (unlikely(!test_all_ref_keys)) break;
    }
  }  // for each key

  return best_ref;
}

/**
  Calculate the cost of range/table/index scanning table 'tab'.

  Returns a hybrid scan cost number: the cost of fetching rows from
  the storage engine plus CPU cost during execution for evaluating the
  rows (estimate) that will be filtered out by predicates relevant to
  the table. The cost does not include the CPU cost during execution
  for rows that are not filtered out.

  This hybrid cost is needed because if join buffering is used to
  reduce the number of scans, then the final cost depends on how many
  times the join buffer had to be filled.

  @param tab                  the table to be joined by the function
  @param idx                  the index in join->position[] where 'tab'
                              is added to the partial plan.
  @param best_ref             description of the best ref access method
                              for 'tab'
  @param prefix_rowcount      estimate for the number of records returned
                              by the partial plan
  @param found_condition      whether or not there exists a condition
                              that filters away rows for this table.
                              @see find_best_ref()
  @param disable_jbuf         don't use join buffering if true
  @param[out] rows_after_filtering fanout of the access method after taking
                              condition filtering into account
  @param trace_access_scan    The optimizer trace object info is appended to

  @return                     Cost of fetching rows from the storage
                              engine plus CPU execution cost of the
                              rows that are estimated to be filtered out
                              by query conditions.
*/
double Optimize_table_order::calculate_scan_cost(
    const JOIN_TAB *tab, const uint idx, const Key_use *best_ref,
    const double prefix_rowcount, const bool found_condition,
    const bool disable_jbuf, double *rows_after_filtering,
    Opt_trace_object *trace_access_scan) {
  double scan_and_filter_cost;
  TABLE *const table = tab->table();
  const Cost_model_server *const cost_model = join->cost_model();
  *rows_after_filtering = static_cast<double>(tab->found_records);

  trace_access_scan->add("rows_to_scan", tab->found_records);

  /*
    This block should only affect the cost of scans using join
    buffering. Consider moving it to the if () block that handles join
    buffering.
  */
  if (thd->optimizer_switch_flag(OPTIMIZER_SWITCH_COND_FANOUT_FILTER)) {
    const float const_cond_filter = calculate_condition_filter(
        tab, nullptr, 0, static_cast<double>(tab->found_records), !disable_jbuf,
        true, *trace_access_scan);

    /*
      For high found_records values, multiplication by float may
      result in a higher value than the original for
      const_cond_filter=1.0. Cast to double to increase precision.
    */
    *rows_after_filtering = rows2double(tab->found_records) * const_cond_filter;
  } else if (table->quick_condition_rows != tab->found_records)
    *rows_after_filtering = static_cast<double>(table->quick_condition_rows);
  else if (found_condition) {
    /*
      If there is a filtering condition on the table (i.e. ref
      analyzer found at least one "table.keyXpartY= exprZ", where
      exprZ refers only to tables preceding this table in the join
      order we're now considering), and optimizer condition filtering
      is turned off, then assume that 25% of the rows will be filtered
      out by this condition.

      This heuristic is supposed to force tables used in exprZ to be
      before this table in join order.
    */

    *rows_after_filtering = tab->found_records * 0.75;
  }

  /*
    Range optimizer never proposes a RANGE if it isn't better
    than FULL: so if RANGE is present, it's always preferred to FULL.
    Here we estimate its cost.
  */
  if (tab->quick()) {
    trace_access_scan->add_alnum("access_type", "range");
    tab->quick()->trace_quick_description(&thd->opt_trace);
    /*
      For each record we:
      - read record range through 'quick'
      - skip rows which does not satisfy WHERE constraints
      TODO:
      We take into account possible use of join cache for ALL/index
      access (see first else-branch below), but we don't take it into
      account here for range/index_merge access. Find out why this is so.
    */
    scan_and_filter_cost =
        prefix_rowcount * (tab->quick()->cost_est.total_cost() +
                           cost_model->row_evaluate_cost(
                               tab->found_records - *rows_after_filtering));
  } else {
    trace_access_scan->add_alnum("access_type", "scan");

    // Cost of scanning the table once
    Cost_estimate scan_cost;
    if (table->force_index && !best_ref)  // index scan
      scan_cost = table->file->read_cost(tab->ref().key, 1,
                                         static_cast<double>(tab->records()));
    else
      scan_cost = table->file->table_scan_cost();  // table scan
    const double single_scan_read_cost = scan_cost.total_cost();

    /* Estimate total cost of reading table. */
    if (disable_jbuf) {
      /*
        For each record from the prefix we have to:
        - read the whole table
        - skip rows which does not satisfy join condition

        Note that there is also the cost of evaluating rows that DO
        satisfy the WHERE condition, but this is added
        a) temporarily in best_access_path(), before comparing this
           scan cost to the best 'ref' access method, and
        b) permanently by the caller of best_access_path() (@see e.g.
           best_extension_by_limited_search())
      */
      scan_and_filter_cost =
          prefix_rowcount *
          (single_scan_read_cost + cost_model->row_evaluate_cost(
                                       tab->records() - *rows_after_filtering));
    } else {
      /*
        IO cost: We read the table as many times as join buffer
        becomes full. (It would be more exact to round the result of
        the division with floor(), but that takes 5% of time in a
        20-table query plan search.)

        CPU cost: For every full join buffer, attached conditions are
        evaluated for each row in the scanned table. We assume that
        the conditions evaluate to 'true' for 'rows_after_filtering'
        number of rows. The rows that pass are then joined with the
        prefix rows.

        The CPU cost for the rows that do NOT satisfy the attached
        conditions is considered to be part of the read cost and is
        added below. The cost of joining the rows that DO satisfy the
        attached conditions with all prefix rows is added in
        greedy_search().
      */
      const double buffer_count =
          1.0 + ((double)cache_record_length(join, idx) * prefix_rowcount /
                 (double)thd->variables.join_buff_size);

      scan_and_filter_cost =
          buffer_count *
          (single_scan_read_cost + cost_model->row_evaluate_cost(
                                       tab->records() - *rows_after_filtering));

      trace_access_scan->add("using_join_cache", true);
      trace_access_scan->add(
          "buffers_needed",
          buffer_count >= static_cast<double>(std::numeric_limits<ulong>::max())
              ? std::numeric_limits<ulong>::max()
              : static_cast<ulong>(buffer_count));
    }
  }

  return scan_and_filter_cost;
}

/**
  If table is a lateral derived table, calculates the "cost of
  materialization", which is the cost of a single materialization (available
  in the DT's underlying JOIN final plan) multiplied by the number of rows
  output by the last-in-plan table which DT references (available in a
  POSITION structure). For example if plan is
  t1 (outputs 2 rows) - t2 (outputs 20 rows) - dt
  and dt's definition references only t1, we multiply by 2, not by 20.
  This cost is divided by the number of times the DT will be read (20, here),
  to provide a number which best_access_path() can add to best_read_cost.
*/
double Optimize_table_order::lateral_derived_cost(
    const JOIN_TAB *tab, const uint idx, const double prefix_rowcount,
    const Cost_model_server *cost_model) {
  DBUG_ASSERT(tab->table_ref->is_derived() &&
              tab->table_ref->derived_unit()->m_lateral_deps);
  if (prefix_rowcount == 0)  // no input rows: no materialization needed
    return 0;
  table_map deps = tab->table_ref->derived_unit()->m_lateral_deps;
  POSITION *positions = got_final_plan ? join->best_positions : join->positions;
  double derived_mat_cost = 0;
  for (int j = idx; j >= (int)join->const_tables; j--) {
    if (deps & join->best_ref[j]->table_ref->map()) {
      // We found the last table in plan, on which 'tab' depends.
      auto res = tab->table_ref->derived_unit()->query_result();
      double inner_query_cost = res->estimated_cost;
      double inner_query_rowcount = res->estimated_rowcount;
      // copied and simplified from calculate_materialization_costs()
      Cost_model_server::enum_tmptable_type tmp_table_type;
      if (tab->table()->s->reclength * inner_query_rowcount <
          thd->variables.max_heap_table_size)
        tmp_table_type = Cost_model_server::MEMORY_TMPTABLE;
      else
        tmp_table_type = Cost_model_server::DISK_TMPTABLE;
      double write_cost = cost_model->tmptable_readwrite_cost(
          tmp_table_type, inner_query_rowcount, 0.0);
      double mat_times = positions[j].prefix_rowcount;
      double total_mat_cost = mat_times * (inner_query_cost + write_cost);
      // average per read request:
      derived_mat_cost = total_mat_cost / prefix_rowcount;
      Opt_trace_context *const trace = &thd->opt_trace;
      Opt_trace_object trace_lateral(trace);
      Opt_trace_object trace_details(trace, "lateral_materialization");
      trace_details.add("cost_for_one_run_of_inner_query", inner_query_cost)
          .add("cost_for_writing_to_tmp_table", write_cost)
          .add("count_of_runs", mat_times)
          .add("total_cost", total_mat_cost)
          .add("cost_per_read", derived_mat_cost);
      break;
    }
  }
  return derived_mat_cost;
}

/**
  Find the best access path for an extension of a partial execution
  plan and add this path to the plan.

  The function finds the best access path to table 'tab' from the
  passed partial plan where an access path is the general term for any
  means to access the data in 'tab'. An access path may use either an
  index scan, a table scan, a range scan or ref access, whichever is
  cheaper. The input partial plan is passed via the array
  'join->positions' of length 'idx'. The chosen access method for
  'tab' and its cost is stored in 'join->positions[idx]'.

  @param tab               the table to be joined by the function
  @param remaining_tables  set of tables not included in the partial plan yet.
  @param idx               the index in join->position[] where 'tab' is added
                           to the partial plan.
  @param disable_jbuf      true<=> Don't use join buffering
  @param prefix_rowcount   estimate for the number of records returned by the
                           partial plan
  @param[out] pos          Table access plan
*/

void Optimize_table_order::best_access_path(JOIN_TAB *tab,
                                            const table_map remaining_tables,
                                            const uint idx, bool disable_jbuf,
                                            const double prefix_rowcount,
                                            POSITION *pos) {
  bool found_condition = false;
  bool best_uses_jbuf = false;
  Opt_trace_context *const trace = &thd->opt_trace;
  TABLE *const table = tab->table();
  const Cost_model_server *const cost_model = join->cost_model();

  float filter_effect = 1.0;

  thd->m_current_query_partial_plans++;

  /*
    Cannot use join buffering if either
     1. This is the first table in the join sequence, or
     2. Join buffering is not enabled
        (Only Block Nested Loop is considered in this context)
     3. If first-dependency-of-remaining-lateral-table < table-we-plan-for.
     Reason for 3: @see setup_join_buffering().
  */
  disable_jbuf =
      disable_jbuf || idx == join->const_tables ||  // 1
      !hint_table_state(join->thd, tab->table_ref,  // 2
                        BNL_HINT_ENUM, OPTIMIZER_SWITCH_BNL) ||
      join->deps_of_remaining_lateral_derived_tables & ~remaining_tables;  // 3

  DBUG_TRACE;

  Opt_trace_object trace_wrapper(trace, "best_access_path");
  Opt_trace_array trace_paths(trace, "considered_access_paths");

  // The 'ref' access method with lowest cost as found by find_best_ref()
  Key_use *best_ref = nullptr;

  table_map ref_depend_map = 0;
  uint used_key_parts = 0;

  // Look for the best ref access if the storage engine supports index access.
  if (tab->keyuse() != nullptr &&
      (table->file->ha_table_flags() & HA_NO_INDEX_ACCESS) == 0)
    best_ref =
        find_best_ref(tab, remaining_tables, idx, prefix_rowcount,
                      &found_condition, &ref_depend_map, &used_key_parts);

  double rows_fetched = best_ref ? best_ref->fanout : DBL_MAX;
  /*
    Cost of executing the best access method prefix_rowcount
    number of times
  */
  double best_read_cost = best_ref ? best_ref->read_cost : DBL_MAX;

  double derived_mat_cost =
      (tab->table_ref->is_derived() &&
       tab->table_ref->derived_unit()->m_lateral_deps)
          ? lateral_derived_cost(tab, idx, prefix_rowcount, cost_model)
          : 0;

  Opt_trace_object trace_access_scan(trace);
  /*
    Don't test table scan if it can't be better.
    Prefer key lookup if we would use the same key for scanning.

    Don't do a table scan on InnoDB tables, if we can read the used
    parts of the row from any of the used index.
    This is because table scans uses index and we would not win
    anything by using a table scan. The only exception is INDEX_MERGE
    quick select. We can not say for sure that INDEX_MERGE quick select
    is always faster than ref access. So it's necessary to check if
    ref access is more expensive.

    We do not consider index/table scan or range access if:

    1a) The best 'ref' access produces fewer records than a table scan
        (or index scan, or range acces), and
    1b) The best 'ref' executed for all partial row combinations, is
        cheaper than a single scan. The rationale for comparing

        COST(ref_per_partial_row) * E(#partial_rows)
           vs
        COST(single_scan)

        is that if join buffering is used for the scan, then scan will
        not be performed E(#partial_rows) times, but
        E(#partial_rows)/E(#partial_rows_fit_in_buffer). At this point
        in best_access_path() we don't know this ratio, but it is
        somewhere between 1 and E(#partial_rows). To avoid
        overestimating the total cost of scanning, the heuristic used
        here has to assume that the ratio is 1. A more fine-grained
        cost comparison will be done later in this function.
    (2) The best way to perform table or index scan is to use 'range' access
        using index IDX. If it is a 'tight range' scan (i.e not a loose index
        scan' or 'index merge'), then ref access on the same index will
        perform equal or better if ref access can use the same or more number
        of key parts.
    (3) See above note about InnoDB.
    (4) NOT ("FORCE INDEX(...)" is used for table and there is 'ref' access
             path, but there is no quick select)
        If the condition in the above brackets holds, then the only possible
        "table scan" access method is ALL/index (there is no quick select).
        Since we have a 'ref' access path, and FORCE INDEX instructs us to
        choose it over ALL/index, there is no need to consider a full table
        scan.
  */
  if (rows_fetched < tab->found_records &&  // (1a)
      best_read_cost <= tab->read_time)     // (1b)
  {
    // "scan" means (full) index scan or (full) table scan.
    if (tab->quick()) {
      trace_access_scan.add_alnum("access_type", "range");
      tab->quick()->trace_quick_description(trace);
    } else
      trace_access_scan.add_alnum("access_type", "scan");

    trace_access_scan
        .add("cost",
             tab->read_time + cost_model->row_evaluate_cost(
                                  static_cast<double>(tab->found_records)))
        .add("rows", tab->found_records)
        .add("chosen", false)
        .add_alnum("cause", "cost");
  } else if (tab->quick() && best_ref &&              // (2)
             tab->quick()->index == best_ref->key &&  // (2)
             (used_key_parts >=
              table->quick_key_parts[best_ref->key]) &&  // (2)
             (tab->quick()->get_type() !=
              QUICK_SELECT_I::QS_TYPE_GROUP_MIN_MAX) &&
             (tab->quick()->get_type() !=
              QUICK_SELECT_I::QS_TYPE_SKIP_SCAN))  // (2)
  {
    trace_access_scan.add_alnum("access_type", "range");
    tab->quick()->trace_quick_description(trace);
    trace_access_scan.add("chosen", false)
        .add_alnum("cause", "heuristic_index_cheaper");
  } else if ((table->file->ha_table_flags() & HA_TABLE_SCAN_ON_INDEX) &&  //(3)
             !table->covering_keys.is_clear_all() && best_ref &&          //(3)
             (!tab->quick() ||                                            //(3)
              (tab->quick()->get_type() ==
                   QUICK_SELECT_I::QS_TYPE_ROR_INTERSECT &&  //(3)
               best_ref->read_cost <
                   tab->quick()->cost_est.total_cost())))  //(3)
  {
    if (tab->quick()) {
      trace_access_scan.add_alnum("access_type", "range");
      tab->quick()->trace_quick_description(trace);
    } else
      trace_access_scan.add_alnum("access_type", "scan");

    trace_access_scan.add("chosen", false)
        .add_alnum("cause", "covering_index_better_than_full_scan");
  } else if ((table->force_index && best_ref && !tab->quick()))  // (4)
  {
    trace_access_scan.add_alnum("access_type", "scan")
        .add("chosen", false)
        .add_alnum("cause", "force_index");
  } else {
    /*
      None of the heuristics found that table/index/range scan is
      obviously more expensive than 'ref' access. The 'ref' cost
      therefore has to be compared to the cost of scanning.
    */
    double rows_after_filtering;

    double scan_read_cost = calculate_scan_cost(
        tab, idx, best_ref, prefix_rowcount, found_condition, disable_jbuf,
        &rows_after_filtering, &trace_access_scan);

    /*
      We estimate the cost of evaluating WHERE clause for found
      records as row_evaluate_cost(prefix_rowcount * rows_after_filtering).
      This cost plus scan_cost gives us total cost of using
      TABLE/INDEX/RANGE SCAN.
    */
    const double scan_total_cost =
        scan_read_cost +
        cost_model->row_evaluate_cost(prefix_rowcount * rows_after_filtering);

    trace_access_scan.add("resulting_rows", rows_after_filtering);
    trace_access_scan.add("cost", scan_total_cost);

    if (best_ref == nullptr ||
        (scan_total_cost <
         best_read_cost +
             cost_model->row_evaluate_cost(prefix_rowcount * rows_fetched))) {
      /*
        If the table has a range (tab->quick is set) make_join_select()
        will ensure that this will be used
      */
      best_read_cost = scan_read_cost;
      rows_fetched = rows_after_filtering;

      if (tab->found_records) {
        /*
          Although join buffering may be used for this table, this
          filter calculation is not done to calculate the cost of join
          buffering itself (that is done inside
          calculate_scan_cost()). The is_join_buffering parameter is
          therefore 'false'.
        */
        const float full_filter = calculate_condition_filter(
            tab, nullptr, ~remaining_tables & ~excluded_tables,
            static_cast<double>(tab->found_records), false, false,
            trace_access_scan);
        filter_effect = static_cast<float>(std::min(
            1.0, tab->found_records * full_filter / rows_after_filtering));
      }
      best_ref = nullptr;
      best_uses_jbuf = !disable_jbuf;
      ref_depend_map = 0;
    }

    trace_access_scan.add("chosen", best_ref == nullptr);
  }

  /*
    Storage engines that track exact sizes may report an empty table
    as having row count equal to 0.
    If this table is an inner table of an outer join, adjust row count to 1,
    so that the join planner can make a better fanout calculation for
    the remaining tables of the join. (With size 0, the fanout would always
    become 0, meaning that the cost of adding one more table would also
    become 0, regardless of access method).
  */
  if (rows_fetched == 0.0 &&
      (join->select_lex->outer_join & tab->table_ref->map()))
    rows_fetched = 1.0;

  /*
    Do not calculate condition filtering unless 'ref' access is
    chosen. The filtering effect for all the scan types of access
    (range/index scan/table scan) has already been calculated.
  */
  if (best_ref)
    filter_effect = calculate_condition_filter(
        tab, best_ref, ~remaining_tables & ~excluded_tables, rows_fetched,
        false, false, trace_access_scan);

  best_read_cost += derived_mat_cost;
  pos->filter_effect = filter_effect;
  pos->rows_fetched = rows_fetched;
  pos->read_cost = best_read_cost;
  pos->key = best_ref;
  pos->table = tab;
  pos->ref_depend_map = ref_depend_map;
  pos->loosescan_key = MAX_KEY;
  pos->use_join_buffer = best_uses_jbuf;

  if (!best_ref && idx == join->const_tables && table == join->sort_by_table &&
      join->unit->select_limit_cnt >= rows_fetched) {
    trace_access_scan.add("use_tmp_table", true);
    join->sort_by_table = (TABLE *)1;  // Must use temporary table
  }
}

float calculate_condition_filter(const JOIN_TAB *const tab,
                                 const Key_use *const keyuse,
                                 table_map used_tables, double fanout,
                                 bool is_join_buffering, bool write_to_trace,
                                 Opt_trace_object &parent_trace) {
  /*
    Because calculating condition filtering has a cost, it should only
    be done if the filter is meaningful. It is meaningful if the query
    is an EXPLAIN, or if the filter may influence the QEP.

    Note that this means that EXPLAIN FOR CONNECTION will typically
    not find a calculated filtering value for the last table in a QEP
    (i.e., it will be 1.0).

    Calculate condition filter if
    1)  Condition filtering is enabled, and
    2a) Condition filtering is about to be calculated for a scan that
        might do join buffering. Rationale: When a table is scanned
        and joined with rows in a buffer, constant predicates are
        evaluated on rows in the joined table. Only rows that pass the
        constant predicates are attempted joined with the prefix rows
        in the buffer. The filtering effect is the estimate of how
        many rows pass the constant predicate evaluation.
    2b) 'tab' is not the last table that will be added to the plan.
        Rationale: filtering only reduces the number of rows sent to
        the next step in the join ordering and therefore has no effect
        on the last table in the join order, or
    2c) 'tab' is in a subselect. Rationale: for subqueries, view/table
        materializations, the filtering effect is needed to
        estimate the number of rows in the potentially materialized
        subquery, or
    2d) 'tab' is in a select_lex with a semijoin nest. Rationale: the
        cost of some of the duplicate elimination strategies depends
        on the size of the output, or
    2e) The query has either an order by or group by clause and a limit clause.
        Rationale: some of the limit optimizations take the filtering effect
        on the last table into account.
    2f) Statement is EXPLAIN

    Note: Even in the case of a single table query, the filtering
    effect may effect the QEP because the cost of sorting fewer rows
    is lower. This is currently ignored since single table
    optimization performance is so important.
  */
  const THD *thd = tab->join()->thd;
  TABLE *const table = tab->table();
  const table_map remaining_tables =
      ~used_tables & ~tab->table_ref->map() & tab->join()->all_table_map;
  if (!(thd->optimizer_switch_flag(
            OPTIMIZER_SWITCH_COND_FANOUT_FILTER) &&  // 1)
        (is_join_buffering ||                        // 2a
         remaining_tables != 0 ||                    // 2b
         tab->join()->select_lex->master_unit()->outer_select() !=
             nullptr ||                                 // 2c
         !tab->join()->select_lex->sj_nests.empty() ||  // 2d
         ((tab->join()->order || tab->join()->group_list) &&
          tab->join()->unit->select_limit_cnt != HA_POS_ERROR) ||  // 2e
         thd->lex->is_explain())))                                 // 2f
    return COND_FILTER_ALLPASS;

  // No filtering is calculated if we expect less than one row to be fetched
  if (fanout < 1.0 || tab->found_records < 1.0 || tab->records() < 1.0)
    return COND_FILTER_ALLPASS;

  /*
    cond_set has the column bit set for each column involved in a
    predicate. If no bits are set, there are no predicates on this
    table.
  */
  if (bitmap_is_clear_all(&table->cond_set)) return COND_FILTER_ALLPASS;

  /*
    Use TABLE::tmp_set to keep track of fields that should not
    contribute to filtering effect.
    First, verify it's not used.
  */
  DBUG_ASSERT(bitmap_is_clear_all(&table->tmp_set));

  float filter = COND_FILTER_ALLPASS;

  Opt_trace_context *const trace = &tab->join()->thd->opt_trace;

  Opt_trace_disable_I_S disable_trace(trace, !write_to_trace);
  Opt_trace_array filtering_effect_trace(trace, "filtering_effect");

  /*
    If ref/range access, the condition is already included in the
    record estimate. The fields used by the ref/range access method
    shall not contribute to the filtering estimate since 'filter' is
    percentage of fetched rows that are filtered away.
  */
  if (keyuse) {
    const KEY *key = table->key_info + keyuse->key;

    if (keyuse[0].keypart == FT_KEYPART) {
      /*
        Fulltext indexes are special because keyuse->keypart does not
        contain the keypart number but a constant (FT_KEYPART)
        defining that it is a fulltext index. However, since fulltext
        search demands that all indexed keyparts are used, iterating
        over the next 'actual_key_parts' works.
      */
      for (uint i = 0; i < key->actual_key_parts; i++)
        bitmap_set_bit(&table->tmp_set, key->key_part[i].field->field_index);
    } else {
      const Key_use *curr_ku = keyuse;

      /*
        'keyuse' describes the chosen ref access method for 'tab'. It
        is a pointer into JOIN::keyuse_array which describes all
        possible ways to perform ref access for all indexes of all
        tables. E.g., keyuse for the index "t1.idx(kp1, kp2)" and
        query condition

          "WHERE t1.kp1=1 AND t1.kp1=t2.col AND t1.kp2=2"
        will be
          [keyuse(t1.kp1,1),keyuse(t1.kp1,t2.col),keyuse(t1.kp2,2)]

        1) Since there may be multiple ways to ref-access any index it
        is not enough to look at keyuse[0..actual_key_parts-1].
        Instead, stop iterating when curr_ku no longer points to the
        specified index in 'tab'.

        2) In addition, there may be predicates that are relevant for
        an index but that will not be used by the 'ref' access (the
        keypart is not bound). This could e.g. be because the
        predicate depends on a value from a table later in the join
        sequence or because there is ref_or_null access:

          "WHERE t1.kp1=1 AND t1.kp2=t2.col"
             => t1.kp2 not used by ref since it depends on
                table later in join sequenc
          "WHERE (t1.kp1=1 OR t1.kp1 IS NULL) AND t1.kp2=2"
             => t1.kp2 not used by ref since kp1 is ref_or_null
      */
      while (curr_ku->table_ref == tab->table_ref &&         // 1)
             curr_ku->key == keyuse->key &&                  // 1)
             curr_ku->keypart_map & keyuse->bound_keyparts)  // 2)
      {
        bitmap_set_bit(&table->tmp_set,
                       key->key_part[curr_ku->keypart].field->field_index);
        curr_ku++;
      }
    }
  } else if (tab->quick())
    tab->quick()->get_fields_used(&table->tmp_set);

  /*
    Early exit if the only conditions for the table refers to columns
    used by the access method.
  */
  if (bitmap_is_subset(&table->cond_set, &table->tmp_set)) {
    DBUG_ASSERT(filter == COND_FILTER_ALLPASS);
    goto cleanup;
  }
  /*
    If the range optimizer has made row estimates for predicates that
    are not used by the chosen access method, the estimate from the
    range optimizer is used as filtering effect for those fields. We
    do this because the range optimizer is more accurate than index
    statistics.
  */
  if (!table->quick_keys.is_clear_all()) {
    char buf[MAX_FIELDS / 8];
    my_bitmap_map *bitbuf =
        static_cast<my_bitmap_map *>(static_cast<void *>(&buf));
    MY_BITMAP fields_current_quick;

    for (uint keyno = 0; keyno < table->s->keys; keyno++) {
      if (table->quick_keys.is_set(keyno)) {
        // The range optimizer made a row estimate for this index

        bitmap_init(&fields_current_quick, bitbuf, table->s->fields);

        const KEY *key = table->key_info + keyno;
        for (uint i = 0; i < table->quick_key_parts[keyno]; i++)
          bitmap_set_bit(&fields_current_quick,
                         key->key_part[i].field->field_index);

        /*
          If any of the fields used to get the rows estimate for this
          index were used to get a rows estimate for another index
          already contributing to 'filter', or by the access method we
          ignore it.
        */
        if (bitmap_is_overlapping(&table->tmp_set, &fields_current_quick))
          continue;

        bitmap_union(&table->tmp_set, &fields_current_quick);

        const float selectivity = static_cast<float>(table->quick_rows[keyno]) /
                                  static_cast<float>(tab->records());
        // Cannot possible access more rows than there are in the table
        filter *= std::min(selectivity, 1.0f);
      }
    }
  }

  /*
    Filtering effect for predicates that can be gathered from the
    range optimizer is now reflected in 'filter', and the fields of
    those predicates are set in 'tmp_set' to avoid that a
    single predicate contributes twice to 'filter'.

    Only calculate the filtering effect if
    1) There are query conditions, and
    2) At least one of the query conditions affect a field that is not
       going to be ignored in 'tab'. In other words, there has to
       exist a condition on a field that is not used by the ref/range
       access method.
  */
  if (tab->join()->where_cond &&                             // 1)
      !bitmap_is_subset(&table->cond_set, &table->tmp_set))  // 2)
  {
    /*
      Get filtering effect for predicates that are not already
      reflected in 'filter'. The below call gets this filtering effect
      based on index statistics and guesstimates.
    */
    filter *= tab->join()->where_cond->get_filtering_effect(
        tab->join()->thd, tab->table_ref->map(), used_tables, &table->tmp_set,
        static_cast<double>(tab->records()));
  }

  /*
    Cost calculations and picking the right join order assumes that a
    positive number of output rows from each joined table. We assume
    that at least one row in the table match the condition.  Not all
    code is able to cope with estimates of less than one row.  (For
    example, DupsWeedout may include extra tables in its
    duplicate-eliminating range in such cases.)
  */
  filter = max(filter, 1.0f / tab->records());

  /*
    For large tables, the restriction above may still give very small
    numbers when calculating fan-out.  The code below makes sure that
    there is a lower limit on fan-out.
    TODO: Should evaluate whether this restriction makes sense.  It
          can cause the estimated size of the result set to be
          different for different join orders. However, some unwanted
          effects on DBT-3 was observed when removing it, so keeping
          it for now.
  */
  if ((filter * fanout) < 0.05f) filter = 0.05f / static_cast<float>(fanout);

cleanup:
  filtering_effect_trace.end();
  parent_trace.add("final_filtering_effect", filter);

  // Clear tmp_set so it can be used elsewhere
  bitmap_clear_all(&table->tmp_set);
  DBUG_ASSERT(filter >= 0.0f && filter <= 1.0f);
  return filter;
}

/**
   Returns a bitmap of bound semi-join equalities.

   If we consider (oe1, .. oeN) IN (SELECT ie1, .. ieN) then ieK=oeK is
   called sj-equality. If ieK or oeK depends only on tables available before
   'tab' in this plan, then such equality is called "bound".

   @param tab                   table
   @param not_available_tables  bitmap of not-available tables.
*/
static ulonglong get_bound_sj_equalities(const JOIN_TAB *tab,
                                         table_map not_available_tables) {
  ulonglong bound_sj_equalities = 0;
  List_iterator<Item> it_o(tab->emb_sj_nest->nested_join->sj_outer_exprs);
  List_iterator_fast<Item> it_i(tab->emb_sj_nest->nested_join->sj_inner_exprs);
  Item *outer, *inner;
  for (uint i = 0;; ++i) {
    outer = it_o++;
    if (!outer) break;
    inner = it_i++;
    if (!((not_available_tables)&outer->used_tables())) {
      bound_sj_equalities |= 1ULL << i;
      continue;
    }
    /*
      Now we look at equality propagation, to discover that a semi-join
      equality is bound, when the outer or inner expression is a field
      involved in some other non-semi-join equality.
      For example (propagation with inner field):
      select * from t2 where (b+0,a+0) in (select a,b from t1 where a=3);
      if the plan is t1-t2, 1st sj equality is bound, even though the
      corresponding outer expression t2.b+0 refers to 't2' which is not yet
      available.
      Other example (propagation with outer field):
      select * from t2 as t3, t2
      where t2.filler=t3.filler and
      (t2.b,t2.a,t2.filler) in (select a,b,a*3 from t1);
      if the plan is t3-t1-t2, 3rd sj equality is bound.

      We locate the relevant multiple equalities for the field. They are in
      the COND_EQUAL of the join nest which embeds the field's table. For
      example:
      select * from t1 left join t1 as t2
      on (t2.a= t1.a and (t2.a,t2.b) in (select a,b from t1 as t3))
      here we have:
      - a join nest (t2,t3) (called "wrap-nest"), which has a COND_EQUAL
      containing, among others: t2.a=t1.a
      - no COND_EQUAL for the WHERE clause.
      If the plan is t1-t3-t2, by looking at t2.a=t1.a we can deduce that
      the first semi join equality is bound.
    */
    Item *item;
    if (outer->type() == Item::FIELD_ITEM)
      item = outer;
    else if (inner->type() == Item::FIELD_ITEM)
      item = inner;
    else
      continue;
    Item_field *const item_field = static_cast<Item_field *>(item);
    Item_equal *item_equal = item_field->item_equal;
    if (!item_equal) {
      TABLE_LIST *const nest = item_field->table_ref->outer_join_nest();
      item_equal = item_field->find_item_equal(nest ? nest->cond_equal
                                                    : tab->join()->cond_equal);
    }
    if (item_equal) {
      /*
        If the multiple equality {[optional_constant,] col1, col2...} contains
        (1) a constant
        (2) or a column from an available table
        then the semi-join equality is bound.
      */
      if (item_equal->get_const() ||                            // (1)
          (item_equal->used_tables() & ~not_available_tables))  // (2)
        bound_sj_equalities |= 1ULL << i;
    }
  }
  return bound_sj_equalities;
}

/**
  Fills a POSITION object of the driving table of a semi-join LooseScan
  range, with the cheapest access path.

  This function was created by copying the code from best_access_path, and
  then eliminating everything which isn't related to semi-join LooseScan.

  Preconditions:
  1. Those checked by advance_sj_state(), ensuring that 'tab' is a valid
  LooseScan candidate.
  2. This function uses the members 'bound_keyparts', 'cost' and 'records' of
  each Key_use; thus best_access_path () must have been called, for this
  table, with the current join prefix, so that the members are up to date.

  @param tab               the driving table
  @param remaining_tables  set of tables not included in the partial plan yet.
  @param idx               the index in join->position[] where 'tab' is
                           added to the partial plan.
  @param prefix_rowcount   estimate for the number of records returned
                           by the partial plan
  @param[out] pos  If return code is 'true': table access path that uses
                   loosescan

  @returns true if it found a loosescan access path for this table.
*/

bool Optimize_table_order::semijoin_loosescan_fill_driving_table_position(
    const JOIN_TAB *tab, table_map remaining_tables, uint idx,
    double prefix_rowcount, POSITION *pos) {
  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_object trace_ls(trace, "searching_loose_scan_index");

  TABLE *const table = tab->table();
  DBUG_ASSERT(remaining_tables & tab->table_ref->map());

  const ulonglong bound_sj_equalities =
      get_bound_sj_equalities(tab, excluded_tables | remaining_tables);

  // Use of quick select is a special case. Some of its properties:
  bool quick_uses_applicable_index = false;
  uint quick_max_keypart = 0;

  pos->read_cost = DBL_MAX;
  pos->use_join_buffer = false;
  /*
    No join buffer, so no need to manage any
    Deps_of_remaining_lateral_derived_tables object.
    As this function calculates some read cost, we have to include any lateral
    materialization cost:
  */
  double derived_mat_cost =
      (tab->table_ref->is_derived() &&
       tab->table_ref->derived_unit()->m_lateral_deps)
          ? lateral_derived_cost(tab, idx, prefix_rowcount, join->cost_model())
          : 0;

  Opt_trace_array trace_all_idx(trace, "indexes");

  /*
    For each index, we calculate how many key segments of this index
    we can use.
  */
  for (Key_use *keyuse = tab->keyuse(); keyuse->table_ref == tab->table_ref;) {
    const uint key = keyuse->key;

    Key_use *const start_key = keyuse;
    Opt_trace_object trace_idx(trace);
    trace_idx.add_utf8("index", table->key_info[key].name);

    /*
      Equalities where one comparand is in index and other comparand is a
      not-yet-available expression.
    */
    ulonglong handled_sj_equalities = 0;
    key_part_map handled_keyparts = 0;
    /*
      Biggest index (starting at 0) of keyparts used for the "handled", not
      "bound", equalities.
    */
    uint max_keypart = 0;

    // For each keypart
    while (keyuse->table_ref == tab->table_ref && keyuse->key == key) {
      const uint keypart = keyuse->keypart;
      // For each way to access the keypart
      for (; keyuse->table_ref == tab->table_ref && keyuse->key == key &&
             keyuse->keypart == keypart;
           ++keyuse) {
        /*
          If this Key_use is not about a semi-join equality, or references an
          excluded table, or does not reference a not-yet-available table, or
          is for fulltext, or is over a prefix, then it is not a "handled sj
          equality".
        */
        if ((keyuse->sj_pred_no == UINT_MAX) ||
            (excluded_tables & keyuse->used_tables) ||
            !(remaining_tables & keyuse->used_tables) ||
            (keypart == FT_KEYPART) ||
            (table->key_info[key].key_part[keypart].key_part_flag &
             HA_PART_KEY_SEG))
          continue;
        handled_sj_equalities |= 1ULL << keyuse->sj_pred_no;
        handled_keyparts |= keyuse->keypart_map;
        DBUG_ASSERT(max_keypart <= keypart);  // see sort_keyuse()
        max_keypart = keypart;
      }
    }

    const key_part_map bound_keyparts = start_key->bound_keyparts;

    /*
      We can use semi-join LooseScan if duplicate elimination is going to work
      for all semi-join equalities. Duplicate elimination:
      - works for a bound semi-join equality, because this equality is tested
      before the nested loop leaves the last inner table of this semi-join
      nest.
      - works for a handled semi-join equality thanks to key comparison; key
      comparison works if:
        * the handled key parts are over a full field (not a prefix, otherwise
        two values, differing only after the prefix, would be treated as
        duplicates)
        * and any key part before the handled key parts, is bound (same
        justification as for "works for a bound semi-join equality" above).

      That gives us these requirements:
      1. All IN-equalities are either bound or handled.
      2. No hole in sequence of key parts.

      An example where (2) matters:
        SELECT * FROM ot1
        WHERE a IN (SELECT it1.b FROM it1 JOIN it2 ON it1.a = it2.a).
      Say the plan is it1-ot1-it2 and it1 has an index on (a,b). The semi-join
      equality is handled, by the second key part (it1.b). But the first key
      part is not bound (it2.a is not available). So there is a hole. If the
      rows of it1 are, in index order: (X,Z),(Y,Z), then the key comparison
      will let both rows pass; after joining with ot1 this will duplicate
      any row of ot1 having ot1.a=Z.

      We add this third requirement:
      3. At least one IN-equality is handled.
      In theory it is a superfluous restriction. Consider:
        select * from t2 as t3, t2
        where t2.b=t3.b and
              (t2.b) in (select b*3 from t1 where a=10);
      If the plan is t3-t1-t2, and we are looking at an index on t1.a:
      bound_sj_equalities==1 (because outer expression is equal to t3.b which
      is available), handled_sj_equalities==0 (no index on 'b*3'),
      handled_keyparts==0, bound_keyparts==1 (t1.a=10).
      We could set up 'ref' on t1.a (=10), with a "LooseScan key comparison
      length" (join_tab->loosescan_key_len) of size(t1.a), and a condition on
      t1 (t1->m_condition) of "t1.b*3=t3.b". After finding a match in t2
      (t2->m_condition="t2.b=t3.b"), the key comparison would skip all other
      rows of t1 returned by ref access. But this is a bit degenerate,
      FirstMatch-like.
    */
    if ((handled_sj_equalities | bound_sj_equalities) !=  // (1)
        LOWER_BITS(
            ulonglong,
            tab->emb_sj_nest->nested_join->sj_inner_exprs.elements))  // (1)
    {
      trace_idx.add("index_handles_needed_semijoin_equalities", false);
      continue;
    }
    if (handled_keyparts == 0)  // (3)
    {
      trace_idx.add("some_index_part_used", false);
      continue;
    }
    if ((LOWER_BITS(key_part_map, max_keypart + 1) &  // (2)
         ~(bound_keyparts | handled_keyparts)) != 0)  // (2)
    {
      trace_idx.add("index_can_remove_duplicates", false);
      continue;
    }

    // Ok, can use the strategy

    if (tab->quick() && tab->quick()->index == key &&
        tab->quick()->get_type() == QUICK_SELECT_I::QS_TYPE_RANGE) {
      quick_uses_applicable_index = true;
      quick_max_keypart = max_keypart;
    }

    if (bound_keyparts & 1) {
      Opt_trace_object trace_ref(trace, "ref");
      trace_ref.add("cost", start_key->read_cost);
      if (start_key->read_cost < pos->read_cost) {
        // @TODO use rec-per-key-based fanout calculations
        pos->loosescan_key = key;
        pos->read_cost = start_key->read_cost;
        pos->rows_fetched = start_key->fanout;
        pos->loosescan_parts = max_keypart + 1;
        pos->key = start_key;
        trace_ref.add("chosen", true);
      }
    } else if (tab->table()->covering_keys.is_set(key)) {
      /*
        There are no usable bound IN-equalities, e.g. we have

        outer_expr IN (SELECT innertbl.key FROM ...)

        and outer_expr cannot be evaluated yet, so it's actually full
        index scan and not a ref access
      */
      Opt_trace_object trace_cov_scan(trace, "covering_scan");

      // Calculate the cost of complete loose index scan.
      double rowcount = rows2double(tab->table()->file->stats.records);

      // The cost is entire index scan cost
      const double cost =
          tab->table()->file->index_scan_cost(key, 1, rowcount).total_cost();

      /*
        Now find out how many different keys we will get (for now we
        ignore the fact that we have "keypart_i=const" restriction for
        some key components, that may make us think that loose
        scan will produce more distinct records than it actually will)
      */
      if (tab->table()->key_info[key].has_records_per_key(max_keypart)) {
        const rec_per_key_t rpc =
            tab->table()->key_info[key].records_per_key(max_keypart);
        rowcount = rowcount / rpc;
      }

      trace_cov_scan.add("cost", cost);
      // @TODO: previous version also did /2
      if (cost < pos->read_cost) {
        pos->loosescan_key = key;
        pos->read_cost = cost;
        pos->rows_fetched = rowcount;
        pos->loosescan_parts = max_keypart + 1;
        pos->key = nullptr;
        trace_cov_scan.add("chosen", true);
      }
    } else
      trace_idx.add("ref_possible", false).add("covering_scan_possible", false);

  }  // ... for (Key_use *keyuse=tab->keyuse(); etc

  trace_all_idx.end();

  if (quick_uses_applicable_index && idx == join->const_tables) {
    Opt_trace_object trace_range(trace, "range_scan");
    trace_range.add("cost", tab->quick()->cost_est);
    // @TODO: this the right part restriction:
    if (tab->quick()->cost_est.total_cost() < pos->read_cost) {
      pos->loosescan_key = tab->quick()->index;
      pos->read_cost = tab->quick()->cost_est.total_cost();
      // this is ok because idx == join->const_tables
      pos->rows_fetched = rows2double(tab->quick()->records);
      pos->loosescan_parts = quick_max_keypart + 1;
      pos->key = nullptr;
      trace_range.add("chosen", true);
    }
  }

  if (pos->read_cost != DBL_MAX) {
    pos->read_cost += derived_mat_cost;
    pos->filter_effect = calculate_condition_filter(
        tab, pos->key, ~remaining_tables & ~excluded_tables, pos->rows_fetched,
        false, false, trace_ls);
    return true;
  }

  return false;

  // @todo need ref_depend_map ?
}

bool Join_tab_compare_default::operator()(const JOIN_TAB *jt1,
                                          const JOIN_TAB *jt2) const {
  // Sorting distinct tables, so a table should not be compared with itself
  DBUG_ASSERT(jt1 != jt2);

  if (jt1->dependent & jt2->table_ref->map()) return false;
  if (jt2->dependent & jt1->table_ref->map()) return true;

  const bool jt1_keydep_jt2 = jt1->key_dependent & jt2->table_ref->map();
  const bool jt2_keydep_jt1 = jt2->key_dependent & jt1->table_ref->map();

  if (jt1_keydep_jt2 && !jt2_keydep_jt1) return false;
  if (jt2_keydep_jt1 && !jt1_keydep_jt2) return true;

  if (jt1->found_records > jt2->found_records) return false;
  if (jt1->found_records < jt2->found_records) return true;

  return jt1 < jt2;
}

namespace {

/**
  "Less than" comparison function object used to compare two JOIN_TAB
  objects that are joined using STRAIGHT JOIN. For STRAIGHT JOINs,
  the join order is dictated by the relative order of the tables in the
  query which is reflected in JOIN_TAB::dependent. Table size and key
  dependencies are ignored here.
*/
class Join_tab_compare_straight {
 public:
  bool operator()(const JOIN_TAB *jt1, const JOIN_TAB *jt2) const {
    // Sorting distinct tables, so a table should not be compared with itself
    DBUG_ASSERT(jt1 != jt2);

    /*
      We don't do subquery flattening if the parent or child select has
      STRAIGHT_JOIN modifier. It is complicated to implement and the semantics
      is hardly useful.
    */
    DBUG_ASSERT(!jt1->emb_sj_nest);
    DBUG_ASSERT(!jt2->emb_sj_nest);

    if (jt1->dependent & jt2->table_ref->map()) return false;
    if (jt2->dependent & jt1->table_ref->map()) return true;

    return jt1 < jt2;
  }
};

/*
  Same as Join_tab_compare_default but tables from within the given
  semi-join nest go first. Used when optimizing semi-join
  materialization nests.
*/
class Join_tab_compare_embedded_first {
 private:
  const TABLE_LIST *emb_nest;

 public:
  explicit Join_tab_compare_embedded_first(const TABLE_LIST *nest)
      : emb_nest(nest) {}

  bool operator()(const JOIN_TAB *jt1, const JOIN_TAB *jt2) const {
    // Sorting distinct tables, so a table should not be compared with itself
    DBUG_ASSERT(jt1 != jt2);

    if (jt1->emb_sj_nest == emb_nest && jt2->emb_sj_nest != emb_nest)
      return true;
    if (jt1->emb_sj_nest != emb_nest && jt2->emb_sj_nest == emb_nest)
      return false;

    Join_tab_compare_default cmp;
    return cmp(jt1, jt2);
  }
};

}  // namespace

/**
  Selects and invokes a search strategy for an optimal query join order.

  The function checks user-configurable parameters that control the search
  strategy for an optimal plan, selects the search method and then invokes
  it. Each specific optimization procedure stores the final optimal plan in
  the array 'join->best_positions', and the cost of the plan in
  'join->best_read'.
  The function can be invoked to produce a plan for all tables in the query
  (in this case, the const tables are usually filtered out), or it can be
  invoked to produce a plan for a materialization of a semijoin nest.
  Set a non-NULL emb_sjm_nest pointer when producing a plan for a semijoin
  nest to be materialized and a NULL pointer when producing a full query plan.

  @return false if successful, true if error
*/

bool Optimize_table_order::choose_table_order() {
  DBUG_TRACE;

  got_final_plan = false;

  // Make consistent prefix cost estimates also for the const tables:
  for (uint i = 0; i < join->const_tables; i++)
    (join->positions + i)->set_prefix_cost(0.0, 1.0);

  /* Are there any tables to optimize? */
  if (join->const_tables == join->tables) {
    memcpy(join->best_positions, join->positions,
           sizeof(POSITION) * join->const_tables);
    join->best_read = 1.0;
    join->best_rowcount = 1;
    got_final_plan = true;
    return false;
  }

  join->select_lex->reset_nj_counters();

  const bool straight_join =
      join->select_lex->active_options() & SELECT_STRAIGHT_JOIN;
  table_map join_tables;  ///< The tables involved in order selection

  if (emb_sjm_nest) {
    /* We're optimizing semi-join materialization nest, so put the
       tables from this semi-join as first
    */
    merge_sort(join->best_ref + join->const_tables,
               join->best_ref + join->tables,
               Join_tab_compare_embedded_first(emb_sjm_nest));
    join_tables = emb_sjm_nest->sj_inner_tables;
  } else {
    /*
      if (SELECT_STRAIGHT_JOIN option is set)
        reorder tables so dependent tables come after tables they depend
        on, otherwise keep tables in the order they were specified in the query
      else
        Apply heuristic: pre-sort all access plans with respect to the number of
        records accessed.
    */
    if (straight_join)
      merge_sort(join->best_ref + join->const_tables,
                 join->best_ref + join->tables, Join_tab_compare_straight());
    else
      merge_sort(join->best_ref + join->const_tables,
                 join->best_ref + join->tables, Join_tab_compare_default());

    join_tables = join->all_table_map & ~join->const_table_map;
  }

  Opt_trace_object wrapper(&join->thd->opt_trace);
  Opt_trace_array trace_plan(&join->thd->opt_trace,
                             "considered_execution_plans",
                             Opt_trace_context::GREEDY_SEARCH);

  if (thd->optimizer_switch_flag(OPTIMIZER_SWITCH_COND_FANOUT_FILTER) &&
      join->where_cond) {
    for (uint idx = join->const_tables; idx < join->tables; ++idx)
      bitmap_clear_all(&join->best_ref[idx]->table()->cond_set);

    /*
      Set column bits for all columns involved in predicates in
      cond_set. Used to avoid calculating condition filtering in
      best_access_path() et al. when no filtering effect is possible.
    */
    join->where_cond->walk(&Item::add_field_to_cond_set_processor,
                           enum_walk::POSTFIX, nullptr);
  }

  Deps_of_remaining_lateral_derived_tables deps_lateral(join, ~excluded_tables);
  deps_lateral.init();

  if (straight_join)
    optimize_straight_join(join_tables);
  else {
    if (greedy_search(join_tables)) return true;
  }

  deps_lateral.assert_unchanged();

  got_final_plan = true;

  // Remaining part of this function not needed when processing semi-join nests.
  if (emb_sjm_nest) return false;

  // Fix semi-join strategies and perform final cost calculation.
  if (fix_semijoin_strategies()) return true;

  return false;
}

/**
  Heuristic procedure to automatically guess a reasonable degree of
  exhaustiveness for the greedy search procedure.

  The procedure estimates the optimization time and selects a search depth
  big enough to result in a near-optimal QEP, that doesn't take too long to
  find. If the number of tables in the query exceeds some constant, then
  search_depth is set to this constant.

  @param search_depth Search depth value specified.
                      If zero, calculate a default value.
  @param table_count  Number of tables to be optimized (excludes const tables)

  @note
    This is an extremely simplistic implementation that serves as a stub for a
    more advanced analysis of the join. Ideally the search depth should be
    determined by learning from previous query optimizations, because it will
    depend on the CPU power (and other factors).

  @todo
    this value should be determined dynamically, based on statistics:
    uint max_tables_for_exhaustive_opt= 7;

  @todo
    this value could be determined by some mapping of the form:
    depth : table_count -> [max_tables_for_exhaustive_opt..MAX_EXHAUSTIVE]

  @return
    A positive integer that specifies the search depth (and thus the
    exhaustiveness) of the depth-first search algorithm used by
    'greedy_search'.
*/

uint Optimize_table_order::determine_search_depth(uint search_depth,
                                                  uint table_count) {
  if (search_depth > 0) return search_depth;
  /* TODO: this value should be determined dynamically, based on statistics: */
  const uint max_tables_for_exhaustive_opt = 7;

  if (table_count <= max_tables_for_exhaustive_opt)
    search_depth =
        table_count + 1;  // use exhaustive for small number of tables
  else
    /*
      TODO: this value could be determined by some mapping of the form:
      depth : table_count -> [max_tables_for_exhaustive_opt..MAX_EXHAUSTIVE]
    */
    search_depth = max_tables_for_exhaustive_opt;  // use greedy search

  return search_depth;
}

/**
  Select the best ways to access the tables in a query without reordering them.

    Find the best access paths for each query table and compute their costs
    according to their order in the array 'join->best_ref' (thus without
    reordering the join tables). The function calls sequentially
    'best_access_path' for each table in the query to select the best table
    access method. The final optimal plan is stored in the array
    'join->best_positions', and the corresponding cost in 'join->best_read'.

  @param join_tables   set of the tables in the query

  @note
    This function can be applied to:
    - queries with STRAIGHT_JOIN
    - internally to compute the cost of an arbitrary QEP
  @par
    Thus 'optimize_straight_join' can be used at any stage of the query
    optimization process to finalize a QEP as it is.
*/

void Optimize_table_order::optimize_straight_join(table_map join_tables) {
  uint idx = join->const_tables;
  double rowcount = 1.0;
  double cost = 0.0;
  const Cost_model_server *const cost_model = join->cost_model();

  // resolve_subquery() disables semijoin if STRAIGHT_JOIN
  DBUG_ASSERT(join->select_lex->sj_nests.empty());

  Deps_of_remaining_lateral_derived_tables deps_lateral(join, ~excluded_tables);

  Opt_trace_context *const trace = &join->thd->opt_trace;
  for (JOIN_TAB **pos = join->best_ref + idx; *pos; idx++, pos++) {
    JOIN_TAB *const s = *pos;
    POSITION *const position = join->positions + idx;
    Opt_trace_object trace_table(trace);
    if (unlikely(trace->is_started())) {
      trace_plan_prefix(join, idx, excluded_tables);
      trace_table.add_utf8_table(s->table_ref);
    }
    /*
      Dependency computation (JOIN::make_join_plan()) and proper ordering
      based on them (join_tab_cmp*) guarantee that this order is compatible
      with execution, check it:
    */
    DBUG_ASSERT(!check_interleaving_with_nj(s));

    /* Find the best access method from 's' to the current partial plan */
    best_access_path(s, join_tables, idx, false, rowcount, position);

    // compute the cost of the new plan extended with 's'
    position->set_prefix_join_cost(idx, cost_model);

    position->no_semijoin();  // advance_sj_state() is not needed

    rowcount = position->prefix_rowcount;
    cost = position->prefix_cost;

    trace_table.add("condition_filtering_pct", position->filter_effect * 100)
        .add("rows_for_plan", rowcount)
        .add("cost_for_plan", cost);
    join_tables &= ~(s->table_ref->map());

    deps_lateral.recalculate(s, idx + 1);
  }

  if (join->sort_by_table &&
      join->sort_by_table != join->positions[join->const_tables].table->table())
    cost += rowcount;  // We have to make a temp table

  memcpy(join->best_positions, join->positions, sizeof(POSITION) * idx);

  /**
   * If many plans have identical cost, which one will be used
   * depends on how compiler optimizes floating-point calculations.
   * this fix adds repeatability to the optimizer.
   * (Similar code in best_extension_by_li...)
   */
  join->best_read = cost - 0.001;
  join->best_rowcount = (ha_rows)rowcount;
}

/**
  Check whether a semijoin materialization strategy is allowed for
  the current (semi)join table order.

  @param join              Join object
  @param remaining_tables  Tables that have not yet been added to the join plan
  @param tab               Join_tab of the table being considered
  @param idx               Index in join->position[] with Join_tab "tab"

  @retval SJ_OPT_NONE               - Materialization not applicable
  @retval SJ_OPT_MATERIALIZE_LOOKUP - Materialization with lookup applicable
  @retval SJ_OPT_MATERIALIZE_SCAN   - Materialization with scan applicable

  @details
  The function checks applicability of both MaterializeLookup and
  MaterializeScan strategies.
  No checking is made until "tab" is pointing to the last inner table
  of a semijoin nest that can be executed using materialization -
  for all other cases SJ_OPT_NONE is returned.

  MaterializeLookup and MaterializeScan are both applicable in the following
  two cases:

   1. There are no correlated outer tables, or
   2. There are correlated outer tables within the prefix only.

  In this case, MaterializeLookup is returned based on a heuristic decision.
*/

static int semijoin_order_allows_materialization(const JOIN *join,
                                                 table_map remaining_tables,
                                                 const JOIN_TAB *tab,
                                                 uint idx) {
  DBUG_ASSERT(!(remaining_tables & tab->table_ref->map()));
  /*
   Check if
    1. We're in a semi-join nest that can be run with SJ-materialization
    2. All the tables from the subquery are in the prefix
  */
  const TABLE_LIST *emb_sj_nest = tab->emb_sj_nest;
  if (!emb_sj_nest || !emb_sj_nest->nested_join->sjm.positions ||
      (remaining_tables & emb_sj_nest->sj_inner_tables))
    return SJ_OPT_NONE;

  /*
    Walk back and check if all immediately preceding tables are from
    this semi-join.
  */
  const uint n_tables = my_count_bits(emb_sj_nest->sj_inner_tables);
  for (uint i = 1; i < n_tables; i++) {
    if (join->positions[idx - i].table->emb_sj_nest != emb_sj_nest)
      return SJ_OPT_NONE;
  }

  /*
    Must use MaterializeScan strategy if there are outer correlated tables
    among the remaining tables, otherwise, if possible, use MaterializeLookup.
  */
  if ((remaining_tables & emb_sj_nest->nested_join->sj_depends_on) ||
      !emb_sj_nest->nested_join->sjm.lookup_allowed) {
    if (emb_sj_nest->nested_join->sjm.scan_allowed)
      return SJ_OPT_MATERIALIZE_SCAN;
    return SJ_OPT_NONE;
  }
  return SJ_OPT_MATERIALIZE_LOOKUP;
}

/**
  Find a good, possibly optimal, query execution plan (QEP) by a greedy search.

    The search procedure uses a hybrid greedy/exhaustive search with controlled
    exhaustiveness. The search is performed in N = card(remaining_tables)
    steps. Each step evaluates how promising is each of the unoptimized tables,
    selects the most promising table, and extends the current partial QEP with
    that table.  Currenly the most 'promising' table is the one with least
    expensive extension.\

    There are two extreme cases:
    -# When (card(remaining_tables) < search_depth), the estimate finds the
    best complete continuation of the partial QEP. This continuation can be
    used directly as a result of the search.
    -# When (search_depth == 1) the 'best_extension_by_limited_search'
    consideres the extension of the current QEP with each of the remaining
    unoptimized tables.

    All other cases are in-between these two extremes. Thus the parameter
    'search_depth' controlls the exhaustiveness of the search. The higher the
    value, the longer the optimizaton time and possibly the better the
    resulting plan. The lower the value, the fewer alternative plans are
    estimated, but the more likely to get a bad QEP.

    All intermediate and final results of the procedure are stored in 'join':
    - join->positions     : modified for every partial QEP that is explored
    - join->best_positions: modified for the current best complete QEP
    - join->best_read     : modified for the current best complete QEP
    - join->best_ref      : might be partially reordered

    The final optimal plan is stored in 'join->best_positions', and its
    corresponding cost in 'join->best_read'.

  @note
    The following pseudocode describes the algorithm of 'greedy_search':

    @code
    procedure greedy_search
    input: remaining_tables
    output: pplan;
    {
      pplan = <>;
      do {
        (t, a) = best_extension(pplan, remaining_tables);
        pplan = concat(pplan, (t, a));
        remaining_tables = remaining_tables - t;
      } while (remaining_tables != {})
      return pplan;
    }

  @endcode
    where 'best_extension' is a placeholder for a procedure that selects the
    most "promising" of all tables in 'remaining_tables'.
    Currently this estimate is performed by calling
    'best_extension_by_limited_search' to evaluate all extensions of the
    current QEP of size 'search_depth', thus the complexity of 'greedy_search'
    mainly depends on that of 'best_extension_by_limited_search'.

  @par
    If 'best_extension()' == 'best_extension_by_limited_search()', then the
    worst-case complexity of this algorithm is <=
    O(N*N^search_depth/search_depth). When serch_depth >= N, then the
    complexity of greedy_search is O(N!).
    'N' is the number of 'non eq_ref' tables + 'eq_ref groups' which normally
    are considerable less than total numbers of tables in the query.

  @par
    In the future, 'greedy_search' might be extended to support other
    implementations of 'best_extension'.

  @par
    @c search_depth from Optimize_table_order controls the exhaustiveness
    of the search, and @c prune_level controls the pruning heuristics that
    should be applied during search.

  @param remaining_tables set of tables not included into the partial plan yet

  @return false if successful, true if error
*/

bool Optimize_table_order::greedy_search(table_map remaining_tables) {
  uint idx = join->const_tables;  // index into 'join->best_ref'
  uint best_idx;
  POSITION best_pos;
  JOIN_TAB *best_table;  // the next plan node to be added to the curr QEP
  DBUG_TRACE;

  /* Number of tables that we are optimizing */
  const uint n_tables = my_count_bits(remaining_tables);

  /* Number of tables remaining to be optimized */
  uint size_remain = n_tables;

  do {
    /* Find the extension of the current QEP with the lowest cost */
    join->best_read = DBL_MAX;
    join->best_rowcount = HA_POS_ERROR;
    found_plan_with_allowed_sj = false;
    if (best_extension_by_limited_search(remaining_tables, idx, search_depth))
      return true;
    /*
      'best_read < DBL_MAX' means that optimizer managed to find
      some plan and updated 'best_positions' array accordingly.
    */
    DBUG_ASSERT(join->best_read < DBL_MAX);

    if (size_remain <= search_depth) {
      /*
        'join->best_positions' contains a complete optimal extension of the
        current partial QEP.
      */
      DBUG_EXECUTE(
          "opt",
          print_plan(join, n_tables,
                     idx ? join->best_positions[idx - 1].prefix_rowcount : 1.0,
                     idx ? join->best_positions[idx - 1].prefix_cost : 0.0,
                     idx ? join->best_positions[idx - 1].prefix_cost : 0.0,
                     "optimal"););
      return false;
    }

    /* select the first table in the optimal extension as most promising */
    best_pos = join->best_positions[idx];
    best_table = best_pos.table;
    /*
      Each subsequent loop of 'best_extension_by_limited_search' uses
      'join->positions' for cost estimates, therefore we have to update its
      value.
    */
    join->positions[idx] = best_pos;

    /*
      Search depth is smaller than the number of remaining tables to join.
      - Update the interleaving state after extending the current partial plan
      with a new table. We are doing this here because
      best_extension_by_limited_search reverts the interleaving state to the
      one of the non-extended partial plan on exit.
      - The semi join state is entirely in POSITION, so it is transferred fine
      when we copy POSITION objects (no special handling needed).
      - After we have chosen the final plan covering all tables, the nested
      join state will not be reverted back to its initial state because we
      don't "pop" tables already present in the partial plan.
    */
    bool is_interleave_error MY_ATTRIBUTE((unused)) =
        check_interleaving_with_nj(best_table);
    /* This has been already checked by best_extension_by_limited_search */
    DBUG_ASSERT(!is_interleave_error);

    /* find the position of 'best_table' in 'join->best_ref' */
    best_idx = idx;
    JOIN_TAB *pos = join->best_ref[best_idx];
    while (pos && best_table != pos) pos = join->best_ref[++best_idx];
    DBUG_ASSERT((pos != nullptr));  // should always find 'best_table'
    /*
      Maintain '#rows-sorted' order of 'best_ref[]':
       - Shift 'best_ref[]' to make first position free.
       - Insert 'best_table' at the first free position in the array of joins.
    */
    memmove(join->best_ref + idx + 1, join->best_ref + idx,
            sizeof(JOIN_TAB *) * (best_idx - idx));
    join->best_ref[idx] = best_table;

    remaining_tables &= ~(best_table->table_ref->map());

    DBUG_EXECUTE("opt",
                 print_plan(join, idx, join->positions[idx].prefix_rowcount,
                            join->positions[idx].prefix_cost,
                            join->positions[idx].prefix_cost, "extended"););
    --size_remain;
    ++idx;
  } while (true);
}

/**
  Calculate a cost of given partial join order

  @param join               Join to use. @c positions holds the partial join
  order
  @param n_tables           Number of tables in the partial join order
  @param [out] cost_arg     Store read time here
  @param [out] rowcount_arg Store record count here

    This is needed for semi-join materialization code. The idea is that
    we detect sj-materialization after we've put all sj-inner tables into
    the join prefix

      prefix-tables semi-join-inner-tables  tN
                                             ^--we're here

    and we'll need to get the cost of prefix-tables prefix again.
*/

void get_partial_join_cost(JOIN *join, uint n_tables, double *cost_arg,
                           double *rowcount_arg) {
  double rowcount = 1.0;
  double cost = 0.0;
  const Cost_model_server *const cost_model = join->cost_model();

  for (uint i = join->const_tables; i < n_tables + join->const_tables; i++) {
    POSITION *const pos = join->best_positions + i;

    if (pos->rows_fetched > 0.0) {
      rowcount *= pos->rows_fetched;
      cost += pos->read_cost + cost_model->row_evaluate_cost(rowcount);
      rowcount *= pos->filter_effect;
    }
  }
  *cost_arg = cost;
  *rowcount_arg = rowcount;
}

/**
  Returns the handlerton of the secondary engine that will execute the current
  statement, or nullptr if a secondary engine is not used.
*/
static const handlerton *secondary_engine_handlerton(const THD *thd) {
  const Sql_cmd *sql_cmd = thd->lex->m_sql_cmd;
  if (sql_cmd == nullptr) return nullptr;
  return sql_cmd->secondary_engine();
}

/**
  Cost calculation of another (partial-)QEP has been completed.

  If this is our 'best' plan explored so far, we record this
  query plan and its cost.

  @param idx        length of the partial QEP in 'join->positions';
                    also corresponds to the current depth of the search tree;
                    also an index in the array 'join->best_ref';
  @param trace_obj  trace object where information is to be added

  @return false if successful, true if error
*/
bool Optimize_table_order::consider_plan(uint idx,
                                         Opt_trace_object *trace_obj) {
  double cost = join->positions[idx].prefix_cost;
  double sort_cost = 0;
  double windowing_cost = 0;
  /*
    We may have to make a temp table, note that this is only a
    heuristic since we cannot know for sure at this point.
    Hence it may be too pessimistic.

    @todo Windowing that uses sorting may force a sort cost both prior
    to windowing (i.e. GROUP BY) and after (i.e. ORDER BY or DISTINCT).
    In such cases we should add the cost twice here, but currently this is
    tweaked in Explain_join::shallow_explain. If would be preferable to do it
    here.
  */
  if (join->sort_by_table &&
      join->sort_by_table !=
          join->positions[join->const_tables].table->table()) {
    sort_cost = join->positions[idx].prefix_rowcount;
    cost += sort_cost;
    trace_obj->add("sort_cost", sort_cost).add("new_cost_for_plan", cost);
  }

  /*
    Check if the plan uses a disabled strategy.  (This may happen if this join
    order does not support any of the enabled strategies.)  Currently
    DuplicateWeedout is the only strategy for which this may happen.
    If we have found a previous plan with only allowed strategies,
    we only choose the current plan if it is both cheaper and does not use
    disabled strategies.  If all previous plans use a disabled strategy,
    we choose the current plan if it is either cheaper or does not use a
    disabled strategy.
  */
  bool plan_uses_allowed_sj = true;
  if (has_sj)
    for (uint i = join->const_tables; i <= idx && plan_uses_allowed_sj; i++)
      if (join->positions[i].sj_strategy == SJ_OPT_DUPS_WEEDOUT) {
        uint first = join->positions[i].first_dupsweedout_table;
        for (uint j = first; j <= i; j++) {
          TABLE_LIST *emb_sj_nest = join->positions[j].table->emb_sj_nest;
          if (emb_sj_nest && !(emb_sj_nest->nested_join->sj_enabled_strategies &
                               OPTIMIZER_SWITCH_DUPSWEEDOUT))
            plan_uses_allowed_sj = false;
        }
      }

  bool cheaper = cost < join->best_read;
  bool chosen = found_plan_with_allowed_sj ? (plan_uses_allowed_sj && cheaper)
                                           : (plan_uses_allowed_sj || cheaper);

  /*
    If the statement is executed on a secondary engine, and the secondary engine
    has implemented a custom cost comparison function, ask the secondary engine
    to compare the cost. The secondary engine is only consulted when a complete
    join order is considered.
  */
  if (idx + 1 == join->tables) {  // this is a complete join order
    const handlerton *secondary_engine = secondary_engine_handlerton(thd);
    if (secondary_engine != nullptr &&
        secondary_engine->compare_secondary_engine_cost != nullptr) {
      double secondary_engine_cost;
      if (secondary_engine->compare_secondary_engine_cost(
              thd, *join, Candidate_table_order(join), cost, &cheaper,
              &secondary_engine_cost))
        return true;
      chosen = cheaper;
      trace_obj->add("secondary_engine_cost", secondary_engine_cost);

      // If this is the first plan seen, it must be chosen.
      DBUG_ASSERT(join->best_read != DBL_MAX || chosen);
    }
  }

  trace_obj->add("chosen", chosen);
  if (chosen) {
    if (!cheaper)
      trace_obj->add_alnum("cause", "previous_plan_used_disabled_strategy");

    memcpy((uchar *)join->best_positions, (uchar *)join->positions,
           sizeof(POSITION) * (idx + 1));

    if (join->m_windows_sort) {
      windowing_cost = Window::compute_cost(
          join->positions[idx].prefix_rowcount, join->m_windows);
      cost += windowing_cost;
      trace_obj->add("windowing_sort_cost", windowing_cost)
          .add("new_cost_for_plan", cost);
    }

    /*
      If many plans have identical cost, which one will be used
      depends on how compiler optimizes floating-point calculations.
      this fix adds repeatability to the optimizer.
      (Similar code in best_extension_by_li...)
    */
    join->best_read = cost - 0.001;
    join->best_rowcount =
        join->positions[idx].prefix_rowcount >=
                static_cast<double>(std::numeric_limits<ha_rows>::max())
            ? std::numeric_limits<ha_rows>::max()
            : (ha_rows)join->positions[idx].prefix_rowcount;
    join->sort_cost = sort_cost;
    join->windowing_cost = windowing_cost;
    found_plan_with_allowed_sj = plan_uses_allowed_sj;
  } else if (cheaper)
    trace_obj->add_alnum("cause", "plan_uses_disabled_strategy");

  DBUG_EXECUTE("opt",
               print_plan(join, idx + 1, join->positions[idx].prefix_rowcount,
                          cost, cost, "full_plan"););

  return false;
}

/**
  Find a good, possibly optimal, query execution plan (QEP) by a possibly
  exhaustive search.

    The procedure searches for the optimal ordering of the query tables in set
    'remaining_tables' of size N, and the corresponding optimal access paths to
    each table. The choice of a table order and an access path for each table
    constitutes a query execution plan (QEP) that fully specifies how to
    execute the query.

    The maximal size of the found plan is controlled by the parameter
    'search_depth'. When search_depth == N, the resulting plan is complete and
    can be used directly as a QEP. If search_depth < N, the found plan consists
    of only some of the query tables. Such "partial" optimal plans are useful
    only as input to query optimization procedures, and cannot be used directly
    to execute a query.

    The algorithm begins with an empty partial plan stored in 'join->positions'
    and a set of N tables - 'remaining_tables'. Each step of the algorithm
    evaluates the cost of the partial plan extended by all access plans for
    each of the relations in 'remaining_tables', expands the current partial
    plan with the access plan that results in lowest cost of the expanded
    partial plan, and removes the corresponding relation from
    'remaining_tables'. The algorithm continues until it either constructs a
    complete optimal plan, or constructs an optimal plartial plan with size =
    search_depth.

    The final optimal plan is stored in 'join->best_positions'. The
    corresponding cost of the optimal plan is in 'join->best_read'.

  @note
    The procedure uses a recursive depth-first search where the depth of the
    recursion (and thus the exhaustiveness of the search) is controlled by the
    parameter 'search_depth'.

  @note
    The pseudocode below describes the algorithm of
    'best_extension_by_limited_search'. The worst-case complexity of this
    algorithm is O(N*N^search_depth/search_depth). When serch_depth >= N, then
    the complexity of greedy_search is O(N!).

  @note
    @c best_extension_by_limited_search() and @c
  eq_ref_extension_by_limited_search() are closely related to each other and
  intentionally implemented using the same pattern wherever possible. If a
  change/bug fix is done to either of these also consider if it is relevant for
  the other.

    @code
    procedure best_extension_by_limited_search(
      pplan in,             // in, partial plan of tables-joined-so-far
      pplan_cost,           // in, cost of pplan
      remaining_tables,     // in, set of tables not referenced in pplan
      best_plan_so_far,     // in/out, best plan found so far
      best_plan_so_far_cost,// in/out, cost of best_plan_so_far
      search_depth)         // in, maximum size of the plans being considered
    {
      for each table T from remaining_tables
      {
        // Calculate the cost of using table T as above
        cost = complex-series-of-calculations;

        // Add the cost to the cost so far.
        pplan_cost+= cost;

        if (pplan_cost >= best_plan_so_far_cost)
          // pplan_cost already too great, stop search
          continue;

        pplan= expand pplan by best_access_method;
        remaining_tables= remaining_tables - table T;
        if (remaining_tables is not an empty set
            and
            search_depth > 1)
        {
          if (table T is EQ_REF-joined)
            eq_ref_eq_ref_extension_by_limited_search(
                                             pplan, pplan_cost,
                                             remaining_tables,
                                             best_plan_so_far,
                                             best_plan_so_far_cost,
                                             search_depth - 1);

          else
            best_extension_by_limited_search(pplan, pplan_cost,
                                             remaining_tables,
                                             best_plan_so_far,
                                             best_plan_so_far_cost,
                                             search_depth - 1);
        }
        else
        {
          best_plan_so_far_cost= pplan_cost;
          best_plan_so_far= pplan;
        }
      }
    }
    @endcode

  @note
    The arguments pplan, plan_cost, best_plan_so_far and best_plan_so_far_cost
    are actually found in the POSITION object.

  @note
    When 'best_extension_by_limited_search' is called for the first time,
    'join->best_read' must be set to the largest possible value (e.g. DBL_MAX).
    The actual implementation provides a way to optionally use pruning
    heuristic (controlled by the parameter 'prune_level') to reduce the search
    space by skipping some partial plans.

  @note
    The parameter 'search_depth' provides control over the recursion
    depth, and thus the size of the resulting optimal plan.

  @param remaining_tables set of tables not included into the partial plan yet
  @param idx              length of the partial QEP in 'join->positions';
                          since a depth-first search is used, also corresponds
                          to the current depth of the search tree;
                          also an index in the array 'join->best_ref';
  @param current_search_depth  maximum depth of recursion and thus size of the
                          found optimal plan
                          (0 < current_search_depth <= join->tables+1).

  @return false if successful, true if error
*/

bool Optimize_table_order::best_extension_by_limited_search(
    table_map remaining_tables, uint idx, uint current_search_depth) {
  DBUG_TRACE;

  DBUG_EXECUTE_IF("bug13820776_2", thd->killed = THD::KILL_QUERY;);
  if (thd->killed)  // Abort
    return true;

  const Cost_model_server *const cost_model = join->cost_model();
  Opt_trace_context *const trace = &thd->opt_trace;

  /*
     'join' is a partial plan with lower cost than the best plan so far,
     so continue expanding it further with the tables in 'remaining_tables'.
  */
  double best_rowcount = DBL_MAX;
  double best_cost = DBL_MAX;

  DBUG_EXECUTE("opt",
               print_plan(join, idx,
                          idx ? join->positions[idx - 1].prefix_rowcount : 1.0,
                          idx ? join->positions[idx - 1].prefix_cost : 0.0,
                          idx ? join->positions[idx - 1].prefix_cost : 0.0,
                          "part_plan"););

  /*
    'eq_ref_extended' are the 'remaining_tables' which has already been
    involved in an partial query plan extension if this QEP. These
    will not be considered in further EQ_REF extensions based
    on current (partial) QEP.
  */
  table_map eq_ref_extended(0);

  JOIN_TAB *saved_refs[MAX_TABLES];
  // Save 'best_ref[]' as we has to restore before return.
  memcpy(saved_refs, join->best_ref + idx,
         sizeof(JOIN_TAB *) * (join->tables - idx));

  Deps_of_remaining_lateral_derived_tables deps_lateral(join, ~excluded_tables);

  for (JOIN_TAB **pos = join->best_ref + idx; *pos; pos++) {
    JOIN_TAB *const s = *pos;
    const table_map real_table_bit = s->table_ref->map();

    /*
      Don't move swap inside conditional code: All items should
      be uncond. swapped to maintain '#rows-ordered' best_ref[].
      This is critical for early pruning of bad plans.
    */
    std::swap(join->best_ref[idx], *pos);

    if ((remaining_tables & real_table_bit) &&
        !(eq_ref_extended & real_table_bit) &&
        !(remaining_tables & s->dependent) &&
        (!idx || !check_interleaving_with_nj(s))) {
      Opt_trace_object trace_one_table(trace);
      if (unlikely(trace->is_started())) {
        trace_plan_prefix(join, idx, excluded_tables);
        trace_one_table.add_utf8_table(s->table_ref);
      }
      POSITION *const position = join->positions + idx;

      // If optimizing a sj-mat nest, tables in this plan must be in nest:
      DBUG_ASSERT(emb_sjm_nest == nullptr || emb_sjm_nest == s->emb_sj_nest);

      deps_lateral.restore();  // as we "popped" the previously-tried table

      /* Find the best access method from 's' to the current partial plan */
      best_access_path(s, remaining_tables, idx, false,
                       idx ? (position - 1)->prefix_rowcount : 1.0, position);

      // Compute the cost of extending the plan with 's'
      position->set_prefix_join_cost(idx, cost_model);

      trace_one_table
          .add("condition_filtering_pct", position->filter_effect * 100)
          .add("rows_for_plan", position->prefix_rowcount)
          .add("cost_for_plan", position->prefix_cost);

      if (has_sj) {
        /*
          Even if there are no semijoins, advance_sj_state() has a significant
          cost (takes 9% of time in a 20-table plan search), hence the if()
          above, which is also more efficient than the same if() inside
          advance_sj_state() would be.
          Besides, never call advance_sj_state() when calculating the plan
          for a materialized semi-join nest.
        */
        advance_sj_state(remaining_tables, s, idx);
      } else
        position->no_semijoin();

      /*
        Expand only partial plans with lower cost than the best QEP so far.
        However, if the best plan so far uses a disabled semi-join strategy,
        we continue the search since this partial plan may support other
        semi-join strategies.
      */
      if (position->prefix_cost >= join->best_read &&
          found_plan_with_allowed_sj) {
        DBUG_EXECUTE("opt",
                     print_plan(join, idx + 1, position->prefix_rowcount,
                                position->read_cost, position->prefix_cost,
                                "prune_by_cost"););
        trace_one_table.add("pruned_by_cost", true);
        backout_nj_state(remaining_tables, s);
        continue;
      }

      /*
        Prune some less promising partial plans. This heuristic may miss
        the optimal QEPs, thus it results in a non-exhaustive search.
      */
      if (prune_level == 1) {
        if (best_rowcount > position->prefix_rowcount ||
            best_cost > position->prefix_cost ||
            (idx == join->const_tables &&  // 's' is the first table in the QEP
             s->table() == join->sort_by_table)) {
          if (best_rowcount >= position->prefix_rowcount &&
              best_cost >= position->prefix_cost &&
              /* TODO: What is the reasoning behind this condition? */
              (!(s->key_dependent & remaining_tables) ||
               position->rows_fetched < 2.0)) {
            best_rowcount = position->prefix_rowcount;
            best_cost = position->prefix_cost;
          }
        } else if (found_plan_with_allowed_sj) {
          DBUG_EXECUTE("opt",
                       print_plan(join, idx + 1, position->prefix_rowcount,
                                  position->read_cost, position->prefix_cost,
                                  "pruned_by_heuristic"););
          trace_one_table.add("pruned_by_heuristic", true);
          backout_nj_state(remaining_tables, s);
          continue;
        }
      }

      deps_lateral.recalculate(s, idx + 1);

      const table_map remaining_tables_after =
          (remaining_tables & ~real_table_bit);
      if ((current_search_depth > 1) && remaining_tables_after) {
        /*
          Explore more extensions of plan:
          If possible, use heuristic to avoid a full expansion of partial QEP.
          Evaluate a simplified EQ_REF extension of QEP if:
            1) Pruning is enabled.
            2) and, There are tables joined by (EQ_)REF key.
            3) and, There is a 1::1 relation between those tables
        */
        if (prune_level == 1 &&             // 1)
            position->key != nullptr &&     // 2)
            position->rows_fetched <= 1.0)  // 3)
        {
          /*
            Join in this 'position' is an EQ_REF-joined table, append more
            EQ_REFs. We do this only for the first EQ_REF we encounter which
            will then include other EQ_REFs from 'remaining_tables' and inform
            about which tables was 'eq_ref_extended'. These are later 'pruned'
            as they was processed here.
          */
          if (eq_ref_extended == (table_map)0) {
            /* Try an EQ_REF-joined expansion of the partial plan */
            Opt_trace_array trace_rest(trace, "rest_of_plan");
            eq_ref_extended =
                real_table_bit |
                eq_ref_extension_by_limited_search(
                    remaining_tables_after, idx + 1, current_search_depth - 1);
            if (eq_ref_extended == ~(table_map)0) return true;  // Failed

            backout_nj_state(remaining_tables, s);

            if (eq_ref_extended == remaining_tables) goto done;

            continue;
          } else  // Skip, as described above
          {
            DBUG_EXECUTE("opt",
                         print_plan(join, idx + 1, position->prefix_rowcount,
                                    position->read_cost, position->prefix_cost,
                                    "pruned_by_eq_ref_heuristic"););
            trace_one_table.add("pruned_by_eq_ref_heuristic", true);
            backout_nj_state(remaining_tables, s);
            continue;
          }
        }  // if (prunable...)

        /* Fallthrough: Explore more best extensions of plan */
        Opt_trace_array trace_rest(trace, "rest_of_plan");
        if (best_extension_by_limited_search(remaining_tables_after, idx + 1,
                                             current_search_depth - 1))
          return true;
      } else  // if ((current_search_depth > 1) && ...
      {
        if (consider_plan(idx, &trace_one_table)) return true;
        /*
          If plan is complete, there should be no "open" outer join nest, and
          all semi join nests should be handled by a strategy:
        */
        DBUG_ASSERT((remaining_tables_after != 0) ||
                    ((cur_embedding_map == 0) &&
                     (join->positions[idx].dups_producing_tables == 0) &&
                     (join->deps_of_remaining_lateral_derived_tables == 0)));
      }
      backout_nj_state(remaining_tables, s);
    }
  }

done:
  // Restore previous #rows sorted best_ref[]
  memcpy(join->best_ref + idx, saved_refs,
         sizeof(JOIN_TAB *) * (join->tables - idx));
  return false;
}

/**
  Helper function that compares two doubles and accept these as
  "almost equal" if they are within 10 percent of each other.

  Handling of exact 0.0 values: if one of the values are exactly 0.0, the
  other value must also be exactly 0.0 to be considered to be equal.

  @param left  First double number to compare
  @param right Second double number to compare

  @return true if the two numbers are almost equal, false otherwise.
*/

static inline bool almost_equal(double left, double right) {
  const double boundary = 0.1;  // 10 percent limit
  if ((left >= right * (1.0 - boundary)) && (left <= right * (1.0 + boundary)))
    return true;
  else
    return false;
}

/**
  Heuristic utility used by best_extension_by_limited_search().
  Adds EQ_REF-joined tables to the partial plan without
  extensive 'greedy' cost calculation.

  When a table is joined by an unique key there is a
  1::1 relation between the rows being joined. Assuming we
  have multiple such 1::1 (star-)joined relations in a
  sequence, without other join types inbetween. Then all of
  these 'eq_ref-joins' will be estimated to return the exact
  same number of rows and having identical 'cost' (or 'read_time').

  This leads to that we can append such a contiguous sequence
  of eq_ref-joins to a partial plan in any order without
  affecting the total cost of the query plan. Exploring the
  different permutations of these eq_refs in the 'greedy'
  optimizations will simply be a waste of precious CPU cycles.

  Once we have appended a single eq_ref-join to a partial
  plan, we may use eq_ref_extension_by_limited_search() to search
  'remaining_tables' for more eq_refs which will form a contiguous
  set of eq_refs in the QEP.

  Effectively, this chain of eq_refs will be handled as a single
  entity wrt. the full 'greedy' exploration of the possible
  join plans. This will reduce the 'N' in the O(N!) complexity
  of the full greedy search.

  The algorithm start by already having a eq_ref joined table
  in position[idx-1] when called. It then search for more
  eq_ref-joinable 'remaining_tables' which are added directly
  to the partial QEP without further cost analysis. The algorithm
  continues until it either has constructed a complete plan,
  constructed a partial plan with size = search_depth, or could not
  find more eq_refs to append.

  In the later case the algorithm continues into
  'best_extension_by_limited_search' which does a 'greedy'
  search for the next table to add - Possibly with later
  eq_ref_extensions.

  The final optimal plan is stored in 'join->best_positions'. The
  corresponding cost of the optimal plan is in 'join->best_read'.

  @note
    @c best_extension_by_limited_search() and @c
  eq_ref_extension_by_limited_search() are closely related to each other and
  intentionally implemented using the same pattern wherever possible. If a
  change/bug fix is done to either of these also consider if it is relevant for
  the other.

  @code
    procedure eq_ref_extension_by_limited_search(
      pplan in,             // in, partial plan of tables-joined-so-far
      pplan_cost,           // in, cost of pplan
      remaining_tables,     // in, set of tables not referenced in pplan
      best_plan_so_far,     // in/out, best plan found so far
      best_plan_so_far_cost,// in/out, cost of best_plan_so_far
      search_depth)         // in, maximum size of the plans being considered
    {
      if find 'eq_ref' table T from remaining_tables
      {
        // Calculate the cost of using table T as above
        cost = complex-series-of-calculations;

        // Add the cost to the cost so far.
        pplan_cost+= cost;

        if (pplan_cost >= best_plan_so_far_cost)
          // pplan_cost already too great, stop search
          continue;

        pplan= expand pplan by best_access_method;
        remaining_tables= remaining_tables - table T;
        eq_ref_extension_by_limited_search(pplan, pplan_cost,
                                           remaining_tables,
                                           best_plan_so_far,
                                           best_plan_so_far_cost,
                                           search_depth - 1);
      }
      else
      {
        best_extension_by_limited_search(pplan, pplan_cost,
                                         remaining_tables,
                                         best_plan_so_far,
                                         best_plan_so_far_cost,
                                         search_depth - 1);
      }
    }
    @endcode

  @note
    The parameter 'search_depth' provides control over the recursion
    depth, and thus the size of the resulting optimal plan.

  @param remaining_tables set of tables not included into the partial plan yet
  @param idx              length of the partial QEP in 'join->positions';
                          since a depth-first search is used, also corresponds
                          to the current depth of the search tree;
                          also an index in the array 'join->best_ref';
  @param current_search_depth
                          maximum depth of recursion and thus size of the
                          found optimal plan
                          (0 < current_search_depth <= join->tables+1).

  @retval
    'table_map'          Map of those tables appended to the EQ_REF-joined
  sequence
  @retval
    ~(table_map)0        Fatal error
*/

table_map Optimize_table_order::eq_ref_extension_by_limited_search(
    table_map remaining_tables, uint idx, uint current_search_depth) {
  DBUG_TRACE;

  if (remaining_tables == 0) return 0;

  /*
    The section below adds 'eq_ref' joinable tables to the QEP in the order
    they are found in the 'remaining_tables' set.
    See above description for why we can add these without greedy
    cost analysis.
  */
  Opt_trace_context *const trace = &thd->opt_trace;
  table_map eq_ref_ext(0);
  JOIN_TAB *s;
  JOIN_TAB *saved_refs[MAX_TABLES];
  // Save 'best_ref[]' as we has to restore before return.
  memcpy(saved_refs, join->best_ref + idx,
         sizeof(JOIN_TAB *) * (join->tables - idx));

  Deps_of_remaining_lateral_derived_tables deps_lateral(join, ~excluded_tables);

  for (JOIN_TAB **pos = join->best_ref + idx; (s = *pos); pos++) {
    const table_map real_table_bit = s->table_ref->map();

    /*
      Don't move swap inside conditional code: All items
      should be swapped to maintain '#rows' ordered tables.
      This is critical for early pruning of bad plans.
    */
    std::swap(join->best_ref[idx], *pos);

    /*
      Consider table for 'eq_ref' heuristic if:
        1)      It might use a keyref for best_access_path
        2) and, Table remains to be handled.
        3) and, It is independent of those not yet in partial plan.
        4) and, It is key dependent on at least one already handled table
        5) and, It passed the interleaving check.
    */
    if (s->keyuse() &&                             // 1)
        (remaining_tables & real_table_bit) &&     // 2)
        !(remaining_tables & s->dependent) &&      // 3)
        (~remaining_tables & s->key_dependent) &&  // 4)
        (!idx || !check_interleaving_with_nj(s)))  // 5)
    {
      Opt_trace_object trace_one_table(trace);
      if (unlikely(trace->is_started())) {
        trace_plan_prefix(join, idx, excluded_tables);
        trace_one_table.add_utf8_table(s->table_ref);
      }
      POSITION *const position = join->positions + idx;

      DBUG_ASSERT(emb_sjm_nest == nullptr || emb_sjm_nest == s->emb_sj_nest);

      deps_lateral.restore();

      /* Find the best access method from 's' to the current partial plan */
      best_access_path(s, remaining_tables, idx, false,
                       idx ? (position - 1)->prefix_rowcount : 1.0, position);

      /*
        EQ_REF prune logic is based on that all joins
        in the ref_extension has the same #rows and cost.
        -> The total cost of the QEP is independent of the order
           of joins within this 'ref_extension'.
           Expand QEP with all 'identical' REFs in
          'join->positions' order.
        Note that due to index statistics from the storage engines
        is a floating point number and might not be exact, the
        rows and cost estimates for eq_ref on two tables might not
        be the exact same number.
        @todo This test could likely be re-implemented to use
        information about whether the index is unique or not.
      */
      const bool added_to_eq_ref_extension =
          position->key &&
          almost_equal(position->read_cost, (position - 1)->read_cost) &&
          almost_equal(position->rows_fetched, (position - 1)->rows_fetched);
      trace_one_table.add("added_to_eq_ref_extension",
                          added_to_eq_ref_extension);
      if (added_to_eq_ref_extension) {
        // Add the cost of extending the plan with 's'
        position->set_prefix_join_cost(idx, join->cost_model());

        trace_one_table
            .add("condition_filtering_pct", position->filter_effect * 100)
            .add("rows_for_plan", position->prefix_rowcount)
            .add("cost_for_plan", position->prefix_cost);

        if (has_sj) {
          /*
            Even if there are no semijoins, advance_sj_state() has a
            significant cost (takes 9% of time in a 20-table plan search),
            hence the if() above, which is also more efficient than the
            same if() inside advance_sj_state() would be.
          */
          advance_sj_state(remaining_tables, s, idx);
        } else
          position->no_semijoin();

        // Expand only partial plans with lower cost than the best QEP so far
        if (position->prefix_cost >= join->best_read) {
          DBUG_EXECUTE("opt",
                       print_plan(join, idx + 1, position->prefix_rowcount,
                                  position->read_cost, position->prefix_cost,
                                  "prune_by_cost"););
          trace_one_table.add("pruned_by_cost", true);
          backout_nj_state(remaining_tables, s);
          continue;
        }

        deps_lateral.recalculate(s, idx + 1);

        eq_ref_ext = real_table_bit;
        const table_map remaining_tables_after =
            (remaining_tables & ~real_table_bit);
        if ((current_search_depth > 1) && remaining_tables_after) {
          DBUG_EXECUTE("opt",
                       print_plan(join, idx + 1, position->prefix_rowcount,
                                  position->read_cost, position->prefix_cost,
                                  "EQ_REF_extension"););

          /* Recursively EQ_REF-extend the current partial plan */
          Opt_trace_array trace_rest(trace, "rest_of_plan");
          eq_ref_ext |= eq_ref_extension_by_limited_search(
              remaining_tables_after, idx + 1, current_search_depth - 1);
        } else {
          if (consider_plan(idx, &trace_one_table)) return ~(table_map)0;
          DBUG_ASSERT((remaining_tables_after != 0) ||
                      ((cur_embedding_map == 0) &&
                       (join->positions[idx].dups_producing_tables == 0)));
        }
        backout_nj_state(remaining_tables, s);
        memcpy(join->best_ref + idx, saved_refs,
               sizeof(JOIN_TAB *) * (join->tables - idx));
        return eq_ref_ext;
      }  // if (added_to_eq_ref_extension)

      backout_nj_state(remaining_tables, s);
    }  // if (... !check_interleaving_with_nj() ...)
  }    // for (JOIN_TAB **pos= ...)

  memcpy(join->best_ref + idx, saved_refs,
         sizeof(JOIN_TAB *) * (join->tables - idx));
  deps_lateral.restore();
  /*
    'eq_ref' heuristic didn't find a table to be appended to
    the query plan. We need to use the greedy search
    for finding the next table to be added.
  */
  DBUG_ASSERT(!eq_ref_ext);
  if (best_extension_by_limited_search(remaining_tables, idx,
                                       current_search_depth))
    return ~(table_map)0;

  return eq_ref_ext;
}

/*
  Get the number of different row combinations for subset of partial join

  SYNOPSIS
    prev_record_reads()
      join       The join structure
      idx        Number of tables in the partial join order (i.e. the
                 partial join order is in join->positions[0..idx-1])
      found_ref  Bitmap of tables for which we need to find # of distinct
                 row combinations.

  DESCRIPTION
    Given a partial join order (in join->positions[0..idx-1]) and a subset of
    tables within that join order (specified in found_ref), find out how many
    distinct row combinations of subset tables will be in the result of the
    partial join order.

    This is used as follows: Suppose we have a table accessed with a ref-based
    method. The ref access depends on current rows of tables in found_ref.
    We want to count # of different ref accesses. We assume two ref accesses
    will be different if at least one of access parameters is different.
    Example: consider a query

    SELECT * FROM t1, t2, t3 WHERE t1.key=c1 AND t2.key=c2 AND t3.key=t1.field

    and a join order:
      t1,  ref access on t1.key=c1
      t2,  ref access on t2.key=c2
      t3,  ref access on t3.key=t1.field

    For t1: n_ref_scans = 1, n_distinct_ref_scans = 1
    For t2: n_ref_scans = fanout(t1), n_distinct_ref_scans=1
    For t3: n_ref_scans = fanout(t1)*fanout(t2)
            n_distinct_ref_scans = #fanout(t1)

    Here "fanout(tx)" is the number of rows read by the access method
    of tx minus rows filtered out by condition filtering
    (pos->filter_effect).

    The reason for having this function (at least the latest version of it)
    is that we need to account for buffering in join execution.

    An edge-case example: if we have a non-first table in join accessed via
    ref(const) or ref(param) where there is a small number of different
    values of param, then the access will likely hit the disk cache and will
    not require any disk seeks.

    The proper solution would be to assume an LRU disk cache of some size,
    calculate probability of cache hits, etc. For now we just count
    identical ref accesses as one.

  RETURN
    Expected number of row combinations
*/

static double prev_record_reads(JOIN *join, uint idx, table_map found_ref) {
  double found = 1.0;
  POSITION *pos_end = join->positions - 1;
  for (POSITION *pos = join->positions + idx - 1; pos != pos_end; pos--) {
    const double fanout = pos->rows_fetched * pos->filter_effect;
    if (pos->table->table_ref->map() & found_ref) {
      found_ref |= pos->ref_depend_map;
      /*
        For the case of "t1 LEFT JOIN t2 ON ..." where t2 is a const table
        with no matching row we will get position[t2].rows_fetched==0.
        Actually the size of output is one null-complemented row, therefore
        we will use value of 1 whenever we get rows_fetched==0.

        Note
        - the above case can't occur if inner part of outer join has more
          than one table: table with no matches will not be marked as const.

        - Ideally we should add 1 to rows_fetched for every possible null-
          complemented row. We're not doing it because: 1. it will require
          non-trivial code and add overhead. 2. The value of rows_fetched
          is an inprecise estimate and adding 1 (or, in the worst case,
          #max_nested_outer_joins=64-1) will not make it any more precise.
      */
      if (pos->rows_fetched > DBL_EPSILON) found *= fanout;
    } else if (fanout < 1.0) {
      /*
        With condition filtering it is possible that a table has a
        lower fanout than 1.0. If so, calculate the fanout of this
        table into the found rows estimate so the produced number is
        not too pessimistic. Otherwise, the expected number of row
        combinations returned by this function may be higher than the
        prefix_rowcount for the table. See BUG#18352936
      */
      found *= fanout;
    }
  }
  return found;
}

/**
  @brief Fix semi-join strategies for the picked join order

  @return false if success, true if error

  @details
    Fix semi-join strategies for the picked join order. This is a step that
    needs to be done right after we have fixed the join order. What we do
    here is switch join's semi-join strategy description from backward-based
    to forwards based.

    When join optimization is in progress, we re-consider semi-join
    strategies after we've added another table. Here's an illustration.
    Suppose the join optimization is underway:

    1) ot1  it1  it2
                 sjX  -- looking at (ot1, it1, it2) join prefix, we decide
                         to use semi-join strategy sjX.

    2) ot1  it1  it2  ot2
                 sjX  sjY -- Having added table ot2, we now may consider
                             another semi-join strategy and decide to use a
                             different strategy sjY. Note that the record
                             of sjX has remained under it2. That is
                             necessary because we need to be able to get
                             back to (ot1, it1, it2) join prefix.
      what makes things even worse is that there are cases where the choice
      of sjY changes the way we should access it2.

    3) [ot1  it1  it2  ot2  ot3]
                  sjX  sjY  -- This means that after join optimization is
                               finished, semi-join info should be read
                               right-to-left (while nearly all plan refinement
                               functions, EXPLAIN, etc proceed from left to
                               right)

    This function does the needed reversal, making it possible to read the
    join and semi-join order from left to right.
*/

bool Optimize_table_order::fix_semijoin_strategies() {
  table_map remaining_tables = 0;
  table_map handled_tables = 0;

  DBUG_TRACE;

  if (join->select_lex->sj_nests.empty()) return false;

  Opt_trace_context *const trace = &thd->opt_trace;

  for (uint tableno = join->tables - 1; tableno != join->const_tables - 1;
       tableno--) {
    POSITION *const pos = join->best_positions + tableno;

    if ((handled_tables & pos->table->table_ref->map()) ||
        pos->sj_strategy == SJ_OPT_NONE) {
      remaining_tables |= pos->table->table_ref->map();
      continue;
    }

    uint first = 0;
    if (pos->sj_strategy == SJ_OPT_MATERIALIZE_LOOKUP) {
      TABLE_LIST *const sjm_nest = pos->table->emb_sj_nest;
      const uint table_count = my_count_bits(sjm_nest->sj_inner_tables);
      /*
        This memcpy() copies a partial QEP produced by
        optimize_semijoin_nests_for_materialization() (source) into the final
        top-level QEP (target), in order to re-use the source plan for
        to-be-materialized inner tables.
        It is however possible that the source QEP had picked
        some semijoin strategy (noted SJY), different from
        materialization. The target QEP rules (it has seen more tables), but
        this memcpy() is going to copy the source stale strategy SJY,
        wrongly. Which is why sj_strategy of each table of the
        duplicate-generating range then becomes temporarily unreliable. It is
        fixed for the first table of that range right after the memcpy(), and
        fixed for the rest of that range at the end of this iteration by
        setting it to SJ_OPT_NONE). But until then, pos->sj_strategy should
        not be read.
      */
      memcpy(pos - table_count + 1, sjm_nest->nested_join->sjm.positions,
             sizeof(POSITION) * table_count);
      first = tableno - table_count + 1;
      join->best_positions[first].n_sj_tables = table_count;
      join->best_positions[first].sj_strategy = SJ_OPT_MATERIALIZE_LOOKUP;

      Opt_trace_object trace_final_strategy(trace);
      trace_final_strategy.add_alnum("final_semijoin_strategy",
                                     "MaterializeLookup");
    } else if (pos->sj_strategy == SJ_OPT_MATERIALIZE_SCAN) {
      const uint last_inner = pos->sjm_scan_last_inner;
      TABLE_LIST *const sjm_nest =
          (join->best_positions + last_inner)->table->emb_sj_nest;
      const uint table_count = my_count_bits(sjm_nest->sj_inner_tables);
      first = last_inner - table_count + 1;
      DBUG_ASSERT((join->best_positions + first)->table->emb_sj_nest ==
                  sjm_nest);
      memcpy(join->best_positions + first,  // stale semijoin strategy here too
             sjm_nest->nested_join->sjm.positions,
             sizeof(POSITION) * table_count);
      join->best_positions[first].sj_strategy = SJ_OPT_MATERIALIZE_SCAN;
      join->best_positions[first].n_sj_tables = table_count;

      Opt_trace_object trace_final_strategy(trace);
      trace_final_strategy.add_alnum("final_semijoin_strategy",
                                     "MaterializeScan");
      // Recalculate final access paths for this semi-join strategy
      double rowcount, cost;
      semijoin_mat_scan_access_paths(last_inner, tableno, remaining_tables,
                                     sjm_nest, &rowcount, &cost);

    } else if (pos->sj_strategy == SJ_OPT_FIRST_MATCH) {
      first = pos->first_firstmatch_table;

      Opt_trace_object trace_final_strategy(trace);
      trace_final_strategy.add_alnum("final_semijoin_strategy", "FirstMatch");

      // Recalculate final access paths for this semi-join strategy
      double rowcount, cost;
      (void)semijoin_firstmatch_loosescan_access_paths(
          first, tableno, remaining_tables, false, &rowcount, &cost);

      if (pos->table->emb_sj_nest->is_aj_nest()) {
        /*
          Antijoin doesn't use the execution logic of FirstMatch. So we
          won't set it up; and we won't either have the incompatibilities of
          FirstMatch with outer join. Declare that we don't use it:
        */
        pos->sj_strategy = SJ_OPT_NONE;
      } else {
        join->best_positions[first].sj_strategy = SJ_OPT_FIRST_MATCH;
        join->best_positions[first].n_sj_tables = tableno - first + 1;
      }
    } else if (pos->sj_strategy == SJ_OPT_LOOSE_SCAN) {
      first = pos->first_loosescan_table;

      Opt_trace_object trace_final_strategy(trace);
      trace_final_strategy.add_alnum("final_semijoin_strategy", "LooseScan");

      // Recalculate final access paths for this semi-join strategy
      double rowcount, cost;
      (void)semijoin_firstmatch_loosescan_access_paths(
          first, tableno, remaining_tables, true, &rowcount, &cost);

      POSITION *const first_pos = join->best_positions + first;
      first_pos->sj_strategy = SJ_OPT_LOOSE_SCAN;
      first_pos->n_sj_tables =
          my_count_bits(first_pos->table->emb_sj_nest->sj_inner_tables);
    } else if (pos->sj_strategy == SJ_OPT_DUPS_WEEDOUT) {
      /*
        Duplicate Weedout starting at pos->first_dupsweedout_table, ending at
        this table.
      */
      first = pos->first_dupsweedout_table;
      join->best_positions[first].sj_strategy = SJ_OPT_DUPS_WEEDOUT;
      join->best_positions[first].n_sj_tables = tableno - first + 1;

      Opt_trace_object trace_final_strategy(trace);
      trace_final_strategy.add_alnum("final_semijoin_strategy",
                                     "DuplicateWeedout");
    }

    for (uint i = first; i <= tableno; i++) {
      /*
        Eliminate stale strategies. See comment in the
        SJ_OPT_MATERIALIZE_LOOKUP case above.
      */
      if (i != first) join->best_positions[i].sj_strategy = SJ_OPT_NONE;
      handled_tables |= join->best_positions[i].table->table_ref->map();
    }

    remaining_tables |= pos->table->table_ref->map();
  }

  DBUG_ASSERT(remaining_tables ==
              (join->all_table_map & ~join->const_table_map));

  return false;
}

/**
  Check interleaving with an inner tables of an outer join for
  extension table.

    Check if table tab can be added to current partial join order, and
    if yes, record that it has been added. This recording can be rolled back
    with backout_nj_state().

    The function assumes that both current partial join order and its
    extension with tab are valid wrt table dependencies.

  @verbatim
     IMPLEMENTATION
       LIMITATIONS ON JOIN ORDER
         The nested [outer] joins executioner algorithm imposes these
  limitations on join order:
         1. "Outer tables first" -  any "outer" table must be before any
             corresponding "inner" table.
         2. "No interleaving" - tables inside a nested join must form a
  continuous sequence in join order (i.e. the sequence must not be interrupted
  by tables that are outside of this nested join).

         #1 is checked elsewhere, this function checks #2 provided that #1 has
         been already checked.

       WHY NEED NON-INTERLEAVING
         Consider an example:

           select * from t0 join t1 left join (t2 join t3) on cond1

         The join order "t1 t2 t0 t3" is invalid:

         table t0 is outside of the nested join, so WHERE condition for t0 is
         attached directly to t0 (without triggers, and it may be used to access
         t0). Applying WHERE(t0) to (t2,t0,t3) record is invalid as we may miss
         combinations of (t1, t2, t3) that satisfy condition cond1, and produce
  a null-complemented (t1, t2.NULLs, t3.NULLs) row, which should not have been
  produced.

         If table t0 is not between t2 and t3, the problem doesn't exist:
          If t0 is located after (t2,t3), WHERE(t0) is applied after nested join
           processing has finished.
          If t0 is located before (t2,t3), predicates like WHERE_cond(t0, t2)
  are wrapped into condition triggers, which takes care of correct nested join
  processing.

       HOW IT IS IMPLEMENTED
         The limitations on join order can be rephrased as follows: for valid
         join order one must be able to:
           1. write down the used tables in the join order on one line.
           2. for each nested join, put one '(' and one ')' on the said line
           3. write "LEFT JOIN" and "ON (...)" where appropriate
           4. get a query equivalent to the query we're trying to execute.

         Calls to check_interleaving_with_nj() are equivalent to writing the
         above described line from left to right.
         A single check_interleaving_with_nj(A,B) call is equivalent to writing
         table B and appropriate brackets on condition that table A and
         appropriate brackets is the last what was written. Graphically the
         transition is as follows:

                              +---- current position
                              |
             ... last_tab ))) | ( tab )  )..) | ...
                                X     Y   Z   |
                                              +- need to move to this
                                                 position.

         Notes about the position:
           The caller guarantees that there is no more then one X-bracket by
           checking "!(remaining_tables & s->dependent)" before calling this
           function. X-bracket may have a pair in Y-bracket.

         When "writing" we store/update this auxilary info about the current
         position:
          1. cur_embedding_map - bitmap of pairs of brackets (aka nested
             joins) we've opened but didn't close.
          2. {each NESTED_JOIN structure not simplified away}->counter - number
             of this nested join's children that have already been added to to
             the partial join order.
  @endverbatim

  @param tab   Table we're going to extend the current partial join with

  @retval
    false  Join order extended, nested joins info about current join
    order (see NOTE section) updated.
  @retval
    true   Requested join order extension not allowed.
*/

bool Optimize_table_order::check_interleaving_with_nj(JOIN_TAB *tab) {
  if (cur_embedding_map & ~tab->embedding_map) {
    /*
      tab is outside of the "pair of brackets" we're currently in.
      Cannot add it.
    */
    return true;
  }
  const TABLE_LIST *next_emb = tab->table_ref->embedding;
  /*
    Do update counters for "pairs of brackets" that we've left (marked as
    X,Y,Z in the above picture)
  */
  for (; next_emb != emb_sjm_nest; next_emb = next_emb->embedding) {
    // Ignore join nests that are not outer joins.
    if (!next_emb->join_cond_optim()) continue;

    next_emb->nested_join->nj_counter++;
    cur_embedding_map |= next_emb->nested_join->nj_map;
    if (next_emb->nested_join->nj_total != next_emb->nested_join->nj_counter)
      break;

    /*
      We're currently at Y or Z-bracket as depicted in the above picture.
      Mark that we've left it and continue walking up the brackets hierarchy.
    */
    cur_embedding_map &= ~next_emb->nested_join->nj_map;
  }
  return false;
}

/**
  Find best access paths for semi-join FirstMatch or LooseScan strategy
  and calculate rowcount and cost based on these.

  @param first_tab        The first tab to calculate access paths for,
                          this is always a semi-join inner table.
  @param last_tab         The last tab to calculate access paths for,
                          always a semi-join inner table for FirstMatch,
                          may be inner or outer for LooseScan.
  @param remaining_tables Bitmap of tables that are not in the
                          [0...last_tab] join prefix
  @param loosescan        If true, use LooseScan strategy, otherwise FirstMatch
  @param[out] newcount    New output row count
  @param[out] newcost     New join prefix cost

  @return True if strategy selection successful, false otherwise.

  @details
    Calculate best access paths for the tables of a semi-join FirstMatch or
    LooseScan strategy, given the order of tables provided in join->positions
    (or join->best_positions when calculating the cost of a final plan).
    Calculate estimated cost and rowcount for this plan.
    Given a join prefix [0; ... first_tab-1], change the access to the tables
    in the range [first_tab; last_tab] according to the constraints set by the
    relevant semi-join strategy. Those constraints are:

    - For the LooseScan strategy, join buffering can be used for the outer
      tables following the last inner table.

    - For the FirstMatch strategy, join buffering can be used if there is a
      single inner table in the semi-join nest.

    For FirstMatch, the handled range of tables may be a mix of inner tables
    and non-dependent outer tables. The first and last table in the handled
    range are always inner tables.
    For LooseScan, the handled range can be a mix of inner tables and
    dependent and non-dependent outer tables. The first table is always an
    inner table.

    Depending on member 'got_final_plan', the function uses and updates access
    path data in join->best_positions, otherwise uses join->positions
    and updates a local buffer.
*/

bool Optimize_table_order::semijoin_firstmatch_loosescan_access_paths(
    uint first_tab, uint last_tab, table_map remaining_tables, bool loosescan,
    double *newcount, double *newcost) {
  DBUG_TRACE;
  double cost;                // Contains running estimate of calculated cost.
  double rowcount;            // Rowcount of join prefix (ie before first_tab).
  double outer_fanout = 1.0;  // Fanout contributed by outer tables in range.
  double inner_fanout = 1.0;  // Fanout contributed by inner tables in range.
  const Cost_model_server *const cost_model = join->cost_model();
  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object recalculate(trace, "recalculate_access_paths_and_cost");
  Opt_trace_array trace_tables(trace, "tables");
  POSITION *const positions =
      got_final_plan ? join->best_positions : join->positions;

  if (first_tab == join->const_tables) {
    cost = 0.0;
    rowcount = 1.0;
  } else {
    cost = positions[first_tab - 1].prefix_cost;
    rowcount = positions[first_tab - 1].prefix_rowcount;
  }

  uint table_count = 0;
  uint no_jbuf_before;
  for (uint i = first_tab; i <= last_tab; i++) {
    remaining_tables |= positions[i].table->table_ref->map();
    if (positions[i].table->emb_sj_nest) table_count++;
  }
  if (loosescan) {
    // LooseScan: May use join buffering for all tables after last inner table.
    for (no_jbuf_before = last_tab; no_jbuf_before > first_tab;
         no_jbuf_before--) {
      if (positions[no_jbuf_before].table->emb_sj_nest != nullptr)
        break;  // Encountered the last inner table.
    }
    no_jbuf_before++;
  } else {
    // FirstMatch: May use join buffering if there is only one inner table.
    no_jbuf_before = (table_count > 1) ? last_tab + 1 : first_tab;
  }

  Deps_of_remaining_lateral_derived_tables deps_lateral(join, ~excluded_tables);
  // recalculate, as we go back in the range of "unoptimized" tables:
  deps_lateral.recalculate(first_tab);

  for (uint i = first_tab; i <= last_tab; i++) {
    JOIN_TAB *const tab = positions[i].table;
    POSITION regular_pos;
    POSITION *const dst_pos = got_final_plan ? positions + i : &regular_pos;
    POSITION *pos;  // Position for later calculations
    /*
      We always need a new calculation for the first inner table in
      the LooseScan strategy.
    */
    const bool is_ls_driving_tab = (i == first_tab) && loosescan;
    if (is_ls_driving_tab || positions[i].use_join_buffer) {
      Opt_trace_object trace_one_table(trace);
      trace_one_table.add_utf8_table(tab->table_ref);

      /*
        Find the best access method with specified join buffering strategy.
        If this is a loosescan driving table,
        semijoin_loosescan_fill_driving_table_position will consider all keys,
        so best_access_path() should fill bound_keyparts/read_cost/fanout for
        all keys => test_all_ref_keys==true.
       */
      DBUG_ASSERT(!test_all_ref_keys);
      test_all_ref_keys = is_ls_driving_tab;
      double prefix_rowcount = rowcount * inner_fanout * outer_fanout;
      best_access_path(tab, remaining_tables, i, i < no_jbuf_before,
                       prefix_rowcount, dst_pos);
      test_all_ref_keys = false;
      if (is_ls_driving_tab)  // Use loose scan position
      {
        if (semijoin_loosescan_fill_driving_table_position(
                tab, remaining_tables, i, prefix_rowcount, dst_pos)) {
          dst_pos->table = tab;
          const double rows = rowcount * dst_pos->rows_fetched;
          dst_pos->set_prefix_cost(
              cost + dst_pos->read_cost + cost_model->row_evaluate_cost(rows),
              rows * dst_pos->filter_effect);
        } else {
          DBUG_ASSERT(!got_final_plan);
          return false;
        }
      }
      pos = dst_pos;
    } else
      pos = positions + i;  // Use result from prior calculation

    /*
      Terminate search if best_access_path found no possible plan.
      Otherwise we will be getting infinite cost when summing up below.
     */
    if (pos->read_cost == DBL_MAX) {
      DBUG_ASSERT(loosescan && !got_final_plan);
      return false;
    }

    remaining_tables &= ~tab->table_ref->map();

    cost += pos->read_cost +
            cost_model->row_evaluate_cost(rowcount * inner_fanout *
                                          outer_fanout * pos->rows_fetched);

    if (tab->emb_sj_nest)
      inner_fanout *= pos->rows_fetched * pos->filter_effect;
    else
      outer_fanout *= pos->rows_fetched * pos->filter_effect;

    deps_lateral.recalculate(tab, i + 1);
  }

  *newcount = rowcount * outer_fanout;
  *newcost = cost;

  return true;
}

/**
  Find best access paths for semi-join MaterializeScan strategy
  and calculate rowcount and cost based on these.

  @param last_inner_tab    The last tab in the set of inner tables
  @param last_outer_tab    The last tab in the set of outer tables
  @param remaining_tables  Bitmap of tables that are not in the join prefix
                           including the inner and outer tables processed here.
  @param sjm_nest          Pointer to semi-join nest for inner tables
  @param[out] newcount     New output row count
  @param[out] newcost      New join prefix cost

  @details
    Calculate best access paths for the outer tables of the MaterializeScan
    semi-join strategy. All outer tables may use join buffering.
    The prefix row count is adjusted with the estimated number of rows in
    the materialized tables, before taking into consideration the rows
    contributed by the outer tables.
*/

void Optimize_table_order::semijoin_mat_scan_access_paths(
    uint last_inner_tab, uint last_outer_tab, table_map remaining_tables,
    TABLE_LIST *sjm_nest, double *newcount, double *newcost) {
  DBUG_TRACE;

  const Cost_model_server *const cost_model = join->cost_model();
  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object recalculate(trace, "recalculate_access_paths_and_cost");
  Opt_trace_array trace_tables(trace, "tables");
  double cost;      // Calculated running cost of operation
  double rowcount;  // Rowcount of join prefix (ie before first_inner).

  POSITION *const positions =
      got_final_plan ? join->best_positions : join->positions;
  const uint inner_count = my_count_bits(sjm_nest->sj_inner_tables);

  // Get the prefix cost.
  const uint first_inner = last_inner_tab + 1 - inner_count;
  if (first_inner == join->const_tables) {
    rowcount = 1.0;
    cost = 0.0;
  } else {
    rowcount = positions[first_inner - 1].prefix_rowcount;
    cost = positions[first_inner - 1].prefix_cost;
  }

  // Add materialization cost.
  cost += sjm_nest->nested_join->sjm.materialization_cost.total_cost() +
          rowcount * sjm_nest->nested_join->sjm.scan_cost.total_cost();

  for (uint i = last_inner_tab + 1; i <= last_outer_tab; i++)
    remaining_tables |= positions[i].table->table_ref->map();
  /*
    Materialization removes duplicates from the materialized table, so
    number of rows to scan is probably less than the number of rows
    from a full join, on which the access paths of outer tables are currently
    based. Rerun best_access_path to adjust for reduced rowcount.
  */
  const double inner_fanout = sjm_nest->nested_join->sjm.expected_rowcount;
  double outer_fanout = 1.0;

  Deps_of_remaining_lateral_derived_tables deps_lateral(join, ~excluded_tables);
  // recalculate, as we go back in the range of "unoptimized" tables:
  deps_lateral.recalculate(last_inner_tab + 1);

  for (uint i = last_inner_tab + 1; i <= last_outer_tab; i++) {
    Opt_trace_object trace_one_table(trace);
    JOIN_TAB *const tab = positions[i].table;
    trace_one_table.add_utf8_table(tab->table_ref);
    POSITION regular_pos;
    POSITION *const dst_pos = got_final_plan ? positions + i : &regular_pos;
    best_access_path(tab, remaining_tables, i, false,
                     rowcount * inner_fanout * outer_fanout, dst_pos);
    remaining_tables &= ~tab->table_ref->map();
    outer_fanout *= dst_pos->rows_fetched;
    cost += dst_pos->read_cost + cost_model->row_evaluate_cost(
                                     rowcount * inner_fanout * outer_fanout);
    outer_fanout *= dst_pos->filter_effect;
    deps_lateral.recalculate(tab, i + 1);
  }

  *newcount = rowcount * outer_fanout;
  *newcost = cost;
}

/**
  Find best access paths for semi-join MaterializeLookup strategy.
  and calculate rowcount and cost based on these.

  @param last_inner        Index of the last inner table
  @param sjm_nest          Pointer to semi-join nest for inner tables
  @param[out] newcount     New output row count
  @param[out] newcost      New join prefix cost

  @details
    All outer tables may use join buffering, so there is no need to recalculate
    access paths nor costs for these.
    Add cost of materialization and scanning the materialized table to the
    costs of accessing the outer tables.
*/

void Optimize_table_order::semijoin_mat_lookup_access_paths(
    uint last_inner, TABLE_LIST *sjm_nest, double *newcount, double *newcost) {
  DBUG_TRACE;

  const uint inner_count = my_count_bits(sjm_nest->sj_inner_tables);
  double rowcount, cost;

  const uint first_inner = last_inner + 1 - inner_count;
  if (first_inner == join->const_tables) {
    cost = 0.0;
    rowcount = 1.0;
  } else {
    cost = join->positions[first_inner - 1].prefix_cost;
    rowcount = join->positions[first_inner - 1].prefix_rowcount;
  }

  cost += sjm_nest->nested_join->sjm.materialization_cost.total_cost() +
          rowcount * sjm_nest->nested_join->sjm.lookup_cost.total_cost();

  *newcount = rowcount;
  *newcost = cost;
}

/**
  Find best access paths for semi-join DuplicateWeedout strategy
  and calculate rowcount and cost based on these.

  @param first_tab        The first tab to calculate access paths for
  @param last_tab         The last tab to calculate access paths for
  @param[out] newcount    New output row count
  @param[out] newcost     New join prefix cost

  @details
    Notice that new best access paths need not be calculated.
    The proper access path information is already in join->positions,
    because DuplicateWeedout can handle any join buffering strategy.
    The only action performed by this function is to calculate
    output rowcount, and an updated cost estimate.

    The cost estimate is based on performing a join over the involved
    tables, but we must also add the cost of creating and populating
    the temporary table used for duplicate removal, and the cost of
    doing lookups against this table.
*/

void Optimize_table_order::semijoin_dupsweedout_access_paths(uint first_tab,
                                                             uint last_tab,
                                                             double *newcount,
                                                             double *newcost) {
  DBUG_TRACE;

  const Cost_model_server *const cost_model = join->cost_model();
  double cost, rowcount;
  double inner_fanout = 1.0;
  double outer_fanout = 1.0;
  double max_outer_fanout = 1.0;
  uint rowsize;  // Row size of the temporary table
  if (first_tab == join->const_tables) {
    cost = 0.0;
    rowcount = 1.0;
    rowsize = 0;
  } else {
    cost = join->positions[first_tab - 1].prefix_cost;
    rowcount = join->positions[first_tab - 1].prefix_rowcount;
    rowsize = 8;  // This is not true but we'll make it so
  }
  /**
    Some times, some outer fanout is "absorbed" into the inner fanout.
    In this case, we should make a better estimate for outer_fanout that
    is used to calculate the output rowcount.
    If we have inner table(s) before an outer table, there are
    dependencies between these tables. The fanout for the outer table is
    not a good estimate for the final number of rows from the weedout
    execution, therefore we convert some of the inner fanout into an outer
    fanout, limited to the number of possible rows in the outer table.
  */
  for (uint j = first_tab; j <= last_tab; j++) {
    const POSITION *const p = join->positions + j;
    cost += p->read_cost +
            cost_model->row_evaluate_cost(rowcount * inner_fanout *
                                          outer_fanout * p->rows_fetched);

    if (p->table->emb_sj_nest)
      inner_fanout *= p->rows_fetched * p->filter_effect;
    else {
      /*
        max_outer_fanout is the cardinality of the cross product
        of the outer tables.
        @note: We do not consider dependencies between these tables here.
      */
      double total_records = p->table->table()->file->stats.records;
      max_outer_fanout *= total_records * p->filter_effect;
      if (inner_fanout > 1.0) {
        // Absorb inner fanout into the outer fanout
        outer_fanout *= inner_fanout * p->rows_fetched * p->filter_effect;
        inner_fanout = 1;
      } else
        outer_fanout *= p->rows_fetched * p->filter_effect;
      rowsize += p->table->table()->file->ref_length;
    }
  }

  if (max_outer_fanout < outer_fanout) {
    /*
      The calculated fanout for the outer tables is bigger than
      the cardinality of the cross product of the outer tables.
      Adjust outer fanout to the max value, but also adjust
      inner fanout so that inner_fanout * outer_fanout is still
      the same (dups weedout runs a complete join internally).
    */
    if (max_outer_fanout > 0.0) inner_fanout *= outer_fanout / max_outer_fanout;
    outer_fanout = max_outer_fanout;
  }

  /*
    Add the cost of temptable use. The table will have outer_fanout rows,
    and we will make
    - rowcount * outer_fanout writes
    - rowcount * inner_fanout * outer_fanout lookups.
  */
  Cost_model_server::enum_tmptable_type tmp_table_type;
  if (outer_fanout * rowsize < thd->variables.max_heap_table_size)
    tmp_table_type = Cost_model_server::MEMORY_TMPTABLE;
  else
    tmp_table_type = Cost_model_server::DISK_TMPTABLE;

  cost += cost_model->tmptable_create_cost(tmp_table_type);
  cost += cost_model->tmptable_readwrite_cost(
      tmp_table_type, rowcount * outer_fanout,
      rowcount * inner_fanout * outer_fanout);

  *newcount = rowcount * outer_fanout;
  *newcost = cost;
}

/**
  Do semi-join optimization step after we've added a new tab to join prefix

  This function cannot work with nested SJ nests, for two reasons:
  (a) QEP_TAB::emb_sj_nest points to the most inner SJ nest, and this
  function looks only at it, so misses to do any SJ strategy choice for
  outer nests
  (b) POSITION has only one set of SJ-info (e.g. first_firstmatch_table): so
  planning for two nested nests would require more info than we have.
  And indeed, SJ nests cannot be nested, because:
  (c) a SJ nest is not nested in another SJ or anti SJ nest (it would have been
  dissolved into the outer nest by simplify_joins()).
  (d) an anti SJ nest is not nested inside another SJ or anti SJ nest (this case
  is blocked by resolve_subquery()).

  @param remaining_tables Tables not in the join prefix
  @param new_join_tab     Join tab that we are adding to the join prefix
  @param idx              Index in join->position storing this join tab
                          (i.e. number of tables in the prefix)

  @details
    Update semi-join optimization state after we've added another tab (table
    and access method) to the join prefix.

    The state is maintained in join->positions[#prefix_size]. Each of the
    available strategies has its own state variables.

    for each semi-join strategy
    {
      update strategy's state variables;

      if (join prefix has all the tables that are needed to consider
          using this strategy for the semi-join(s))
      {
        calculate cost of using the strategy
        if ((this is the first strategy to handle the semi-join nest(s)  ||
            the cost is less than other strategies))
        {
          // Pick this strategy
          pos->sj_strategy= ..
          ..
        }
      }
    }

    Most of the new state is saved in join->positions[idx] (and hence no undo
    is necessary).

    See setup_semijoin_dups_elimination() for a description of what kinds of
    join prefixes each strategy can handle.

    A note on access path, rowcount and cost estimates:
    - best_extension_by_limited_search() performs *initial calculations*
      of access paths, rowcount and cost based on the operation being
      an inner join or an outer join operation. These estimates are saved
      in join->positions.
    - advance_sj_state() performs *intermediate calculations* based on the
      same table information, but for the supported semi-join strategies.
      The access path part of these calculations are not saved anywhere,
      but the rowcount and cost of the best semi-join strategy are saved
      in join->positions.
    - Because the semi-join access path information was not saved previously,
      fix_semijoin_strategies() must perform *final calculations* of
      access paths, rowcount and cost when saving the selected table order
      in join->best_positions. The results of the final calculations will be
      the same as the results of the "best" intermediate calculations.
*/

void Optimize_table_order::advance_sj_state(table_map remaining_tables,
                                            const JOIN_TAB *new_join_tab,
                                            uint idx) {
  Opt_trace_context *const trace = &thd->opt_trace;
  TABLE_LIST *const emb_sj_nest = new_join_tab->emb_sj_nest;
  POSITION *const pos = join->positions + idx;
  double best_cost = pos->prefix_cost;
  double best_rowcount = pos->prefix_rowcount;
  uint sj_strategy = SJ_OPT_NONE;  // Initially: No chosen strategy

  /*
    Semi-join nests cannot be nested, hence we never need to advance the
    semi-join state of a materialized semi-join query.
    In fact, doing this may cause undesirable effects because all tables
    within a semi-join nest have emb_sj_nest != NULL, which triggers several
    of the actions inside this function.
  */
  DBUG_ASSERT(emb_sjm_nest == nullptr);

  // remaining_tables include the current one:
  DBUG_ASSERT(remaining_tables & new_join_tab->table_ref->map());
  // Save it:
  const table_map remaining_tables_incl = remaining_tables;
  // And add the current table to the join prefix:
  remaining_tables &= ~new_join_tab->table_ref->map();

  DBUG_TRACE;

  Opt_trace_array trace_choices(trace, "semijoin_strategy_choice");

  /* Initialize the state or copy it from prev. tables */
  pos->cur_embedding_map = cur_embedding_map;
  if (idx == join->const_tables) {
    pos->dups_producing_tables = 0;
    pos->first_firstmatch_table = MAX_TABLES;
    pos->first_loosescan_table = MAX_TABLES;
    pos->dupsweedout_tables = 0;
    pos->sjm_scan_need_tables = 0;
    pos->sjm_scan_last_inner = 0;
  } else {
    pos->dups_producing_tables = pos[-1].dups_producing_tables;

    // FirstMatch
    pos->first_firstmatch_table = pos[-1].first_firstmatch_table;
    pos->first_firstmatch_rtbl = pos[-1].first_firstmatch_rtbl;
    pos->firstmatch_need_tables = pos[-1].firstmatch_need_tables;

    // LooseScan
    pos->first_loosescan_table = (pos[-1].sj_strategy == SJ_OPT_LOOSE_SCAN)
                                     ? MAX_TABLES
                                     : pos[-1].first_loosescan_table;
    pos->loosescan_need_tables = pos[-1].loosescan_need_tables;

    // MaterializeScan
    pos->sjm_scan_need_tables = (pos[-1].sj_strategy == SJ_OPT_MATERIALIZE_SCAN)
                                    ? 0
                                    : pos[-1].sjm_scan_need_tables;
    pos->sjm_scan_last_inner = pos[-1].sjm_scan_last_inner;

    // Duplicate Weedout
    pos->dupsweedout_tables = pos[-1].dupsweedout_tables;
    pos->first_dupsweedout_table = pos[-1].first_dupsweedout_table;
  }

  table_map handled_by_fm_or_ls = 0;
  /*
    FirstMatch Strategy
    ===================

    FirstMatch requires that all dependent outer tables are in the join prefix.
    (see "FirstMatch strategy" above setup_semijoin_dups_elimination()).
    The execution strategy will handle multiple semi-join nests correctly,
    and the optimizer will pick execution strategy according to these rules:
    - If tables from multiple semi-join nests are intertwined, they will
      be processed as one FirstMatch evaluation.
    - If tables from each semi-join nest are grouped together, each semi-join
      nest is processed as one FirstMatch evaluation.

    Example: Let's say we have an outer table ot and two semi-join nests with
    two tables each: it11 and it12, and it21 and it22.

    Intertwined tables: ot - FM(it11 - it21 - it12 - it22)
    Grouped tables: ot - FM(it11 - it12) - FM(it21 - it22)
  */

  if (pos->first_firstmatch_table != MAX_TABLES) {
    const TABLE_LIST *first_emb_sj_nest =
        join->positions[pos->first_firstmatch_table].table->emb_sj_nest;
    if (emb_sj_nest != first_emb_sj_nest) {
      // Can't handle interleaving between tables from the
      // semi-join that FirstMatch is handling and any other tables.
      pos->first_firstmatch_table = MAX_TABLES;
    }
  }

  if (emb_sj_nest && emb_sj_nest->nested_join->sj_enabled_strategies &
                         OPTIMIZER_SWITCH_FIRSTMATCH) {
    const table_map outer_corr_tables = emb_sj_nest->nested_join->sj_depends_on;
    const table_map sj_inner_tables = emb_sj_nest->sj_inner_tables;
    /*
      Enter condition:
       1. The next join tab belongs to semi-join nest
          (verified for the encompassing code block above).
       2. We're not in a duplicate producer range yet
       3. All outer tables that
           - the subquery is correlated with, or
           - referred to from the outer_expr
          are in the join prefix
    */
    if (pos->dups_producing_tables == 0 &&        // (2)
        !(remaining_tables & outer_corr_tables))  // (3)
    {
      /* Start tracking potential FirstMatch range */
      pos->first_firstmatch_table = idx;
      pos->firstmatch_need_tables = 0;
      pos->first_firstmatch_rtbl = remaining_tables;
      // All inner tables should still be part of remaining_tables_inc
      DBUG_ASSERT(sj_inner_tables == (remaining_tables_incl & sj_inner_tables));
    }

    if (pos->first_firstmatch_table != MAX_TABLES) {
      /* Record that we need all of this semi-join's inner tables */
      pos->firstmatch_need_tables |= sj_inner_tables;

      if (outer_corr_tables & pos->first_firstmatch_rtbl) {
        /*
          Trying to add an sj-inner table whose sj-nest has an outer correlated
          table that was not in the prefix. This means FirstMatch can't be used.
        */
        pos->first_firstmatch_table = MAX_TABLES;
      } else if (!(pos->firstmatch_need_tables & remaining_tables)) {
        // Got a complete FirstMatch range.

        // We cannot FirstMatch to a different embedding nest,
        // e.g., for B LEFT JOIN (C SEMIJOIN D ON B.X=D.Y) and table order
        // B-D-C we cannot jump from D to B. This would cause non-hierarchical
        // joins. So we check that the jump won't leave from a still-open
        // nest: cur_embedding_map at the last table of this firstmatch range
        // must be included in cur_embedding_map at the target of the jump.
        nested_join_map cur_embedding_map_at_jump_target =
            pos->first_firstmatch_table > join->const_tables
                ? join->positions[pos->first_firstmatch_table - 1]
                      .cur_embedding_map
                : 0;
        if ((cur_embedding_map_at_jump_target & cur_embedding_map) !=
            cur_embedding_map) {
          pos->first_firstmatch_table = MAX_TABLES;
        } else {
          // Calculate access paths and cost
          double cost, rowcount;
          /* We use the same FirstLetterUpcase as in EXPLAIN */
          Opt_trace_object trace_one_strategy(trace);
          trace_one_strategy.add_alnum("strategy", "FirstMatch");
          (void)semijoin_firstmatch_loosescan_access_paths(
              pos->first_firstmatch_table, idx, remaining_tables, false,
              &rowcount, &cost);
          /*
            We don't yet know what are the other strategies, so pick FirstMatch.

            We ought to save the alternate POSITIONs produced by
            semijoin_firstmatch_loosescan_access_paths() but the problem is that
            providing save space uses too much space.
            Instead, we will re-calculate the alternate POSITIONs after we've
            picked the best QEP.
          */
          sj_strategy = SJ_OPT_FIRST_MATCH;
          best_cost = cost;
          best_rowcount = rowcount;
          trace_one_strategy.add("cost", best_cost).add("rows", best_rowcount);
          handled_by_fm_or_ls = pos->firstmatch_need_tables;

          trace_one_strategy.add("chosen", true);
        }
      }
    }
  }
  /*
    LooseScan Strategy
    ==================

    LooseScan requires that all dependent outer tables are not in the join
    prefix. (see "LooseScan strategy" above setup_semijoin_dups_elimination()).
    The tables must come in a rather strictly defined order:
    1. The LooseScan driving table (which is a subquery inner table).
    2. The remaining tables from the same semi-join nest as the above table.
    3. The outer dependent tables, possibly mixed with outer non-dependent
       tables.
    Notice that any other semi-joined tables must be outside this table range.
  */
  {
    if (pos->first_loosescan_table != MAX_TABLES) {
      const TABLE_LIST *first_emb_sj_nest =
          join->positions[pos->first_loosescan_table].table->emb_sj_nest;
      if (first_emb_sj_nest->sj_inner_tables & remaining_tables_incl) {
        // Stage 2: Accept remaining tables from the semi-join nest:
        if (emb_sj_nest != first_emb_sj_nest) {
          /*
            LooseScan strategy can't handle interleaving between tables from
            the semi-join that LooseScan is handling and any other tables.
          */
          pos->first_loosescan_table = MAX_TABLES;
        } else {
          /*
            NestedLoopSemiJoinWithDuplicateRemovalIterator takes a
            single-table iterator as left argument, and inner-joins
            it with the set of other SJ-inner tables. E.g. it doesn't work for
            A SEMI JOIN (B LEFT JOIN C) with B as LooseScan table. So:
            - if we're now at the second SJ-inner table (1) , and
            - this table belongs to a join nest which is outer-joined to
            the first SJ-inner table (2), or is directly outer-joined to the
            first SJ-inner table (3),
            - then both tables are not inner-joined together and LooseScan is
            impossible.
          */
          if (idx == pos->first_loosescan_table + 1 &&  // (1)
              ((pos->table->table_ref->outer_join_nest() !=
                join->positions[pos->first_loosescan_table]
                    .table->table_ref->outer_join_nest())  // (2)
               || pos->table->table_ref->outer_join))      // (3)
            pos->first_loosescan_table = MAX_TABLES;
        }
      } else {
        // Stage 3: Accept outer dependent and non-dependent tables:
        DBUG_ASSERT(emb_sj_nest != first_emb_sj_nest);
        if (emb_sj_nest != nullptr) pos->first_loosescan_table = MAX_TABLES;
      }
    }

    /*
      We may consider the LooseScan strategy if
      1a. The next table is an SJ-inner table, and
      1b. LooseScan is enabled for this SJ nest, and
      2. We have no more than 64 IN expressions (must fit in bitmap), and
      3. It is the first table from that semijoin, and
      4. We're not within a semi-join range, except
      new_join_tab->emb_sj_nest (which we've just entered, see #3), and
      5. All non-IN-equality correlation references from this sj-nest are
      bound, and
      6. But some of the IN-equalities aren't (so this can't be handled by
      FirstMatch strategy), and
      7. There are equalities (including maybe semi-join ones) which can be
      handled with an index of this table, and
      8. Not a derived table/view. (a temporary restriction)
    */
    if (emb_sj_nest &&  // (1a)
        emb_sj_nest->nested_join->sj_enabled_strategies &
            OPTIMIZER_SWITCH_LOOSE_SCAN &&                          // (1b)
        emb_sj_nest->nested_join->sj_inner_exprs.elements <= 64 &&  // (2)
        ((remaining_tables_incl & emb_sj_nest->sj_inner_tables) ==  // (3)
         emb_sj_nest->sj_inner_tables) &&                           // (3)
        pos->dups_producing_tables == 0 &&                          // (4)
        !(remaining_tables_incl &
          emb_sj_nest->nested_join->sj_corr_tables) &&  // (5)
        (remaining_tables_incl &
         emb_sj_nest->nested_join->sj_depends_on) &&       // (6)
        new_join_tab->keyuse() != nullptr &&               // (7)
        !new_join_tab->table_ref->uses_materialization())  // (8)
    {
      // start considering using LooseScan strategy
      pos->first_loosescan_table = idx;
      pos->loosescan_need_tables = emb_sj_nest->sj_inner_tables |
                                   emb_sj_nest->nested_join->sj_depends_on;
    }

    if ((pos->first_loosescan_table != MAX_TABLES) &&
        !(remaining_tables & pos->loosescan_need_tables)) {
      /*
        Ok we have all LooseScan sj-nest's inner tables and outer correlated
        tables into the prefix.
      */

      // Got a complete LooseScan range. Calculate access paths and cost
      double cost, rowcount;
      Opt_trace_object trace_one_strategy(trace);
      trace_one_strategy.add_alnum("strategy", "LooseScan");
      /*
        The same problem as with FirstMatch - we need to save POSITIONs
        somewhere but reserving space for all cases would require too
        much space. We will re-calculate POSITION structures later on.
        If this function returns 'false', it means LS is impossible (didn't
        find a suitable index, etc).
      */
      if (semijoin_firstmatch_loosescan_access_paths(pos->first_loosescan_table,
                                                     idx, remaining_tables,
                                                     true, &rowcount, &cost)) {
        /*
          We don't yet have any other strategies that could handle this
          semi-join nest (the other options are Duplicate Elimination or
          Materialization, which need at least the same set of tables in
          the join prefix to be considered) so unconditionally pick the
          LooseScan.
        */
        sj_strategy = SJ_OPT_LOOSE_SCAN;
        best_cost = cost;
        best_rowcount = rowcount;
        trace_one_strategy.add("cost", best_cost).add("rows", best_rowcount);
        handled_by_fm_or_ls = join->positions[pos->first_loosescan_table]
                                  .table->emb_sj_nest->sj_inner_tables;
      }
      trace_one_strategy.add("chosen", sj_strategy == SJ_OPT_LOOSE_SCAN);
    }
  }

  if (emb_sj_nest) pos->dups_producing_tables |= emb_sj_nest->sj_inner_tables;

  pos->dups_producing_tables &= ~handled_by_fm_or_ls;

  /* MaterializeLookup and MaterializeScan strategy handler */
  const int sjm_strategy = semijoin_order_allows_materialization(
      join, remaining_tables, new_join_tab, idx);
  if (sjm_strategy == SJ_OPT_MATERIALIZE_SCAN) {
    /*
      We cannot evaluate this option now. This is because we cannot
      account for fanout of sj-inner tables yet:

        ntX  SJM-SCAN(it1 ... itN) | ot1 ... otN  |
                                   ^(1)           ^(2)

      we're now at position (1). SJM temptable in general has multiple
      records, so at point (1) we'll get the fanout from sj-inner tables (ie
      there will be multiple record combinations).

      The final join result will not contain any semi-join produced
      fanout, i.e. tables within SJM-SCAN(...) will not contribute to
      the cardinality of the join output.  Extra fanout produced by
      SJM-SCAN(...) will be 'absorbed' into fanout produced by ot1 ...  otN.

      The simple way to model this is to remove SJM-SCAN(...) fanout once
      we reach the point #2.
    */
    if (pos->sjm_scan_need_tables && emb_sj_nest != nullptr &&
        emb_sj_nest !=
            join->positions[pos->sjm_scan_last_inner].table->emb_sj_nest)
      /*
        Prevent that inner tables of different semijoin nests are
        interleaved for MatScan.
      */
      pos->sjm_scan_need_tables = 0;
    else {
      pos->sjm_scan_need_tables = emb_sj_nest->sj_inner_tables |
                                  emb_sj_nest->nested_join->sj_depends_on;
      pos->sjm_scan_last_inner = idx;
      Opt_trace_object(trace)
          .add_alnum("strategy", "MaterializeScan")
          .add_alnum("choice", "deferred");
    }
  } else if (sjm_strategy == SJ_OPT_MATERIALIZE_LOOKUP) {
    // Calculate access paths and cost for MaterializeLookup strategy
    double cost, rowcount;
    semijoin_mat_lookup_access_paths(idx, emb_sj_nest, &rowcount, &cost);

    Opt_trace_object trace_one_strategy(trace);
    trace_one_strategy.add_alnum("strategy", "MaterializeLookup")
        .add("cost", cost)
        .add("rows", rowcount)
        .add("duplicate_tables_left", pos->dups_producing_tables != 0);
    if (cost < best_cost || pos->dups_producing_tables) {
      /*
        NOTE: When we pick to use SJM[-Scan] we don't memcpy its POSITION
        elements to join->positions as that makes it hard to return things
        back when making one step back in join optimization. That's done
        after the QEP has been chosen.
      */
      sj_strategy = SJ_OPT_MATERIALIZE_LOOKUP;
      best_cost = cost;
      best_rowcount = rowcount;
      pos->dups_producing_tables &= ~emb_sj_nest->sj_inner_tables;
    }
    trace_one_strategy.add("chosen", sj_strategy == SJ_OPT_MATERIALIZE_LOOKUP);
  }

  /* MaterializeScan second phase check */
  /*
    The optimizer does not support that we have inner tables from more
    than one semi-join nest within the table range.
  */
  if (pos->sjm_scan_need_tables && emb_sj_nest != nullptr &&
      emb_sj_nest !=
          join->positions[pos->sjm_scan_last_inner].table->emb_sj_nest)
    pos->sjm_scan_need_tables = 0;

  if (pos->sjm_scan_need_tables && /* Have SJM-Scan prefix */
      !(pos->sjm_scan_need_tables & remaining_tables)) {
    TABLE_LIST *const sjm_nest =
        join->positions[pos->sjm_scan_last_inner].table->emb_sj_nest;

    double cost, rowcount;

    Opt_trace_object trace_one_strategy(trace);
    trace_one_strategy.add_alnum("strategy", "MaterializeScan");

    semijoin_mat_scan_access_paths(pos->sjm_scan_last_inner, idx,
                                   remaining_tables, sjm_nest, &rowcount,
                                   &cost);
    trace_one_strategy.add("cost", cost)
        .add("rows", rowcount)
        .add("duplicate_tables_left", pos->dups_producing_tables != 0);
    /*
      Use the strategy if
       * it is cheaper then what we've had, or
       * we haven't picked any other semi-join strategy yet
      In the second case, we pick this strategy unconditionally because
      comparing cost without semi-join duplicate removal with cost with
      duplicate removal is not an apples-to-apples comparison.
    */
    if (cost < best_cost || pos->dups_producing_tables) {
      sj_strategy = SJ_OPT_MATERIALIZE_SCAN;
      best_cost = cost;
      best_rowcount = rowcount;
      pos->dups_producing_tables &= ~sjm_nest->sj_inner_tables;
    }
    trace_one_strategy.add("chosen", sj_strategy == SJ_OPT_MATERIALIZE_SCAN);
  }

  /* Duplicate Weedout strategy handler */
  {
    /*
       Duplicate weedout can be applied after all ON-correlated and
       correlated
    */
    if (emb_sj_nest) {
      if (!pos->dupsweedout_tables) pos->first_dupsweedout_table = idx;

      pos->dupsweedout_tables |= emb_sj_nest->sj_inner_tables |
                                 emb_sj_nest->nested_join->sj_depends_on;
    }

    if (pos->dupsweedout_tables &&
        !(remaining_tables & pos->dupsweedout_tables)) {
      Opt_trace_object trace_one_strategy(trace);
      trace_one_strategy.add_alnum("strategy", "DuplicatesWeedout");
      /*
        Ok, reached a state where we could put a dups weedout point.
        Walk back and calculate
          - the join cost (this is needed as the accumulated cost may assume
            some other duplicate elimination method)
          - extra fanout that will be removed by duplicate elimination
          - duplicate elimination cost
        There are two cases:
          1. We have other strategy/ies to remove all of the duplicates.
          2. We don't.

        We need to calculate the cost in case #2 also because we need to make
        choice between this join order and others.
      */
      double rowcount, cost;
      semijoin_dupsweedout_access_paths(pos->first_dupsweedout_table, idx,
                                        &rowcount, &cost);
      /*
        Use the strategy if
         * it is cheaper then what we've had, and strategy is enabled, or
         * we haven't picked any other semi-join strategy yet
        The second part is necessary because this strategy is the last one
        to consider (it needs "the most" tables in the prefix) and we can't
        leave duplicate-producing tables not handled by any strategy.
      */
      trace_one_strategy.add("cost", cost)
          .add("rows", rowcount)
          .add("duplicate_tables_left", pos->dups_producing_tables != 0);
      if ((cost < best_cost &&
           join->positions[pos->first_dupsweedout_table]
                   .table->emb_sj_nest->nested_join->sj_enabled_strategies &
               OPTIMIZER_SWITCH_DUPSWEEDOUT) ||
          pos->dups_producing_tables) {
        sj_strategy = SJ_OPT_DUPS_WEEDOUT;
        best_cost = cost;
        best_rowcount = rowcount;
        /*
          Note, dupsweedout_tables contains inner and outer tables, even though
          "dups_producing_tables" are always inner table. Ok for this use.
        */
        pos->dups_producing_tables &= ~pos->dupsweedout_tables;
      }
      trace_one_strategy.add("chosen", sj_strategy == SJ_OPT_DUPS_WEEDOUT);
    }
  }
  pos->sj_strategy = sj_strategy;
  /*
    If a semi-join strategy is chosen, update cost and rowcount in positions
    as well. These values may be used as prefix cost and rowcount for later
    semi-join calculations, e.g for plans like "ot1 - it1 - it2 - ot2",
    where we have two semi-join nests containing it1 and it2, respectively,
    and we have a dependency between ot1 and it1, and between ot2 and it2.
    When looking at a semi-join plan for "it2 - ot2", the correct prefix cost
   (located in the join_tab for it1) must be filled in properly.

    Tables in a semijoin range, except the last in range, won't have their
    prefix_costs changed below; this is normal: when we process them, this is
    a regular join so regular costs calculated in best_ext...() are ok;
    duplicates elimination happens only at the last table in range, so it
    makes sense to correct prefix_costs of that last table.
  */
  if (sj_strategy != SJ_OPT_NONE)
    pos->set_prefix_cost(best_cost, best_rowcount);
}

/**
  Nested joins perspective: Remove the last table from the join order.

  @details
  Remove the last table from the partial join order and update the nested
  joins counters and cur_embedding_map. It is ok to call this
  function for the first table in join order (for which
  check_interleaving_with_nj has not been called)

  This function rolls back changes done by:
   - check_interleaving_with_nj(): removes the last table from the partial join
     order and update the nested joins counters and cur_embedding_map. It
     is ok to call this for the first table in join order (for which
     check_interleaving_with_nj() has not been called).

  The algorithm is the reciprocal of check_interleaving_with_nj(), hence
  parent join nest nodes are updated only when the last table in its child
  node is removed. The ASCII graphic below will clarify.

  %A table nesting such as <tt> t1 x [ ( t2 x t3 ) x ( t4 x t5 ) ] </tt>is
  represented by the below join nest tree.

  @verbatim
                     NJ1
                  _/ /  \
                _/  /    NJ2
              _/   /     / \
             /    /     /   \
   t1 x [ (t2 x t3) x (t4 x t5) ]
  @endverbatim

  At the point in time when check_interleaving_with_nj() adds the table t5 to
  the query execution plan, QEP, it also directs the node named NJ2 to mark
  the table as covered. NJ2 does so by incrementing its @c counter
  member. Since all of NJ2's tables are now covered by the QEP, the algorithm
  proceeds up the tree to NJ1, incrementing its counter as well. All join
  nests are now completely covered by the QEP.

  backout_nj_state() does the above in reverse. As seen above, the node
  NJ1 contains the nodes t2, t3, and NJ2. Its counter being equal to 3 means
  that the plan covers t2, t3, and NJ2, @e and that the sub-plan (t4 x t5)
  completely covers NJ2. The removal of t5 from the partial plan will first
  decrement NJ2's counter to 1. It will then detect that NJ2 went from being
  completely to partially covered, and hence the algorithm must continue
  upwards to NJ1 and decrement its counter to 2. A subsequent removal of t4
  will however not influence NJ1 since it did not un-cover the last table in
  NJ2.

  @param remaining_tables remaining tables to optimize, must contain 'tab'
  @param tab              join table to remove, assumed to be the last in
                          current partial join order.
*/

void Optimize_table_order::backout_nj_state(const table_map remaining_tables
                                                MY_ATTRIBUTE((unused)),
                                            const JOIN_TAB *tab) {
  DBUG_ASSERT(remaining_tables & tab->table_ref->map());

  /* Restore the nested join state */
  TABLE_LIST *last_emb = tab->table_ref->embedding;

  for (; last_emb != emb_sjm_nest; last_emb = last_emb->embedding) {
    // Ignore join nests that are not outer joins.
    if (!last_emb->join_cond_optim()) continue;

    NESTED_JOIN *const nest = last_emb->nested_join;

    DBUG_ASSERT(nest->nj_counter > 0);

    cur_embedding_map |= nest->nj_map;

    bool was_fully_covered = nest->nj_total == nest->nj_counter;

    if (--nest->nj_counter == 0) cur_embedding_map &= ~nest->nj_map;

    if (!was_fully_covered) break;
  }
}

/**
   Helper function to write the current plan's prefix to the optimizer trace.
*/
static void trace_plan_prefix(JOIN *join, uint idx, table_map excluded_tables) {
  THD *const thd = join->thd;
  Opt_trace_array plan_prefix(&thd->opt_trace, "plan_prefix");
  for (uint i = 0; i < idx; i++) {
    TABLE_LIST *const tr = join->positions[i].table->table_ref;
    if (!(tr->map() & excluded_tables)) {
      StringBuffer<32> str;
      tr->print(
          thd, &str,
          enum_query_type(QT_TO_SYSTEM_CHARSET | QT_SHOW_SELECT_NUMBER |
                          QT_NO_DEFAULT_DB | QT_DERIVED_TABLE_ONLY_ALIAS));
      plan_prefix.add_utf8(str.ptr(), str.length());
    }
  }
}

/**
  @} (end of group Query_Planner)
*/
