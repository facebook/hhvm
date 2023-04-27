/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file sql/histograms/singleton.cc
  Singleton histogram (implementation).
*/

#include "sql/histograms/singleton.h"

#include <iterator>
#include <new>
#include <utility>  // std::make_pair

#include "field_types.h"  // enum_field_types
#include "my_base.h"      // ha_rows
#include "my_dbug.h"
#include "my_inttypes.h"
#include "mysql_time.h"
#include "sql/histograms/value_map.h"  // Value_map
#include "sql/json_dom.h"              // Json_*
#include "template_utils.h"

struct MEM_ROOT;

namespace histograms {

template <class T>
Singleton<T>::Singleton(MEM_ROOT *mem_root, const std::string &db_name,
                        const std::string &tbl_name,
                        const std::string &col_name, Value_map_type data_type)
    : Histogram(mem_root, db_name, tbl_name, col_name,
                enum_histogram_type::SINGLETON, data_type),
      m_buckets(Histogram_comparator(), singleton_buckets_allocator(mem_root)) {
}

template <class T>
Singleton<T>::Singleton(MEM_ROOT *mem_root, const Singleton<T> &other)
    : Histogram(mem_root, other),
      m_buckets(other.m_buckets.begin(), other.m_buckets.end(),
                Histogram_comparator(), singleton_buckets_allocator(mem_root)) {
}

template <>
Singleton<String>::Singleton(MEM_ROOT *mem_root, const Singleton<String> &other)
    : Histogram(mem_root, other),
      m_buckets(Histogram_comparator(), singleton_buckets_allocator(mem_root)) {
  /*
    Copy bucket contents. We need to make duplicates of String data, since they
    are allocated on a MEM_ROOT that most likely will be freed way too early.
  */
  for (const auto &bucket : other.m_buckets) {
    char *string_data = bucket.first.dup(mem_root);
    if (string_data == nullptr) {
      DBUG_ASSERT(false); /* purecov: deadcode */
      return;             // OOM
    }

    String string_dup(string_data, bucket.first.length(),
                      bucket.first.charset());
    m_buckets.emplace(string_dup, bucket.second);
  }
}

template <class T>
bool Singleton<T>::build_histogram(const Value_map<T> &value_map,
                                   size_t num_buckets) {
  // Clear any existing data.
  m_buckets.clear();
  m_null_values_fraction = INVALID_NULL_VALUES_FRACTION;
  m_sampling_rate = value_map.get_sampling_rate();

  // Set the number of buckets that was specified/requested by the user.
  m_num_buckets_specified = num_buckets;

  // Set the character set for the histogram data.
  m_charset = value_map.get_character_set();

  // Get total frequency count.
  ha_rows num_non_null_values = 0;
  for (const auto &node : value_map) num_non_null_values += node.second;

  // No values, nothing to do.
  if (num_non_null_values == 0) {
    if (value_map.get_num_null_values() > 0)
      m_null_values_fraction = 1.0;
    else
      m_null_values_fraction = 0.0;

    return false;
  }

  const ha_rows total_count =
      value_map.get_num_null_values() + num_non_null_values;

  // Set the fractions of NULL values.
  m_null_values_fraction =
      value_map.get_num_null_values() / static_cast<double>(total_count);

  // Create buckets with relative frequency, and not absolute frequency.
  double cumulative_frequency = 0.0;

  /*
    Since we are using a std::map with Mem_root_allocator, we are forced to wrap
    the following section in a try-catch. The Mem_root_allocator will throw an
    exception of class std::bad_alloc when it runs out of memory.
  */
  try {
    for (const auto &node : value_map) {
      const double frequency = node.second / static_cast<double>(total_count);
      cumulative_frequency += frequency;

      m_buckets.emplace(node.first, cumulative_frequency);
    }
  } catch (const std::bad_alloc &) {
    // Out of memory.
    return true;
  }

  return false;
}

template <class T>
bool Singleton<T>::histogram_to_json(Json_object *json_object) const {
  /*
    Call the base class implementation first. This will add the properties that
    are common among different histogram types, such as "last-updated" and
    "histogram-type".
  */
  if (Histogram::histogram_to_json(json_object))
    return true; /* purecov: inspected */

  // Add the Singleton buckets.
  Json_array json_buckets;
  for (const auto &bucket : m_buckets) {
    Json_array json_bucket;
    if (create_json_bucket(bucket, &json_bucket))
      return true; /* purecov: inspected */
    if (json_buckets.append_clone(&json_bucket))
      return true; /* purecov: inspected */
  }

  if (json_object->add_clone(buckets_str(), &json_buckets))
    return true; /* purecov: inspected */

  if (histogram_data_type_to_json(json_object))
    return true; /* purecov: inspected */
  return false;
}

template <class T>
bool Singleton<T>::create_json_bucket(const std::pair<T, double> &bucket,
                                      Json_array *json_bucket) {
  // Value
  if (add_value_json_bucket(bucket.first, json_bucket))
    return true; /* purecov: inspected */

  // Cumulative frequency
  const Json_double frequency(bucket.second);
  if (json_bucket->append_clone(&frequency))
    return true; /* purecov: inspected */
  return false;
}

template <>
bool Singleton<double>::add_value_json_bucket(const double &value,
                                              Json_array *json_bucket) {
  const Json_double json_value(value);
  if (json_bucket->append_clone(&json_value))
    return true; /* purecov: inspected */
  return false;
}

template <>
bool Singleton<String>::add_value_json_bucket(const String &value,
                                              Json_array *json_bucket) {
  const Json_opaque json_value(enum_field_types::MYSQL_TYPE_STRING, value.ptr(),
                               value.length());
  if (json_bucket->append_clone(&json_value))
    return true; /* purecov: inspected */
  return false;
}

template <>
bool Singleton<ulonglong>::add_value_json_bucket(const ulonglong &value,
                                                 Json_array *json_bucket) {
  const Json_uint json_value(value);
  if (json_bucket->append_clone(&json_value))
    return true; /* purecov: inspected */
  return false;
}

template <>
bool Singleton<longlong>::add_value_json_bucket(const longlong &value,
                                                Json_array *json_bucket) {
  const Json_int json_value(value);
  if (json_bucket->append_clone(&json_value))
    return true; /* purecov: inspected */
  return false;
}

template <>
bool Singleton<MYSQL_TIME>::add_value_json_bucket(const MYSQL_TIME &value,
                                                  Json_array *json_bucket) {
  enum_field_types field_type;
  switch (value.time_type) {
    case MYSQL_TIMESTAMP_DATE:
      field_type = MYSQL_TYPE_DATE;
      break;
    case MYSQL_TIMESTAMP_DATETIME:
      field_type = MYSQL_TYPE_DATETIME;
      break;
    case MYSQL_TIMESTAMP_TIME:
      field_type = MYSQL_TYPE_TIME;
      break;
    default:
      /* purecov: begin deadcode */
      DBUG_ASSERT(false);
      return true;
      /* purecov: end */
  }

  const Json_datetime json_value(value, field_type);
  if (json_bucket->append_clone(&json_value))
    return true; /* purecov: inspected */
  return false;
}

template <>
bool Singleton<my_decimal>::add_value_json_bucket(const my_decimal &value,
                                                  Json_array *json_bucket) {
  const Json_decimal json_value(value);
  if (json_bucket->append_clone(&json_value))
    return true; /* purecov: inspected */
  return false;
}

template <class T>
std::string Singleton<T>::histogram_type_to_str() const {
  return singleton_str();
}

template <class T>
bool Singleton<T>::json_to_histogram(const Json_object &json_object) {
  if (Histogram::json_to_histogram(json_object))
    return true; /* purecov: deadcode */

  const Json_dom *buckets_dom = json_object.get(buckets_str());
  if (buckets_dom == nullptr ||
      buckets_dom->json_type() != enum_json_type::J_ARRAY)
    return true; /* purecov: deadcode */

  const Json_array *buckets = down_cast<const Json_array *>(buckets_dom);
  for (size_t i = 0; i < buckets->size(); ++i) {
    const Json_dom *bucket_dom = (*buckets)[i];
    if (bucket_dom == nullptr ||
        bucket_dom->json_type() != enum_json_type::J_ARRAY)
      return true; /* purecov: deadcode */

    const Json_array *bucket = down_cast<const Json_array *>(bucket_dom);
    if (bucket->size() != 2) return true; /* purecov: deadcode */

    // First item is the value, second is the cumulative frequency
    const Json_dom *cumulative_frequency_dom = (*bucket)[1];
    if (cumulative_frequency_dom->json_type() != enum_json_type::J_DOUBLE)
      return true; /* purecov: deadcode */

    const Json_double *cumulative_frequency =
        down_cast<const Json_double *>(cumulative_frequency_dom);

    const Json_dom *value_dom = (*bucket)[0];
    T value;
    if (extract_json_dom_value(value_dom, &value))
      return true; /* purecov: deadcode */

    m_buckets.emplace(value, cumulative_frequency->value());
  }
  return false;
}

template <class T>
Histogram *Singleton<T>::clone(MEM_ROOT *mem_root) const {
  DBUG_EXECUTE_IF("fail_histogram_clone", return nullptr;);

  try {
    return new (mem_root) Singleton<T>(mem_root, *this);
  } catch (const std::bad_alloc &) {
    return nullptr; /* purecov: deadcode */
  }
}

template <class T>
double Singleton<T>::get_equal_to_selectivity(const T &value) const {
  /*
    Find the first histogram bucket where the value is not less than the
    user-provided value.
  */
  const auto found = m_buckets.lower_bound(value);

  if (found == m_buckets.end()) return 0.0;

  if (Histogram_comparator()(value, found->first) == 0) {
    if (found == m_buckets.begin())
      return found->second;
    else {
      const auto previous = std::prev(found, 1);
      return found->second - previous->second;
    }
  }

  return 0.0;
}

template <class T>
double Singleton<T>::get_less_than_selectivity(const T &value) const {
  /*
    Find the first histogram bucket where the value is not less than the
    user-provided value.
  */
  const auto found = m_buckets.lower_bound(value);
  if (found == m_buckets.begin())
    return 0.0;
  else {
    const auto previous = std::prev(found, 1);
    return previous->second;
  }
}

template <class T>
double Singleton<T>::get_greater_than_selectivity(const T &value) const {
  /*
    Find the first histogram bucket where the value is greater than the
    user-provided value.
  */
  const auto found = m_buckets.upper_bound(value);

  if (found == m_buckets.begin())
    return get_non_null_values_frequency();
  else {
    const auto previous = std::prev(found, 1);
    return get_non_null_values_frequency() - previous->second;
  }
}

// Explicit template instantiations.
template class Singleton<double>;
template class Singleton<String>;
template class Singleton<ulonglong>;
template class Singleton<longlong>;
template class Singleton<MYSQL_TIME>;
template class Singleton<my_decimal>;

}  // namespace histograms
