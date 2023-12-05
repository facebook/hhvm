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

#include <thrift/lib/cpp/util/VarintUtils.h>

#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp/util/test/VarintUtilsTestUtil.h>

#ifndef __BMI2__
#define writeVarintBMI2 writeVarintUnrolled
#endif

using namespace apache::thrift::util;
using namespace apache::thrift::util::detail;

class VarintUtilsTest : public testing::Test {};

TEST_F(VarintUtilsTest, example) {
  folly::IOBufQueue queueUnrolled;
  folly::IOBufQueue queueBMI2;
  folly::io::QueueAppender appenderUnrolled(&queueUnrolled, 1000);
  folly::io::QueueAppender appenderBMI2(&queueBMI2, 1000);

  auto test = [&](int bit) {
    std::string u;
    std::string b;
    queueUnrolled.appendToString(u);
    queueBMI2.appendToString(b);
    CHECK_EQ(u.size(), b.size()) << "bit: " << bit;
    CHECK_EQ(u, b) << "bit: " << bit;
  };

  int64_t v = 1;
  writeVarintUnrolled(appenderUnrolled, 0);
  writeVarintBMI2(appenderBMI2, 0);
  for (int bit = 0; bit < 64; bit++, v <<= int(bit < 64)) {
    if (bit < 8) {
      writeVarintUnrolled(appenderUnrolled, int8_t(v));
      writeVarintBMI2(appenderBMI2, int8_t(v));
      test(bit);
    }
    if (bit < 16) {
      writeVarintUnrolled(appenderUnrolled, int16_t(v));
      writeVarintBMI2(appenderBMI2, int16_t(v));
      test(bit);
    }
    if (bit < 32) {
      writeVarintUnrolled(appenderUnrolled, int32_t(v));
      writeVarintBMI2(appenderBMI2, int32_t(v));
      test(bit);
    }
    writeVarintUnrolled(appenderUnrolled, v);
    writeVarintBMI2(appenderBMI2, v);
    test(bit);
  }
  int32_t oversize = 1000000;
  writeVarintUnrolled(appenderUnrolled, oversize);
  writeVarintBMI2(appenderBMI2, oversize);

  {
    folly::io::Cursor rcursor(queueUnrolled.front());
    EXPECT_EQ(0, readVarint<int8_t>(rcursor));
    v = 1;
    for (int bit = 0; bit < 64; bit++, v <<= int(bit < 64)) {
      if (bit < 8) {
        EXPECT_EQ(int8_t(v), readVarint<int8_t>(rcursor));
      }
      if (bit < 16) {
        EXPECT_EQ(int16_t(v), readVarint<int16_t>(rcursor));
      }
      if (bit < 32) {
        EXPECT_EQ(int32_t(v), readVarint<int32_t>(rcursor));
      }
      EXPECT_EQ(v, readVarint<int64_t>(rcursor));
    }
    EXPECT_THROW(readVarint<uint8_t>(rcursor), std::out_of_range);
  }

  {
    folly::io::Cursor rcursor(queueBMI2.front());
    EXPECT_EQ(0, readVarint<int8_t>(rcursor));
    v = 1;
    for (int bit = 0; bit < 64; bit++, v <<= int(bit < 64)) {
      if (bit < 8) {
        EXPECT_EQ(int8_t(v), readVarint<int8_t>(rcursor));
      }
      if (bit < 16) {
        EXPECT_EQ(int16_t(v), readVarint<int16_t>(rcursor));
      }
      if (bit < 32) {
        EXPECT_EQ(int32_t(v), readVarint<int32_t>(rcursor));
      }
      EXPECT_EQ(v, readVarint<int64_t>(rcursor));
    }
    EXPECT_THROW(readVarint<uint8_t>(rcursor), std::out_of_range);
  }
}

template <typename Param>
struct VarintUtilsMegaTest : public testing::TestWithParam<Param> {};
TYPED_TEST_SUITE_P(VarintUtilsMegaTest);

TYPED_TEST_P(VarintUtilsMegaTest, example) {
  auto ints = TypeParam::gen();
  std::string strUnrolled;
  {
    folly::IOBufQueue q(folly::IOBufQueue::cacheChainLength());
    constexpr size_t kDesiredGrowth = 1 << 14;
    folly::io::QueueAppender c(&q, kDesiredGrowth);
    for (auto v : ints) {
      writeVarintUnrolled(c, v);
    }
    q.appendToString(strUnrolled);
  }

  std::string strBmi2;
  {
    folly::IOBufQueue q(folly::IOBufQueue::cacheChainLength());
    constexpr size_t kDesiredGrowth = 1 << 14;
    folly::io::QueueAppender c(&q, kDesiredGrowth);
    for (auto v : ints) {
      writeVarintBMI2(c, v);
    }
    q.appendToString(strBmi2);
  }
  EXPECT_EQ(strUnrolled, strBmi2);
}

REGISTER_TYPED_TEST_SUITE_P( //
    VarintUtilsMegaTest,
    example);

INSTANTIATE_TYPED_TEST_SUITE_P(u8_1b, VarintUtilsMegaTest, u8_1b);
INSTANTIATE_TYPED_TEST_SUITE_P(u16_1b, VarintUtilsMegaTest, u16_1b);
INSTANTIATE_TYPED_TEST_SUITE_P(u32_1b, VarintUtilsMegaTest, u32_1b);
INSTANTIATE_TYPED_TEST_SUITE_P(u64_1b, VarintUtilsMegaTest, u64_1b);

INSTANTIATE_TYPED_TEST_SUITE_P(u8_2b, VarintUtilsMegaTest, u8_2b);
INSTANTIATE_TYPED_TEST_SUITE_P(u16_2b, VarintUtilsMegaTest, u16_2b);
INSTANTIATE_TYPED_TEST_SUITE_P(u32_2b, VarintUtilsMegaTest, u32_2b);
INSTANTIATE_TYPED_TEST_SUITE_P(u64_2b, VarintUtilsMegaTest, u64_2b);

INSTANTIATE_TYPED_TEST_SUITE_P(u16_3b, VarintUtilsMegaTest, u16_3b);
INSTANTIATE_TYPED_TEST_SUITE_P(u32_3b, VarintUtilsMegaTest, u32_3b);
INSTANTIATE_TYPED_TEST_SUITE_P(u64_3b, VarintUtilsMegaTest, u64_3b);

INSTANTIATE_TYPED_TEST_SUITE_P(u32_4b, VarintUtilsMegaTest, u32_4b);
INSTANTIATE_TYPED_TEST_SUITE_P(u64_4b, VarintUtilsMegaTest, u64_4b);

INSTANTIATE_TYPED_TEST_SUITE_P(u32_5b, VarintUtilsMegaTest, u32_5b);
INSTANTIATE_TYPED_TEST_SUITE_P(u64_5b, VarintUtilsMegaTest, u64_5b);

INSTANTIATE_TYPED_TEST_SUITE_P(u64_6b, VarintUtilsMegaTest, u64_6b);
INSTANTIATE_TYPED_TEST_SUITE_P(u64_7b, VarintUtilsMegaTest, u64_7b);
INSTANTIATE_TYPED_TEST_SUITE_P(u64_8b, VarintUtilsMegaTest, u64_8b);
INSTANTIATE_TYPED_TEST_SUITE_P(u64_9b, VarintUtilsMegaTest, u64_9b);
INSTANTIATE_TYPED_TEST_SUITE_P(u64_10b, VarintUtilsMegaTest, u64_10b);

INSTANTIATE_TYPED_TEST_SUITE_P(u8_any, VarintUtilsMegaTest, u8_any);
INSTANTIATE_TYPED_TEST_SUITE_P(u16_any, VarintUtilsMegaTest, u16_any);
INSTANTIATE_TYPED_TEST_SUITE_P(u32_any, VarintUtilsMegaTest, u32_any);
INSTANTIATE_TYPED_TEST_SUITE_P(u64_any, VarintUtilsMegaTest, u64_any);

INSTANTIATE_TYPED_TEST_SUITE_P(s8_1b, VarintUtilsMegaTest, s8_1b);
INSTANTIATE_TYPED_TEST_SUITE_P(s16_1b, VarintUtilsMegaTest, s16_1b);
INSTANTIATE_TYPED_TEST_SUITE_P(s32_1b, VarintUtilsMegaTest, s32_1b);
INSTANTIATE_TYPED_TEST_SUITE_P(s64_1b, VarintUtilsMegaTest, s64_1b);

INSTANTIATE_TYPED_TEST_SUITE_P(s8_2b, VarintUtilsMegaTest, s8_2b);
INSTANTIATE_TYPED_TEST_SUITE_P(s16_2b, VarintUtilsMegaTest, s16_2b);
INSTANTIATE_TYPED_TEST_SUITE_P(s32_2b, VarintUtilsMegaTest, s32_2b);
INSTANTIATE_TYPED_TEST_SUITE_P(s64_2b, VarintUtilsMegaTest, s64_2b);

INSTANTIATE_TYPED_TEST_SUITE_P(s16_3b, VarintUtilsMegaTest, s16_3b);
INSTANTIATE_TYPED_TEST_SUITE_P(s32_3b, VarintUtilsMegaTest, s32_3b);
INSTANTIATE_TYPED_TEST_SUITE_P(s64_3b, VarintUtilsMegaTest, s64_3b);

INSTANTIATE_TYPED_TEST_SUITE_P(s32_4b, VarintUtilsMegaTest, s32_4b);
INSTANTIATE_TYPED_TEST_SUITE_P(s64_4b, VarintUtilsMegaTest, s64_4b);

INSTANTIATE_TYPED_TEST_SUITE_P(s32_5b, VarintUtilsMegaTest, s32_5b);
INSTANTIATE_TYPED_TEST_SUITE_P(s64_5b, VarintUtilsMegaTest, s64_5b);

INSTANTIATE_TYPED_TEST_SUITE_P(s64_6b, VarintUtilsMegaTest, s64_6b);
INSTANTIATE_TYPED_TEST_SUITE_P(s64_7b, VarintUtilsMegaTest, s64_7b);
INSTANTIATE_TYPED_TEST_SUITE_P(s64_8b, VarintUtilsMegaTest, s64_8b);
INSTANTIATE_TYPED_TEST_SUITE_P(s64_9b, VarintUtilsMegaTest, s64_9b);
INSTANTIATE_TYPED_TEST_SUITE_P(s64_10b, VarintUtilsMegaTest, s64_10b);

INSTANTIATE_TYPED_TEST_SUITE_P(s8_any, VarintUtilsMegaTest, s8_any);
INSTANTIATE_TYPED_TEST_SUITE_P(s16_any, VarintUtilsMegaTest, s16_any);
INSTANTIATE_TYPED_TEST_SUITE_P(s32_any, VarintUtilsMegaTest, s32_any);
INSTANTIATE_TYPED_TEST_SUITE_P(s64_any, VarintUtilsMegaTest, s64_any);

template <typename Param>
struct ReadVarintMediumSlowTest : public testing::TestWithParam<Param> {};
TYPED_TEST_SUITE_P(ReadVarintMediumSlowTest);

TYPED_TEST_P(ReadVarintMediumSlowTest, Simple) {
  if (TypeParam::skip) {
    return;
  }

  for (int i = 1; i < TypeParam::kMaxVarintSize; i++) {
    // A bunch of continuation bytes that are otherwise zero...
    unsigned char buf[TypeParam::kMaxVarintSize];
    memset(buf, 0x80, sizeof(buf));
    // But with a 1 (and no continuation bit) in byte i.
    buf[i] = 0x01;

    typename TypeParam::UIntType expected = 1ULL << (7 * i);

    typename TypeParam::UIntType value;
    size_t bytesRead = TypeParam::doReadVarintMediumSlow(value, buf);
    EXPECT_EQ(expected, value);
    EXPECT_EQ(i + 1, bytesRead);
  }
}

TYPED_TEST_P(ReadVarintMediumSlowTest, Overflow) {
  if (TypeParam::skip) {
    return;
  }

  unsigned char buf[TypeParam::kMaxVarintSize];
  memset(buf, 0x80, sizeof(buf));
  typename TypeParam::UIntType value;
  EXPECT_THROW(
      TypeParam::doReadVarintMediumSlow(value, buf), std::out_of_range);
}

TYPED_TEST_P(ReadVarintMediumSlowTest, JunkHighBits) {
  if (TypeParam::skip) {
    return;
  }

  unsigned char buf[TypeParam::kMaxVarintSize];
  memset(buf, 0x80, sizeof(buf));
  // Impossibly large varint, but allowed by the reference parser (with the
  // semantics of dropping the high bits). Check that the accelerated ones do
  // the same thing.
  buf[TypeParam::kMaxVarintSize - 1] = 0x7F;
  typename TypeParam::UIntType value;
  TypeParam::doReadVarintMediumSlow(value, buf);
  EXPECT_EQ(1ULL << (sizeof(typename TypeParam::UIntType) * 8 - 1), value);
}

TYPED_TEST_P(ReadVarintMediumSlowTest, BigZeros) {
  if (TypeParam::skip) {
    return;
  }

  for (int i = 1; i < TypeParam::kMaxVarintSize; i++) {
    unsigned char buf[TypeParam::kMaxVarintSize];
    // A space-consuming way of expressing 0, but currently allowed.
    memset(buf, 0x80, sizeof(buf));
    buf[i] = 0;
    typename TypeParam::UIntType value;
    size_t bytesRead = TypeParam::doReadVarintMediumSlow(value, buf);
    EXPECT_EQ(0, value);
    EXPECT_EQ(i + 1, bytesRead);
  }
}

TYPED_TEST_P(ReadVarintMediumSlowTest, Decodes) {
  if (TypeParam::skip) {
    return;
  }
  // We check all possible combinations of one, two, or three bits being set.
  // The reasoning is that the high-performance implementations all do some
  // degree of bit permuting, so a 1-bit difference in each position should
  // catch any permutation errors.
  const static int kNumBits = 8 * sizeof(typename TypeParam::UIntType);
  // We start the lowest value for the high bit at 7, so that we only test
  // 2-bytes-or-larger varints, which is all that the readVarintMediumSlow
  // functions are expected to handle.
  for (int i = 7; i < kNumBits; i++) {
    for (int j = 0; j <= i; j++) {
      for (int k = 0; k <= j; k++) {
        unsigned char buf[TypeParam::kMaxVarintSize];
        memset(buf, 0x80, sizeof(buf));
        buf[i / 7] = 0;
        buf[i / 7] |= (1 << (i % 7));
        buf[j / 7] |= (1 << (j % 7));
        buf[k / 7] |= (1 << (k % 7));
        typename TypeParam::UIntType expected =
            (1ULL << i) | (1ULL << j) | (1ULL << k);
        typename TypeParam::UIntType value;
        size_t bytesRead = TypeParam::doReadVarintMediumSlow(value, buf);
        EXPECT_EQ(expected, value);
        EXPECT_EQ(i / 7 + 1, bytesRead);
      }
    }
  }
}

REGISTER_TYPED_TEST_SUITE_P(
    ReadVarintMediumSlowTest,
    Simple,
    Overflow,
    JunkHighBits,
    BigZeros,
    Decodes);

struct SkippingU64Impl {
  const static bool skip = true;
  const static int kMaxVarintSize = 10;
  using UIntType = uint64_t;
  template <typename CursorT>
  static void doReadVarintMediumSlow(uint64_t&, const uint8_t) {
    ADD_FAILURE();
  }
};

struct UnrolledU64Impl {
  const static bool skip = false;
  const static int kMaxVarintSize = 10;
  using UIntType = uint64_t;
  static size_t doReadVarintMediumSlow(uint64_t& value, const uint8_t* p) {
    return readVarintMediumSlowUnrolled(value, p);
  }
};

#if THRIFT_UTIL_VARINTUTILS_SIMD_DECODER
struct SIMDU64Impl {
  const static bool skip = false;
  const static int kMaxVarintSize = 10;
  using UIntType = uint64_t;
  static size_t doReadVarintMediumSlow(uint64_t& value, const uint8_t* p) {
    return readContiguousVarintMediumSlowU64SIMD(value, p);
  }
};
#else
using SIMDU64Impl = SkippingU64Impl;
#endif

#if THRIFT_UTIL_VARINTUTILS_BMI2_DECODER
struct BMI2U64Impl {
  const static bool skip = false;
  const static int kMaxVarintSize = 10;
  using UIntType = uint64_t;
  static size_t doReadVarintMediumSlow(uint64_t& value, const uint8_t* p) {
    return readContiguousVarintMediumSlowU64BMI2(value, p);
  }
};
#else
using BMI2U64Impl = SkippingU64Impl;
#endif

INSTANTIATE_TYPED_TEST_SUITE_P(
    UnrolledU64Impl, ReadVarintMediumSlowTest, UnrolledU64Impl);
INSTANTIATE_TYPED_TEST_SUITE_P(
    SIMDU64Impl, ReadVarintMediumSlowTest, SIMDU64Impl);
INSTANTIATE_TYPED_TEST_SUITE_P(
    BMI2U64Impl, ReadVarintMediumSlowTest, BMI2U64Impl);
