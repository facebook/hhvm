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

#include <thrift/lib/cpp2/type/AlignedPtr.h>

#include <folly/ConstexprMath.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

namespace apache::thrift::type {
namespace {

std::uintptr_t nBitMask(int bits) {
  return (1ULL << bits) - 1;
}

TEST(AlignedPtr, ImplicitAlignment) {
  detail::AlignedPtr<int16_t> i16;
  EXPECT_EQ(~i16.kMask, nBitMask(1));

  detail::AlignedPtr<int32_t> i32;
  EXPECT_EQ(~i32.kMask, nBitMask(2));

  detail::AlignedPtr<int64_t> i64;
  EXPECT_EQ(~i64.kMask, nBitMask(3));

  struct alignas(16) SixteenByteStruct {
    uint8_t __padding[16];
  };
  EXPECT_EQ(alignof(SixteenByteStruct), 16);
  detail::AlignedPtr<SixteenByteStruct> sixteen;
  EXPECT_EQ(~sixteen.kMask, nBitMask(4));
}

TEST(AlignedPtr, OverAlignment) {
  detail::AlignedPtr<int16_t, /*Bits=*/3, /*MaxBits=*/3> i16;
  EXPECT_EQ(~i16.kMask, nBitMask(3));

  detail::AlignedPtr<int32_t, /*Bits=*/3, /*MaxBits=*/3> i32;
  EXPECT_EQ(~i32.kMask, nBitMask(3));

  detail::AlignedPtr<int64_t, /*Bits=*/3, /*MaxBits=*/4> i64;
  EXPECT_EQ(~i64.kMask, nBitMask(3));

  detail::AlignedPtr<
      unsigned int,
      /*Bits=*/3,
      /*MaxBits=*/std::max(3UL, folly::constexpr_log2(alignof(unsigned int)))>
      uint;
  EXPECT_EQ(~uint.kMask, nBitMask(3));

  struct alignas(16) SixteenByteStruct {
    uint8_t __padding[16];
  };
  detail::AlignedPtr<SixteenByteStruct, /*Bits=*/5, /*MaxBits=*/5> sixteen;
  EXPECT_EQ(~sixteen.kMask, nBitMask(5));
}

TEST(AlignedPtr, UnderAlignment) {
  detail::AlignedPtr<int32_t, 2> i32;
  EXPECT_EQ(~i32.kMask, nBitMask(2));

  detail::AlignedPtr<int64_t, 1> i64;
  EXPECT_EQ(~i64.kMask, nBitMask(1));

  struct alignas(16) SixteenByteStruct {
    uint8_t __padding[16];
  };
  detail::AlignedPtr<SixteenByteStruct, 3> sixteen;
  EXPECT_EQ(~sixteen.kMask, nBitMask(3));
}

} // namespace
} // namespace apache::thrift::type
