#ifndef SQL_PREPARE_H
#define SQL_PREPARE_H
/* Copyright (c) 2009, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <stddef.h>
#include <sys/types.h>
#include <new>

#include "lex_string.h"
#include "my_alloc.h"
#include "my_command.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_psi_config.h"
#include "mysql/components/services/psi_statement_bits.h"
#include "mysql_com.h"
#include "sql/sql_class.h"  // Query_arena
#include "sql/sql_error.h"
#include "sql/sql_list.h"

class Item;
class Item_param;
class Prepared_statement;
class Query_result_send;
class String;
struct LEX;
struct PS_PARAM;
struct TABLE_LIST;
union COM_DATA;

/**
  An interface that is used to take an action when
  the locking module notices that a table version has changed
  since the last execution. "Table" here may refer to any kind of
  table -- a base table, a temporary table, a view or an
  information schema table.

  When we open and lock tables for execution of a prepared
  statement, we must verify that they did not change
  since statement prepare. If some table did change, the statement
  parse tree *may* be no longer valid, e.g. in case it contains
  optimizations that depend on table metadata.

  This class provides an interface (a method) that is
  invoked when such a situation takes place.
  The implementation of the method simply reports an error, but
  the exact details depend on the nature of the SQL statement.

  At most 1 instance of this class is active at a time, in which
  case THD::m_reprepare_observer is not NULL.

  @sa check_and_update_table_version() for details of the
  version tracking algorithm

  @sa Open_tables_state::m_reprepare_observer for the life cycle
  of metadata observers.
*/

class Reprepare_observer final {
 public:
  /**
    Check if a change of metadata is OK. In future
    the signature of this method may be extended to accept the old
    and the new versions, but since currently the check is very
    simple, we only need the THD to report an error.
  */
  bool report_error(THD *thd);
  bool is_invalidated() const { return m_invalidated; }
  void reset_reprepare_observer() { m_invalidated = false; }

 private:
  bool m_invalidated;
};

bool mysql_stmt_precheck(THD *thd, const COM_DATA *com_data,
                         enum enum_server_command cmd,
                         Prepared_statement **stmt);
void mysqld_stmt_prepare(THD *thd, const char *query, uint length,
                         Prepared_statement *stmt);
void mysqld_stmt_execute(THD *thd, Prepared_statement *stmt, bool has_new_types,
                         ulong execute_flags, PS_PARAM *parameters);
void mysqld_stmt_close(THD *thd, Prepared_statement *stmt);
void mysql_sql_stmt_prepare(THD *thd);
void mysql_sql_stmt_execute(THD *thd);
void mysql_sql_stmt_close(THD *thd);
void mysqld_stmt_fetch(THD *thd, Prepared_statement *stmt, ulong num_rows);
void mysqld_stmt_reset(THD *thd, Prepared_statement *stmt);
void mysql_stmt_get_longdata(THD *thd, Prepared_statement *stmt,
                             uint param_number, uchar *longdata, ulong length);
bool reinit_stmt_before_use(THD *thd, LEX *lex);
bool select_like_stmt_cmd_test(THD *thd, class Sql_cmd_dml *cmd,
                               ulong setup_tables_done_option);
bool mysql_test_show(Prepared_statement *stmt, TABLE_LIST *tables);

/**
  Execute a fragment of server code in an isolated context, so that
  it doesn't leave any effect on THD. THD must have no open tables.
  The code must not leave any open tables around.
  The result of execution (if any) is stored in Ed_result.
*/

class Server_runnable {
 public:
  virtual bool execute_server_code(THD *thd) = 0;
  virtual ~Server_runnable();
};

/**
  Execute direct interface.

  @todo Implement support for prelocked mode.
*/

class Ed_row;

/**
  Ed_result_set -- a container with result set rows.
  @todo Implement support for result set metadata and
  automatic type conversion.
*/

class Ed_result_set final {
 public:
  operator List<Ed_row> &() { return *m_rows; }
  unsigned int size() const { return m_rows->elements; }
  Ed_row *get_fields() { return m_fields; }

  Ed_result_set(List<Ed_row> *rows_arg, Ed_row *fields, size_t column_count,
                MEM_ROOT *mem_root_arg);

  /** We don't call member destructors, they all are POD types. */
  ~Ed_result_set() {}

  size_t get_field_count() const { return m_column_count; }

  static void operator delete(void *, size_t) noexcept {
    // Does nothing because m_mem_root is deallocated in the destructor
  }

  static void operator delete(
      void *, MEM_ROOT *, const std::nothrow_t &)noexcept { /* never called */
  }

 private:
  Ed_result_set(const Ed_result_set &);      /* not implemented */
  Ed_result_set &operator=(Ed_result_set &); /* not implemented */
 private:
  MEM_ROOT m_mem_root;
  size_t m_column_count;
  List<Ed_row> *m_rows;
  Ed_row *m_fields;
  Ed_result_set *m_next_rset;
  friend class Ed_connection;
};

class Ed_connection final {
 public:
  /**
    Construct a new "execute direct" connection.

    The connection can be used to execute SQL statements.
    If the connection failed to initialize, the error
    will be returned on the attempt to execute a statement.

    @pre thd  must have no open tables
              while the connection is used. However,
              Ed_connection works okay in LOCK TABLES mode.
              Other properties of THD, such as the current warning
              information, errors, etc. do not matter and are
              preserved by Ed_connection. One thread may have many
              Ed_connections created for it.
  */
  Ed_connection(THD *thd);

  /**
    Execute one SQL statement.

    Until this method is executed, no other methods of
    Ed_connection can be used. Life cycle of Ed_connection is:

    Initialized -> a statement has been executed ->
    look at result, move to next result ->
    look at result, move to next result ->
    ...
    moved beyond the last result == Initialized.

    This method can be called repeatedly. Once it's invoked,
    results of the previous execution are lost.

    A result of execute_direct() can be either:

    - success, no result set rows. In this case get_field_count()
    returns 0. This happens after execution of INSERT, UPDATE,
    DELETE, DROP and similar statements. Some other methods, such
    as get_affected_rows() can be used to retrieve additional
    result information.

    - success, there are some result set rows (maybe 0). E.g.
    happens after SELECT. In this case get_field_count() returns
    the number of columns in a result set and store_result()
    can be used to retrieve a result set..

    - an error, methods to retrieve error information can
    be used.

    @return execution status
    @retval false  success, use get_field_count()
                   to determine what to do next.
    @retval true   error, use get_last_error()
                   to see the error number.
  */
  bool execute_direct(LEX_STRING sql_text);

  /**
    Same as the previous, but takes an instance of Server_runnable
    instead of SQL statement text.

    @return execution status

    @retval  false  success, use get_field_count()
                    if your code fragment is supposed to
                    return a result set
    @retval  true   failure
  */
  bool execute_direct(Server_runnable *server_runnable);

  /**
    The following three members are only valid if execute_direct()
    or move_to_next_result() returned an error.
    They never fail, but if they are called when there is no
    result, or no error, the result is not defined.
  */
  const char *get_last_error() const {
    return m_diagnostics_area.message_text();
  }

  unsigned int get_last_errno() const {
    return m_diagnostics_area.mysql_errno();
  }

  Ed_result_set *get_result_sets() { return m_rsets; }

  ~Ed_connection() { free_old_result(); }

 private:
  Diagnostics_area m_diagnostics_area;
  /**
    Execute direct interface does not support multi-statements, only
    multi-results. So we never have a situation when we have
    a mix of result sets and OK or error packets. We either
    have a single result set, a single error, or a single OK,
    or we have a series of result sets, followed by an OK or error.
  */
  THD *m_thd;
  Ed_result_set *m_rsets;
  Ed_result_set *m_current_rset;
  friend class Protocol_local;

 private:
  void free_old_result();
  void add_result_set(Ed_result_set *ed_result_set);

 private:
  Ed_connection(const Ed_connection &);      /* not implemented */
  Ed_connection &operator=(Ed_connection &); /* not implemented */
};

/** One result set column. */

struct Ed_column final : public LEX_STRING {
  /** Implementation note: destructor for this class is never called. */
};

/** One result set record. */

class Ed_row final {
 public:
  const Ed_column &operator[](const unsigned int column_index) const {
    return *get_column(column_index);
  }
  const Ed_column *get_column(const unsigned int column_index) const {
    DBUG_ASSERT(column_index < size());
    return m_column_array + column_index;
  }
  size_t size() const { return m_column_count; }

  Ed_row(Ed_column *column_array_arg, size_t column_count_arg)
      : m_column_array(column_array_arg), m_column_count(column_count_arg) {}

 private:
  Ed_column *m_column_array;
  size_t m_column_count; /* TODO: change to point to metadata */
};

class Server_side_cursor;

/**
  Prepared_statement: a statement that can contain placeholders.
*/

class Prepared_statement final {
  enum flag_values { IS_IN_USE = 1, IS_SQL_PREPARE = 2 };

 public:
  Query_arena m_arena;
  THD *thd;
  Item_param **param_array;
  Server_side_cursor *cursor;
  uint param_count;
  uint last_errno;
  char last_error[MYSQL_ERRMSG_SIZE];

  /*
    Uniquely identifies each statement object in thread scope; change during
    statement lifetime.
  */
  const ulong id;

  LEX *lex;  // parse tree descriptor

  /**
    The query associated with this statement.
  */
  LEX_CSTRING m_query_string;

  /* Performance Schema interface for a prepared statement. */
  PSI_prepared_stmt *m_prepared_stmt;

 private:
  Query_result_send *result;

  uint flags;
  bool with_log;
  LEX_CSTRING m_name; /* name for named prepared statements */
  /**
    Name of the current (default) database.

    If there is the current (default) database, "db" contains its name. If
    there is no current (default) database, "db" is NULL and "db_length" is
    0. In other words, "db", "db_length" must either be NULL, or contain a
    valid database name.

    @note this attribute is set and alloced by the slave SQL thread (for
    the THD of that thread); that thread is (and must remain, for now) the
    only responsible for freeing this member.
  */
  LEX_CSTRING m_db;

  /**
    The memory root to allocate parsed tree elements (instances of Item,
    SELECT_LEX and other classes).
  */
  MEM_ROOT main_mem_root;

 public:
  Prepared_statement(THD *thd_arg);
  virtual ~Prepared_statement();
  bool set_name(const LEX_CSTRING &name);
  const LEX_CSTRING &name() const { return m_name; }
  void close_cursor();
  bool is_in_use() const { return flags & (uint)IS_IN_USE; }
  bool is_sql_prepare() const { return flags & (uint)IS_SQL_PREPARE; }
  void set_sql_prepare() { flags |= (uint)IS_SQL_PREPARE; }
  bool prepare(const char *packet, size_t packet_length);
  bool execute_loop(String *expanded_query, bool open_cursor);
  bool execute_server_runnable(Server_runnable *server_runnable);
#ifdef HAVE_PSI_PS_INTERFACE
  PSI_prepared_stmt *get_PS_prepared_stmt() { return m_prepared_stmt; }
#endif
  /* Destroy this statement */
  void deallocate();
  bool set_parameters(String *expanded_query, bool has_new_types,
                      PS_PARAM *parameters);
  bool set_parameters(String *expanded_query);

 private:
  void cleanup_stmt();
  void setup_set_params();
  bool set_db(const LEX_CSTRING &db_length);

  bool execute(String *expanded_query, bool open_cursor);
  bool reprepare();
  bool validate_metadata(Prepared_statement *copy);
  void swap_prepared_statement(Prepared_statement *copy);
  bool insert_params_from_vars(List<LEX_STRING> &varnames, String *query);
  bool insert_params(String *query, PS_PARAM *parameters);
};

#endif  // SQL_PREPARE_H
