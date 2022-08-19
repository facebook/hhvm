/*
   Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef MYSQL_QUERY_RUNNER_INCLUDED
#define MYSQL_QUERY_RUNNER_INCLUDED

#include <algorithm>
#include <atomic>
#include <functional>
#include <string>
#include <vector>

#include "client/base/message_data.h"
#include "my_inttypes.h"
#include "mysql.h"

namespace Mysql {
namespace Tools {
namespace Base {

/**
  Helper class to run SQL query on existing MySQL database server connection,
  receive all data and all errors, warnings and notes returned during query
  execution. All acquired information is passed to set of callbacks to make
  data flows more customizable.
 */
class Mysql_query_runner {
 public:
  class Row;

  /**
    Standard constructor based on MySQL connection.
   */
  Mysql_query_runner(MYSQL *connection);
  /**
    Copy constructor.
   */
  Mysql_query_runner(const Mysql_query_runner &source);

  ~Mysql_query_runner();
  /**
    Adds new callback to be called on every result row of query.
    If callback return value other than 0 then query execution, passing
    current row to other callbacks and error messages processing, and
    Mysql_query_runner::run_query() will return value returned from this
    callback.
    Callbacks are called in reverse order of addition, i.e. newest are first.
   */
  Mysql_query_runner &add_result_callback(
      std::function<int64(const Row &)> *result_callback);
  /**
    Adds new callback to be called on every message after query execution,
    this includes errors, warnings and other notes. Return value from callback
    of 0 will lead to next handler being called, positive number return value
    will cause Mysql_query_runner::run_query() will return immediately this
    value and negative number will continue query execution and other messages
    processing, but will not pass current message to rest of callbacks.
    Callbacks are called in reverse order of addition, i.e. newest are first.

    The optional cleanup function is called when the callback is deleted.
   */
  Mysql_query_runner &add_message_callback(
      std::function<int64(const Message_data &)> *message_callback,
      std::function<void()> cleanup_callback = nullptr);
  /**
    Runs specified query and processes result rows and messages to callbacks.
   */
  int64 run_query(std::string query);
  /**
    Runs specified query, fills result vector with processed result rows
    and processes messages to callbacks.
   */
  int64 run_query_store(std::string query, std::vector<const Row *> *result);
  /**
    Runs specified query with result callback specified. Does not add specified
    callback to list of callbacks, next queries will not process rows to this
    callback.
   */
  int64 run_query(std::string query,
                  std::function<int64(const Row &)> *result_callback);
  /**
    Returns escaped copy of string to use in queries.
   */
  std::string escape_string(const std::string &original);
  /**
    Escapes specified input string and appends it escaped to destination
    string.
   */
  void append_escape_string(std::string *destination_string,
                            const std::string &original);
  /**
    Escapes specified input string specified as characters buffer and its size,
    and appends it escaped to destination string.
   */
  void append_escape_string(std::string *destination_string,
                            const char *original, size_t original_length);
  /**
    Converts to HEX specified input string specified as characters buffer and
    its size, and appends it escaped to destination string.
   */
  static void append_hex_string(std::string *destination_string,
                                const char *original, size_t original_length);

  /**
    Empties memory used by result strings.
   */
  static void cleanup_result(const Row &result);
  /**
    Empties memory used by result strings.
   */
  static void cleanup_result(std::vector<const Row *> *result);

  MYSQL *get_low_level_connection() const;

  class Row {
   public:
    class Iterator;

    Row(MYSQL_RES *mysql_result_info, unsigned int column_count, MYSQL_ROW row);
    ~Row();
    std::string operator[](std::size_t index) const;
    void push_back(char *buff, std::size_t length);
    const char *get_buffer(std::size_t index, std::size_t &length) const;
    std::size_t size_of_element(std::size_t index) const;
    bool is_value_null(std::size_t index) const;
    std::size_t size() const;
    Iterator begin() const;
    Iterator end() const;
    MYSQL_RES *get_mysql_result_info() const;

    class Iterator {
     public:
      Iterator(const Row &row, std::size_t index);
      bool is_value_null();
      std::string operator*();
      void operator++();
      bool operator==(const Iterator &other);
      bool operator!=(const Iterator &other);

     private:
      const Row &m_row;
      std::size_t m_index;
    };

   private:
    void reserve(std::size_t strings, std::size_t buffer_size);

    // Represents table row as a string
    char *m_buffer;
    // Represents offsets to each column in m_buffer
    std::vector<std::size_t> m_buffer_starts;
    // Total buffer size
    std::size_t m_buffer_capacity;
    // Actual buffer size
    std::size_t m_buffer_size;
    MYSQL_RES *m_mysql_result_info;
  };

 private:
  /**
    Runs specified query and process result rows and messages to callbacks.
    Does not check for multiple queries being executed in parallel.
  */
  int64 run_query_unguarded(std::string query);
  /**
    Creates error message from mysql_errno and mysql_error and passes it to
    callbacks.
   */
  int64 report_mysql_error();
  /**
    Creates error message from mysql_errno and mysql_error and passes it to
    callbacks.
   */
  int64 report_message(Message_data &message);
  /**
    Returns parsed Message_type from given MySQL severity string.
   */
  Message_type get_message_type_from_severity(std::string severity);

  class Store_result_helper {
   public:
    Store_result_helper(std::vector<const Row *> *result);
    std::function<int64(const Row &)> *get_result_callback();

   private:
    int64 result_callback(const Row &row);

    std::vector<const Row *> *m_result;
  };

  std::vector<std::function<int64(const Row &)> *> m_result_callbacks;
  std::vector<std::pair<std::function<int64(const Message_data &)> *,
                        std::function<void()>>>
      m_message_callbacks;

  /**
    Indicates if there is query currently executed. Only one query can be
    executed in specified time moment.
   */
  std::atomic<bool> *m_is_processing;

  /**
    Indicates if this is original runner or a copy. In case of original the
    cleanup is performed on destruction.
  */
  bool m_is_original_runner;

  MYSQL *m_connection;
};

}  // namespace Base
}  // namespace Tools
}  // namespace Mysql

#endif
