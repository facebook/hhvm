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

#include <thrift/lib/cpp2/op/StdHasher.h>

#include <cstdint>

#include <folly/Range.h>
#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>

namespace apache {
namespace thrift {
namespace op {

TEST(StdHasherTest, checkCombineBool) {
  StdHasher hasher;
  auto previousResult = hasher.getResult();
  hasher.combine(true);
  EXPECT_NE(previousResult, hasher.getResult());
  previousResult = hasher.getResult();
  hasher.combine(false);
  EXPECT_NE(previousResult, hasher.getResult());
}

TEST(StdHasherTest, checkCombineInt8) {
  StdHasher hasher;
  auto previousResult = hasher.getResult();
  hasher.combine(static_cast<std::int8_t>(-1));
  EXPECT_NE(previousResult, hasher.getResult());
  previousResult = hasher.getResult();
  hasher.combine(static_cast<std::int8_t>(0));
  EXPECT_NE(previousResult, hasher.getResult());
  previousResult = hasher.getResult();
  hasher.combine(static_cast<std::int8_t>(+1));
  EXPECT_NE(previousResult, hasher.getResult());
}

TEST(StdHasherTest, checkCombineInt16) {
  StdHasher hasher;
  auto previousResult = hasher.getResult();
  hasher.combine(static_cast<std::int16_t>(-1));
  EXPECT_NE(previousResult, hasher.getResult());
  previousResult = hasher.getResult();
  hasher.combine(static_cast<std::int16_t>(0));
  EXPECT_NE(previousResult, hasher.getResult());
  previousResult = hasher.getResult();
  hasher.combine(static_cast<std::int16_t>(+1));
  EXPECT_NE(previousResult, hasher.getResult());
}

TEST(StdHasherTest, checkCombineInt32) {
  StdHasher hasher;
  auto previousResult = hasher.getResult();
  hasher.combine(static_cast<std::int32_t>(-1));
  EXPECT_NE(previousResult, hasher.getResult());
  previousResult = hasher.getResult();
  hasher.combine(static_cast<std::int32_t>(0));
  EXPECT_NE(previousResult, hasher.getResult());
  previousResult = hasher.getResult();
  hasher.combine(static_cast<std::int32_t>(+1));
  EXPECT_NE(previousResult, hasher.getResult());
}

TEST(StdHasherTest, checkCombineInt64) {
  StdHasher hasher;
  auto previousResult = hasher.getResult();
  hasher.combine(static_cast<std::int64_t>(-1));
  EXPECT_NE(previousResult, hasher.getResult());
  previousResult = hasher.getResult();
  hasher.combine(static_cast<std::int64_t>(0));
  EXPECT_NE(previousResult, hasher.getResult());
  previousResult = hasher.getResult();
  hasher.combine(static_cast<std::int64_t>(+1));
  EXPECT_NE(previousResult, hasher.getResult());
}

TEST(StdHasherTest, checkCombineFloat) {
  StdHasher hasher;
  auto previousResult = hasher.getResult();
  hasher.combine(static_cast<float>(-1.0));
  EXPECT_NE(previousResult, hasher.getResult());
  previousResult = hasher.getResult();
  hasher.combine(static_cast<float>(0.0));
  EXPECT_NE(previousResult, hasher.getResult());
  previousResult = hasher.getResult();
  hasher.combine(static_cast<float>(+1.0));
  EXPECT_NE(previousResult, hasher.getResult());
}

TEST(StdHasherTest, checkCombineDouble) {
  StdHasher hasher;
  auto previousResult = hasher.getResult();
  hasher.combine(static_cast<double>(-1.0));
  EXPECT_NE(previousResult, hasher.getResult());
  previousResult = hasher.getResult();
  hasher.combine(static_cast<double>(0.0));
  EXPECT_NE(previousResult, hasher.getResult());
  previousResult = hasher.getResult();
  hasher.combine(static_cast<double>(+1.0));
  EXPECT_NE(previousResult, hasher.getResult());
}

TEST(StdHasherTest, checkCombineIOBuf) {
  StdHasher hasher1, hasher2;
  auto bufA = folly::IOBuf::wrapBuffer("abc", 3);
  auto bufB = folly::IOBuf::wrapBuffer("def", 3);
  bufA->prependChain(std::move(bufB));
  hasher1.combine(*bufA);
  hasher2.combine(*folly::IOBuf::wrapBuffer("abcdef", 6));
  EXPECT_EQ(hasher1.getResult(), hasher2.getResult());
}

TEST(StdHasherTest, checkCombineByteRange) {
  StdHasher hasher;
  auto previousResult = hasher.getResult();
  hasher.combine(folly::range("abc"));
  EXPECT_NE(previousResult, hasher.getResult());
  previousResult = hasher.getResult();
  hasher.combine(folly::range(""));
  EXPECT_NE(previousResult, hasher.getResult());
}

TEST(StdHasherTest, checkCombineStdHasher) {
  StdHasher hasher;
  StdHasher hasherOther;
  auto previousResult = hasher.getResult();
  hasher.combine(hasherOther);
  EXPECT_EQ(previousResult, hasher.getResult());
  hasherOther.combine(folly::range("abc"));
  hasher.combine(hasherOther);
  EXPECT_NE(previousResult, hasher.getResult());
  EXPECT_NE(hasherOther.getResult(), hasher.getResult());
}

TEST(StdHasherTest, checkLess) {
  StdHasher hasher;
  hasher.combine(folly::range("abc"));
  StdHasher hasherOther;
  EXPECT_TRUE(hasherOther < hasher);
  hasherOther.combine(folly::range("abc"));
  EXPECT_FALSE(hasherOther < hasher);
  EXPECT_FALSE(hasher < hasherOther);
}

TEST(StdHasherTest, checkFinalize) {
  StdHasher hasher;
  hasher.combine(folly::range("abc"));
  auto previousResult = hasher.getResult();
  hasher.finalize();
  EXPECT_EQ(previousResult, hasher.getResult());
}

} // namespace op
} // namespace thrift
} // namespace apache
