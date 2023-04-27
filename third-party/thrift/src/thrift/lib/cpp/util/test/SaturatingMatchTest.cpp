/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cmath>
#include <type_traits>
#include <gtest/gtest.h>

#include <thrift/lib/cpp/util/SaturatingMath.h>

using namespace ::testing;

namespace apache::thrift::util {

namespace {
template <typename T>
void basicOpsTest() {
  EXPECT_EQ(43, add_saturating<T>(42, 1));
  EXPECT_EQ(41, add_saturating<T>(42, -1));
  EXPECT_EQ(-43, add_saturating<T>(-42, -1));
  EXPECT_EQ(-41, add_saturating<T>(-42, 1));
}

template <typename T>
void boundaryOpsTest() {
  EXPECT_EQ(
      std::numeric_limits<T>::max(),
      add_saturating<T>(std::numeric_limits<T>::max(), 1));

  EXPECT_EQ(
      std::numeric_limits<T>::max() - 1,
      add_saturating<T>(std::numeric_limits<T>::max(), -1));

  EXPECT_EQ(
      0,
      add_saturating<T>(
          std::numeric_limits<T>::max(), -std::numeric_limits<T>::max()));

  EXPECT_EQ(
      std::numeric_limits<T>::lowest(),
      add_saturating<T>(std::numeric_limits<T>::lowest(), -1));

  EXPECT_EQ(
      std::numeric_limits<T>::lowest() + 1,
      add_saturating<T>(std::numeric_limits<T>::lowest(), 1));

  if constexpr (std::is_floating_point_v<T>) {
    EXPECT_TRUE(std::isnan(add_saturating<T>(NAN, 1)));
    EXPECT_TRUE(std::isnan(add_saturating<T>(1, NAN)));
    EXPECT_TRUE(std::isinf(add_saturating<T>(INFINITY, 1)));
    EXPECT_EQ(add_saturating<T>(INFINITY, 1), INFINITY);
    EXPECT_TRUE(std::isinf(add_saturating<T>(1, INFINITY)));
    EXPECT_EQ(add_saturating<T>(1, INFINITY), INFINITY);
    EXPECT_TRUE(std::isnan(add_saturating<T>(INFINITY, -INFINITY)));
    EXPECT_TRUE(std::isinf(add_saturating<T>(1, -INFINITY)));
    EXPECT_EQ(add_saturating<T>(1, -INFINITY), -INFINITY);
    EXPECT_TRUE(std::isinf(add_saturating<T>(-INFINITY, 1)));
    EXPECT_EQ(add_saturating<T>(-INFINITY, 1), -INFINITY);
  }
}

template <typename T>
class SaturatingMatchTest : public testing::Test {};

} // namespace

using Types =
    ::testing::Types<int8_t, int16_t, int32_t, int64_t, float, double>;
TYPED_TEST_SUITE(SaturatingMatchTest, Types);

TYPED_TEST(SaturatingMatchTest, Basic) {
  basicOpsTest<TypeParam>();
}

TYPED_TEST(SaturatingMatchTest, Boundary) {
  boundaryOpsTest<TypeParam>();
}

} // namespace apache::thrift::util
