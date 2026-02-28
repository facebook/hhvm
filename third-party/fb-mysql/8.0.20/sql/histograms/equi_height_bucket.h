#ifndef HISTOGRAMS_EQUI_HEIGHT_BUCKET_INCLUDED
#define HISTOGRAMS_EQUI_HEIGHT_BUCKET_INCLUDED

/* Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

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
  @file sql/histograms/equi_height_bucket.h
  Equi-height bucket.

  This file defines the class representing an equi-height bucket. A bucket holds
  four different values:
    - Lower inclusive value.
    - Upper inclusive value.
    - The cumulative frequency (between 0.0 and 1.0).
    - Number of distinct values in this bucket.
*/

#include "my_base.h"  // ha_rows
#include "my_inttypes.h"
#include "mysql_time.h"
#include "sql/my_decimal.h"
#include "sql_string.h"

class Json_array;

namespace histograms {
namespace equi_height {

/**
  Equi-height bucket.
*/
template <class T>
class Bucket {
 private:
  /// Lower inclusive value contained in this bucket.
  const T m_lower_inclusive;

  /// Upper inclusive value contained in this bucket.
  const T m_upper_inclusive;

  /// The cumulative frequency. 0.0 <= m_cumulative_frequency <= 1.0
  const double m_cumulative_frequency;

  /// Number of distinct values in this bucket.
  const ha_rows m_num_distinct;

  /**
    Add values to a JSON bucket

    This function adds the lower and upper inclusive value to the supplied
    JSON array. The lower value is added first.

    @param      lower_value the lower inclusive value to add
    @param      upper_value the upper inclusive value to add
    @param[out] json_array  a JSON array where the bucket data is stored

    @return     true on error, false otherwise.
  */
  static bool add_values_json_bucket(const T &lower_value, const T &upper_value,
                                     Json_array *json_array);

 public:
  /**
    Equi-height bucket constructor.

    Does nothing more than setting the member variables.

    @param lower         lower inclusive value
    @param upper         upper inclusive value
    @param freq          the cumulative frequency
    @param num_distinct  number of distinct values in this bucket
  */
  Bucket(T lower, T upper, double freq, ha_rows num_distinct);

  /**
    @return lower inclusive value
  */
  const T &get_lower_inclusive() const { return m_lower_inclusive; }

  /**
    @return upper inclusive value
  */
  const T &get_upper_inclusive() const { return m_upper_inclusive; }

  /**
    @return cumulative frequency
  */
  double get_cumulative_frequency() const { return m_cumulative_frequency; }

  /**
    @return number of distinct values
  */
  ha_rows get_num_distinct() const { return m_num_distinct; }

  /**
    Convert this equi-height bucket to a JSON array.

    This function will take the contents of the current equi-height bucket
    and put it in the output parameter "json_array". The result is an array
    with the following contents:
      Index 0: Lower inclusive value.
      Index 1: Upper inclusive value.
      Index 2: Cumulative frequency.
      Index 3: Number of distinct values.

    @param[out] json_array output where the bucket content is to be stored

    @return     true on error, false otherwise
  */
  bool bucket_to_json(Json_array *json_array) const;

  /**
    Returns the "distance" between lower inclusive value and the argument
    "value".

    The return value is a number between 0.0 and 1.0. A value of 0.0 indicates
    that "value" is equal to or less than the lower inclusive value. A value of
    1.0 indicates that "value" is equal or greater to the upper inclusive value.

    @param value The value to caluclate the distance for

    @return The distance between "value" and lower inclusive value.
  */
  double get_distance_from_lower(const T &value) const;

  /**
    Calculate how high the probability is for a single value existing in the
    bucket.

    This is basically equal to the number of distinct values in the bucket
    divided by the number of possible values in the bucket range. For strings,
    double, decimals and such, the probability will be very low since the number
    of possible values is VERY big. For integer values, the probability may
    be rather high if the difference between the lower and upper value is low.

    @return Probability of a value existing in the bucket, between 0.0 and 1.0
            inclusive.
  */
  double value_probability() const;
};

}  // namespace equi_height
}  // namespace histograms

#endif
