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

#include <numeric>

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/frozen/FrozenUtil.h>

namespace apache::thrift {
using namespace frozen;

template <class T>
class FrozenRange : public ::testing::Test {};
TYPED_TEST_CASE_P(FrozenRange);

TYPED_TEST_P(FrozenRange, ArrayLike) {
  TypeParam v{10, 11, 12, 13};
  auto fv = freeze(v);
  EXPECT_EQ(fv.size(), 4);
  EXPECT_FALSE(fv.empty());
  EXPECT_EQ(fv[2], 12);
}

TYPED_TEST_P(FrozenRange, Iterators) {
  TypeParam v{10, 11, 12, 13};
  auto fv = freeze(v);
  auto it = fv.begin();
  EXPECT_EQ(*(it + 2), 12);
  EXPECT_EQ(*it, 10);
  EXPECT_EQ(*it++, 10);
  EXPECT_EQ(*it++, 11);
  EXPECT_EQ(*it, 12);
  EXPECT_EQ(*(it - 2), 10);
  EXPECT_EQ(*++it, 13);
  it = fv.end();
  EXPECT_EQ(*--it, 13);
  EXPECT_EQ(*it--, 13);
  EXPECT_EQ(*it--, 12);
  EXPECT_EQ(*--it, 10);
  std::adjacent_difference(fv.begin(), fv.end(), v.begin());
  TypeParam ref{10, 1, 1, 1};
  EXPECT_EQ(v, ref);
}

TYPED_TEST_P(FrozenRange, IteratorComparisons) {
  TypeParam v{10, 11, 12, 13};
  auto fv = freeze(v);
  auto a = fv.begin();
  auto b = a + 1;
  auto c = b + 1;

  // ==
  EXPECT_EQ(1, a == a);
  EXPECT_EQ(0, a == b);
  EXPECT_EQ(0, a == c);
  EXPECT_EQ(0, b == a);
  EXPECT_EQ(1, b == b);
  EXPECT_EQ(0, b == c);
  EXPECT_EQ(0, c == a);
  EXPECT_EQ(0, c == b);
  EXPECT_EQ(1, c == c);

  // !=
  EXPECT_EQ(0, a != a);
  EXPECT_EQ(1, a != b);
  EXPECT_EQ(1, a != c);
  EXPECT_EQ(1, b != a);
  EXPECT_EQ(0, b != b);
  EXPECT_EQ(1, b != c);
  EXPECT_EQ(1, c != a);
  EXPECT_EQ(1, c != b);
  EXPECT_EQ(0, c != c);

  // <
  EXPECT_EQ(0, a < a);
  EXPECT_EQ(1, a < b);
  EXPECT_EQ(1, a < c);
  EXPECT_EQ(0, b < a);
  EXPECT_EQ(0, b < b);
  EXPECT_EQ(1, b < c);
  EXPECT_EQ(0, c < a);
  EXPECT_EQ(0, c < b);
  EXPECT_EQ(0, c < c);

  // <=
  EXPECT_EQ(1, a <= a);
  EXPECT_EQ(1, a <= b);
  EXPECT_EQ(1, a <= c);
  EXPECT_EQ(0, b <= a);
  EXPECT_EQ(1, b <= b);
  EXPECT_EQ(1, b <= c);
  EXPECT_EQ(0, c <= a);
  EXPECT_EQ(0, c <= b);
  EXPECT_EQ(1, c <= c);

  // >
  EXPECT_EQ(0, a > a);
  EXPECT_EQ(0, a > b);
  EXPECT_EQ(0, a > c);
  EXPECT_EQ(1, b > a);
  EXPECT_EQ(0, b > b);
  EXPECT_EQ(0, b > c);
  EXPECT_EQ(1, c > a);
  EXPECT_EQ(1, c > b);
  EXPECT_EQ(0, c > c);

  // >=
  EXPECT_EQ(1, a >= a);
  EXPECT_EQ(0, a >= b);
  EXPECT_EQ(0, a >= c);
  EXPECT_EQ(1, b >= a);
  EXPECT_EQ(1, b >= b);
  EXPECT_EQ(0, b >= c);
  EXPECT_EQ(1, c >= a);
  EXPECT_EQ(1, c >= b);
  EXPECT_EQ(1, c >= c);
}

TYPED_TEST_P(FrozenRange, Zeros) {
  TypeParam v(1000, 0);
  // If all the values are zero-sized, we only need space to store the count.
  EXPECT_EQ(frozenSize(v), 2);
  // only a single byte for 200 empty strings
  EXPECT_EQ(frozenSize(std::vector<std::string>(200)), 1);
  auto fv = freeze(v);
  EXPECT_EQ(fv.size(), 1000);
  auto it = fv.begin();
  EXPECT_EQ(*it++, 0);
  EXPECT_EQ(*it++, 0);
  size_t n = 0;
  for (const auto& item : fv) {
    (void)item;
    ++n; // ensure iteration still works despite zero-byte items
  }
  EXPECT_EQ(n, 1000);
}

REGISTER_TYPED_TEST_CASE_P(
    FrozenRange, ArrayLike, Iterators, IteratorComparisons, Zeros);
using MyTypes = ::testing::Types<std::vector<int>, folly::fbvector<int>>;
INSTANTIATE_TYPED_TEST_CASE_P(Ranges, FrozenRange, MyTypes);

template <class T>
class FrozenRangeNested : public ::testing::Test {};
TYPED_TEST_CASE_P(FrozenRangeNested);

TYPED_TEST_P(FrozenRangeNested, VectorVectorInt) {
  TypeParam vvi{{2, 3, 5, 7}, {11, 13, 17, 19}};
  auto fvvi = freeze(vvi);
  auto tvvi = fvvi.thaw();
  EXPECT_EQ(tvvi, vvi);
}

TYPED_TEST_P(FrozenRangeNested, FrontBack) {
  TypeParam vvi{{2, 3, 5, 7}, {11}};
  auto fvvi = freeze(vvi);
  EXPECT_EQ(2, fvvi.front().front());
  EXPECT_EQ(7, fvvi.front().back());
  EXPECT_EQ(11, fvvi.back().front());
  EXPECT_EQ(11, fvvi.back().back());
}

REGISTER_TYPED_TEST_CASE_P(FrozenRangeNested, VectorVectorInt, FrontBack);
using MyTypesNested = ::testing::Types<
    std::vector<std::vector<int>>,
    std::vector<folly::fbvector<int>>,
    folly::fbvector<folly::fbvector<int>>,
    folly::fbvector<std::vector<int>>>;
INSTANTIATE_TYPED_TEST_CASE_P(RangesNested, FrozenRangeNested, MyTypesNested);

} // namespace apache::thrift
