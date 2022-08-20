/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/carbon/test/Util.h"

#include <gtest/gtest.h>

#include "mcrouter/lib/IOBufUtil.h"
#include "mcrouter/lib/carbon/test/gen/CarbonTest.h"

using facebook::memcache::coalesceAndGetRange;

namespace carbon {
namespace test {
namespace util {

std::string longString() {
  return std::string(1024, 'a');
}

namespace {
void compareOptionalIobuf(
    const std::optional<folly::IOBuf> a,
    const std::optional<folly::IOBuf> b) {
  EXPECT_EQ(a.has_value(), b.has_value());
  if (a.has_value()) {
    folly::IOBuf aCopy = *a;
    folly::IOBuf bCopy = *b;
    EXPECT_EQ(coalesceAndGetRange(aCopy), coalesceAndGetRange(bCopy));
  }
}
} // namespace

void expectEqTestRequest(const TestRequest& a, const TestRequest& b) {
  EXPECT_EQ(a.key_ref()->fullKey(), b.key_ref()->fullKey());

  EXPECT_EQ(*a.testBool_ref(), *b.testBool_ref());
  EXPECT_EQ(*a.testChar_ref(), *b.testChar_ref());

  EXPECT_EQ(*a.testInt8_ref(), *b.testInt8_ref());
  EXPECT_EQ(*a.testInt16_ref(), *b.testInt16_ref());
  EXPECT_EQ(*a.testInt32_ref(), *b.testInt32_ref());
  EXPECT_EQ(*a.testInt64_ref(), *b.testInt64_ref());

  EXPECT_EQ(*a.testUInt8_ref(), *b.testUInt8_ref());
  EXPECT_EQ(*a.testUInt16_ref(), *b.testUInt16_ref());
  EXPECT_EQ(*a.testUInt32_ref(), *b.testUInt32_ref());
  EXPECT_EQ(*a.testUInt64_ref(), *b.testUInt64_ref());

  EXPECT_FLOAT_EQ(*a.testFloat_ref(), *b.testFloat_ref());
  EXPECT_DOUBLE_EQ(*a.testDouble_ref(), *b.testDouble_ref());

  EXPECT_EQ(*a.testShortString_ref(), *b.testShortString_ref());
  EXPECT_EQ(*a.testLongString_ref(), *b.testLongString_ref());

  EXPECT_EQ(
      coalesceAndGetRange(const_cast<folly::IOBuf&>(*a.testIobuf_ref())),
      coalesceAndGetRange(const_cast<folly::IOBuf&>(*b.testIobuf_ref())));

  EXPECT_EQ(*a.testList_ref(), *b.testList_ref());

  EXPECT_EQ(*a.testNestedVec_ref(), *b.testNestedVec_ref());

  EXPECT_EQ(*a.testUMap_ref(), *b.testUMap_ref());
  EXPECT_EQ(*a.testMap_ref(), *b.testMap_ref());
  EXPECT_EQ(*a.testF14FastMap_ref(), *b.testF14FastMap_ref());
  EXPECT_EQ(*a.testF14NodeMap_ref(), *b.testF14NodeMap_ref());
  EXPECT_EQ(*a.testF14ValueMap_ref(), *b.testF14ValueMap_ref());
  EXPECT_EQ(*a.testF14VectorMap_ref(), *b.testF14VectorMap_ref());
  EXPECT_EQ(*a.testComplexMap_ref(), *b.testComplexMap_ref());

  EXPECT_EQ(*a.testUSet_ref(), *b.testUSet_ref());
  EXPECT_EQ(*a.testSet_ref(), *b.testSet_ref());
  EXPECT_EQ(*a.testF14FastSet_ref(), *b.testF14FastSet_ref());
  EXPECT_EQ(*a.testF14NodeSet_ref(), *b.testF14NodeSet_ref());
  EXPECT_EQ(*a.testF14ValueSet_ref(), *b.testF14ValueSet_ref());
  EXPECT_EQ(*a.testF14VectorSet_ref(), *b.testF14VectorSet_ref());

  EXPECT_EQ(
      a.testOptionalString_ref().value_or(""),
      b.testOptionalString_ref().value_or(""));
  compareOptionalIobuf(
      a.testOptionalIobuf_ref().to_optional(),
      b.testOptionalIobuf_ref().to_optional());

  EXPECT_EQ(
      a.testOptionalBool_ref().value_or(false),
      b.testOptionalBool_ref().value_or(false));
}

} // namespace util
} // namespace test
} // namespace carbon
