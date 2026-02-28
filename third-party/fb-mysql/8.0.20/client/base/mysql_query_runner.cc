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

#include "client/base/mysql_query_runner.h"

#include <stdlib.h>
#include <sys/types.h>
#include <atomic>
#include <functional>

#include "m_ctype.h"
#include "sql_string.h"
#include "template_utils.h"

using namespace Mysql::Tools::Base;
using std::string;
using std::vector;
using std::placeholders::_1;

Mysql_query_runner::Mysql_query_runner(MYSQL *connection)
    : m_is_processing(new std::atomic<bool>(false)),
      m_is_original_runner(true),
      m_connection(connection) {}

Mysql_query_runner::Mysql_query_runner(const Mysql_query_runner &source)
    : m_result_callbacks(source.m_result_callbacks),
      m_message_callbacks(source.m_message_callbacks),
      m_is_processing(source.m_is_processing),
      m_is_original_runner(false),
      m_connection(source.m_connection) {}

Mysql_query_runner::~Mysql_query_runner() {
  if (m_is_original_runner) {
    delete_container_pointers(m_result_callbacks);
    for (auto &callback_and_cleanup : m_message_callbacks) {
      if (callback_and_cleanup.second) {
        callback_and_cleanup.second();
      }
      delete callback_and_cleanup.first;
    }

    delete m_is_processing;

    mysql_close(this->m_connection);
  }
}

Mysql_query_runner &Mysql_query_runner::add_result_callback(
    std::function<int64(const Row &)> *result_callback) {
  m_result_callbacks.push_back(result_callback);
  return *this;
}

Mysql_query_runner &Mysql_query_runner::add_message_callback(
    std::function<int64(const Message_data &)> *message_callback,
    std::function<void()> cleanup_callback) {
  m_message_callbacks.emplace_back(message_callback, cleanup_callback);
  return *this;
}

MYSQL *Mysql_query_runner::get_low_level_connection() const {
  return m_connection;
}

int64 Mysql_query_runner::run_query_store(std::string query,
                                          std::vector<const Row *> *result) {
  Store_result_helper helper(result);
  return this->run_query(query, helper.get_result_callback());
}

int64 Mysql_query_runner::run_query(
    std::string query, std::function<int64(const Row &)> *result_callback) {
  Mysql_query_runner copy(*this);
  copy.add_result_callback(result_callback);
  int64 result = copy.run_query(query);
  delete result_callback;
  copy.m_result_callbacks.clear();
  return result;
}

int64 Mysql_query_runner::run_query(std::string query) {
  bool expected_value = false;

  /*
    Try to change processing flag to true. If it's not false already, this
    means that another query is in progress, which is not allowed.
   */
  if (!m_is_processing->compare_exchange_strong(expected_value, true)) {
    Message_data message(1,
                         "Cannot execute more than one "
                         "MySQL query in parallel on single MySQL connection.",
                         Message_type_error);
    return this->report_message(message);
  }
  uint64 result = this->run_query_unguarded(query);
  *m_is_processing = false;

  return result;
}

int64 Mysql_query_runner::run_query_unguarded(string query) {
  if (0 != mysql_query(m_connection, query.c_str()))
    return this->report_mysql_error();

  MYSQL_RES *results = mysql_use_result(m_connection);

  if (results != nullptr) {
    for (;;) {
      // Feed result callbacks with results.
      MYSQL_ROW row = mysql_fetch_row(results);

      if (row == nullptr) {
        // NULL row indicates end of rows or error
        if (mysql_errno(m_connection) == 0)
          break;
        else {
          mysql_free_result(results);
          return this->report_mysql_error();
        }
      }

      unsigned int columns = mysql_field_count(m_connection);
      Row *processed_row = new Row(results, columns, row);

      vector<std::function<int64(const Row &)> *>::reverse_iterator it;
      for (it = m_result_callbacks.rbegin(); it != m_result_callbacks.rend();
           it++) {
        int64 callback_result = (**it)(*processed_row);
        if (callback_result != 0) {
          mysql_free_result(results);
          return callback_result;
        }
      }
    }
    mysql_free_result(results);
  } else {
    if (mysql_errno(m_connection) != 0) return this->report_mysql_error();
  }

  // Get all notes, warnings and errors of last query.
  if (0 != mysql_query(m_connection, "SHOW WARNINGS") ||
      nullptr == (results = mysql_use_result(m_connection))) {
    this->report_mysql_error();
    return 0;
  }

  // Process all errors and warnings.
  for (;;) {
    // Feed message callbacks with results.
    MYSQL_ROW row = mysql_fetch_row(results);

    if (row == nullptr) {
      // End of rows or an error.
      if (mysql_errno(m_connection) != 0) this->report_mysql_error();
      break;
    }

    unsigned int columns = mysql_field_count(m_connection);
    Row *processed_row = new Row(results, columns, row);

    Warning_data warning(
        atoi((*processed_row)[1].c_str()), (*processed_row)[2],
        this->get_message_type_from_severity((*processed_row)[0]));
    this->report_message(warning);
    delete processed_row;
  }
  mysql_free_result(results);

  return 0;
}

int64 Mysql_query_runner::report_mysql_error() {
  Message_data message(mysql_errno(m_connection), mysql_error(m_connection),
                       Message_type_error);
  return this->report_message(message);
}

int64 Mysql_query_runner::report_message(Message_data &message) {
  for (auto it = m_message_callbacks.rbegin(); it != m_message_callbacks.rend();
       it++) {
    int64 callback_result = (*it->first)(message);
    if (callback_result < 0) {
      return 0;
    } else if (callback_result != 0) {
      return callback_result;
    }
  }
  return message.get_code();
}

enum Message_type Mysql_query_runner::get_message_type_from_severity(
    string severity) {
  for (int i = 0; i + 1 < Message_data::message_type_strings_count; i++) {
    String severity_string;
    uint dummy_errors;
    severity_string.copy(Message_data::message_type_strings[i],
                         strlen(Message_data::message_type_strings[i]),
                         &my_charset_latin1, m_connection->charset,
                         &dummy_errors);

    if (my_strcasecmp(m_connection->charset, severity.c_str(),
                      severity_string.c_ptr_safe()) == 0) {
      return (Message_type)i;
    }
  }
  return Message_type_unknown;
}

Mysql_query_runner::Store_result_helper::Store_result_helper(
    std::vector<const Row *> *result)
    : m_result(result) {}

std::function<int64(const Mysql_query_runner::Row &)>
    *Mysql_query_runner::Store_result_helper::get_result_callback() {
  return new std::function<int64(const Row &)>(
      std::bind(&Store_result_helper::result_callback, this, _1));
}

int64 Mysql_query_runner::Store_result_helper::result_callback(const Row &row) {
  m_result->push_back(&row);
  return 0;
}

void Mysql_query_runner::cleanup_result(const Mysql_query_runner::Row &result) {
  delete &result;
}

std::string Mysql_query_runner::escape_string(const std::string &original) {
  std::string res;

  this->append_escape_string(&res, original);

  return res;
}

void Mysql_query_runner::append_escape_string(std::string *destination_string,
                                              const std::string &original) {
  this->append_escape_string(destination_string, original.c_str(),
                             original.size());
}

void Mysql_query_runner::append_escape_string(std::string *destination_string,
                                              const char *original,
                                              size_t original_length) {
  size_t start_lenght = destination_string->size();
  size_t required_capacity = start_lenght + original_length * 2 + 1;
  destination_string->resize(required_capacity);

  int length = mysql_real_escape_string_quote(
      m_connection, &((*destination_string)[0]) + start_lenght, original,
      (ulong)original_length, '"');
  destination_string->resize(start_lenght + length);
}

void Mysql_query_runner::append_hex_string(std::string *destination_string,
                                           const char *original,
                                           size_t original_length) {
  size_t start_lenght = destination_string->size();
  size_t required_capacity = start_lenght + original_length * 2 + 1;
  destination_string->resize(required_capacity);

  int length = mysql_hex_string(&((*destination_string)[0]) + start_lenght,
                                original, original_length);
  destination_string->resize(start_lenght + length);
}

void Mysql_query_runner::cleanup_result(
    std::vector<const Mysql_query_runner::Row *> *result) {
  for (vector<const Mysql_query_runner::Row *>::const_iterator it =
           result->begin();
       it != result->end(); it++) {
    delete *it;
  }
  result->clear();
}

Mysql_query_runner::Row::Row(MYSQL_RES *mysql_result_info,
                             unsigned int column_count, MYSQL_ROW row)
    : m_buffer(nullptr),
      m_buffer_capacity(0),
      m_buffer_size(0),
      m_mysql_result_info(mysql_result_info) {
  size_t total_length = 0;
  unsigned long *column_lengths = mysql_fetch_lengths(mysql_result_info);
  for (unsigned int column = 0; column < column_count; column++)
    total_length += column_lengths[column] + 1;

  this->reserve(column_count, total_length);
  // first column always starts at 0 offset
  m_buffer_starts.push_back(0);
  for (unsigned int column = 0; column < column_count; column++) {
    this->push_back(row[column], column_lengths[column]);
  }
}

Mysql_query_runner::Row::~Row() {
  if (m_buffer != nullptr) free(m_buffer);
}

std::string Mysql_query_runner::Row::operator[](std::size_t index) const {
  std::size_t length;
  const char *buffer = this->get_buffer(index, length);
  return std::string(buffer, length);
}

void Mysql_query_runner::Row::push_back(char *buff, std::size_t length) {
  if (buff != nullptr) {
    // Prepare buffer to be able to contain new data.
    this->reserve(m_buffer_starts.size(), m_buffer_size + length + 1);
    // Copy data.
    memcpy(&m_buffer[m_buffer_size], buff, length);
    // Set new buffer length.
    m_buffer_size += length + 1;
    // Set sentinel NULL terminating character.
    m_buffer[m_buffer_size - 1] = 0;
    // Add new end marker.
    m_buffer_starts.push_back(m_buffer_size);
  } else {
    // Prepare to be able to contain new data NULL value marker.
    this->reserve(m_buffer_starts.size(), m_buffer_size);
    // Add new end marker specifying NULL value.
    m_buffer_starts.push_back(SIZE_T_MAX);
  }
}

const char *Mysql_query_runner::Row::get_buffer(std::size_t index,
                                                std::size_t &length) const {
  std::size_t start = SIZE_T_MAX;
  for (std::size_t start_index = index; start == SIZE_T_MAX; start_index--) {
    start = m_buffer_starts[start_index];
  }

  std::size_t end = m_buffer_starts[index + 1];
  length = (end - start - 1);
  return &m_buffer[start];
}

std::size_t Mysql_query_runner::Row::size_of_element(std::size_t index) const {
  std::size_t res = 0;
  if (!is_value_null(index)) this->get_buffer(index, res);
  return res;
}

bool Mysql_query_runner::Row::is_value_null(std::size_t index) const {
  return m_buffer_starts[index + 1] == SIZE_T_MAX;
}

std::size_t Mysql_query_runner::Row::size() const {
  return m_buffer_starts.size() - 1;
}

void Mysql_query_runner::Row::reserve(std::size_t strings,
                                      std::size_t buffer_size) {
  if (strings >= m_buffer_starts.capacity()) {
    m_buffer_starts.reserve(strings + 1);
  }
  if (buffer_size > m_buffer_capacity) {
    m_buffer_capacity = buffer_size;
    m_buffer = (char *)realloc(m_buffer, m_buffer_capacity);
  }
}

Mysql_query_runner::Row::Iterator Mysql_query_runner::Row::begin() const {
  return Mysql_query_runner::Row::Iterator(*this, 0);
}

Mysql_query_runner::Row::Iterator Mysql_query_runner::Row::end() const {
  return Mysql_query_runner::Row::Iterator(*this, m_buffer_starts.size() - 1);
}

MYSQL_RES *Mysql_query_runner::Row::get_mysql_result_info() const {
  return m_mysql_result_info;
}

Mysql_query_runner::Row::Iterator::Iterator(const Row &row, std::size_t index)
    : m_row(row), m_index(index) {}

bool Mysql_query_runner::Row::Iterator::is_value_null() {
  return m_row.is_value_null(m_index);
}

std::string Mysql_query_runner::Row::Iterator::operator*() {
  return m_row[m_index];
}

void Mysql_query_runner::Row::Iterator::operator++() { m_index++; }

bool Mysql_query_runner::Row::Iterator::operator==(const Iterator &other) {
  return &m_row == &other.m_row && m_index == other.m_index;
}

bool Mysql_query_runner::Row::Iterator::operator!=(const Iterator &other) {
  return !(*this == other);
}
