/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/opt_explain_traditional.h"

#include <sys/types.h>

#include "m_ctype.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "sql/current_thd.h"
#include "sql/item.h"
#include "sql/query_result.h"
#include "sql/sql_class.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql_string.h"

/**
  Heads of "extra" column parts

  This array must be in sync with Extra_tag enum.
*/
static const char *traditional_extra_tags[ET_total] = {
    nullptr,                            // ET_none
    "Using temporary",                  // ET_USING_TEMPORARY
    "Using filesort",                   // ET_USING_FILESORT
    "Using index condition",            // ET_USING_INDEX_CONDITION
    "Using",                            // ET_USING
    "Range checked for each record",    // ET_RANGE_CHECKED_FOR_EACH_RECORD
    "Using pushed condition",           // ET_USING_PUSHED_CONDITION
    "Using where",                      // ET_USING_WHERE
    "Not exists",                       // ET_NOT_EXISTS
    "Using MRR",                        // ET_USING_MRR
    "Using index",                      // ET_USING_INDEX
    "Full scan on NULL key",            // ET_FULL_SCAN_ON_NULL_KEY
    "Using index for group-by",         // ET_USING_INDEX_FOR_GROUP_BY
    "Using index for skip scan",        // ET_USING_INDEX_FOR_SKIP_SCAN,
    "Distinct",                         // ET_DISTINCT
    "LooseScan",                        // ET_LOOSESCAN
    "Start temporary",                  // ET_START_TEMPORARY
    "End temporary",                    // ET_END_TEMPORARY
    "FirstMatch",                       // ET_FIRST_MATCH
    "Materialize",                      // ET_MATERIALIZE
    "Start materialize",                // ET_START_MATERIALIZE
    "End materialize",                  // ET_END_MATERIALIZE
    "Scan",                             // ET_SCAN
    "Using join buffer",                // ET_USING_JOIN_BUFFER
    "const row not found",              // ET_CONST_ROW_NOT_FOUND
    "unique row not found",             // ET_UNIQUE_ROW_NOT_FOUND
    "Impossible ON condition",          // ET_IMPOSSIBLE_ON_CONDITION
    "",                                 // ET_PUSHED_JOIN
    "Ft_hints:",                        // ET_FT_HINTS
    "Backward index scan",              // ET_BACKWARD_SCAN
    "Recursive",                        // ET_RECURSIVE
    "Table function:",                  // ET_TABLE_FUNCTION
    "Index dive skipped due to FORCE",  // ET_SKIP_RECORDS_IN_RANGE
    "Using secondary engine",           // ET_USING_SECONDARY_ENGINE
    "Rematerialize"                     // ET_REMATERIALIZE
};

static const char *mod_type_name[] = {"NONE", "INSERT", "UPDATE", "DELETE",
                                      "REPLACE"};

bool Explain_format_traditional::send_headers(Query_result *result) {
  return ((nil = new Item_null) == nullptr ||
          Explain_format::send_headers(result) ||
          current_thd->send_explain_fields(output));
}

static bool push(List<Item> *items, qep_row::mem_root_str &s, Item_null *nil) {
  if (s.is_empty()) return items->push_back(nil);
  Item_string *item = new Item_string(s.str, s.length, system_charset_info);
  return item == nullptr || items->push_back(item);
}

static bool push(List<Item> *items, const char *s, size_t length) {
  Item_string *item = new Item_string(s, length, system_charset_info);
  return item == nullptr || items->push_back(item);
}

static bool push(List<Item> *items, List<const char> &c, Item_null *nil) {
  if (c.is_empty()) return items->push_back(nil);

  StringBuffer<1024> buff;
  List_iterator<const char> it(c);
  const char *s;
  while ((s = it++)) {
    buff.append(s);
    buff.append(",");
  }
  if (!buff.is_empty()) buff.length(buff.length() - 1);  // remove last ","
  Item_string *item = new Item_string(buff.dup(current_thd->mem_root),
                                      buff.length(), system_charset_info);
  return item == nullptr || items->push_back(item);
}

static bool push(List<Item> *items, const qep_row::column<uint> &c,
                 Item_null *nil) {
  if (c.is_empty()) return items->push_back(nil);
  Item_uint *item = new Item_uint(c.get());
  return item == nullptr || items->push_back(item);
}

static bool push(List<Item> *items, const qep_row::column<ulonglong> &c,
                 Item_null *nil) {
  if (c.is_empty()) return items->push_back(nil);
  Item_int *item = new Item_int(c.get(), MY_INT64_NUM_DECIMAL_DIGITS);
  return item == nullptr || items->push_back(item);
}

static bool push(List<Item> *items, const qep_row::column<float> &c,
                 Item_null *nil) {
  if (c.is_empty()) return items->push_back(nil);
  Item_float *item = new Item_float(c.get(), 2);
  return item == nullptr || items->push_back(item);
}

bool Explain_format_traditional::push_select_type(List<Item> *items) {
  DBUG_ASSERT(!column_buffer.col_select_type.is_empty());
  StringBuffer<32> buff;
  if (column_buffer.is_dependent) {
    if (buff.append(STRING_WITH_LEN("DEPENDENT "), system_charset_info))
      return true;
  } else if (!column_buffer.is_cacheable) {
    if (buff.append(STRING_WITH_LEN("UNCACHEABLE "), system_charset_info))
      return true;
  }
  const enum_explain_type sel_type = column_buffer.col_select_type.get();
  const char *type = (column_buffer.mod_type != MT_NONE &&
                      (sel_type == enum_explain_type::EXPLAIN_PRIMARY ||
                       sel_type == enum_explain_type::EXPLAIN_SIMPLE))
                         ? mod_type_name[column_buffer.mod_type]
                         : SELECT_LEX::get_type_str(sel_type);

  if (buff.append(type)) return true;

  Item_string *item = new Item_string(buff.dup(current_thd->mem_root),
                                      buff.length(), system_charset_info);
  return item == nullptr || items->push_back(item);
}

class Buffer_cleanup {
 public:
  explicit Buffer_cleanup(qep_row *row) : m_row(row) {}
  ~Buffer_cleanup() { m_row->cleanup(); }

 private:
  qep_row *m_row;
};

bool Explain_format_traditional::flush_entry() {
  /*
    Buffer_cleanup will empty column_buffer upon exit. So column values start
    clear for the next row.
  */
  Buffer_cleanup bc(&column_buffer);
  List<Item> items;
  if (push(&items, column_buffer.col_id, nil) || push_select_type(&items) ||
      push(&items, column_buffer.col_table_name, nil) ||
      push(&items, column_buffer.col_partitions, nil) ||
      push(&items, column_buffer.col_join_type, nil) ||
      push(&items, column_buffer.col_possible_keys, nil) ||
      push(&items, column_buffer.col_key, nil) ||
      push(&items, column_buffer.col_key_len, nil) ||
      push(&items, column_buffer.col_ref, nil) ||
      push(&items, column_buffer.col_rows, nil) ||
      push(&items, column_buffer.col_filtered, nil))
    return true;

  if (column_buffer.col_message.is_empty() &&
      column_buffer.col_extra.is_empty()) {
    if (items.push_back(nil)) return true;
  } else if (!column_buffer.col_extra.is_empty()) {
    StringBuffer<64> buff(system_charset_info);
    List_iterator<qep_row::extra> it(column_buffer.col_extra);
    qep_row::extra *e;
    while ((e = it++)) {
      DBUG_ASSERT(traditional_extra_tags[e->tag] != nullptr);
      if (buff.append(traditional_extra_tags[e->tag])) return true;
      if (e->data) {
        bool brackets = false;
        switch (e->tag) {
          case ET_RANGE_CHECKED_FOR_EACH_RECORD:
          case ET_USING_INDEX_FOR_GROUP_BY:
          case ET_USING_JOIN_BUFFER:
          case ET_FIRST_MATCH:
          case ET_REMATERIALIZE:
            brackets = true;  // for backward compatibility
            break;
          default:
            break;
        }
        if (e->tag != ET_FIRST_MATCH &&  // for backward compatibility
            e->tag != ET_PUSHED_JOIN && buff.append(" "))
          return true;
        if (brackets && buff.append("(")) return true;
        if (buff.append(e->data)) return true;
        if (brackets && buff.append(")")) return true;
      }
      if (buff.append("; ")) return true;
    }
    if (!buff.is_empty()) buff.length(buff.length() - 2);  // remove last "; "
    if (push(&items, buff.dup(current_thd->mem_root), buff.length()))
      return true;
  } else {
    if (push(&items, column_buffer.col_message, nil)) return true;
  }

  if (output->send_data(current_thd, items)) return true;
  return false;
}
