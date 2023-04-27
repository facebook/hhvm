/* Copyright (c) 2002, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/item_row.h"

#include <stddef.h>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"
#include "sql/current_thd.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/thr_malloc.h"
#include "sql_string.h"

Item_row::Item_row(const POS &pos, Item *head, List<Item> &tail)
    : super(pos),
      used_tables_cache(0),
      not_null_tables_cache(0),
      with_null(false) {
  // TODO: think placing 2-3 component items in item (as it done for function)
  arg_count = 1 + tail.elements;
  items = (Item **)(*THR_MALLOC)->Alloc(sizeof(Item *) * arg_count);
  if (items == nullptr) {
    arg_count = 0;
    return;  // OOM
  }
  items[0] = head;
  List_iterator<Item> li(tail);
  uint i = 1;
  Item *item;
  while ((item = li++)) {
    items[i] = item;
    i++;
  }
}

Item_row::Item_row(Item *head, List<Item> &tail)
    : used_tables_cache(0), not_null_tables_cache(0), with_null(false) {
  // TODO: think placing 2-3 component items in item (as it done for function)
  arg_count = 1 + tail.elements;
  items = (Item **)(*THR_MALLOC)->Alloc(sizeof(Item *) * arg_count);
  if (items == nullptr) {
    arg_count = 0;
    return;  // OOM
  }
  items[0] = head;
  List_iterator<Item> li(tail);
  uint i = 1;
  Item *item;
  while ((item = li++)) {
    items[i] = item;
    i++;
  }
}

bool Item_row::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  for (uint i = 0; i < arg_count; i++) {
    if (items[i]->itemize(pc, &items[i])) return true;
  }
  return false;
}

void Item_row::illegal_method_call(
    const char *method MY_ATTRIBUTE((unused))) const {
  DBUG_TRACE;
  DBUG_PRINT("error", ("!!! %s method was called for row item", method));
  DBUG_ASSERT(0);
  my_error(ER_OPERAND_COLUMNS, MYF(0), 1);
}

bool Item_row::fix_fields(THD *thd, Item **) {
  DBUG_ASSERT(fixed == 0);
  null_value = false;
  maybe_null = false;
  Item **arg, **arg_end;
  for (arg = items, arg_end = items + arg_count; arg != arg_end; arg++) {
    if ((!(*arg)->fixed && (*arg)->fix_fields(thd, arg))) return true;
    // we can't assign 'item' before, because fix_fields() can change arg
    Item *item = *arg;
    used_tables_cache |= item->used_tables();

    not_null_tables_cache |= item->not_null_tables();

    if (const_item()) {
      if (item->cols() > 1)
        with_null |= item->null_inside();
      else
        with_null |= item->is_null();
    }

    // item->is_null() may have raised an error.
    if (thd->is_error()) return true;

    maybe_null |= item->maybe_null;
    add_accum_properties(item);
  }
  fixed = true;
  return false;
}

void Item_row::cleanup() {
  DBUG_TRACE;

  Item::cleanup();
  /* Reset to the original values */
  used_tables_cache = 0;
  with_null = false;
}

void Item_row::split_sum_func(THD *thd, Ref_item_array ref_item_array,
                              List<Item> &fields) {
  Item **arg, **arg_end;
  for (arg = items, arg_end = items + arg_count; arg != arg_end; arg++)
    (*arg)->split_sum_func2(thd, ref_item_array, fields, arg, true);
}

void Item_row::update_used_tables() {
  used_tables_cache = 0;
  m_accum_properties = 0;
  not_null_tables_cache = 0;
  for (uint i = 0; i < arg_count; i++) {
    items[i]->update_used_tables();
    used_tables_cache |= items[i]->used_tables();
    not_null_tables_cache |= items[i]->not_null_tables();
    add_accum_properties(items[i]);
  }
}

void Item_row::fix_after_pullout(SELECT_LEX *parent_select,
                                 SELECT_LEX *removed_select) {
  used_tables_cache = 0;
  not_null_tables_cache = 0;
  for (uint i = 0; i < arg_count; i++) {
    items[i]->fix_after_pullout(parent_select, removed_select);
    used_tables_cache |= items[i]->used_tables();
    not_null_tables_cache |= items[i]->not_null_tables();
  }
}

bool Item_row::check_cols(uint c) {
  if (c != arg_count) {
    my_error(ER_OPERAND_COLUMNS, MYF(0), c);
    return true;
  }
  return false;
}

void Item_row::print(const THD *thd, String *str,
                     enum_query_type query_type) const {
  str->append('(');
  for (uint i = 0; i < arg_count; i++) {
    if (i) str->append(',');
    items[i]->print(thd, str, query_type);
  }
  str->append(')');
}

bool Item_row::walk(Item_processor processor, enum_walk walk, uchar *arg) {
  if ((walk & enum_walk::PREFIX) && (this->*processor)(arg)) return true;

  for (uint i = 0; i < arg_count; i++) {
    if (items[i]->walk(processor, walk, arg)) return true;
  }
  return (walk & enum_walk::POSTFIX) && (this->*processor)(arg);
}

Item *Item_row::transform(Item_transformer transformer, uchar *arg) {
  for (uint i = 0; i < arg_count; i++) {
    Item *new_item = items[i]->transform(transformer, arg);
    if (new_item == nullptr) return nullptr; /* purecov: inspected */

    /*
      THD::change_item_tree() should be called only if the tree was
      really transformed, i.e. when a new item has been created.
      Otherwise we'll be allocating a lot of unnecessary memory for
      change records at each execution.
    */
    if (items[i] != new_item)
      current_thd->change_item_tree(&items[i], new_item);
  }
  return (this->*transformer)(arg);
}

void Item_row::bring_value() {
  for (uint i = 0; i < arg_count; i++) items[i]->bring_value();
}
