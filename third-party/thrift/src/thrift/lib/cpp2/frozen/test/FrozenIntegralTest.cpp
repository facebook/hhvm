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

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/frozen/Frozen.h>
#include <thrift/lib/cpp2/frozen/FrozenUtil.h>

using namespace apache::thrift::frozen;

TEST(FrozenIntegral, UIntBounds) {
  EXPECT_EQ(0, frozenSize(0UL));
  EXPECT_EQ(1, frozenSize(0xFFUL));
  EXPECT_EQ(2, frozenSize(0x0100UL));
  EXPECT_EQ(2, frozenSize(0xFFFFUL));
  EXPECT_EQ(3, frozenSize(0x010000UL));
  EXPECT_EQ(3, frozenSize(0x010000UL));
  EXPECT_EQ(7, frozenSize(0xFFFFFFFFFFFFFFUL));
  EXPECT_EQ(8, frozenSize(0x0100000000000000UL));
  EXPECT_EQ(8, frozenSize(0xFFFFFFFFFFFFFFFFUL));
  EXPECT_EQ(0, frozenSize(std::numeric_limits<uint8_t>::min()));
  EXPECT_EQ(1, frozenSize(std::numeric_limits<uint8_t>::max()));
  EXPECT_EQ(0, frozenSize(std::numeric_limits<uint16_t>::min()));
  EXPECT_EQ(2, frozenSize(std::numeric_limits<uint16_t>::max()));
  EXPECT_EQ(0, frozenSize(std::numeric_limits<uint32_t>::min()));
  EXPECT_EQ(4, frozenSize(std::numeric_limits<uint32_t>::max()));
  EXPECT_EQ(0, frozenSize(std::numeric_limits<uint64_t>::min()));
  EXPECT_EQ(8, frozenSize(std::numeric_limits<uint64_t>::max()));
}

TEST(FrozenIntegral, IntBounds) {
  EXPECT_EQ(0, frozenSize(0L));
  EXPECT_EQ(1, frozenSize(0x7FL));
  EXPECT_EQ(1, frozenSize(-0x80L));
  EXPECT_EQ(2, frozenSize(0x80L));
  EXPECT_EQ(2, frozenSize(0x7FFFL));
  EXPECT_EQ(2, frozenSize(-0x8000L));
  EXPECT_EQ(3, frozenSize(0x8000L));
  EXPECT_EQ(7, frozenSize(0x7FFFFFFFFFFFFFL));
  EXPECT_EQ(7, frozenSize(-0x80000000000000L));
  EXPECT_EQ(8, frozenSize(0x7FFFFFFFFFFFFFFFL));
  EXPECT_EQ(8, frozenSize(-0x8000000000000000L));
  EXPECT_EQ(8, frozenSize(std::numeric_limits<int64_t>::min()));
  EXPECT_EQ(8, frozenSize(std::numeric_limits<int64_t>::max()));
  EXPECT_EQ(4, frozenSize(std::numeric_limits<int32_t>::min()));
  EXPECT_EQ(4, frozenSize(std::numeric_limits<int32_t>::max()));
  EXPECT_EQ(2, frozenSize(std::numeric_limits<int16_t>::min()));
  EXPECT_EQ(2, frozenSize(std::numeric_limits<int16_t>::max()));
  EXPECT_EQ(1, frozenSize(std::numeric_limits<int8_t>::min()));
  EXPECT_EQ(1, frozenSize(std::numeric_limits<int8_t>::max()));
}

TEST(FrozenIntegral, UIntPacking) {
  class DummyFreezer : public FreezeRoot {
   private:
    void doAppendBytes(
        byte*, size_t, folly::MutableByteRange&, size_t&, size_t) override {}
  };
  DummyFreezer fr;
  size_t value = 5;
  size_t width = 3;
  for (size_t start = 0; start <= 64 - width; start += 7) {
    for (size_t bits = width; bits <= 64 - start; bits += 5) {
      apache::thrift::frozen::Layout<size_t> l;
      l.bits = bits;
      size_t container = 0xDEADBEEFDEADBEEF;
      FreezePosition fpos{(byte*)&container, start};
      l.freeze(fr, value, fpos);
      ViewPosition vpos{(byte*)&container, start};
      size_t confirm;
      l.thaw(vpos, confirm);
      EXPECT_EQ(value, confirm) << bits << "@" << start;
    }
  }
}
