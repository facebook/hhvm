/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/table_function.h"

#include <string.h>
#include <memory>
#include <new>
#include <utility>

#include "field_types.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_sys.h"
#include "mysql/psi/psi_base.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "prealloced_array.h"
#include "sql/error_handler.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/item_json_func.h"
#include "sql/json_dom.h"
#include "sql/json_path.h"
#include "sql/psi_memory_key.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_exception_handler.h"
#include "sql/sql_list.h"
#include "sql/sql_show.h"
#include "sql/sql_table.h"      // create_typelib
#include "sql/sql_tmp_table.h"  // create_tmp_table_from_fields
#include "sql/table.h"
#include "sql/thd_raii.h"
#include "sql_string.h"

/******************************************************************************
  Implementation of Table_function
******************************************************************************/

bool Table_function::create_result_table(ulonglong options,
                                         const char *table_alias) {
  DBUG_ASSERT(table == nullptr);

  table = create_tmp_table_from_fields(thd, *get_field_list(), false, options,
                                       table_alias);
  return table == nullptr;
}

bool Table_function::write_row() {
  int error;

  if ((error = table->file->ha_write_row(table->record[0]))) {
    if (!table->file->is_ignorable_error(error) &&
        create_ondisk_from_heap(thd, table, error, true, nullptr))
      return true;  // Not a table_is_full error
  }
  return false;
}

void Table_function::empty_table() {
  DBUG_ASSERT(table->is_created());
  (void)table->empty_result_table();
}

bool Table_function::init_args() {
  if (inited) return false;
  if (do_init_args()) return true;
  table->pos_in_table_list->dep_tables |= used_tables();
  inited = true;
  return false;
}

/******************************************************************************
  Implementation of JSON_TABLE function
******************************************************************************/
Table_function_json::Table_function_json(THD *thd_arg, const char *alias,
                                         Item *a, List<Json_table_column> *cols)
    : Table_function(thd_arg),
      m_columns(cols),
      m_all_columns(thd->mem_root),
      m_table_alias(alias),
      is_source_parsed(false),
      source(a) {}

bool Table_function_json::walk(Item_processor processor, enum_walk walk,
                               uchar *arg) {
  // Only 'source' may reference columns of other tables; rest is literals.
  return source->walk(processor, walk, arg);
}

List<Create_field> *Table_function_json::get_field_list() {
  // It's safe as Json_table_column is derived from Create_field
  return reinterpret_cast<List<Create_field> *>(&m_vt_list);
}

/**
  Initialize columns and lists for json table

  @details This function does several things:
  1) sets up list of fields (vt_list) for result table creation
  2) fills array of all columns (m_all_columns) for execution
  3) for each column that has default ON EMPTY or ON ERROR clauses, checks
    the value to be proper json and initializes column appropriately
  4) for each column that involves path, the path is checked to be correct.
  The function goes recursively, starting from the top NESTED PATH clause
  and going in the depth-first way, traverses the tree of columns.

  @param nest_idx  index of parent's element in the nesting data array
  @param parent    Parent of the NESTED PATH clause being initialized

  @returns
    false  ok
    true   an error occurred
*/

bool Table_function_json::init_json_table_col_lists(uint *nest_idx,
                                                    Json_table_column *parent) {
  List_iterator<Json_table_column> li(*parent->m_nested_columns);
  Json_table_column *col;
  const uint current_nest_idx = *nest_idx;
  // Used to set fast track between sibling NESTED PATH nodes
  Json_table_column *nested = nullptr;
  /*
    This need to be set up once per statement, as it doesn't change between
    EXECUTE calls.
  */
  Prepared_stmt_arena_holder ps_arena_holder(thd);

  while ((col = li++)) {
    String buffer;
    col->is_unsigned = (col->flags & UNSIGNED_FLAG);
    col->m_jds_elt = &m_jds[current_nest_idx];
    if (col->m_jtc_type != enum_jt_column::JTC_NESTED_PATH) {
      col->m_field_idx = m_vt_list.elements;
      m_vt_list.push_back(col);
      if (check_column_name(col->field_name)) {
        my_error(ER_WRONG_COLUMN_NAME, MYF(0), col->field_name);
        return true;
      }
      if ((col->sql_type == MYSQL_TYPE_ENUM ||
           col->sql_type == MYSQL_TYPE_SET) &&
          !col->interval)
        col->interval = create_typelib(thd->mem_root, col);
    }
    m_all_columns.push_back(col);

    switch (col->m_jtc_type) {
      case enum_jt_column::JTC_ORDINALITY: {
        // No special handling is needed
        break;
      }
      case enum_jt_column::JTC_PATH: {
        const String *path = col->m_path_string->val_str(&buffer);
        DBUG_ASSERT(path != nullptr);
        if (parse_path(*path, false, &col->m_path_json)) return true;
        if (col->m_on_empty == enum_jtc_on::JTO_DEFAULT) {
          const String *default_string =
              col->m_default_empty_string->val_str(&buffer);
          DBUG_ASSERT(default_string != nullptr);
          Json_dom_ptr dom;  //@< we'll receive a DOM here
          bool parse_error;
          if (parse_json(*default_string, 0, "JSON_TABLE", false, &dom, true,
                         &parse_error) ||
              (col->sql_type != MYSQL_TYPE_JSON && !dom->is_scalar())) {
            my_error(ER_INVALID_DEFAULT, MYF(0), col->field_name);
            return true;
          }
          col->m_default_empty_json = Json_wrapper(std::move(dom));
        }
        if (col->m_on_error == enum_jtc_on::JTO_DEFAULT) {
          const String *default_string =
              col->m_default_error_string->val_str(&buffer);
          DBUG_ASSERT(default_string != nullptr);
          Json_dom_ptr dom;  //@< we'll receive a DOM here
          bool parse_error;
          if (parse_json(*default_string, 0, "JSON_TABLE", false, &dom, true,
                         &parse_error) ||
              (col->sql_type != MYSQL_TYPE_JSON && !dom->is_scalar())) {
            my_error(ER_INVALID_DEFAULT, MYF(0), col->field_name);
            return true;
          }
          col->m_default_error_json = Json_wrapper(std::move(dom));
        }
        break;
      }
      case enum_jt_column::JTC_EXISTS: {
        const String *path = col->m_path_string->val_str(&buffer);
        DBUG_ASSERT(path != nullptr);
        if (parse_path(*path, false, &col->m_path_json)) return true;
        break;
      }
      case enum_jt_column::JTC_NESTED_PATH: {
        (*nest_idx)++;
        if (*nest_idx >= MAX_NESTED_PATH) {
          my_error(ER_JT_MAX_NESTED_PATH, MYF(0), MAX_NESTED_PATH,
                   m_table_alias);
          return true;
        }
        col->m_child_jds_elt = &m_jds[*nest_idx];

        const String *path = col->m_path_string->val_str(&buffer);
        DBUG_ASSERT(path != nullptr);
        if (nested) {
          nested->m_next_nested = col;
          col->m_prev_nested = nested;
        }
        nested = col;

        if (parse_path(*path, false, &col->m_path_json) ||
            init_json_table_col_lists(nest_idx, col))
          return true;
        break;
      }
      default:
        DBUG_ASSERT(0);
    }
  }
  return false;
}

/**
  Check whether given default values can be saved to fields

  @returns
    true    a conversion error occurred
    false   defaults can be saved or aren't specified
*/

bool Table_function_json::do_init_args() {
  DBUG_ASSERT(!is_source_parsed);

  Item *dummy = source;
  if (source->fix_fields(thd, &dummy)) return true;

  DBUG_ASSERT(source->data_type() != MYSQL_TYPE_VAR_STRING);
  if (source->has_aggregation() || source->has_subquery() || source != dummy) {
    my_error(ER_WRONG_ARGUMENTS, MYF(0), "JSON_TABLE");
    return true;
  }
  try {
    /*
      Check whether given JSON source is a const and it's valid, see also
      Table_function_json::fill_result_table().
    */
    if (source->const_item()) {
      String buf;
      Item *args[] = {source};
      if (get_json_wrapper(args, 0, &buf, func_name(), &m_jds[0].jdata))
        return true;  // Error is already thrown
      is_source_parsed = true;
    }
  } catch (...) {
    /* purecov: begin inspected */
    handle_std_exception(func_name());
    return true;
    /* purecov: end */
  }

  // Validate that all the DEFAULT values are convertible to the target type.
  for (const Json_table_column *col : m_all_columns) {
    if (col->m_jtc_type != enum_jt_column::JTC_PATH) continue;
    DBUG_ASSERT(col->m_field_idx >= 0);
    if (col->m_on_empty == enum_jtc_on::JTO_DEFAULT) {
      if (save_json_to_field(thd, get_field(col->m_field_idx),
                             &col->m_default_empty_json, false)) {
        return true;
      }
    }
    if (col->m_on_error == enum_jtc_on::JTO_DEFAULT) {
      if (save_json_to_field(thd, get_field(col->m_field_idx),
                             &col->m_default_error_json, false)) {
        return true;
      }
    }
  }
  return false;
}

bool Table_function_json::init() {
  Json_table_column top(nullptr, m_columns);
  if (m_vt_list.elements == 0) {
    uint nest_idx = 0;
    if (init_json_table_col_lists(&nest_idx, &top)) return true;
    List_iterator<Json_table_column> li(m_vt_list);

    /*
      Check for duplicate names.
      Two iterators over vt_list are used. First is used to get a field,
      second - to compare the field with fields in the rest of the list.
      For each iteration of the first list, we skip fields prior to the
      first iterator's field.
    */
    Json_table_column *first;
    while ((first = li++)) {
      Json_table_column *col;
      List_iterator<Json_table_column> li2(m_vt_list);
      // Compare 'first' with all columns prior to it
      while ((col = li2++) && col != first) {
        if (!strncmp(first->field_name, col->field_name, NAME_CHAR_LEN)) {
          my_error(ER_DUP_FIELDNAME, MYF(0), first->field_name);
          return true;
        }
      }
    }
  }
  return false;
}

/**
  A helper function which sets all columns under given NESTED PATH column
  to nullptr. Used to evaluate sibling NESTED PATHS.

  @param       root  root NESTED PATH column
  @param [out] last  last column which belongs to the given NESTED PATH
*/

void Table_function_json::set_subtree_to_null(Json_table_column *root,
                                              Json_table_column **last) {
  List_iterator<Json_table_column> li(*root->m_nested_columns);
  Json_table_column *col;
  while ((col = li++)) {
    *last = col;
    switch (col->m_jtc_type) {
      case enum_jt_column::JTC_NESTED_PATH:
        set_subtree_to_null(col, last);
        break;
      default:
        get_field(col->m_field_idx)->set_null();
        break;
    }
  }
}

/**
  Fill a json table column

  @details Fills a column with data, according to specification in
  JSON_TABLE. This function handles all kinds of columns:
  Ordinality)  just saves the counter into the column's field
  Path)        extracts value, saves it to the column's field and handles
               ON ERROR/ON EMPTY clauses
  Exists)      checks the path existence and saves either 1 or 0 into result
               field
  Nested path) matches the path expression against data source. If there're
               matches, this function sets NESTED PATH's iterator over those
               matches and resets ordinality counter.

  @param[in]   table_function the JSON table function
  @param[out]  skip  true <=> it's a NESTED PATH node and its path
                     expression didn't return any matches or a
                     previous sibling NESTED PATH clause still producing
                     records, thus all columns of this NESTED PATH node
                     should be skipped

  @returns
    false column is filled
    true  an error occurred, execution should be stopped
*/

bool Json_table_column::fill_column(Table_function_json *table_function,
                                    jt_skip_reason *skip) {
  *skip = JTS_NONE;

  Field *const fld = m_jtc_type == enum_jt_column::JTC_NESTED_PATH
                         ? nullptr
                         : table_function->get_field(m_field_idx);
  DBUG_ASSERT(m_jtc_type == enum_jt_column::JTC_NESTED_PATH ||
              (fld != nullptr && fld->field_index == m_field_idx));

  switch (m_jtc_type) {
    case enum_jt_column::JTC_ORDINALITY: {
      if (fld->store(m_jds_elt->m_rowid, true)) return true;
      fld->set_notnull();
      break;
    }
    case enum_jt_column::JTC_PATH: {
      THD *thd = fld->table->in_use;
      // Vector of matches
      Json_wrapper_vector data_v(key_memory_JSON);
      m_jds_elt->jdata.seek(m_path_json, m_path_json.leg_count(), &data_v, true,
                            false);
      if (data_v.size() > 0) {
        Json_wrapper buf;
        bool is_error = false;
        if (data_v.size() > 1) {
          // Make result array
          if (fld->type() == MYSQL_TYPE_JSON) {
            Json_array *a = new (std::nothrow) Json_array();
            if (!a) return true;
            for (Json_wrapper &w : data_v) {
              if (a->append_alias(w.clone_dom(thd))) {
                delete a; /* purecov: inspected */
                return true;
              }
            }
            buf = Json_wrapper(a);
          } else {
            is_error = true;
            // Thrown an error when save_json_to_field() isn't called
            if (m_on_error == enum_jtc_on::JTO_ERROR)
              my_error(ER_WRONG_JSON_TABLE_VALUE, MYF(0), field_name);
          }
        } else
          buf = std::move(data_v[0]);
        if (!is_error) {
          // Save the extracted value to the field in JSON_TABLE. Make sure an
          // error is raised for conversion errors if ERROR ON ERROR is
          // specified. Don't raise any warnings when DEFAULT/NULL ON ERROR is
          // specified, as they may be promoted to errors by
          // Strict_error_handler and prevent the ON ERROR clause from being
          // respected.
          Ignore_warnings_error_handler ignore_warnings;
          const bool no_error = m_on_error != enum_jtc_on::JTO_ERROR;
          if (no_error) thd->push_internal_handler(&ignore_warnings);
          is_error = save_json_to_field(thd, fld, &buf, no_error);
          if (no_error) thd->pop_internal_handler();
        }
        if (is_error) {
          switch (m_on_error) {
            case enum_jtc_on::JTO_ERROR: {
              return true;
              break;
            }
            case enum_jtc_on::JTO_DEFAULT: {
              save_json_to_field(thd, fld, &m_default_error_json, true);
              break;
            }
            case enum_jtc_on::JTO_NULL:
            default: {
              fld->set_null();
              break;
            }
          }
        }
      } else {
        switch (m_on_empty) {
          case enum_jtc_on::JTO_ERROR: {
            my_error(ER_MISSING_JSON_TABLE_VALUE, MYF(0), field_name);
            return true;
          }
          case enum_jtc_on::JTO_DEFAULT: {
            save_json_to_field(thd, fld, &m_default_empty_json, true);
            break;
          }
          case enum_jtc_on::JTO_NULL:
          default: {
            fld->set_null();
            break;
          }
        }
      }
      break;
    }
    case enum_jt_column::JTC_EXISTS: {
      // Vector of matches
      Json_wrapper_vector data_v(key_memory_JSON);
      m_jds_elt->jdata.seek(m_path_json, m_path_json.leg_count(), &data_v, true,
                            true);
      if (data_v.size() >= 1)
        fld->store(1, true);
      else
        fld->store(0, true);
      fld->set_notnull();
      break;
    }
    case enum_jt_column::JTC_NESTED_PATH: {
      // If this node sends data, advance ts iterator
      if (m_child_jds_elt->producing_records) {
        ++m_child_jds_elt->it;
        m_child_jds_elt->m_rowid++;

        if ((m_child_jds_elt->it != m_child_jds_elt->v.end()))
          m_child_jds_elt->jdata = std::move(*m_child_jds_elt->it);
        else {
          m_child_jds_elt->producing_records = false;
          *skip = JTS_EOD;
        }
        return false;
      }
      // Run only one sibling nested path at a time
      for (Json_table_column *tc = m_prev_nested; tc; tc = tc->m_prev_nested) {
        DBUG_ASSERT(tc->m_jtc_type == enum_jt_column::JTC_NESTED_PATH);
        if (tc->m_child_jds_elt->producing_records) {
          *skip = JTS_SIBLING;
          return false;
        }
      }
      m_child_jds_elt->v.clear();
      if (m_jds_elt->jdata.seek(m_path_json, m_path_json.leg_count(),
                                &m_child_jds_elt->v, true, false))
        return true;
      if (m_child_jds_elt->v.size() == 0) {
        *skip = JTS_EOD;
        return false;
      }
      m_child_jds_elt->it = m_child_jds_elt->v.begin();
      m_child_jds_elt->producing_records = true;
      m_child_jds_elt->m_rowid = 1;
      m_child_jds_elt->jdata = std::move(*m_child_jds_elt->it);
      break;
    }
    default: {
      DBUG_ASSERT(0);
      break;
    }
  }
  return false;
}

void Json_table_column::cleanup() {
  // Reset paths and wrappers to free allocated memory.
  m_path_json = Json_path();
  if (m_on_empty == enum_jtc_on::JTO_DEFAULT)
    m_default_empty_json = Json_wrapper();
  if (m_on_error == enum_jtc_on::JTO_DEFAULT)
    m_default_error_json = Json_wrapper();
}

/**
  Fill json table

  @details This function goes along the flattened list of columns and
  updates them by calling fill_column(). As it goes, it pushes all nested
  path nodes to 'nested' list, using it as a stack. After writing a row, it
  checks whether there's more data in the right-most nested path (top in the
  stack). If there is, it advances path's iterator, if no - pops the path
  from stack and goes to the next nested path (i.e more to left). When stack
  is empty, then the loop is over and all data (if any) was stored in the table,
  and function exits. Otherwise, the list of columns is positioned to the top
  nested path in the stack and incremented to the column after the nested
  path, then the loop of updating columns is executed again. So, whole
  execution could look as follows:

      columns (                      <-- npr
        cr1,
        cr2,
        nested path .. columns (     <-- np1
          c11,
          nested path .. columns (   <-- np2
            c21
          )
        )
      )

      iteration | columns updated in the loop
      1           npr cr1 cr2 np1 c11 np2 c21
      2                                   c21
      3                                   c21
      4                           c11 np2 c21
      5                                   c21
      6                           c11 np2 c21
      7                                   c21
      8           npr cr1 cr2 np1 c11 np2 c21
      9                                   c21
     10                           c11 np2 c21
  Note that result table's row isn't automatically reset and if a column
  isn't updated, its data is written multiple times. E.g. cr1 in the
  example above is updated 2 times, but is written 10 times. This allows to
  save cycles on updating fields that for sure haven't been changed.

  When there's sibling nested paths, i.e two or more nested paths in the
  same columns clause, then they're processed one at a time. Started with
  first, and the rest are set to null with help f set_subtree_to_null().
  When the first sibling nested path runs out of rows, it's set to null and
  processing moves on to the next one.

  @returns
    false table filled
    true  error occurred
*/

bool Table_function_json::fill_json_table() {
  // 'Stack' of nested NESTED PATH clauses
  Prealloced_array<uint, MAX_NESTED_PATH> nested(PSI_NOT_INSTRUMENTED);
  // The column being processed
  uint col_idx = 0;
  jt_skip_reason skip_subtree;
  const enum_check_fields check_save = thd->check_for_truncated_fields;

  do {
    skip_subtree = JTS_NONE;
    /*
      When a NESTED PATH runs out of matches, we set it to null, and
      continue filling the row, so next sibling NESTED PATH could start
      sending rows. But if there's no such NESTED PATH, then this row must be
      skipped as it's not a result of a match.
    */
    bool skip_row = true;
    for (; col_idx < m_all_columns.size(); col_idx++) {
      /*
        When NESTED PATH doesn't have a match for any reason, set its
        columns to nullptr.
      */
      Json_table_column *col = m_all_columns[col_idx];
      if (col->fill_column(this, &skip_subtree)) return true;
      if (skip_subtree) {
        set_subtree_to_null(col, &col);
        // Position iterator to the last element of subtree
        while (m_all_columns[col_idx] != col) col_idx++;
      } else if (col->m_jtc_type == enum_jt_column::JTC_NESTED_PATH) {
        nested.push_back(col_idx);
        // Found a NESTED PATH which produced a record
        skip_row = false;
      }
    }
    if (!skip_row) write_row();
    // Find next nested path and advance its iterator.
    if (nested.size() > 0) {
      uint j = nested.back();
      nested.pop_back();
      Json_table_column *col = m_all_columns[j];

      /*
        When there're sibling NESTED PATHs and the first one is producing
        records, second one will skip_subtree and we need to reset it here,
        as it's not relevant.
      */
      if (col->m_child_jds_elt->producing_records) skip_subtree = JTS_NONE;
      col_idx = j;
    }
  } while (nested.size() != 0 || skip_subtree != JTS_EOD);

  thd->check_for_truncated_fields = check_save;
  return false;
}

bool Table_function_json::fill_result_table() {
  String buf;
  DBUG_ASSERT(!table->materialized);
  // reset table
  empty_table();

  try {
    Item *args[] = {source};
    /*
      There are 3 possible cases of data source expression const-ness:

      1. Always const, e.g. a plain string, source will be parsed once at
         Table_function_json::init()
      2. Non-const during init(), but become const after it, e.g a field from a
         const table: source will be parsed here ONCE
      3. Non-const, e.g. a table field: source will be parsed here EVERY TIME
         fill_result_table() is called
    */
    if (((!source->const_item() || !is_source_parsed) &&
         get_json_wrapper(args, 0, &buf, func_name(), &m_jds[0].jdata)) ||
        args[0]->null_value)
      // No need to set null_value as it's not used by table functions
      return false;
    is_source_parsed = true;
    return fill_json_table();
  } catch (...) {
    /* purecov: begin inspected */
    handle_std_exception(func_name());
    return true;
    /* purecov: end */
  }
  return false;
}

static bool print_on_empty_error(const THD *thd, String *str,
                                 enum_query_type query_type, enum_jtc_on jto,
                                 const Item *default_string) {
  switch (jto) {
    case enum_jtc_on::JTO_ERROR:
      return str->append(STRING_WITH_LEN(" error on"));
    case enum_jtc_on::JTO_NULL:
      return str->append(STRING_WITH_LEN(" null on"));
    case enum_jtc_on::JTO_DEFAULT:
      if (str->append(STRING_WITH_LEN(" default "))) return true;
      default_string->print(thd, str, query_type);
      return str->append(STRING_WITH_LEN(" on"));
    case enum_jtc_on::JTO_IMPLICIT:
      break;
  };
  DBUG_ASSERT(false);
  return true;
}

/**
  Helper function to print a single NESTED PATH column.

  @param thd        the current session
  @param table      the TABLE object representing the JSON_TABLE expression
  @param col        the column to print
  @param query_type the type of the query
  @param str        the string to print to

  @returns true on error, false on success
*/
static bool print_nested_path(const THD *thd, const TABLE *table,
                              const Json_table_column *col,
                              enum_query_type query_type, String *str) {
  col->m_path_string->print(thd, str, query_type);
  if (str->append(STRING_WITH_LEN(" columns ("))) return true;
  bool first = true;
  for (const Json_table_column &jtc : *col->m_nested_columns) {
    if (!first && str->append(STRING_WITH_LEN(", "))) return true;
    first = false;

    switch (jtc.m_jtc_type) {
      case enum_jt_column::JTC_ORDINALITY: {
        append_identifier(thd, str, jtc.field_name, strlen(jtc.field_name));
        if (str->append(STRING_WITH_LEN(" for ordinality"))) return true;
        break;
      }
      case enum_jt_column::JTC_EXISTS:
      case enum_jt_column::JTC_PATH: {
        append_identifier(thd, str, jtc.field_name, strlen(jtc.field_name));
        if (str->append(' ')) return true;
        const Field *field = table->field[jtc.m_field_idx];
        StringBuffer<STRING_BUFFER_USUAL_SIZE> type;
        field->sql_type(type);
        if (str->append(type)) return true;
        if (field->has_charset()) {
          // Append the character set.
          if (str->append(STRING_WITH_LEN(" character set ")) ||
              str->append(field->charset()->csname))
            return true;
          // Append the collation, if it is not the primary collation of the
          // character set.
          if ((field->charset()->state & MY_CS_PRIMARY) == 0 &&
              (str->append(STRING_WITH_LEN(" collate ")) ||
               str->append(field->charset()->name)))
            return true;
        }
        if (jtc.m_jtc_type == enum_jt_column::JTC_EXISTS) {
          if (str->append(STRING_WITH_LEN(" exists"))) return true;
        }
        if (str->append(STRING_WITH_LEN(" path "))) return true;
        jtc.m_path_string->print(thd, str, query_type);
        if (jtc.m_jtc_type == enum_jt_column::JTC_EXISTS) break;
        if (jtc.m_on_empty != enum_jtc_on::JTO_IMPLICIT) {
          if (print_on_empty_error(thd, str, query_type, jtc.m_on_empty,
                                   jtc.m_default_empty_string) ||
              str->append(STRING_WITH_LEN(" empty")))
            return true;
        }
        if (jtc.m_on_error != enum_jtc_on::JTO_IMPLICIT) {
          if (print_on_empty_error(thd, str, query_type, jtc.m_on_error,
                                   jtc.m_default_error_string) ||
              str->append(STRING_WITH_LEN(" error")))
            return true;
        }
        break;
      }
      case enum_jt_column::JTC_NESTED_PATH: {
        if (str->append(STRING_WITH_LEN("nested path ")) ||
            print_nested_path(thd, table, &jtc, query_type, str))
          return true;
        break;
      }
    };
  }
  return str->append(')');
}

bool Table_function_json::print(String *str, enum_query_type query_type) const {
  if (str->append(STRING_WITH_LEN("json_table("))) return true;
  source->print(thd, str, query_type);
  return (thd->is_error() || str->append(STRING_WITH_LEN(", ")) ||
          print_nested_path(thd, table, m_columns->head(), query_type, str) ||
          str->append(')'));
}

table_map Table_function_json::used_tables() { return source->used_tables(); }

void Table_function_json::do_cleanup() {
  source->cleanup();
  is_source_parsed = false;
  for (uint i = 0; i < MAX_NESTED_PATH; i++) m_jds[i].cleanup();
  for (uint i = 0; i < m_all_columns.size(); i++) m_all_columns[i]->cleanup();
  m_all_columns.clear();
  m_vt_list.empty();
}

void JT_data_source::cleanup() {
  jdata = Json_wrapper();
  v.clear();
  v.shrink_to_fit();
  producing_records = false;
}
