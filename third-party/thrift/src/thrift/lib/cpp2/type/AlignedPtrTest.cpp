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

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

namespace apache::thrift::type {
namespace {

TEST(AlignedPtr, ExplicitAlignments) {
  detail::AlignedPtr<int16_t> i16;
  EXPECT_EQ(~i16.kMask, 1);

  detail::AlignedPtr<int32_t> i32;
  EXPECT_EQ(~i32.kMask, 3);

  detail::AlignedPtr<int64_t> i64;
  EXPECT_EQ(~i64.kMask, 7);

  struct alignas(16) SixteenByteStruct {
    uint8_t __padding[16];
  };
  EXPECT_EQ(alignof(SixteenByteStruct), 16);
  detail::AlignedPtr<SixteenByteStruct> sixteen;
  EXPECT_EQ(~sixteen.kMask, 15);
}

} // namespace
} // namespace apache::thrift::type
