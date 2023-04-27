/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_CMD_DML_INCLUDED
#define SQL_CMD_DML_INCLUDED

#include "my_dbug.h"
#include "sql/sql_cmd.h"

struct LEX;
class Query_result;

class Sql_cmd_dml : public Sql_cmd {
 public:
  /// @return true if data change statement, false if not (SELECT statement)
  virtual bool is_data_change_stmt() const { return true; }

  /**
    Command-specific resolving (doesn't include LEX::prepare())

    @param thd  Current THD.

    @returns false on success, true on error
  */
  virtual bool prepare(THD *thd);

  /**
    Execute this query once.

    @param thd Thread handler

    @returns false on success, true on error
  */
  virtual bool execute(THD *thd);

  virtual bool is_dml() const { return true; }

  virtual bool is_single_table_plan() const { return false; }

 protected:
  Sql_cmd_dml()
      : Sql_cmd(), lex(nullptr), result(nullptr), m_empty_query(false) {}

  /// @return true if query is guaranteed to return no data
  /**
    @todo Also check this for the following cases:
          - Empty source for multi-table UPDATE and DELETE.
          - Check empty query expression for INSERT
  */
  bool is_empty_query() const {
    DBUG_ASSERT(is_prepared());
    return m_empty_query;
  }

  /// Set statement as returning no data
  void set_empty_query() { m_empty_query = true; }

  /**
    Perform a precheck of table privileges for the specific operation.

    @details
    Check that user has some relevant privileges for all tables involved in
    the statement, e.g. SELECT privileges for tables selected from, INSERT
    privileges for tables inserted into, etc. This function will also populate
    TABLE_LIST::grant with all privileges the user has for each table, which
    is later used during checking of column privileges.
    Note that at preparation time, views are not expanded yet. Privilege
    checking is thus rudimentary and must be complemented with later calls to
    SELECT_LEX::check_view_privileges().
    The reason to call this function at such an early stage is to be able to
    quickly reject statements for which the user obviously has insufficient
    privileges.

    @param thd thread handler

    @returns false if success, true if false
  */
  virtual bool precheck(THD *thd) = 0;

  /**
    Perform the command-specific parts of DML command preparation,
    to be called from prepare()

    @param thd the current thread

    @returns false if success, true if error
  */
  virtual bool prepare_inner(THD *thd) = 0;

  /**
    The inner parts of query optimization and execution.
    Single-table DML operations needs to reimplement this.

    @param thd Thread handler

    @returns false on success, true on error
  */
  virtual bool execute_inner(THD *thd);

 protected:
  LEX *lex;              ///< Pointer to LEX for this statement
  Query_result *result;  ///< Pointer to object for handling of the result
  bool m_empty_query;    ///< True if query will produce no rows
};

#endif /* SQL_CMD_DML_INCLUDED */
