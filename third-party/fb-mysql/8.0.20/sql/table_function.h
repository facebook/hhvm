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

#ifndef TABLE_FUNCTION_INCLUDED
#define TABLE_FUNCTION_INCLUDED

#include <sys/types.h>
#include <array>  // std::array

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_table_map.h"
#include "sql/create_field.h"
#include "sql/enum_query_type.h"
#include "sql/json_dom.h"   // Json_wrapper
#include "sql/json_path.h"  // Json_path
#include "sql/mem_root_array.h"
#include "sql/psi_memory_key.h"  // key_memory_JSON
#include "sql/sql_const.h"       // Item_processor, enum_walk
#include "sql/sql_list.h"        // List
#include "sql/table.h"           // TABLE

class Field;
class Item;
class String;
class Table_function_json;
class THD;

/**
  Class representing a table function.
*/

class Table_function {
 protected:
  /// Thread handler
  THD *thd;
  /// Table function's result table
  TABLE *table;
  /// Whether the table funciton was already initialized
  bool inited;

 public:
  explicit Table_function(THD *thd_arg)
      : thd(thd_arg), table(nullptr), inited(false) {}

  virtual ~Table_function() {}
  /**
    Create, but not instantiate the result table

    @param options     options to create table
    @param table_alias table's alias

    @returns
      true  on error
      false on success
  */
  bool create_result_table(ulonglong options, const char *table_alias);
  /**
    Write current record to the result table and handle overflow to disk

    @returns
      true  on error
      false on success
  */
  bool write_row();

  /**
    Returns a field with given index

    @param i field's index

    @returns
      field with given index
  */
  Field *get_field(uint i) {
    DBUG_ASSERT(i < table->s->fields);
    return table->field[i];
  }
  /**
    Delete all rows in the table
  */
  void empty_table();

  /**
    Set the default row
  */
  void default_row() {}
  /**
    Initialize table function
    @returns
      true  on error
      false on success
  */
  virtual bool init() = 0;
  /**
    Initialize table function after the result table has been created
    @returns
      true  on error
      false on success
  */
  virtual bool init_args();
  /**
    Execute the table function - fill the result table
    @returns
      true  on error
      false on success
  */
  virtual bool fill_result_table() = 0;
  /**
    Returns table function's name
  */
  virtual const char *func_name() const = 0;
  /**
    Return table_map of tables used by the function
  */
  virtual table_map used_tables() { return 0; }
  /**
    Print table function

    @param str         string to print to
    @param query_type  type of the query

    @returns
      true  on error
      false on success
  */
  virtual bool print(String *str, enum_query_type query_type) const = 0;
  /**
    Clean up table function
  */
  void cleanup() {
    do_cleanup();
    table = nullptr;
    inited = false;
  }

  virtual bool walk(Item_processor processor, enum_walk walk, uchar *arg) = 0;

 private:
  /**
    Get the list of fields to create the result table
  */
  virtual List<Create_field> *get_field_list() = 0;
  /**
    Initialize table function's arguments

    @returns
      true  on error
      false on success
  */
  virtual bool do_init_args() = 0;
  friend bool TABLE_LIST::setup_table_function(THD *thd);
  virtual void do_cleanup() {}
};

/****************************************************************************
  JSON_TABLE function
****************************************************************************/

/// Type of columns for JSON_TABLE function
enum class enum_jt_column {
  JTC_ORDINALITY,
  JTC_PATH,
  JTC_EXISTS,
  JTC_NESTED_PATH
};

/// Types of ON ERROR/ON EMPTY clause for JSON_TABLE function
/// @note uint16 enum base limitation is necessary for YYSTYPE.
enum class enum_jtc_on : uint16 {
  JTO_ERROR,
  JTO_NULL,
  JTO_DEFAULT,
  JTO_IMPLICIT
};

/**
  JT_data_source is used as a data source. It's assigned to each NESTED PATH
  node.
*/

class JT_data_source {
 public:
  /// Vector of found values
  Json_wrapper_vector v;
  /// Iterator for vector above
  Json_wrapper_vector::iterator it;
  /// JSON data to seek columns' paths in
  Json_wrapper jdata;
  /// Current m_rowid, used for ORDINALITY columns
  uint m_rowid;
  /**
    true <=> NESTED PATH associated with this element is producing records.
    Used to turn off (set to null) sibling NESTED PATHs, when one of them is
    used to fill result table.
  */
  bool producing_records;

  JT_data_source() : v(key_memory_JSON), producing_records(false) {}
  ~JT_data_source() {}

  void cleanup();
};

/**
  Reason for skipping a NESTED PATH
*/
enum jt_skip_reason {
  JTS_NONE = 0,  // NESTED PATH isn't skipped
  JTS_EOD,       // No more data
  JTS_SIBLING    // Skipped due to other sibling NESTED PATH is running
};

/// Column description for JSON_TABLE function
class Json_table_column : public Create_field {
 public:
  /// Column type
  enum_jt_column m_jtc_type;
  /// Type of ON ERROR clause
  enum_jtc_on m_on_error{enum_jtc_on::JTO_IMPLICIT};
  /// Type of ON EMPTY clause
  enum_jtc_on m_on_empty{enum_jtc_on::JTO_IMPLICIT};
  /// Default value string for ON EMPTY clause
  Item *m_default_empty_string{nullptr};
  /// Parsed JSON for default value of ON MISSING clause
  Json_wrapper m_default_empty_json;
  /// Default value string for ON ERROR clause
  Item *m_default_error_string{nullptr};
  /// Parsed JSON string for ON ERROR clause
  Json_wrapper m_default_error_json;
  /// List of nested columns, valid only for NESTED PATH
  List<Json_table_column> *m_nested_columns{nullptr};
  /// Nested path
  Item *m_path_string{nullptr};
  /// parsed nested path
  Json_path m_path_json;
  /// An element in table function's data source array
  JT_data_source *m_jds_elt{nullptr};
  /**
    Element in table function's data source array to feed data to child
    nodes. Valid only for NESTED PATH.
  */
  JT_data_source *m_child_jds_elt{nullptr};
  ///  Next sibling NESTED PATH
  Json_table_column *m_next_nested{nullptr};
  ///  Previous sibling NESTED PATH
  Json_table_column *m_prev_nested{nullptr};
  /// Index of field in the result table
  int m_field_idx{-1};

 public:
  explicit Json_table_column(enum_jt_column type) : m_jtc_type(type) {}
  Json_table_column(enum_jt_column col_type, Item *path, enum_jtc_on on_err,
                    Item *error_default, enum_jtc_on on_miss,
                    Item *missing_default)
      : m_jtc_type(col_type),
        m_on_error(on_err),
        m_on_empty(on_miss),
        m_default_empty_string(missing_default),
        m_default_error_string(error_default),
        m_path_string(path) {}
  Json_table_column(Item *path, List<Json_table_column> *cols)
      : m_jtc_type(enum_jt_column::JTC_NESTED_PATH),
        m_nested_columns(cols),
        m_path_string(path) {}
  void cleanup();

  /**
    Process JSON_TABLE's column

    @param table_function the JSON table function
    @param[out] skip  whether current NESTED PATH column should be
                      completely skipped
    @returns
      true  on error
      false on success
  */
  bool fill_column(Table_function_json *table_function, jt_skip_reason *skip);
};

#define MAX_NESTED_PATH 16

class Table_function_json final : public Table_function {
  /// Array of JSON Data Source for each NESTED PATH clause
  std::array<JT_data_source, MAX_NESTED_PATH> m_jds;
  /// List of fields for tmp table creation
  List<Json_table_column> m_vt_list;
  /// Tree of COLUMN clauses
  List<Json_table_column> *m_columns;
  /// Array of all columns - the flattened tree above
  Mem_root_array<Json_table_column *> m_all_columns;
  /// JSON_TABLE's alias, for error reporting
  const char *m_table_alias;

  /** Whether source data has been parsed. */
  bool is_source_parsed;
  /// JSON_TABLE's data source expression
  Item *source;

 public:
  Table_function_json(THD *thd_arg, const char *alias, Item *a,
                      List<Json_table_column> *cols);

  /**
    Returns function's name
  */
  const char *func_name() const override { return "json_table"; }
  /**
    Initialize the table function before creation of result table

    @returns
      true  on error
      false on success
  */
  bool init() override;

  /**
    Execute table function

    @returns
      true  on error
      false on success
  */
  bool fill_result_table() override;

  /**
    Return table_map of tables used by function's data source
  */
  table_map used_tables() override;

  /**
    JSON_TABLE printout

    @param str        string to print to
    @param query_type type of query

    @returns
      true  on error
      false on success
  */
  bool print(String *str, enum_query_type query_type) const override;

  bool walk(Item_processor processor, enum_walk walk, uchar *arg) override;

 private:
  /**
    Fill the result table

    @returns
      true  on error
      false on success
  */
  bool fill_json_table();

  /**
    Prepare lists used to create tmp table and function execution

    @param nest_idx  index of parent's element in the nesting data array
    @param parent    Parent of the NESTED PATH clause being initialized

    @returns
      true  on error
      false on success
  */
  bool init_json_table_col_lists(uint *nest_idx, Json_table_column *parent);
  /**
    Set all underlying columns of a NESTED PATH to nullptr

    @param       root  root NESTED PATH column
    @param [out] last  last column which belongs to the given NESTED PATH
  */
  void set_subtree_to_null(Json_table_column *root, Json_table_column **last);

  /**
    Return list of fields to create result table from
  */
  List<Create_field> *get_field_list() override;
  bool do_init_args() override;
  void do_cleanup() override;
};
#endif /* TABLE_FUNCTION_INCLUDED */
