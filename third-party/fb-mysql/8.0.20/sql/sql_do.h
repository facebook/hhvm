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

#ifndef SQL_DO_INCLUDED
#define SQL_DO_INCLUDED

#include <sys/types.h>

#include "my_sqlcommand.h"
#include "sql/query_result.h"
#include "sql/sql_select.h"

class Item;
class THD;
template <class T>
class List;

class Sql_cmd_do final : public Sql_cmd_select {
 public:
  explicit Sql_cmd_do(Query_result *result_arg) : Sql_cmd_select(result_arg) {}

  enum_sql_command sql_command_code() const override { return SQLCOM_DO; }

  const MYSQL_LEX_CSTRING *eligible_secondary_storage_engine() const override {
    return nullptr;
  }
};

class Query_result_do final : public Query_result {
 public:
  Query_result_do() : Query_result() {}
  bool send_result_set_metadata(THD *, List<Item> &, uint) override {
    return false;
  }
  bool send_data(THD *thd, List<Item> &items) override;
  bool send_eof(THD *thd) override;
  bool check_simple_select() const override { return false; }
  void abort_result_set(THD *) override {}
  void cleanup(THD *) override {}
};

#endif /* SQL_DO_INCLUDED */
