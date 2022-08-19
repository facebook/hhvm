/* Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_CALL_INCLUDED
#define SQL_CALL_INCLUDED

#include "my_sqlcommand.h"
#include "sql/sql_cmd_dml.h"  // Sql_cmd_dml

class Item;
class THD;
class sp_name;
template <class T>
class List;

class Sql_cmd_call : public Sql_cmd_dml {
 public:
  explicit Sql_cmd_call(sp_name *proc_name_arg, List<Item> *prog_args_arg)
      : Sql_cmd_dml(), proc_name(proc_name_arg), proc_args(prog_args_arg) {}

  virtual enum_sql_command sql_command_code() const { return SQLCOM_CALL; }

  virtual bool is_data_change_stmt() const { return false; }

 protected:
  virtual bool precheck(THD *thd);

  virtual bool prepare_inner(THD *thd);

  virtual bool execute_inner(THD *thd);

 private:
  sp_name *proc_name;
  List<Item> *proc_args;
};

#endif /* SQL_CALL_INCLUDED */
