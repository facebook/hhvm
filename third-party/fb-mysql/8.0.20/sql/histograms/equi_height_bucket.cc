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
  @file sql/histograms/equi_height_bucket.cc
  Equi-height bucket (implementation).
*/

#include "sql/histograms/equi_height_bucket.h"  // equi_height::Bucket

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <algorithm>
#include <limits>
#include <memory>  // std::unique_ptr

#include "field_types.h"  // enum_field_types
#include "m_ctype.h"
#include "my_base.h"  // ha_rows
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_time.h"
#include "mysql_time.h"
#include "sql/histograms/value_map.h"  // Histogram_comparator
#include "sql/json_dom.h"              // Json_*
#include "sql/my_decimal.h"            // my_decimal_cmp
#include "sql/sql_time.h"              // calc_time_diff
#include "template_utils.h"

namespace histograms {
namespace equi_height {

template <typename T>
Bucket<T>::Bucket(T lower, T upper, double freq, ha_rows num_distinct)
    : m_lower_inclusive(lower),
      m_upper_inclusive(upper),
      m_cumulative_frequency(freq),
      m_num_distinct(num_distinct) {
  DBUG_ASSERT(m_cumulative_frequency >= 0.0);
  DBUG_ASSERT(m_cumulative_frequency <= 1.0);
  DBUG_ASSERT(m_num_distinct >= 1);
  DBUG_ASSERT(!histograms::Histogram_comparator()(upper, lower));
}

template <typename T>
bool Bucket<T>::bucket_to_json(Json_array *json_array) const {
  // Lower and upper inclusive value.
  if (add_values_json_bucket(get_lower_inclusive(), get_upper_inclusive(),
                             json_array))
    return true; /* purecov: inspected */

  // Cumulative frequency.
  const Json_double frequency(get_cumulative_frequency());
  if (json_array->append_clone(&frequency))
    return true; /* purecov: inspected */

  // Number of distinct values.
  const Json_uint num_distinct(get_num_distinct());
  if (json_array->append_clone(&num_distinct))
    return true; /* purecov: inspected */
  return false;
}

template <>
bool Bucket<double>::add_values_json_bucket(const double &lower_value,
                                            const double &upper_value,
                                            Json_array *json_array) {
  const Json_double json_lower_value(lower_value);
  if (json_array->append_clone(&json_lower_value))
    return true; /* purecov: inspected */

  const Json_double json_upper_value(upper_value);
  if (json_array->append_clone(&json_upper_value))
    return true; /* purecov: inspected */
  return false;
}

template <>
bool Bucket<String>::add_values_json_bucket(const String &lower_value,
                                            const String &upper_value,
                                            Json_array *json_array) {
  const Json_opaque json_lower_value(enum_field_types::MYSQL_TYPE_STRING,
                                     lower_value.ptr(), lower_value.length());
  if (json_array->append_clone(&json_lower_value))
    return true; /* purecov: inspected */

  const Json_opaque json_upper_value(enum_field_types::MYSQL_TYPE_STRING,
                                     upper_value.ptr(), upper_value.length());
  if (json_array->append_clone(&json_upper_value))
    return true; /* purecov: inspected */
  return false;
}

template <>
bool Bucket<ulonglong>::add_values_json_bucket(const ulonglong &lower_value,
                                               const ulonglong &upper_value,
                                               Json_array *json_array) {
  const Json_uint json_lower_value(lower_value);
  if (json_array->append_clone(&json_lower_value))
    return true; /* purecov: inspected */

  const Json_uint json_upper_value(upper_value);
  if (json_array->append_clone(&json_upper_value))
    return true; /* purecov: inspected */
  return false;
}

template <>
bool Bucket<longlong>::add_values_json_bucket(const longlong &lower_value,
                                              const longlong &upper_value,
                                              Json_array *json_array) {
  const Json_int json_lower_value(lower_value);
  if (json_array->append_clone(&json_lower_value))
    return true; /* purecov: inspected */

  const Json_int json_upper_value(upper_value);
  if (json_array->append_clone(&json_upper_value))
    return true; /* purecov: inspected */
  return false;
}

template <>
bool Bucket<MYSQL_TIME>::add_values_json_bucket(const MYSQL_TIME &lower_value,
                                                const MYSQL_TIME &upper_value,
                                                Json_array *json_array) {
  DBUG_ASSERT(lower_value.time_type == upper_value.time_type);

  enum_field_types field_type;
  switch (lower_value.time_type) {
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

  const Json_datetime json_lower_value(lower_value, field_type);
  if (json_array->append_clone(&json_lower_value))
    return true; /* purecov: inspected */

  const Json_datetime json_upper_value(upper_value, field_type);
  if (json_array->append_clone(&json_upper_value))
    return true; /* purecov: inspected */
  return false;
}

template <>
bool Bucket<my_decimal>::add_values_json_bucket(const my_decimal &lower_value,
                                                const my_decimal &upper_value,
                                                Json_array *json_array) {
  const Json_decimal json_lower_value(lower_value);
  if (json_array->append_clone(&json_lower_value))
    return true; /* purecov: inspected */

  const Json_decimal json_upper_value(upper_value);
  if (json_array->append_clone(&json_upper_value))
    return true; /* purecov: inspected */
  return false;
}

template <class T>
static bool values_are_equal(const T &val1, const T &val2) {
  return (!Histogram_comparator()(val1, val2) &&
          !Histogram_comparator()(val2, val1));
}

template <class T>
double Bucket<T>::get_distance_from_lower(const T &value) const {
  if (Histogram_comparator()(value, get_lower_inclusive()))
    return 0.0;
  else if (values_are_equal(get_lower_inclusive(), get_upper_inclusive()))
    return 1.0;

  // Make sure that double arithmeric is used in case of very large values.
  const double lower_inclusive = static_cast<double>(get_lower_inclusive());
  return (value - lower_inclusive + 1.0) /
         (get_upper_inclusive() - lower_inclusive + 1.0);
}

template <>
double Bucket<double>::get_distance_from_lower(const double &value) const {
  if (Histogram_comparator()(value, get_lower_inclusive()))
    return 0.0;
  else if (values_are_equal(get_lower_inclusive(), get_upper_inclusive()))
    return 1.0;

  return (value - get_lower_inclusive()) /
         (get_upper_inclusive() - get_lower_inclusive());
}

/**
  Convert an array of uchar values into an 64bit unsigned integer. It simply
  does an bitwise concatenation of the eight first values of the array into a
  single integer. It will do something like this, given an array of five
  uchar values {62, 28, 62, 28, 51}:

  result:      0000000000000000000000000000000000000000000000000000000000000000
  first byte:  00111110 (byte value 62)
  second byte: |       00011100 (byte value 28)
  third byte:  |       |       00111110 (byte value 62)
  fourth byte: |       |       |       00011100 (byte value 28)
  fifth byte:  |       |       |       |       00111101 (byte value 51)
               |       |       |       |       |
  result:      0011111000011100001111100001110000111101000000000000000000000000

  @param ptr The input value to convert.
  @param len The length of the input array.

  @return The converted value.
*/
static std::uint64_t uchar_array_to_64bit_unsigned(const uchar *ptr,
                                                   size_t len) {
  std::uint64_t result = 0U;
  for (size_t i = 0; i < sizeof(result) && i < len; ++i)
    result = (result << 8) | ptr[i];

  for (size_t i = len; i < sizeof(result); ++i) result = (result << 8) | 0;

  return result;
}

/**
  An attempt to calculate the distance for string values. It goes like this:

  First we transform the lower inclusive value, the upper inclusive value and
  the query value (COLUMN = "query value") using the method "my_strnxfrm". The
  result of "my_strnfxfrm" is a transformed string that gives us the correct
  sort order by using "memcmp" on the transformed values. For example, the
  strings "110", "120" and "130" in utf8mb4_0900_ai_ci gives us the following
  byte values:

    "110":  28  62  28  62  28  61
    "120":  28  63  28  63  28  61
    "130":  28  64  28  64  28  61

  We then find the common prefix for these three transformed strings, which is
  the byte value 28 in this case. We remove this, and ends up with the
  following:

    "110":  62  28  62  28  61
    "120":  63  28  63  28  61
    "130":  64  28  64  28  61

  The reason for removing the common prefix is that if the eight first byte
  values are the same, the next step will fail.

  We then make a 8-byte unsigned integer representation for each of these
  values by concatenating together all the byte values.

  @see uchar_array_to_64bit_unsigned

  If the eight first bytes were equal, we would end up with equal values here.
  Do this for all three, and with these values we can find out the distance
  from the lower inclusive value to the "query value". Note that this will NOT
  be 100% accurate, but it will gives us a rough estimate (and a far better
  estimate than nothing at all).

  @param value The value to calculate the distance for.

  @return The distance, between 0.0 and 1.0 inclusive.
*/
template <>
double Bucket<String>::get_distance_from_lower(const String &value) const {
  DBUG_ASSERT(value.charset()->number ==
              get_lower_inclusive().charset()->number);

  if (Histogram_comparator()(value, get_lower_inclusive()))
    return 0.0;
  else if (values_are_equal(get_lower_inclusive(), get_upper_inclusive()))
    return 1.0;

  uint max_strnxfrm_len = value.charset()->coll->strnxfrmlen(
      value.charset(), HISTOGRAM_MAX_COMPARE_LENGTH);

  std::unique_ptr<uchar[]> value_buf(new uchar[max_strnxfrm_len]());
  std::unique_ptr<uchar[]> upper_buf(new uchar[max_strnxfrm_len]());
  std::unique_ptr<uchar[]> lower_buf(new uchar[max_strnxfrm_len]());

  const uchar *ptr = pointer_cast<const uchar *>(value.ptr());
  size_t value_len = my_strnxfrm(value.charset(), value_buf.get(),
                                 max_strnxfrm_len, ptr, value.length());

  const uchar *ptr2 = pointer_cast<const uchar *>(get_lower_inclusive().ptr());
  size_t lower_len =
      my_strnxfrm(value.charset(), lower_buf.get(), max_strnxfrm_len, ptr2,
                  get_lower_inclusive().length());

  const uchar *ptr3 = pointer_cast<const uchar *>(get_upper_inclusive().ptr());
  size_t upper_len =
      my_strnxfrm(value.charset(), upper_buf.get(), max_strnxfrm_len, ptr3,
                  get_upper_inclusive().length());

  // Find the common prefix from these three arrays
  size_t shortest_buffer = std::min(value_len, lower_len);
  shortest_buffer = std::min(shortest_buffer, upper_len);
  size_t start_index = 0;
  while (value_buf.get()[start_index] == upper_buf.get()[start_index] &&
         upper_buf.get()[start_index] == lower_buf.get()[start_index] &&
         start_index < shortest_buffer) {
    start_index++;
  }

  std::uint64_t lower_converted = uchar_array_to_64bit_unsigned(
      lower_buf.get() + start_index, lower_len - start_index);
  std::uint64_t upper_converted = uchar_array_to_64bit_unsigned(
      upper_buf.get() + start_index, upper_len - start_index);
  if (upper_converted == lower_converted) {
    /*
      This should not be hit, since we already have checked for equal upper
      and lower values at the beginning of this function.
    */
    return 1.0; /* purecov: deadcode */
  }

  std::uint64_t value_converted = uchar_array_to_64bit_unsigned(
      value_buf.get() + start_index, value_len - start_index);

  DBUG_ASSERT(lower_converted <= value_converted);
  DBUG_ASSERT(value_converted <= upper_converted);

  return (value_converted - lower_converted) /
         static_cast<double>(upper_converted - lower_converted);
}

template <>
double Bucket<MYSQL_TIME>::get_distance_from_lower(
    const MYSQL_TIME &value) const {
  MYSQL_TIME lower_modified = get_lower_inclusive();
  MYSQL_TIME upper_modified = get_upper_inclusive();
  MYSQL_TIME value_modified = value;

  if (value.time_type == MYSQL_TIMESTAMP_DATE ||
      get_lower_inclusive().time_type == MYSQL_TIMESTAMP_DATE) {
    // Calculate the difference using only the date part.
    TIME_set_hhmmss(&lower_modified, 0);
    TIME_set_hhmmss(&upper_modified, 0);
    TIME_set_hhmmss(&value_modified, 0);

    lower_modified.second_part = 0;
    upper_modified.second_part = 0;
    value_modified.second_part = 0;
  }

  if (Histogram_comparator()(value_modified, lower_modified))
    return 0.0;
  else if (values_are_equal(lower_modified, upper_modified))
    return 1.0;

  /*
    Calculate the difference in seconds between the upper inclusive value and
    the lower inclusive value of the bucket.
  */
  longlong upper_lower_diff_seconds;
  long upper_lower_diff_microseconds;
  int sign = lower_modified.neg != upper_modified.neg ? -1 : 1;
  calc_time_diff(lower_modified, upper_modified, sign,
                 &upper_lower_diff_seconds, &upper_lower_diff_microseconds);
  double upper_lower_diff =
      upper_lower_diff_seconds + (upper_lower_diff_microseconds / 1000000.0);

  /*
    Calculate the difference in seconds between the lower inclusive value of
    the bucket and the provided parameter value.
  */
  longlong value_lower_diff_seconds;
  long value_lower_diff_microseconds;
  sign = lower_modified.neg != value_modified.neg ? -1 : 1;
  calc_time_diff(lower_modified, value_modified, sign,
                 &value_lower_diff_seconds, &value_lower_diff_microseconds);
  double value_lower_diff =
      value_lower_diff_seconds + (value_lower_diff_microseconds / 1000000.0);

  return value_lower_diff / upper_lower_diff;
}

template <>
double Bucket<my_decimal>::get_distance_from_lower(
    const my_decimal &value) const {
  if (Histogram_comparator()(value, get_lower_inclusive()))
    return 0.0;
  else if (values_are_equal(get_lower_inclusive(), get_upper_inclusive()))
    return 1.0;

  double lower_inclusive_double;
  double upper_inclusive_double;
  double value_double;

  /*
    The documentation for my_decimal2double says "No need to call check_result
    as this will always succeed". So we don't check return result.
  */
  my_decimal2double(0, &get_lower_inclusive(), &lower_inclusive_double);
  my_decimal2double(0, &get_upper_inclusive(), &upper_inclusive_double);
  my_decimal2double(0, &value, &value_double);

  return (value_double - lower_inclusive_double) /
         (upper_inclusive_double - lower_inclusive_double);
}

template <class T>
double Bucket<T>::value_probability() const {
  /*
    As default, assume that the number of possible values between the lower and
    upper value of the bucket is VERY high.
  */
  if (values_are_equal(get_lower_inclusive(), get_upper_inclusive()))
    return 1.0;
  return get_num_distinct() / std::numeric_limits<double>::max();
}

template <>
double Bucket<longlong>::value_probability() const {
  return get_num_distinct() / (static_cast<double>(get_upper_inclusive()) -
                               get_lower_inclusive() + 1);
}

template <>
double Bucket<ulonglong>::value_probability() const {
  return get_num_distinct() / (static_cast<double>(get_upper_inclusive()) -
                               get_lower_inclusive() + 1);
}

// Explicit template instantiations.
template class Bucket<double>;
template class Bucket<String>;
template class Bucket<ulonglong>;
template class Bucket<longlong>;
template class Bucket<MYSQL_TIME>;
template class Bucket<my_decimal>;

}  // namespace equi_height
}  // namespace histograms
