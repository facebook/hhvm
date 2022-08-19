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
  @file sql/histograms/equi_height.cc
  Equi-height histogram (implementation).
*/

#include "sql/histograms/equi_height.h"

#include <stdlib.h>
#include <cmath>  // std::lround
#include <iterator>
#include <new>

#include "my_base.h"  // ha_rows
#include "my_dbug.h"
#include "my_inttypes.h"
#include "mysql_time.h"
#include "sql/histograms/equi_height_bucket.h"
#include "sql/histograms/value_map.h"  // Value_map
#include "sql/json_dom.h"              // Json_*
#include "sql/mem_root_allocator.h"
#include "sql_string.h"
#include "template_utils.h"

class my_decimal;
struct MEM_ROOT;

namespace histograms {

template <class T>
Equi_height<T>::Equi_height(MEM_ROOT *mem_root, const std::string &db_name,
                            const std::string &tbl_name,
                            const std::string &col_name,
                            Value_map_type data_type)
    : Histogram(mem_root, db_name, tbl_name, col_name,
                enum_histogram_type::EQUI_HEIGHT, data_type),
      m_buckets(Histogram_comparator(),
                Mem_root_allocator<equi_height::Bucket<T>>(mem_root)) {}

template <class T>
Equi_height<T>::Equi_height(MEM_ROOT *mem_root, const Equi_height<T> &other)
    : Histogram(mem_root, other),
      m_buckets(Histogram_comparator(),
                Mem_root_allocator<equi_height::Bucket<T>>(mem_root)) {
  for (const auto &bucket : other.m_buckets) m_buckets.emplace(bucket);
}

template <>
Equi_height<String>::Equi_height(MEM_ROOT *mem_root,
                                 const Equi_height<String> &other)
    : Histogram(mem_root, other),
      m_buckets(Histogram_comparator(),
                Mem_root_allocator<equi_height::Bucket<String>>(mem_root)) {
  /*
    Copy bucket contents. We need to make duplicates of String data, since they
    are allocated on a MEM_ROOT that most likely will be freed way too early.
  */
  for (const auto &pair : other.m_buckets) {
    char *lower_string_data = pair.get_lower_inclusive().dup(mem_root);
    char *upper_string_data = pair.get_upper_inclusive().dup(mem_root);
    if (lower_string_data == nullptr || upper_string_data == nullptr) {
      DBUG_ASSERT(false); /* purecov: deadcode */
      return;
    }

    String lower_string_dup(lower_string_data,
                            pair.get_lower_inclusive().length(),
                            pair.get_lower_inclusive().charset());
    String upper_string_dup(upper_string_data,
                            pair.get_upper_inclusive().length(),
                            pair.get_upper_inclusive().charset());

    m_buckets.emplace(lower_string_dup, upper_string_dup,
                      pair.get_cumulative_frequency(), pair.get_num_distinct());
  }
}

/*
  This function will build an equi-height histogram. The algorithm works like
  the following:

  - If the number of buckets specified is euqal to or greater than the number
    of distinct values, a single bucket is created for each value.

  - If we have more distinct values than the number of buckets, we calculate a
    threshold T for each bucket. The threshold T for bucket number N (counting
    from 1) is calculated as;

      num_non_null_values
      -------------------  * N = T
         num_buckets;

    When adding a value to a bucket, we check if including the next bucket will
    make the accumulated frequency become larger than the threshold. If that is
    the case, check whether only including the current value is closer to the
    threshold than including the next value as well. We select the option that
    is closest to the threshold.
*/
template <class T>
bool Equi_height<T>::build_histogram(const Value_map<T> &value_map,
                                     size_t num_buckets) {
  DBUG_ASSERT(num_buckets > 0);
  if (num_buckets < 1) return true; /* purecov: inspected */

  // Set the number of buckets that was specified/requested by the user.
  m_num_buckets_specified = num_buckets;

  // Clear any existing data.
  m_buckets.clear();
  m_null_values_fraction = INVALID_NULL_VALUES_FRACTION;
  m_sampling_rate = value_map.get_sampling_rate();

  // Set the character set for the histogram contents.
  m_charset = value_map.get_character_set();

  // Get total frequency count.
  ha_rows num_non_null_values = 0;
  for (const auto &node : value_map) num_non_null_values += node.second;

  // No non-null values, nothing to do.
  if (num_non_null_values == 0) {
    if (value_map.get_num_null_values() > 0)
      m_null_values_fraction = 1.0;
    else
      m_null_values_fraction = 0.0;

    return false;
  }

  DBUG_ASSERT(num_buckets > 0);

  // Set the fraction of NULL values.
  const ha_rows total_count =
      value_map.get_num_null_values() + num_non_null_values;

  m_null_values_fraction =
      value_map.get_num_null_values() / static_cast<double>(total_count);

  /*
    Divide the frequencies into evenly-ish spaced buckets, and set the bucket
    threshold accordingly.
  */
  const double avg_bucket_size =
      num_non_null_values / static_cast<double>(num_buckets);
  double current_threshold = avg_bucket_size;

  ha_rows cumulative_sum = 0;
  ha_rows sum = 0;
  ha_rows num_distinct = 0;
  size_t values_remaining = value_map.size();

  // Number of values that occurs only one time.
  int num_singlecount_values = 0;
  auto freq_it = value_map.begin();
  const T *lowest_value = &freq_it->first;

  for (; freq_it != value_map.end(); ++freq_it) {
    if (freq_it->second == 1) num_singlecount_values++;

    sum += freq_it->second;
    cumulative_sum += freq_it->second;
    num_distinct++;
    values_remaining--;
    auto next = std::next(freq_it);

    if (next != value_map.end()) {
      /*
        Check if including the next bucket will make the frequency become
        larger than the threshold. If that is the case, check whether only
        including the current value is closer to the threshold than
        including the next value as well.
      */
      if ((cumulative_sum + next->second) > current_threshold) {
        double current_distance = std::abs(current_threshold - cumulative_sum);
        double next_distance =
            std::abs(current_threshold - (cumulative_sum + next->second));

        if (current_distance >= next_distance) continue;
      } else if (values_remaining >= (num_buckets - m_buckets.size())) {
        /*
          Ensure that we don't end up with more buckets than the maximum
          specified.
        */
        continue;
      }
    }

    // Create a bucket.
    double cumulative_frequency =
        cumulative_sum / static_cast<double>(total_count);

    ha_rows num_distinct_estimate;

    /*
      If the sampling rate is less than 80%, we use the "unsmoothed first-order
      jackknife estimator" to estimate the number of distinct values. If the
      sampling rate is 80% or above, using the estimator seems to yield worse
      results than using the non-estimated count.
    */
    const double estimator_threshold = 0.8;
    if (value_map.get_sampling_rate() < estimator_threshold) {
      double num_distinct_rounded =
          std::round(1.0 /
                     (1.0 - ((1.0 - value_map.get_sampling_rate()) *
                             num_singlecount_values) /
                                sum) *
                     num_distinct);
      num_distinct_estimate = static_cast<ha_rows>(num_distinct_rounded);
    } else {
      num_distinct_estimate = num_distinct;
    }

    equi_height::Bucket<T> bucket(*lowest_value, freq_it->first,
                                  cumulative_frequency, num_distinct_estimate);

    /*
      Since we are using a std::vector with Mem_root_allocator, we are forced to
      wrap the following section in a try-catch. The Mem_root_allocator will
      throw an exception of class std::bad_alloc when it runs out of memory.
    */
    try {
      m_buckets.emplace(bucket);
    } catch (const std::bad_alloc &) {
      // Out of memory.
      return true;
    }
    /*
      In debug, check that the lower value actually is less than or equal to
      the upper value.
    */
    DBUG_ASSERT(!Histogram_comparator()(bucket.get_upper_inclusive(),
                                        bucket.get_lower_inclusive()));

    /*
      We also check that the lower inclusive value of the current bucket is
      greater than the upper inclusive value of the previous bucket.
    */
    if (m_buckets.size() > 1) {
      DBUG_ASSERT(Histogram_comparator()(
          std::prev(m_buckets.end(), 2)->get_upper_inclusive(),
          bucket.get_lower_inclusive()));
    }

    num_singlecount_values = 0;
    sum = 0;
    num_distinct = 0;
    current_threshold = avg_bucket_size * (m_buckets.size() + 1);
    if (next != value_map.end()) lowest_value = &next->first;
  }

  DBUG_ASSERT(m_buckets.size() <= num_buckets);
  return false;
}

template <class T>
bool Equi_height<T>::histogram_to_json(Json_object *json_object) const {
  /*
    Call the base class implementation first. This will add the properties that
    are common among different histogram types, such as "last-updated" and
    "histogram-type".
  */
  if (Histogram::histogram_to_json(json_object))
    return true; /* purecov: inspected */

  // Add the equi-height buckets.
  Json_array buckets;
  for (const auto &bucket : m_buckets) {
    Json_array json_bucket;
    if (bucket.bucket_to_json(&json_bucket))
      return true; /* purecov: inspected */
    if (buckets.append_clone(&json_bucket))
      return true; /* purecov: inspected */
  }

  if (json_object->add_clone(buckets_str(), &buckets))
    return true; /* purecov: inspected */

  if (histogram_data_type_to_json(json_object))
    return true; /* purecov: inspected */
  return false;
}

template <class T>
std::string Equi_height<T>::histogram_type_to_str() const {
  return equi_height_str();
}

template <class T>
bool Equi_height<T>::json_to_histogram(const Json_object &json_object) {
  if (Histogram::json_to_histogram(json_object))
    return true; /* purecov: deadcode */

  const Json_dom *buckets_dom = json_object.get(buckets_str());
  DBUG_ASSERT(buckets_dom->json_type() == enum_json_type::J_ARRAY);

  const Json_array *buckets = down_cast<const Json_array *>(buckets_dom);
  for (size_t i = 0; i < buckets->size(); ++i) {
    const Json_dom *bucket_dom = (*buckets)[i];
    DBUG_ASSERT(bucket_dom->json_type() == enum_json_type::J_ARRAY);

    const Json_array *bucket = down_cast<const Json_array *>(bucket_dom);
    DBUG_ASSERT(bucket->size() == 4);

    if (add_bucket_from_json(bucket)) return true; /* purecov: deadcode */
  }
  return false;
}

template <class T>
bool Equi_height<T>::add_bucket_from_json(const Json_array *json_bucket) {
  const Json_dom *cumulative_frequency_dom = (*json_bucket)[2];
  if (cumulative_frequency_dom->json_type() != enum_json_type::J_DOUBLE)
    return true; /* purecov: deadcode */

  const Json_dom *num_distinct_dom = (*json_bucket)[3];
  if (num_distinct_dom->json_type() != enum_json_type::J_UINT)
    return true; /* purecov: deadcode */

  const Json_double *cumulative_frequency =
      down_cast<const Json_double *>(cumulative_frequency_dom);

  const Json_uint *num_distinct =
      down_cast<const Json_uint *>(num_distinct_dom);

  const Json_dom *lower_inclusive_dom = (*json_bucket)[0];
  const Json_dom *upper_inclusive_dom = (*json_bucket)[1];

  T upper_value;
  T lower_value;
  if (extract_json_dom_value(upper_inclusive_dom, &upper_value) ||
      extract_json_dom_value(lower_inclusive_dom, &lower_value))
    return true; /* purecov: deadcode */

  try {
    m_buckets.emplace(lower_value, upper_value, cumulative_frequency->value(),
                      num_distinct->value());
  } catch (const std::bad_alloc &) {
    return true; /* purecov: deadcode */
  }
  return false;
}

template <class T>
Histogram *Equi_height<T>::clone(MEM_ROOT *mem_root) const {
  DBUG_EXECUTE_IF("fail_histogram_clone", return nullptr;);

  try {
    return new (mem_root) Equi_height<T>(mem_root, *this);
  } catch (const std::bad_alloc &) {
    return nullptr; /* purecov: deadcode */
  }
}

template <class T>
size_t Equi_height<T>::get_num_distinct_values() const {
  size_t distinct_values = 0;
  for (const auto &bucket : m_buckets) {
    distinct_values += bucket.get_num_distinct();
  }
  return distinct_values;
}

template <class T>
double Equi_height<T>::get_equal_to_selectivity(const T &value) const {
  /*
    Find the first bucket where the upper inclusive value is not less than the
    provided value.
  */
  const auto found = std::lower_bound(m_buckets.begin(), m_buckets.end(), value,
                                      Histogram_comparator());

  // Check if we are after the last bucket
  if (found == m_buckets.end()) return 0.0;

  // Check if we are before the first bucket, or between two buckets.
  if (Histogram_comparator()(value, found->get_lower_inclusive())) return 0.0;

  double bucket_frequency;
  if (found == m_buckets.begin()) {
    /*
      If the value we are looking for is in the first bucket, we will end up
      here.
    */
    bucket_frequency = found->get_cumulative_frequency();
  } else {
    /*
      If the value we are looking for is NOT in the first bucket, we will end up
      here.
    */
    const auto previous = std::prev(found, 1);
    bucket_frequency = found->get_cumulative_frequency() -
                       previous->get_cumulative_frequency();

    DBUG_ASSERT(bucket_frequency >= 0.0);
    DBUG_ASSERT(bucket_frequency <= get_non_null_values_frequency());
  }

  /*
    Take into account how high the probability is for a given value existing
    in the bucket. For example, consider the following bucket:

      DOUBLE VALUES
      Lower inclusive value: 0.0
      Upper inclusive value: 1000000.0
      Number of distinct values: 10

    Any of the values between have a very low probability since there are so
    many possible values for this data type.

    For a different example, consider this bucket:

      INTEGER VALUES
      Lower inclusive value: 1
      Upper inclusive value: 4
      Number of distinct values: 4

    Here we can see that all values must be present, since there are only four
    possible values between 1 and 4 (1, 2, 3 and 4), and we have 4 distinct
    values in the bucket.
  */
  return (bucket_frequency / found->get_num_distinct()) *
         found->value_probability();
}

template <class T>
double Equi_height<T>::get_less_than_equal_selectivity(const T &value) const {
  /*
    Find the first bucket where the upper inclusive value is not less than the
    provided value.
  */
  const auto found = std::lower_bound(m_buckets.begin(), m_buckets.end(), value,
                                      Histogram_comparator());

  if (found == m_buckets.end()) return get_non_null_values_frequency();

  double previous_bucket_cumulative_frequency;
  double found_bucket_frequency;
  if (found == m_buckets.begin()) {
    previous_bucket_cumulative_frequency = 0.0;
    found_bucket_frequency = found->get_cumulative_frequency();
  } else {
    const auto previous = std::prev(found, 1);
    previous_bucket_cumulative_frequency = previous->get_cumulative_frequency();
    found_bucket_frequency = found->get_cumulative_frequency() -
                             previous->get_cumulative_frequency();
  }

  const double distance = found->get_distance_from_lower(value);

  DBUG_ASSERT(distance >= 0.0);
  DBUG_ASSERT(distance <= 1.0);

  const double selectivity = previous_bucket_cumulative_frequency +
                             (found_bucket_frequency * distance);

  /*
    If we found the distance from lower to be zero and the value actually
    is equal to the lower inclusive value, we must add the "equal_to"
    selectivity in order to include the selectivity for the lower value.

    Imagine these two buckets: [1   4]  [5   7]
    Given the following predicate: foo <= 5;

    We would get the second bucket from std::lower_bound. The distance function
    will return 0.0, since 5 is actually equal to 5. But that would cause the
    selectivity to NOT include the selectivity for the value 5 itself.

    We do this adjustment only if distance == 0.0, because if we have a bucket
    where the upper and lower value are equal, the distance function will return
    1.0 and we already have included the selectivity for the value itself.
  */
  if (distance > 0.0 ||
      Histogram_comparator()(found->get_lower_inclusive(), value))
    return selectivity;

  return selectivity + get_equal_to_selectivity(value);
}

template <class T>
double Equi_height<T>::get_greater_than_selectivity(const T &value) const {
  const double less_than_equal = get_less_than_equal_selectivity(value);

  return get_non_null_values_frequency() -
         std::min(less_than_equal, get_non_null_values_frequency());
}

template <class T>
double Equi_height<T>::get_less_than_selectivity(const T &value) const {
  const double less_than_equal = get_less_than_equal_selectivity(value);
  const double equal_to = get_equal_to_selectivity(value);

  return less_than_equal - equal_to;
}

// Explicit template instantiations.
template class Equi_height<double>;
template class Equi_height<String>;
template class Equi_height<ulonglong>;
template class Equi_height<longlong>;
template class Equi_height<MYSQL_TIME>;
template class Equi_height<my_decimal>;

}  // namespace histograms
