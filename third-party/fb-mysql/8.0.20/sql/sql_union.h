/* Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_UNION_INCLUDED
#define SQL_UNION_INCLUDED

#include "my_base.h"
#include "my_compiler.h"
#include "my_inttypes.h"
#include "sql/query_result.h"  // Query_result_interceptor
#include "sql/table.h"
#include "sql/temp_table_param.h"  // Temp_table_param

class Item;
class SELECT_LEX_UNIT;
class THD;
template <class T>
class List;

class Query_result_union : public Query_result_interceptor {
  Temp_table_param tmp_table_param;
  /// Count of rows successfully stored in tmp table
  ha_rows m_rows_in_table;

 public:
  TABLE *table;

  Query_result_union()
      : Query_result_interceptor(), m_rows_in_table(0), table(nullptr) {}
  bool prepare(THD *thd, List<Item> &list, SELECT_LEX_UNIT *u) override;
  /**
    Do prepare() if preparation has been postponed until column type
    information is computed (used by Query_result_union_direct).

    @param thd   Thread handle
    @param types Column types

    @return false on success, true on failure
  */
  virtual bool postponed_prepare(THD *thd MY_ATTRIBUTE((unused)),
                                 List<Item> &types MY_ATTRIBUTE((unused))) {
    return false;
  }
  bool send_data(THD *thd, List<Item> &items) override;
  bool send_eof(THD *thd) override;
  virtual bool flush();
  void cleanup(THD *) override { (void)reset(); }
  bool reset() override;
  bool create_result_table(THD *thd, List<Item> *column_types, bool is_distinct,
                           ulonglong options, const char *alias,
                           bool bit_fields_as_long, bool create_table);
  friend bool TABLE_LIST::create_materialized_table(THD *thd);
  virtual const ha_rows *row_count() const override { return &m_rows_in_table; }
};

#endif /* SQL_UNION_INCLUDED */
