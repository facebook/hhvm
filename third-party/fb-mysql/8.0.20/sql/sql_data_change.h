#ifndef SQL_DATA_CHANGE_INCLUDED
#define SQL_DATA_CHANGE_INCLUDED
/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file sql_data_change.h

  Contains classes representing SQL-data change statements. The
  actual implementions of the functionality are found in files
  sql_{insert, update}.{h,cc}
*/

#include <stddef.h>
#include <sys/types.h>

#include "my_base.h"    // ha_rows
#include "my_bitmap.h"  // MY_BITMAP
#include "my_dbug.h"

class Item;
struct TABLE;
template <class T>
class List;

enum enum_duplicates { DUP_ERROR, DUP_REPLACE, DUP_UPDATE };

/**
   This class encapsulates a data change operation. There are three such
   operations.

   -# Insert statements, i.e. INSERT INTO .. VALUES

   -# Update statements. UPDATE @<table@> SET ...

   -# Delete statements. Currently this class is not used for delete statements
      and thus has not yet been adapted to handle it.

   @todo Rename this class.

  The COPY_INFO structure is used by INSERT/REPLACE code.
  The schema of the row counting by the INSERT/INSERT ... ON DUPLICATE KEY
  UPDATE code:
    If a row is inserted then the copied variable is incremented.
    If a row is updated by the INSERT ... ON DUPLICATE KEY UPDATE and the
      new data differs from the old one then the copied and the updated
      variables are incremented.
    The touched variable is incremented if a row was touched by the update part
      of the INSERT ... ON DUPLICATE KEY UPDATE no matter whether the row
      was actually changed or not.
*/
class COPY_INFO {
 public:
  class Statistics {
   public:
    Statistics()
        : records(0),
          deleted(0),
          updated(0),
          copied(0),
          error_count(0),
          touched(0) {}

    ha_rows records; /**< Number of processed records */
    ha_rows deleted; /**< Number of deleted records */
    ha_rows updated; /**< Number of updated records */
    ha_rows copied;  /**< Number of copied records */
    ha_rows error_count;
    ha_rows touched; /* Number of touched records */
  };

  enum operation_type { INSERT_OPERATION, UPDATE_OPERATION };

 private:
  COPY_INFO(const COPY_INFO &other);  ///< undefined
  void operator=(COPY_INFO &);        ///< undefined

  /// Describes the data change operation that this object represents.
  const operation_type m_optype;

  /**
     List of columns of the target table which the statement will explicitely
     fill; and thus we must not set a function default for them.
     NULL means "empty list".
  */
  List<Item> *m_changed_columns;

  /**
     A second list of columns like m_changed_columns. See the constructor
     specific of LOAD DATA INFILE, below.
  */
  List<Item> *m_changed_columns2;

  /** Whether this object must manage function defaults */
  const bool m_manage_defaults;
  /** Bitmap: bit is set if we should set column number i to its function
   * default */
  MY_BITMAP *m_function_default_columns;

  /// Policy for handling insertion of duplicate values.
  const enum enum_duplicates handle_duplicates;

 protected:
  /**
     This function will, unless done already, calculate and keep the set of
     function default columns.

     Function default columns are those columns declared DEFAULT @<function@>
     and/or ON UPDATE @<function@>. These will store the return value of
     @<function@> when the relevant operation is applied on the table.

     Calling this function, without error, is a prerequisite for calling
     COPY_INFO::set_function_defaults().

     @param table The table to be used for instantiating the column set.

     @retval false Success.
     @retval true Memory allocation error.
  */
  bool get_function_default_columns(TABLE *table);

  /**
     The column bitmap which has been cached for this data change operation.
     @see COPY_INFO::get_function_default_columns()

     @return The cached bitmap, or NULL if no bitmap was cached.
   */
  MY_BITMAP *get_cached_bitmap() const { return m_function_default_columns; }

 public:
  Statistics stats;
  int escape_char, last_errno;
  /** Values for UPDATE; needed by write_record() if INSERT with DUP_UPDATE */
  List<Item> *update_values;

  /**
     Initializes this data change operation as an SQL @c INSERT (with all
     possible syntaxes and variants).

     @param optype           The data change operation type.
     @param inserted_columns List of columns of the target table which
                             the statement will explicitely fill; COPY_INFO
                             must not set a function default for them. NULL
                             means "empty list".
     @param manage_defaults  Whether this object should manage function
                             defaults.
     @param duplicate_handling The policy for handling duplicates.

  */
  COPY_INFO(operation_type optype, List<Item> *inserted_columns,
            bool manage_defaults, enum_duplicates duplicate_handling)
      : m_optype(optype),
        m_changed_columns(inserted_columns),
        m_changed_columns2(nullptr),
        m_manage_defaults(manage_defaults),
        m_function_default_columns(nullptr),
        handle_duplicates(duplicate_handling),
        stats(),
        escape_char(0),
        last_errno(0),
        update_values(nullptr) {
    DBUG_ASSERT(optype == INSERT_OPERATION);
  }

  /**
     Initializes this data change operation as an SQL @c LOAD @c DATA @c
     INFILE.
     Note that this statement has its inserted columns spread over two
     lists:
@verbatim
     LOAD DATA INFILE a_file
     INTO TABLE a_table (col1, col2)   < first list (col1, col2)
     SET col3=val;                     < second list (col3)
@endverbatim

     @param optype            The data change operation type.
     @param inserted_columns List of columns of the target table which
                             the statement will explicitely fill; COPY_INFO
                             must not set a function default for them. NULL
                             means "empty list".
     @param inserted_columns2 A second list like inserted_columns
     @param manage_defaults   Whether this object should manage function
                              defaults.
     @param duplicates_handling How to handle duplicates.
     @param escape_character    The escape character.
  */
  COPY_INFO(operation_type optype, List<Item> *inserted_columns,
            List<Item> *inserted_columns2, bool manage_defaults,
            enum_duplicates duplicates_handling, int escape_character)
      : m_optype(optype),
        m_changed_columns(inserted_columns),
        m_changed_columns2(inserted_columns2),
        m_manage_defaults(manage_defaults),
        m_function_default_columns(nullptr),
        handle_duplicates(duplicates_handling),
        stats(),
        escape_char(escape_character),
        last_errno(0),
        update_values(nullptr) {
    DBUG_ASSERT(optype == INSERT_OPERATION);
  }

  /**
     Initializes this data change operation as an SQL @c UPDATE (multi- or
     not).

     @param optype  The data change operation type.
     @param fields  The column objects that are to be updated.
     @param values  The values to be assigned to the fields.
     @note that UPDATE always lists columns, so non-listed columns may need a
     default thus m_manage_defaults is always true.
  */
  COPY_INFO(operation_type optype, List<Item> *fields, List<Item> *values)
      : m_optype(optype),
        m_changed_columns(fields),
        m_changed_columns2(nullptr),
        m_manage_defaults(true),
        m_function_default_columns(nullptr),
        handle_duplicates(DUP_ERROR),
        stats(),
        escape_char(0),
        last_errno(0),
        update_values(values) {
    DBUG_ASSERT(optype == UPDATE_OPERATION);
  }

  operation_type get_operation_type() const { return m_optype; }

  List<Item> *get_changed_columns() const { return m_changed_columns; }

  const List<Item> *get_changed_columns2() const { return m_changed_columns2; }

  bool get_manage_defaults() const { return m_manage_defaults; }

  enum_duplicates get_duplicate_handling() const { return handle_duplicates; }

  /**
     Assigns function default values to columns of the supplied table.

     @note COPY_INFO::get_function_default_columns() and
     COPY_INFO::add_function_default_columns() must be called prior to invoking
     this function.

     @param table  The table to which columns belong.

     @note It is assumed that all columns in this COPY_INFO are resolved to the
     table.

     @retval false Success.
     @retval true Some error happened while executing the default expression.
                  my_error has already been called so the calling function
                  only needs to bail out.
  */
  bool set_function_defaults(TABLE *table);

  /**
     Adds the columns that are bound to receive default values from a function
     (e.g. CURRENT_TIMESTAMP) to the set columns. Uses lazy instantiation of the
     set of function default columns.

     @param      table    The table on which the operation is performed.
     @param[out] columns  The function default columns are added to this set.

     @retval false Success.
     @retval true Memory allocation error during lazy instantiation.
  */
  bool add_function_default_columns(TABLE *table, MY_BITMAP *columns) {
    if (get_function_default_columns(table)) return true;
    bitmap_union(columns, m_function_default_columns);
    return false;
  }

  /**
     True if this operation will set some fields to function default result
     values when invoked on the table.

     @note COPY_INFO::add_function_default_columns() must be called prior to
     invoking this function.
  */
  bool function_defaults_apply(const TABLE *) const {
    DBUG_ASSERT(m_function_default_columns != nullptr);
    return !bitmap_is_clear_all(m_function_default_columns);
  }

  /**
    True if any of the columns set in the bitmap have default functions
    that may set the column.
  */
  bool function_defaults_apply_on_columns(MY_BITMAP *map) {
    DBUG_ASSERT(m_function_default_columns != nullptr);
    return bitmap_is_overlapping(m_function_default_columns, map);
  }

  /**
     Tells the object to not manage function defaults for the last 'count'
     columns of 'table'.
     @retval false if success
  */
  bool ignore_last_columns(TABLE *table, uint count);

  /**
     This class allocates its memory in a MEM_ROOT, so there's nothing to
     delete.
  */
  virtual ~COPY_INFO() {}
};

#endif  // SQL_DATA_CHANGE_INCLUDED
