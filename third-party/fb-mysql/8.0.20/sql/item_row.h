#ifndef ITEM_ROW_INCLUDED
#define ITEM_ROW_INCLUDED

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

#include <sys/types.h>

#include "my_compiler.h"
#include "my_inttypes.h"
#include "my_table_map.h"
#include "my_time.h"
#include "mysql/udf_registration_types.h"
#include "mysql_time.h"
#include "sql/enum_query_type.h"
#include "sql/item.h"  // Item
#include "sql/parse_tree_node_base.h"

class SELECT_LEX;
class Send_field;
class String;
class THD;
class my_decimal;
template <class T>
class List;

/**
   Item which stores (x,y,...) and ROW(x,y,...).
   Note that this can be recursive: ((x,y),(z,t)) is a ROW of ROWs.
*/
class Item_row : public Item {
  typedef Item super;

  Item **items;
  table_map used_tables_cache, not_null_tables_cache;
  uint arg_count;
  /**
     If elements are made only of constants, of which one or more are
     NULL. For example, this item is (1,2,NULL), or ( (1,NULL), (2,3) ).
  */
  bool with_null;

 public:
  /**
    Row items used for comparing rows and IN operations on rows:

    @param pos    current parse context
    @param head   first column in the row
    @param tail   rest of columns in the row

    @verbatim
    (a, b, c) > (10, 10, 30)
    (a, b, c) = (select c, d, e, from t1 where x=12)
    (a, b, c) IN ((1,2,2), (3,4,5), (6,7,8)
    (a, b, c) IN (select c, d, e, from t1)
    @endverbatim

    @todo
      think placing 2-3 component items in item (as it done for function
  */
  Item_row(const POS &pos, Item *head, List<Item> &tail);
  Item_row(Item *head, List<Item> &tail);
  Item_row(Item_row *item)
      : Item(),
        items(item->items),
        used_tables_cache(item->used_tables_cache),
        not_null_tables_cache(0),
        arg_count(item->arg_count),
        with_null(false) {}

  bool itemize(Parse_context *pc, Item **res) override;

  enum Type type() const override { return ROW_ITEM; }
  void illegal_method_call(const char *) const MY_ATTRIBUTE((cold));
  bool is_null() override { return null_value; }
  void make_field(Send_field *) override { illegal_method_call("make_field"); }
  double val_real() override {
    illegal_method_call("val_real");
    return 0;
  }
  longlong val_int() override {
    illegal_method_call("val_int");
    return 0;
  }
  String *val_str(String *) override {
    illegal_method_call("val_str");
    return nullptr;
  }
  my_decimal *val_decimal(my_decimal *) override {
    illegal_method_call("val_decimal");
    return nullptr;
  }
  bool get_date(MYSQL_TIME *, my_time_flags_t) override {
    illegal_method_call("get_date");
    return true;
  }
  bool get_time(MYSQL_TIME *) override {
    illegal_method_call("get_time");
    return true;
  }

  bool fix_fields(THD *thd, Item **ref) override;
  void fix_after_pullout(SELECT_LEX *parent_select,
                         SELECT_LEX *removed_select) override;
  void cleanup() override;
  void split_sum_func(THD *thd, Ref_item_array ref_item_array,
                      List<Item> &fields) override;
  table_map used_tables() const override { return used_tables_cache; }
  enum Item_result result_type() const override { return ROW_RESULT; }
  void update_used_tables() override;
  table_map not_null_tables() const override { return not_null_tables_cache; }
  void print(const THD *thd, String *str,
             enum_query_type query_type) const override;

  bool walk(Item_processor processor, enum_walk walk, uchar *arg) override;
  Item *transform(Item_transformer transformer, uchar *arg) override;

  uint cols() const override { return arg_count; }
  Item *element_index(uint i) override { return items[i]; }
  Item **addr(uint i) override { return items + i; }
  bool check_cols(uint c) override;
  bool null_inside() override { return with_null; }
  void bring_value() override;
  bool check_function_as_value_generator(uchar *) override { return false; }
};

#endif /* ITEM_ROW_INCLUDED */
