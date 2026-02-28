/* Copyright (c) 2009, 2020, Oracle and/or its affiliates. All rights reserved.

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
  @file sql/sql_cmd.h
  Representation of an SQL command.
*/

#ifndef SQL_CMD_INCLUDED
#define SQL_CMD_INCLUDED

#include "my_dbug.h"
#include "my_sqlcommand.h"
#include "sql/select_lex_visitor.h"

class THD;
class Prepared_statement;
struct handlerton;
struct MYSQL_LEX_STRING;
struct MYSQL_LEX_CSTRING;

/**
  Representation of an SQL command.

  This class is an interface between the parser and the runtime.
  The parser builds the appropriate derived classes of Sql_cmd
  to represent a SQL statement in the parsed tree.
  The execute() method in the derived classes of Sql_cmd contain the runtime
  implementation.
  Note that this interface is used for SQL statements recently implemented,
  the code for older statements tend to load the LEX structure with more
  attributes instead.
  Implement new statements by sub-classing Sql_cmd, as this improves
  code modularity (see the 'big switch' in dispatch_command()), and decreases
  the total size of the LEX structure (therefore saving memory in stored
  programs).
  The recommended name of a derived class of Sql_cmd is Sql_cmd_<derived>.

  Notice that the Sql_cmd class should not be confused with the Statement class.
  Statement is a class that is used to manage an SQL command or a set
  of SQL commands. When the SQL statement text is analyzed, the parser will
  create one or more Sql_cmd objects to represent the actual SQL commands.
*/
class Sql_cmd {
 private:
  Sql_cmd(const Sql_cmd &);   // No copy constructor wanted
  void operator=(Sql_cmd &);  // No assignment operator wanted

 public:
  /**
    @brief Return the command code for this statement
  */
  virtual enum_sql_command sql_command_code() const = 0;

  /// @return true if this statement is prepared
  bool is_prepared() const { return m_prepared; }

  /**
    Prepare this SQL statement.

    @param thd the current thread

    @returns false if success, true if error
  */
  virtual bool prepare(THD *thd MY_ATTRIBUTE((unused))) {
    // Default behavior for a statement is to have no preparation code.
    /* purecov: begin inspected */
    DBUG_ASSERT(!is_prepared());
    set_prepared();
    return false;
    /* purecov: end */
  }

  /**
    Execute this SQL statement.
    @param thd the current thread.
    @returns false if success, true if error
  */
  virtual bool execute(THD *thd) = 0;

  /**
    Command-specific reinitialization before execution of prepared statement

    @see reinit_stmt_before_use()

    @note Currently this function is overloaded for INSERT/REPLACE stmts only.

    @param thd  Current THD.
  */
  virtual void cleanup(THD *thd MY_ATTRIBUTE((unused))) {
    m_secondary_engine = nullptr;
  }

  /// Set the owning prepared statement
  void set_owner(Prepared_statement *stmt) { m_owner = stmt; }

  /// Get the owning prepared statement
  Prepared_statement *get_owner() { return m_owner; }

  /// @return true if SQL command is a DML statement
  virtual bool is_dml() const { return false; }

  /// @return true if implemented as single table plan, DML statement only
  virtual bool is_single_table_plan() const {
    /* purecov: begin inspected */
    DBUG_ASSERT(is_dml());
    return false;
    /* purecov: end */
  }

  /**
    Temporary function used to "unprepare" a prepared statement after
    preparation, so that a subsequent execute statement will reprepare it.
    This is done because UNIT::cleanup() will un-resolve all resolved QBs.
  */
  virtual void unprepare(THD *thd MY_ATTRIBUTE((unused))) {
    DBUG_ASSERT(is_prepared());
    m_prepared = false;
  }

  virtual bool accept(THD *thd MY_ATTRIBUTE((unused)),
                      Select_lex_visitor *visitor MY_ATTRIBUTE((unused))) {
    return false;
  }

  /**
    Is this statement of a type and on a form that makes it eligible
    for execution in a secondary storage engine?

    @return the name of the secondary storage engine, or nullptr if
    the statement is not eligible for execution in a secondary storage
    engine
  */
  virtual const MYSQL_LEX_CSTRING *eligible_secondary_storage_engine() const {
    return nullptr;
  }

  /**
    Disable use of secondary storage engines in this statement. After
    a call to this function, the statement will not try to use a
    secondary storage engine until it is reprepared.
  */
  void disable_secondary_storage_engine() {
    DBUG_ASSERT(m_secondary_engine == nullptr);
    m_secondary_engine_enabled = false;
  }

  /**
    Has use of secondary storage engines been disabled for this statement?
  */
  bool secondary_storage_engine_disabled() const {
    return !m_secondary_engine_enabled;
  }

  /**
    Mark the current statement as using a secondary storage engine.
    This function must be called before the statement starts opening
    tables in a secondary engine.
  */
  void use_secondary_storage_engine(const handlerton *hton) {
    DBUG_ASSERT(m_secondary_engine_enabled);
    m_secondary_engine = hton;
  }

  /**
    Is this statement using a secondary storage engine?
  */
  bool using_secondary_storage_engine() const {
    return m_secondary_engine != nullptr;
  }

  /**
    Get the handlerton of the secondary engine that is used for
    executing this statement, or nullptr if a secondary engine is not
    used.
  */
  const handlerton *secondary_engine() const { return m_secondary_engine; }

  void set_optional_transform_prepared(bool value) {
    m_prepared_with_optional_transform = value;
  }

  bool is_optional_transform_prepared() {
    return m_prepared_with_optional_transform;
  }

 protected:
  Sql_cmd() : m_owner(nullptr), m_prepared(false), prepare_only(true) {}

  virtual ~Sql_cmd() {
    /*
      Sql_cmd objects are allocated in thd->mem_root.
      In MySQL, the C++ destructor is never called, the underlying MEM_ROOT is
      simply destroyed instead.
      Do not rely on the destructor for any cleanup.
    */
    DBUG_ASSERT(false);
  }

  /**
    @return true if object represents a preparable statement, ie. a query
    that is prepared with a PREPARE statement and executed with an EXECUTE
    statement. False is returned for regular statements (non-preparable
    statements) that are executed directly.
    @todo replace with "m_owner != nullptr" when prepare-once is implemented
  */
  bool needs_explicit_preparation() const { return prepare_only; }

  /// Set this statement as prepared
  void set_prepared() { m_prepared = true; }

 private:
  Prepared_statement
      *m_owner;     /// Owning prepared statement, nullptr if non-prep.
  bool m_prepared;  /// True when statement has been prepared

  /**
    Tells if a secondary storage engine can be used for this
    statement. If it is false, use of a secondary storage engine will
    not be considered for executing this statement.
  */
  bool m_secondary_engine_enabled{true};

  /**
    Keeps track of whether the statement was prepared optional
    transformation.
  */
  bool m_prepared_with_optional_transform{false};

  /**
    The secondary storage engine to use for execution of this
    statement, if any, or nullptr if the primary engine is used.
    This property is reset at the start of each execution.
  */
  const handlerton *m_secondary_engine{nullptr};

 protected:
  bool prepare_only;  /// @see needs_explicit_preparation
                      /// @todo remove when prepare-once is implemented
};

#endif  // SQL_CMD_INCLUDED
