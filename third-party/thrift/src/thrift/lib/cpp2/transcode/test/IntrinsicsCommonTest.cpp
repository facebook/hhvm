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

#include <thrift/lib/cpp2/transcode/Intrinsics.h>

#include <array>
#include <cstdint>
#include <limits>
#include <vector>

#include <folly/portability/GTest.h>

namespace {

TranscodeCursor makeReader(const std::vector<uint8_t>& data) {
  static uint8_t output;
  const uint8_t* begin = data.empty() ? &output : data.data();
  TranscodeCursor cursor{};
  thrift_transcode_cursor_init(
      &cursor,
      {begin, begin + data.size()},
      {&output, &output},
      /*extendFn=*/nullptr,
      /*flushFn=*/nullptr,
      /*userData=*/nullptr);
  return cursor;
}

// Feed a cursor the bytes a writer just produced, so a read intrinsic decodes
// exactly what the matching write intrinsic encoded.
class RoundTrip {
 public:
  TranscodeCursor writer() {
    TranscodeCursor cursor{};
    thrift_transcode_cursor_init(
        &cursor,
        /*input=*/{nullptr, nullptr},
        {writeBuf_.data(), writeBuf_.data() + writeBuf_.size()},
        /*extendFn=*/nullptr,
        /*flushFn=*/nullptr,
        /*userData=*/nullptr);
    return cursor;
  }

  TranscodeCursor reader(const TranscodeCursor& w) {
    size_t written = thrift_transcode_cursor_bytes_written(&w);
    TranscodeCursor cursor{};
    thrift_transcode_cursor_init(
        &cursor,
        {writeBuf_.data(), writeBuf_.data() + written},
        {writeBuf_.data(), writeBuf_.data() + writeBuf_.size()},
        /*extendFn=*/nullptr,
        /*flushFn=*/nullptr,
        /*userData=*/nullptr);
    return cursor;
  }

 private:
  std::vector<uint8_t> writeBuf_ = std::vector<uint8_t>(256);
};

TEST(IntrinsicsCommonTest, UnsignedVarintRoundTrip) {
  for (uint64_t v :
       {uint64_t{0},
        uint64_t{1},
        uint64_t{300},
        uint64_t{1} << 35,
        ~uint64_t{0}}) {
    RoundTrip rt;
    auto w = rt.writer();
    thrift_transcode_write_unsigned_varint(&w, v);
    ASSERT_EQ(w.error, 0);
    auto r = rt.reader(w);
    EXPECT_EQ(thrift_transcode_read_unsigned_varint(&r), v);
    EXPECT_EQ(r.error, 0);
  }
}

TEST(IntrinsicsCommonTest, UnsignedVarintReadIsNoopAfterError) {
  const std::vector<uint8_t> data = {0x01};
  auto cursor = makeReader(data);
  const uint8_t* initialReadPos = cursor.readPos;
  cursor.error = 7;

  EXPECT_EQ(thrift_transcode_read_unsigned_varint(&cursor), 0);
  EXPECT_EQ(cursor.readPos, initialReadPos);
  EXPECT_EQ(cursor.error, 7);
}

TEST(IntrinsicsCommonTest, UnsignedVarintRejectsOverflowingTenthByte) {
  std::vector<uint8_t> data(9, 0xff);
  data.push_back(0x02);
  auto cursor = makeReader(data);

  EXPECT_EQ(thrift_transcode_read_unsigned_varint(&cursor), 0);
  EXPECT_NE(cursor.error, 0);
}

TEST(IntrinsicsCommonTest, UnsignedVarintTruncatedInputReturnsZero) {
  const std::vector<uint8_t> data = {0x81};
  auto cursor = makeReader(data);

  EXPECT_EQ(thrift_transcode_read_unsigned_varint(&cursor), 0);
  EXPECT_NE(cursor.error, 0);
}

TEST(IntrinsicsCommonTest, UnsignedVarintWriteUsesEncodedSizeTailroom) {
  std::array<uint8_t, 1> output{};
  TranscodeCursor cursor{};
  thrift_transcode_cursor_init(
      &cursor,
      /*input=*/{nullptr, nullptr},
      {output.data(), output.data() + output.size()},
      /*extendFn=*/nullptr,
      /*flushFn=*/nullptr,
      /*userData=*/nullptr);

  thrift_transcode_write_unsigned_varint(&cursor, 1);

  EXPECT_EQ(cursor.error, 0);
  EXPECT_EQ(thrift_transcode_cursor_bytes_written(&cursor), 1);
  EXPECT_EQ(output[0], 1);
}

TEST(IntrinsicsCommonTest, UnsignedVarintWriteIsNoopAfterError) {
  std::array<uint8_t, 1> output{};
  TranscodeCursor cursor{};
  thrift_transcode_cursor_init(
      &cursor,
      /*input=*/{nullptr, nullptr},
      {output.data(), output.data() + output.size()},
      /*extendFn=*/nullptr,
      /*flushFn=*/nullptr,
      /*userData=*/nullptr);
  cursor.error = 7;

  thrift_transcode_write_unsigned_varint(&cursor, 1);

  EXPECT_EQ(cursor.error, 7);
  EXPECT_EQ(cursor.writePos, output.data());
  EXPECT_EQ(output[0], 0);
}

TEST(IntrinsicsCommonTest, ZigzagVarintRoundTrip) {
  for (int64_t v :
       {int64_t{0},
        int64_t{-1},
        int64_t{1},
        int64_t{-1000},
        int64_t{1000},
        std::numeric_limits<int64_t>::min(),
        std::numeric_limits<int64_t>::max()}) {
    RoundTrip rt;
    auto w = rt.writer();
    thrift_transcode_write_zigzag_varint(&w, v);
    ASSERT_EQ(w.error, 0);
    auto r = rt.reader(w);
    EXPECT_EQ(thrift_transcode_read_zigzag_varint(&r), v);
    EXPECT_EQ(r.error, 0);
  }
}

TEST(IntrinsicsCommonTest, ByteRoundTrip) {
  RoundTrip rt;
  auto w = rt.writer();
  thrift_transcode_write_byte(&w, 0xAB);
  ASSERT_EQ(w.error, 0);
  auto r = rt.reader(w);
  EXPECT_EQ(thrift_transcode_read_byte(&r), 0xAB);
  EXPECT_EQ(r.error, 0);
}

TEST(IntrinsicsCommonTest, Fixed16BeRoundTrip) {
  RoundTrip rt;
  auto w = rt.writer();
  thrift_transcode_write_fixed16_be(&w, 0x1234);
  ASSERT_EQ(w.error, 0);
  auto r = rt.reader(w);
  EXPECT_EQ(thrift_transcode_read_fixed16_be(&r), 0x1234);
}

TEST(IntrinsicsCommonTest, Fixed32RoundTrip) {
  RoundTrip beRt;
  auto beW = beRt.writer();
  thrift_transcode_write_fixed32_be(&beW, 0x12345678u);
  auto beR = beRt.reader(beW);
  EXPECT_EQ(thrift_transcode_read_fixed32_be(&beR), 0x12345678u);

  RoundTrip leRt;
  auto leW = leRt.writer();
  thrift_transcode_write_fixed32_le(&leW, 0x12345678u);
  auto leR = leRt.reader(leW);
  EXPECT_EQ(thrift_transcode_read_fixed32_le(&leR), 0x12345678u);
}

TEST(IntrinsicsCommonTest, Fixed64RoundTrip) {
  RoundTrip beRt;
  auto beW = beRt.writer();
  thrift_transcode_write_fixed64_be(&beW, 0x1122334455667788ull);
  auto beR = beRt.reader(beW);
  EXPECT_EQ(thrift_transcode_read_fixed64_be(&beR), 0x1122334455667788ull);

  RoundTrip leRt;
  auto leW = leRt.writer();
  thrift_transcode_write_fixed64_le(&leW, 0x1122334455667788ull);
  auto leR = leRt.reader(leW);
  EXPECT_EQ(thrift_transcode_read_fixed64_le(&leR), 0x1122334455667788ull);
}

template <typename Value, typename WriteFn>
void expectFixedWriteFailsWithNoTailroom(WriteFn writeFn, Value value) {
  std::array<uint8_t, 1> output{};
  TranscodeCursor cursor{};
  thrift_transcode_cursor_init(
      &cursor,
      /*input=*/{nullptr, nullptr},
      {output.data(), output.data()},
      /*extendFn=*/nullptr,
      /*flushFn=*/nullptr,
      /*userData=*/nullptr);

  writeFn(&cursor, value);

  EXPECT_NE(cursor.error, 0);
  EXPECT_EQ(cursor.writePos, output.data());
}

TEST(IntrinsicsCommonTest, FixedWritersStopAfterEnsureWriteFailure) {
  expectFixedWriteFailsWithNoTailroom(
      thrift_transcode_write_fixed16_be, uint16_t{0x1234});
  expectFixedWriteFailsWithNoTailroom(
      thrift_transcode_write_fixed32_be, uint32_t{0x12345678});
  expectFixedWriteFailsWithNoTailroom(
      thrift_transcode_write_fixed64_be, uint64_t{0x1122334455667788});
  expectFixedWriteFailsWithNoTailroom(
      thrift_transcode_write_fixed32_le, uint32_t{0x12345678});
  expectFixedWriteFailsWithNoTailroom(
      thrift_transcode_write_fixed64_le, uint64_t{0x1122334455667788});
}

// Big-endian and little-endian must lay the same value out in opposite byte
// order. A round-trip alone would not catch a swapped implementation.
TEST(IntrinsicsCommonTest, BigEndianDiffersFromLittleEndian) {
  RoundTrip beRt;
  auto beW = beRt.writer();
  thrift_transcode_write_fixed32_be(&beW, 0x12345678u);
  auto beR = beRt.reader(beW);

  RoundTrip leRt;
  auto leW = leRt.writer();
  thrift_transcode_write_fixed32_le(&leW, 0x12345678u);
  auto leR = leRt.reader(leW);

  // First emitted byte is the high byte for BE, the low byte for LE.
  EXPECT_EQ(thrift_transcode_read_byte(&beR), 0x12);
  EXPECT_EQ(thrift_transcode_read_byte(&leR), 0x78);
}

TEST(IntrinsicsCommonTest, I32PrefixedWriteRejectsSignedOverflowLength) {
  std::array<uint8_t, 16> output{};
  TranscodeCursor cursor{};
  thrift_transcode_cursor_init(
      &cursor,
      /*input=*/{nullptr, nullptr},
      {output.data(), output.data() + output.size()},
      /*extendFn=*/nullptr,
      /*flushFn=*/nullptr,
      /*userData=*/nullptr);

  const uint8_t data = 0;
  thrift_transcode_write_i32_prefixed(
      &cursor,
      &data,
      static_cast<size_t>(std::numeric_limits<int32_t>::max()) + 1);

  EXPECT_NE(cursor.error, 0);
  EXPECT_EQ(thrift_transcode_cursor_bytes_written(&cursor), 0);
}

TEST(IntrinsicsCommonTest, VarintPrefixedReadReturnsEmptyOnLengthError) {
  const std::vector<uint8_t> data = {0x81};
  auto cursor = makeReader(data);

  size_t len = 123;
  const uint8_t* p = thrift_transcode_read_varint_prefixed(&cursor, &len);

  EXPECT_NE(cursor.error, 0);
  EXPECT_EQ(len, 0);
  EXPECT_EQ(p, cursor.readPos);
}

TEST(IntrinsicsCommonTest, I32PrefixedReadRejectsNegativeLength) {
  const std::vector<uint8_t> data = {0xff, 0xff, 0xff, 0xff};
  auto cursor = makeReader(data);

  size_t len = 123;
  const uint8_t* p = thrift_transcode_read_i32_prefixed(&cursor, &len);

  EXPECT_NE(cursor.error, 0);
  EXPECT_EQ(len, 0);
  EXPECT_EQ(p, cursor.readPos);
}

TEST(IntrinsicsCommonTest, VarintPrefixedBytesRoundTrip) {
  const std::vector<uint8_t> payload = {'h', 'e', 'l', 'l', 'o'};
  RoundTrip rt;
  auto w = rt.writer();
  thrift_transcode_write_varint_prefixed(&w, payload.data(), payload.size());
  ASSERT_EQ(w.error, 0);

  auto r = rt.reader(w);
  size_t len = 0;
  const uint8_t* p = thrift_transcode_read_varint_prefixed(&r, &len);
  ASSERT_EQ(r.error, 0);
  const std::vector<uint8_t> decoded(p, p + len);
  EXPECT_EQ(decoded, payload);
}

} // namespace
