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

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>

#include "sql/dd/string_type.h"

/*
  Tests of dd::String_type
*/

namespace dd_string_type_unit_test {

typedef dd::String_type s_t;

class DDStringTypeTest : public ::testing::Test {};

// Basic string usage
TEST(DDStringTypeTest, BasicTest) {
  s_t x("foobar");
  x += "_tag";
  EXPECT_EQ(10U, x.size());
}

// Create string using stringstream
TEST(DDStringTypeTest, StreamTest) {
  typedef dd::Stringstream_type ss_t;

  ss_t ss;
  double d = 42.65;
  ss << "The value of d is " << d << " this is an integer " << 42 << std::endl;

  s_t x("Stream result: ");
  x += ss.str();
  EXPECT_EQ(61U, x.size());
}

// Strings in vector
TEST(DDStringTypeTest, VectorTest) {
  typedef std::vector<s_t> sv_t;

  sv_t words = {"Mary", "had", "a", "little", "Lamb"};
  std::sort(words.begin(), words.end());
  ASSERT_TRUE(std::is_sorted(words.begin(), words.end()));
}

// Strings as keys and values in maps
TEST(DDStringTypeTest, MapTest) {
  typedef std::map<s_t, s_t> sm_t;

  sm_t dict = {{"large", "great"}, {"small", "little"}, {"medium", "average"}};

  EXPECT_EQ(3U, dict.size());
  EXPECT_EQ("great", dict["large"]);
  EXPECT_EQ("little", dict["small"]);
  EXPECT_EQ("average", dict["medium"]);
}

// Strings as keys and values in unordered (hash-based) maps
TEST(DDStringTypeTest, UnorderedMapTest) {
  typedef std::unordered_map<s_t, s_t> sm_t;

  sm_t dict = {{"large", "great"}, {"small", "little"}, {"medium", "average"}};

  EXPECT_EQ(3U, dict.size());
  EXPECT_EQ("great", dict["large"]);
  EXPECT_EQ("little", dict["small"]);
  EXPECT_EQ("average", dict["medium"]);
}

struct Tracking_alloc {
  static int allocations;
  static int bytes_allocated;

  static int frees;

  void *operator()(size_t s) const {
    ++allocations;
    bytes_allocated += s;
    return operator new(s);
  }

  void operator()(void *p, size_t) const {
    ++frees;
    operator delete(p);
  }
};

int Tracking_alloc::allocations = 0;
int Tracking_alloc::bytes_allocated = 0;
int Tracking_alloc::frees = 0;

template <class T>
using Tracking_allocator =
    Stateless_allocator<T, Tracking_alloc, Tracking_alloc>;

typedef dd::Char_string_template<Tracking_allocator<char>> Tracking_string;
typedef dd::Char_stringstream_template<Tracking_allocator<char>>
    Tracking_stringstream;

class TrackingStringTypeTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    Tracking_alloc::allocations = 0;
    Tracking_alloc::bytes_allocated = 0;
    Tracking_alloc::frees = 0;
  }
  virtual void TearDown() {
    std::cout << "allocations: " << Tracking_alloc::allocations
              << ", frees: " << Tracking_alloc::frees
              << ", bytes allocated: " << Tracking_alloc::bytes_allocated
              << std::endl;
    EXPECT_EQ(Tracking_alloc::allocations, Tracking_alloc::frees);
  }
};

TEST_F(TrackingStringTypeTest, TrackingString) { Tracking_string x = "foobar"; }

TEST_F(TrackingStringTypeTest, TrackingStringstream) {
  Tracking_stringstream x;
  x << "This is a number " << 42 << " followed by a double " << 36.9
    << std::endl;
}

TEST_F(TrackingStringTypeTest, TrackingStringstreamGetString) {
  Tracking_stringstream x;
  x << "This is a number " << 42 << " followed by a double " << 36.9
    << std::endl;
  Tracking_string y = x.str();
  EXPECT_EQ(46u, y.size());
}

}  // namespace dd_string_type_unit_test
