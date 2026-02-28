/* Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.

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
  @file sql/histograms/value_map.cc
  Value map (implementation).
*/

#include "sql/histograms/value_map.h"

#include <algorithm>
#include <new>
#include <string>  // std::string

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "my_time.h"
#include "mysql_time.h"  // MYSQL_TIME
#include "sql/histograms/histogram.h"
#include "sql/my_decimal.h"      // my_decimal_cmp
#include "sql/psi_memory_key.h"  // key_memory_histograms
#include "sql_string.h"          // String
#include "template_utils.h"      // down_cast

namespace histograms {

// Overloading the Histogram_comparator for various data types.
template <>
bool Histogram_comparator::operator()(const String &lhs,
                                      const String &rhs) const {
  // The collation MUST be the same
  DBUG_ASSERT(lhs.charset()->number == rhs.charset()->number);

  // The number of characters should already be limited.
  DBUG_ASSERT(lhs.numchars() <= HISTOGRAM_MAX_COMPARE_LENGTH);
  DBUG_ASSERT(rhs.numchars() <= HISTOGRAM_MAX_COMPARE_LENGTH);

  return sortcmp(&lhs, &rhs, lhs.charset()) < 0;
}

template <>
bool Histogram_comparator::operator()(const MYSQL_TIME &lhs,
                                      const MYSQL_TIME &rhs) const {
  longlong lhs_packed = TIME_to_longlong_packed(lhs);
  longlong rhs_packed = TIME_to_longlong_packed(rhs);
  return lhs_packed < rhs_packed;
}

template <>
bool Histogram_comparator::operator()(const my_decimal &lhs,
                                      const my_decimal &rhs) const {
  return my_decimal_cmp(&lhs, &rhs) < 0;
}

Value_map_base::Value_map_base(const CHARSET_INFO *charset,
                               double sampling_rate, Value_map_type data_type)
    : m_sampling_rate(sampling_rate),
      m_charset(charset),
      m_num_null_values(0),
      m_data_type(data_type) {
  init_alloc_root(key_memory_histograms, &m_mem_root, 256, 0);
}

template <class T>
bool Value_map_base::add_values(const T &value, const ha_rows count) {
  Value_map<T> *value_map = down_cast<Value_map<T> *>(this);
  return value_map->add_values(value, count);
}

template <class T>
bool Value_map<T>::add_values(const T &value, const ha_rows count) {
  try {
    auto res = m_value_map.emplace(value, count);
    if (!res.second) res.first->second += count;
  } catch (const std::bad_alloc &) {
    // Out of memory.
    return true;
  }
  return false;
}

template <>
bool Value_map<String>::add_values(const String &value, const ha_rows count) {
  /*
    We only consider the susbtring. That is, if the strings differs after
    character number HISTOGRAM_MAX_COMPARE_LENGTH, they will be considered
    equal.

    When inserting a new string, we make a duplicate of it so that it survives
    when the original string goes out of scope. The duplicate string is
    allocated in the MEM_ROOT of the Value_map, so that is is automatically
    reclaimed when the Value_map is destroyed.
  */
  String substring = value.substr(0, HISTOGRAM_MAX_COMPARE_LENGTH);
  auto found = m_value_map.find(substring);
  if (found == m_value_map.end()) {
    // Not found, insert a new value.
    try {
      char *string_data = substring.dup(&m_mem_root);
      if (string_data == nullptr) return true; /* purecov: deadcode */

      String string_dup(string_data, substring.length(), substring.charset());
      m_value_map.emplace(string_dup, count);
    } catch (const std::bad_alloc &) {
      // Out of memory.
      return true;
    }
  } else {
    found->second += count;
  }
  return false;
}

template <class T>
bool Value_map<T>::insert(typename value_map_type::const_iterator begin,
                          typename value_map_type::const_iterator end) {
  try {
    DBUG_ASSERT(m_value_map.empty());
    m_value_map.insert(begin, end);
  } catch (const std::bad_alloc &) {
    // Out of memory.
    return true;
  }

  return false;
}

template <class T>
Histogram *Value_map<T>::build_histogram(MEM_ROOT *mem_root, size_t num_buckets,
                                         const std::string &db_name,
                                         const std::string &tbl_name,
                                         const std::string &col_name) const {
  return histograms::build_histogram(mem_root, *this, num_buckets, db_name,
                                     tbl_name, col_name);
}

// Explicit template instantiations.
template class Value_map<double>;
template class Value_map<String>;
template class Value_map<ulonglong>;
template class Value_map<longlong>;
template class Value_map<MYSQL_TIME>;
template class Value_map<my_decimal>;

template bool Value_map_base::add_values(const double &, const ha_rows);
template bool Value_map_base::add_values(const String &, const ha_rows);
template bool Value_map_base::add_values(const ulonglong &, const ha_rows);
template bool Value_map_base::add_values(const longlong &, const ha_rows);
template bool Value_map_base::add_values(const MYSQL_TIME &, const ha_rows);
template bool Value_map_base::add_values(const my_decimal &, const ha_rows);

}  // namespace histograms
