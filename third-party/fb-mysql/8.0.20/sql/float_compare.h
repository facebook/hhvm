#ifndef FLOAT_COMPARE_INCLUDED
#define FLOAT_COMPARE_INCLUDED
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
  @file sql/float_compare.h
  Comparison functions for floating point values.
*/

#include <cmath>    // std::isnan, std::signbit, std::isinf
#include <cstdint>  // int64_t, int32_t
#include <cstdlib>  // std::abs

class Float_compare {
 private:
  union Double_t {
    Double_t(double value) : double_value(value) {}

    int64_t integer_part;
    double double_value;
  };

  union Float_t {
    Float_t(float value) : float_value(value) {}

    int32_t integer_part;
    float float_value;
  };

  /**
    Return the ULP difference between two floats.

    @param   val1     first value
    @param   val2     second value

    @return  the difference in ULPs between the two values
  */
  static int32_t get_ulp_diff(float val1, float val2) {
    Float_t a(val1);
    Float_t b(val2);

    return std::abs(a.integer_part - b.integer_part);
  }

  /**
    Return the ULP difference between two doubles.

    @param   val1   first value
    @param   val2   second value

    @return  the difference in ULPs between the two values
  */
  static int64_t get_ulp_diff(double val1, double val2) {
    Double_t a(val1);
    Double_t b(val2);

    return std::abs(a.integer_part - b.integer_part);
  }

 public:
  /*
    Compare floating point values for "almost equal".

    The implementation is based ULPs (Units in the Last Place), which works
    something like the following:

    A nice property of double values is that if they are interpreted (not
    converted or cast) as an 64-bit integer, increasing the integer value by one
    gives us the next representable double value. Thus, by interpreting two
    double values aD, bD as 64-bit integers aI, bI, the absolute difference
    between aI and bI tells us how many representable double values there are
    between aD and bD.

    The above only works if aD and dB have the same sign. However, if their sign
    is different we can safely assume that the two values aren't "almost equal".

    An interesting side-effect of the above is that the ULP difference between
    DBL_MAX and INFINITY is only 1, since INFINITY is the next representable
    double value after DBL_MAX. This means that DBL_MAX and INFINITY are
    considered "almost equal" unless we have some special handling for INFINITY.

    By adjusting the allowed difference in ULPs, we can control how close the
    numbers should be to compare as "almost equal". The default ULP difference
    is set to 4, which is the same what googletest uses.

    For further reading, see:
      https://randomascii.wordpress.com/2012/02/25/
      comparing-floating-point-numbers-2012-edition/

    @param   val1          first value
    @param   val2          second value
    @param   max_ulp_diff  the maximum difference in ULPs between val1 and
                           val2

    @return  true if the two values are considered to be almost equal
  */
  template <class T>
  static bool almost_equal(T val1, T val2,
                           decltype(get_ulp_diff(val1,
                                                 val2)) max_ulp_diff = 4) {
    /*
      According to IEEE standard, any comparison involving a NaN must return
      false.
    */
    if (std::isnan(val1) || std::isnan(val2)) return false;

    // If the numbers have different signs, they are not equal.
    if (std::signbit(val1) != std::signbit(val2)) {
      //...but check for equality, since we should consider -0.0 equal to 0.0.
      return val1 == val2;
    }

    // Infinity should not be equal to any other than itself.
    if (std::isinf(val1) != std::isinf(val2)) return false;

    // Find the difference in ULPs.
    const auto ulp_diff = get_ulp_diff(val1, val2);
    if (ulp_diff < max_ulp_diff) return true;
    return false;
  }
};

#endif
