/* Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "my_config.h"

#include <gtest/gtest.h>
#include <sys/types.h>
#include <limits>

#include "my_byteorder.h"
#include "my_inttypes.h"

namespace byteorder_unittest {

using std::numeric_limits;

/*
  This class is used to instantiate parameterized tests for float and double.
 */
template <typename T>
class FloatingTest {
 protected:
  T input;
  T output;
  uchar buf[sizeof(T)];
};

class Float4Test : public FloatingTest<float>,
                   public ::testing::TestWithParam<float> {
  virtual void SetUp() {
    input = GetParam();
    output = numeric_limits<float>::quiet_NaN();
  }
};

INSTANTIATE_TEST_CASE_P(Foo, Float4Test,
                        ::testing::Values(numeric_limits<float>::min(),
                                          numeric_limits<float>::max(),
                                          numeric_limits<float>::epsilon(),
                                          -numeric_limits<float>::min(),
                                          -numeric_limits<float>::max(),
                                          -numeric_limits<float>::epsilon(),
                                          -1.0f, 0.0f, 1.0f));
/*
  The actual test case for float: store and get some values.
 */
TEST_P(Float4Test, PutAndGet) {
  float4store(buf, input);
  output = float4get(buf);
  EXPECT_EQ(input, output);
  floatstore(buf, input);
  output = floatget(buf);
  EXPECT_EQ(input, output);
}

class Float8Test : public FloatingTest<double>,
                   public ::testing::TestWithParam<double> {
  virtual void SetUp() {
    input = GetParam();
    output = numeric_limits<double>::quiet_NaN();
  }
};

INSTANTIATE_TEST_CASE_P(Foo, Float8Test,
                        ::testing::Values(numeric_limits<double>::min(),
                                          numeric_limits<double>::max(),
                                          numeric_limits<double>::epsilon(),
                                          -numeric_limits<double>::min(),
                                          -numeric_limits<double>::max(),
                                          -numeric_limits<double>::epsilon(),
                                          -1.0, 0.0, 1.0));
/*
  The actual test case for double: store and get some values.
 */
TEST_P(Float8Test, PutAndGet) {
  float8store(buf, input);
  output = float8get(buf);
  EXPECT_EQ(input, output);
  doublestore(buf, input);
  output = doubleget(buf);
  EXPECT_EQ(input, output);
}

#if defined(GTEST_HAS_TYPED_TEST)

/*
  A test fixture class, parameterized on type.
  Will be instantiated for all IntegralTypes below.
 */
template <typename T>
class IntegralTest : public ::testing::Test {
 protected:
  typedef std::numeric_limits<T> Limit;

  T input;
  T output;
  uchar buf[sizeof(T)];

  typename std::vector<T> values;

  IntegralTest() : input(0), output(0) {}

  virtual void SetUp() {
    values.push_back(Limit::min());
    values.push_back(Limit::min() / T(2));
    values.push_back(T(0));
    values.push_back(T(42));
    values.push_back(Limit::max() / T(2));
    values.push_back(Limit::max());
  }
};

/*
  A class to make our 3, 5 and 6 digit integers look like builtins.
 */
template <int ndigits>
struct sizeNint {
  // For numeric_limits.
  typedef ulonglong value_type;

  sizeNint() : value(0) {}
  explicit sizeNint(ulonglong v) {
    switch (ndigits) {
      case 3:
        value = v & 0xFFFFFFULL;
        break;
      case 5:
        value = v & 0xFFFFFFFFFFULL;
        break;
      case 6:
        value = v & 0xFFFFFFFFFFFFULL;
        break;
      default:
        ADD_FAILURE() << "unxpected number of digits";
    }
  }

  sizeNint operator/(const sizeNint &that) const {
    return sizeNint(this->value / that.value);
  }

  bool operator==(const sizeNint &that) const {
    return this->value == that.value;
  }

  ulonglong value;
};

// googletest needs to be able to print arguments to EXPECT_EQ.
template <int ndigits>
std::ostream &operator<<(std::ostream &s, const sizeNint<ndigits> &v) {
  return s << v.value;
}

// Instantiate the PutAndGet test for all these types:
typedef ::testing::Types<short, ushort, sizeNint<3>, sizeNint<5>, sizeNint<6>,
                         int, unsigned, longlong, ulonglong>
    IntegralTypes;

TYPED_TEST_CASE(IntegralTest, IntegralTypes);

/*
  Wrap all the __get, __store, __korr macros in functions.
 */
template <typename T>
void put_integral(uchar *, T) {
  ADD_FAILURE() << "unknown type in put_integral";
}
template <typename T>
void get_integral(T &, uchar *) {
  ADD_FAILURE() << "unknown type in get_integral";
}

template <>
void put_integral(uchar *buf, short val) {
  shortstore(buf, val);
}
template <>
void get_integral(short &val, uchar *buf) {
  val = shortget(buf);
}

// Hmm, there's no ushortstore...
template <>
void put_integral(uchar *buf, ushort val) {
  shortstore(buf, val);
}
template <>
void get_integral(ushort &val, uchar *buf) {
  val = ushortget(buf);
}

template <>
void put_integral(uchar *buf, int val) {
  longstore(buf, val);
}
template <>
void get_integral(int &val, uchar *buf) {
  val = longget(buf);
}

// Hmm, there's no ulongstore...
template <>
void put_integral(uchar *buf, unsigned val) {
  longstore(buf, val);
}
template <>
void get_integral(unsigned &val, uchar *buf) {
  val = ulongget(buf);
}

template <>
void put_integral(uchar *buf, longlong val) {
  longlongstore(buf, val);
}
template <>
void get_integral(longlong &val, uchar *buf) {
  val = longlongget(buf);
}

// Reading ulonglong is different from all the above ....
template <>
void put_integral(uchar *buf, ulonglong val) {
  int8store(buf, val);
}
template <>
void get_integral(ulonglong &val, uchar *buf) {
  val = uint8korr(buf);
}

template <>
void put_integral(uchar *buf, sizeNint<3> val) {
  int3store(buf, static_cast<uint>(val.value));
}
template <>
void get_integral(sizeNint<3> &val, uchar *buf) {
  val.value = uint3korr(buf);
}

template <>
void put_integral(uchar *buf, sizeNint<5> val) {
  int5store(buf, val.value);
}
template <>
void get_integral(sizeNint<5> &val, uchar *buf) {
  val.value = uint5korr(buf);
}

template <>
void put_integral(uchar *buf, sizeNint<6> val) {
  int6store(buf, val.value);
}
template <>
void get_integral(sizeNint<6> &val, uchar *buf) {
  val.value = uint6korr(buf);
}

/*
  This is the actual test which will be instantiated for all IntegralTypes.
 */
TYPED_TEST(IntegralTest, PutAndGet) {
  for (size_t ix = 0; ix < this->values.size(); ++ix) {
    this->input = this->values[ix];
    put_integral(this->buf, this->input);
    get_integral(this->output, this->buf);
    // Visual studio rejects: EXPECT_EQ(this->input, this->output);
    TypeParam myinput = this->input;
    TypeParam myoutput = this->output;
    EXPECT_EQ(myinput, myoutput);
  }
}

#endif  // GTEST_HAS_TYPED_TEST
}  // namespace byteorder_unittest
