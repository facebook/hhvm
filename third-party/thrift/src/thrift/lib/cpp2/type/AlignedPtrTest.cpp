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

#include <gtest/gtest.h>
#include <folly/ConstexprMath.h>
#include <folly/Memory.h>
#include <folly/portability/GMock.h>

namespace apache::thrift::type::detail {
namespace {

std::uintptr_t nBitMask(int bits) {
  return (1ULL << bits) - 1;
}

template <class T>
std::unique_ptr<T, decltype(folly::aligned_free)*> alignedUniquePtr(
    size_t bits) {
  return std::unique_ptr<T, decltype(folly::aligned_free)*>(
      reinterpret_cast<T*>(aligned_alloc(sizeof(T), 1 << bits)),
      folly::aligned_free);
}

TEST(AlignedPtrDeathTest, PtrConflictsWithTagBits) {
  // Check that program terminates if trying to assign a pointer that conflicts
  // with the reserved lower "tag bits".

  static constexpr std::size_t kMaxTagBits = (sizeof(std::uintptr_t) * 8) - 1;
  int i = 0;
  using AlignedPtrT =
      AlignedPtr<int32_t, /*TagBits=*/kMaxTagBits, /*MaxTagBits=*/kMaxTagBits>;
  ASSERT_DEATH(
      AlignedPtrT(/*ptr=*/&i, /*tagBits=*/0),
      "Cannot initialize AlignPtr: ptr uses lower (.+) tag bits:");

  AlignedPtrT p;
  ASSERT_DEATH(
      p.set(&i), "Cannot initialize AlignPtr: ptr uses lower (.+) tag bits:");
}

TEST(AlignedPtrDeathTest, TagBitsExceedNumTagBits) {
  // Trying to initialize an AlignedPtr with (or set) tagBits that do not fit
  // within the TagBits limit should fail (i.e., terminate the process)
  using AlignedPtrT = AlignedPtr<int32_t, /*TagBits=*/1>;
  ASSERT_DEATH(
      AlignedPtrT(/*ptr=*/nullptr, /*tagBits=*/0xff),
      "Cannot initialize AlignPtr: tagBits exceeds (.+) TagBits:");

  // Same failure, on `set()`
  AlignedPtrT p;
  ASSERT_DEATH(
      p.set(/*ptr=*/nullptr, /*tagBits=*/0xff),
      "Cannot initialize AlignPtr: tagBits exceeds (.+) TagBits:");

  // Same failure, on `setTag()`
  AlignedPtrT p2;
  ASSERT_DEATH(
      p2.setTag(0xff),
      "Cannot initialize AlignPtr: tagBits exceeds (.+) TagBits:");
}

TEST(AlignedPtrTest, ImplicitAlignment) {
  EXPECT_EQ((~AlignedPtr<int16_t>::kPointerMask), nBitMask(1));

  EXPECT_EQ(~AlignedPtr<int32_t>::kPointerMask, nBitMask(2));

  EXPECT_EQ(~AlignedPtr<int64_t>::kPointerMask, nBitMask(3));

  struct alignas(16) SixteenByteStruct {
    uint8_t __padding[16];
  };
  EXPECT_EQ(alignof(SixteenByteStruct), 16);
  EXPECT_EQ(~AlignedPtr<SixteenByteStruct>::kPointerMask, nBitMask(4));
}

TEST(AlignedPtrTest, OverAlignment) {
  EXPECT_EQ(
      (~AlignedPtr<int16_t, /*Bits=*/3, /*MaxBits=*/3>::kPointerMask),
      nBitMask(3));

  EXPECT_EQ(
      (~AlignedPtr<int32_t, /*Bits=*/3, /*MaxBits=*/3>::kPointerMask),
      nBitMask(3));

  EXPECT_EQ(
      (~AlignedPtr<int64_t, /*Bits=*/3, /*MaxBits=*/4>::kPointerMask),
      nBitMask(3));

  using AlignedUintPtr = AlignedPtr<
      unsigned int,
      /*Bits=*/3,
      /*MaxBits=*/std::max(3UL, folly::constexpr_log2(alignof(unsigned int)))>;
  EXPECT_EQ(~AlignedUintPtr::kPointerMask, nBitMask(3));

  struct alignas(16) SixteenByteStruct {
    uint8_t __padding[16];
  };
  EXPECT_EQ(
      (~AlignedPtr<SixteenByteStruct, /*Bits=*/5, /*MaxBits=*/5>::kPointerMask),
      nBitMask(5));
}

TEST(AlignedPtrTest, UnderAlignment) {
  EXPECT_EQ((~AlignedPtr<int32_t, 2>::kPointerMask), nBitMask(2));

  EXPECT_EQ((~AlignedPtr<int64_t, 1>::kPointerMask), nBitMask(1));

  struct alignas(16) SixteenByteStruct {
    uint8_t __padding[16];
  };
  EXPECT_EQ((~AlignedPtr<SixteenByteStruct, 3>::kPointerMask), nBitMask(3));
}

TEST(AlignedPtrTest, set) {
  static constexpr size_t kBits = 3;
  auto a = alignedUniquePtr<int32_t>(kBits);
  AlignedPtr<int32_t, kBits, kBits> uut{a.get(), 2};

  EXPECT_EQ(uut.get(), a.get());
  EXPECT_EQ(uut.getTag(), 2);

  auto b = alignedUniquePtr<int32_t>(kBits);
  uut.set(b.get(), 3);
  EXPECT_EQ(uut.get(), b.get());
  EXPECT_EQ(uut.getTag(), 3);

  auto c = alignedUniquePtr<int32_t>(kBits);
  uut.set(c.get());
  EXPECT_EQ(uut.get(), c.get());
  EXPECT_EQ(uut.getTag(), 0);
}

TEST(AlignedPtrTest, setTag) {
  static constexpr size_t kBits = 3;
  auto a = alignedUniquePtr<int32_t>(kBits);
  AlignedPtr<int32_t, kBits, kBits> uut{a.get()};

  EXPECT_EQ(uut.getTag(), 0);
  for (int i = 0; i < 8; i++) {
    uut.setTag(i);
    EXPECT_EQ(i, uut.getTag());
  }
}

TEST(AlignedPtrTest, clear) {
  static constexpr size_t kBits = 3;
  auto a = alignedUniquePtr<int32_t>(kBits);
  AlignedPtr<int32_t, kBits, kBits> uut{a.get(), (1 << kBits) - 1};

  EXPECT_EQ(uut.get(), a.get());
  EXPECT_EQ(uut.getTag(), (1 << kBits) - 1);

  uut.clear();
  EXPECT_EQ(uut.get(), nullptr);
  EXPECT_EQ(uut.getTag(), 0);
}

TEST(AlignedPtrTest, clearTag) {
  static constexpr size_t kBits = 3;
  auto a = alignedUniquePtr<int32_t>(kBits);
  AlignedPtr<int32_t, kBits, kBits> uut{a.get(), 5};

  EXPECT_EQ(uut.get(), a.get());
  EXPECT_EQ(uut.getTag(), 5);

  uut.clearTag();
  EXPECT_EQ(uut.get(), a.get());
  EXPECT_EQ(uut.getTag(), 0);
}

} // namespace
} // namespace apache::thrift::type::detail
