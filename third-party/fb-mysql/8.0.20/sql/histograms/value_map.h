#ifndef HISTOGRAMS_VALUE_MAP_INCLUDED
#define HISTOGRAMS_VALUE_MAP_INCLUDED

/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file sql/histograms/value_map.h
*/

#include <stddef.h>
#include <functional>  // std::less
#include <map>
#include <string>
#include <utility>  // std::pair

#include "m_ctype.h"
#include "my_alloc.h"
#include "my_base.h"  // ha_rows
#include "mysql_time.h"
#include "sql/histograms/value_map_type.h"

class String;
class my_decimal;
template <class T>
class Mem_root_allocator;

namespace histograms {

class Histogram;
namespace equi_height {
template <class T>
class Bucket;
}  // namespace equi_height

/**
  The maximum number of characters to evaluate when building histograms. For
  binary/blob values, this is the number of bytes to consider.
*/
static const size_t HISTOGRAM_MAX_COMPARE_LENGTH = 42;

/**
  Histogram comparator.

  Typical usage is in a "value map", where we for instance need to sort based
  on string collation and similar.
*/
struct Histogram_comparator {
 public:
  /**
    Overload operator(), so that we can use this struct as a custom comparator
    in std classes/functions.

    @param lhs first value to compare
    @param rhs second value to compare

    @return true if lhs is considered to be smaller/less than rhs. false
            otherwise.
  */
  template <class T>
  bool operator()(const T &lhs, const T &rhs) const {
    return std::less<T>()(lhs, rhs);
  }

  template <class T>
  bool operator()(const equi_height::Bucket<T> &a, const T &b) const {
    return Histogram_comparator()(a.get_upper_inclusive(), b);
  }

  template <class T>
  bool operator()(const equi_height::Bucket<T> &a,
                  const equi_height::Bucket<T> &b) const {
    return Histogram_comparator()(a.get_upper_inclusive(),
                                  b.get_lower_inclusive());
  }
};

/**
  The abstract base class for all Value_map types.

  We would ideally like to have only one class for the Value map concept (no
  inheritance) which would gives us an easier interface. But there are some
  reasons for why we need to to split the class into a non-templated base class
  and a templated subclass:

  - We are collecting Value_maps in a collection/vector where they have a
    different template type. This cannot be achieved unless we have a
    non-templated base class.

  - When working on a collection of Value_maps, it is more convenient to declare
    the interface in the base class (Value_map_base) so that we don't need to
    do a cast to the subclass in order to get hold of the methods we want to
    use.

  - Value_map_base::add_values and Value_map::add_values looks like the same
    function, but they are not. Value_map_base::add_values is a small functions
    that helps us cast the Value_map<T> to the correct type (for instance
    Value_map<longlong>). Ideally, this function would have been pure virtual,
    but it's not possible to have virtual member function templates.
*/
class Value_map_base {
 private:
  double m_sampling_rate;
  const CHARSET_INFO *m_charset;
  ha_rows m_num_null_values;
  const Value_map_type m_data_type;

 protected:
  MEM_ROOT m_mem_root;

 public:
  Value_map_base(const CHARSET_INFO *charset, double sampling_rate,
                 Value_map_type data_type);

  virtual ~Value_map_base() {}

  /**
    Returns the number of [value, count] pairs in the Value_map.

    @return The number of values in the Value_map.
  */
  virtual size_t size() const = 0;

  /**
    Add a value with the given count to this Value_map. If the given value
    already exists, the count will be added to the existing count.

    @param value The value to add.
    @param count Count of the value to add.

    @return false on success, and true in case of errors (OOM).
  */
  template <class T>
  bool add_values(const T &value, const ha_rows count);

  /**
    Increase the number of null values with the given count.

    @param count The number of null values.
  */
  void add_null_values(const ha_rows count) { m_num_null_values += count; }

  /// @return The number of null values in this Value_map.
  ha_rows get_num_null_values() const { return m_num_null_values; }

  /**
    Create a Histogram from this Value_map.

    The resulting histogram will have at most "num_buckets" buckets (might be
    less), and all of its contents will be allocated on the supplied MEM_ROOT.

    @param mem_root The MEM_ROOT to allocate the contents on
    @param num_buckets Maximum number of buckets to create
    @param db_name Database name
    @param tbl_name Table name
    @param col_name Column name

    @return nullptr on error, or a valid histogram if success.
  */
  virtual Histogram *build_histogram(MEM_ROOT *mem_root, size_t num_buckets,
                                     const std::string &db_name,
                                     const std::string &tbl_name,
                                     const std::string &col_name) const = 0;

  /// @return The sampling rate that was used to generate this Value_map.
  double get_sampling_rate() const { return m_sampling_rate; }

  /**
    Set the sampling rate that was used to generate this Value_map.

    @param sampling_rate The sampling rate.
  */
  void set_sampling_rate(double sampling_rate) {
    m_sampling_rate = sampling_rate;
  }

  /// @return the character set for the data this Value_map contains
  const CHARSET_INFO *get_character_set() const { return m_charset; }

  /// @return the data type that this Value_map contains
  Value_map_type get_data_type() const { return m_data_type; }

  /// @return the overhead in bytes for each distinct value stored in the
  ///         Value_map.
  virtual size_t element_overhead() const = 0;
};

/**
  Value_map class.

  This class works as a map. It is a collection of [key, count], where "count"
  is the number of occurances of "key". The class abstracts away things like
  duplicate checking and the underlying container.
*/
template <class T>
class Value_map final : public Value_map_base {
 private:
  using value_map_type =
      std::map<T, ha_rows, Histogram_comparator,
               Mem_root_allocator<std::pair<const T, ha_rows>>>;

  value_map_type m_value_map;

 public:
  Value_map(const CHARSET_INFO *charset, Value_map_type data_type,
            double sampling_rate = 0.0)
      : Value_map_base(charset, sampling_rate, data_type),
        m_value_map(typename value_map_type::allocator_type(&m_mem_root)) {}

  size_t size() const override { return m_value_map.size(); }

  typename value_map_type::const_iterator begin() const {
    return m_value_map.cbegin();
  }

  typename value_map_type::const_iterator end() const {
    return m_value_map.cend();
  }

  bool add_values(const T &value, const ha_rows count);

  /**
    Insert a range of values into the Value_map.

    Values in the range (begin, end] must be sorted according to
    Histogram_comparator. Note that this function is currently only used in
    unit testing.

    @note The value map must be empty before calling this function.

    @param begin Iterator that points to the beginning of the range.
    @param end Iterator that points to the end of the range.

    @return false on success, true on error (OOM or similar).
  */
  bool insert(typename value_map_type::const_iterator begin,
              typename value_map_type::const_iterator end);

  virtual Histogram *build_histogram(
      MEM_ROOT *mem_root, size_t num_buckets, const std::string &db_name,
      const std::string &tbl_name, const std::string &col_name) const override;

  /// @return the overhead in bytes for each distinct value stored in the
  ///         Value_map. The value 32 is obtained from both GCC 8.2 and
  ///         Clang 8.0 (same as sizeof(value_map_type::node_type) in C++17).
  size_t element_overhead() const override {
    // TODO: Replace this with sizeof(value_map_type::node_type) when we have
    // full C++17 support.
    return sizeof(typename value_map_type::value_type) +
           sizeof(typename value_map_type::key_type) + 32;
  }
};

// Explicit template instantiations.
template <>
bool Histogram_comparator::operator()(const String &, const String &) const;

template <>
bool Histogram_comparator::operator()(const MYSQL_TIME &,
                                      const MYSQL_TIME &) const;

template <>
bool Histogram_comparator::operator()(const my_decimal &,
                                      const my_decimal &) const;

}  // namespace histograms

#endif
