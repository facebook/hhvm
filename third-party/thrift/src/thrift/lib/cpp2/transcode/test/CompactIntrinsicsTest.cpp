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

#include <thrift/lib/cpp2/transcode/CompactIntrinsics.h>

#include <cstdint>
#include <utility>
#include <vector>

#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/transcode/Intrinsics.h>
#include <thrift/lib/cpp2/transcode/WireType.h>

namespace {

namespace wire = apache::thrift::transcode::wire;

TranscodeCursor makeReader(const std::vector<uint8_t>& data) {
  static uint8_t output;
  const uint8_t* begin = data.empty() ? &output : data.data();
  TranscodeCursor cursor{};
  thrift_transcode_cursor_init(
      &cursor,
      {begin, begin + data.size()},
      {&output, &output},
      nullptr,
      nullptr,
      nullptr);
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
        {nullptr, nullptr},
        {buf_.data(), buf_.data() + buf_.size()},
        nullptr,
        nullptr,
        nullptr);
    return cursor;
  }

  TranscodeCursor reader(const TranscodeCursor& w) {
    TranscodeCursor cursor{};
    size_t written = thrift_transcode_cursor_bytes_written(&w);
    thrift_transcode_cursor_init(
        &cursor,
        {buf_.data(), buf_.data() + written},
        {buf_.data(), buf_.data() + buf_.size()},
        nullptr,
        nullptr,
        nullptr);
    return cursor;
  }

 private:
  std::vector<uint8_t> buf_ = std::vector<uint8_t>(256);
};

void writeCompactCollectionHeader(
    TranscodeCursor* cursor, uint8_t elemType, uint32_t count) {
  if (count <= 14) {
    thrift_transcode_write_byte_checked(
        cursor, static_cast<uint8_t>((count << 4) | elemType));
    return;
  }
  thrift_transcode_write_byte_checked(
      cursor, static_cast<uint8_t>(0xF0 | elemType));
  thrift_transcode_write_unsigned_varint(cursor, count);
}

void writeCompactMapHeader(
    TranscodeCursor* cursor, uint8_t keyType, uint8_t valType, uint32_t count) {
  thrift_transcode_write_unsigned_varint(cursor, count);
  if (count == 0) {
    return;
  }
  thrift_transcode_write_byte_checked(
      cursor, static_cast<uint8_t>((keyType << 4) | valType));
}

// A small forward field-id delta packs into the single header byte's high
// nibble and round-trips against the same previous id.
TEST(CompactIntrinsicsTest, DeltaFieldHeaderRoundTrip) {
  RoundTrip rt;
  auto w = rt.writer();
  thrift_transcode_compact_write_field_header(
      &w, wire::kCompactI32, /*fieldId=*/3, /*prevFieldId=*/0);
  ASSERT_EQ(w.error, 0);
  EXPECT_EQ(thrift_transcode_cursor_bytes_written(&w), 1u); // single byte

  auto r = rt.reader(w);
  int16_t got = 0;
  EXPECT_EQ(
      thrift_transcode_compact_read_field_header(&r, &got, /*prevFieldId=*/0),
      wire::kCompactI32);
  EXPECT_EQ(got, 3);
}

// A delta out of the 1..15 range (large jump, or a non-increasing id) falls
// back to an explicit zigzag id that ignores the previous id.
TEST(CompactIntrinsicsTest, ExplicitFieldHeaderRoundTrip) {
  for (auto [fieldId, prev] : std::vector<std::pair<int16_t, int16_t>>{
           {100, 0}, // delta 100 > 15
           {5, 10}}) { // negative delta
    RoundTrip rt;
    auto w = rt.writer();
    thrift_transcode_compact_write_field_header(
        &w, wire::kCompactI64, fieldId, prev);
    ASSERT_EQ(w.error, 0);

    auto r = rt.reader(w);
    int16_t got = 0;
    EXPECT_EQ(
        thrift_transcode_compact_read_field_header(&r, &got, prev),
        wire::kCompactI64);
    EXPECT_EQ(got, fieldId);
  }
}

// A STOP marker reads back as stop with a zeroed field id.
TEST(CompactIntrinsicsTest, StopMarker) {
  RoundTrip rt;
  auto w = rt.writer();
  thrift_transcode_compact_write_stop(&w);

  auto r = rt.reader(w);
  int16_t fieldId = -1;
  EXPECT_EQ(
      thrift_transcode_compact_read_field_header(&r, &fieldId, 0),
      wire::kCompactStop);
  EXPECT_EQ(fieldId, 0);
}

TEST(CompactIntrinsicsTest, FieldHeaderRejectsDeltaStopType) {
  for (uint8_t delta = 1; delta <= 15; ++delta) {
    const std::vector<uint8_t> data = {static_cast<uint8_t>(delta << 4)};
    auto r = makeReader(data);

    int16_t fieldId = -1;
    EXPECT_EQ(thrift_transcode_compact_read_field_header(&r, &fieldId, 0), 0);
    EXPECT_EQ(fieldId, 0);
    EXPECT_NE(r.error, 0);
  }
}

TEST(CompactIntrinsicsTest, TruncatedFieldHeaderSetsError) {
  const std::vector<uint8_t> data;
  auto r = makeReader(data);

  int16_t fieldId = -1;
  EXPECT_EQ(thrift_transcode_compact_read_field_header(&r, &fieldId, 0), 0);
  EXPECT_EQ(fieldId, 0);
  EXPECT_NE(r.error, 0);
}

TEST(CompactIntrinsicsTest, TruncatedExplicitFieldIdSetsError) {
  const std::vector<uint8_t> data = {wire::kCompactI32};
  auto r = makeReader(data);

  int16_t fieldId = -1;
  EXPECT_EQ(thrift_transcode_compact_read_field_header(&r, &fieldId, 0), 0);
  EXPECT_EQ(fieldId, 0);
  EXPECT_NE(r.error, 0);
}

TEST(CompactIntrinsicsTest, ExplicitFieldIdOverflowSetsError) {
  const std::vector<uint8_t> data = {wire::kCompactI32, 0x80, 0x80, 0x04};
  auto r = makeReader(data);

  int16_t fieldId = -1;
  EXPECT_EQ(thrift_transcode_compact_read_field_header(&r, &fieldId, 0), 0);
  EXPECT_EQ(fieldId, 0);
  EXPECT_NE(r.error, 0);
}

TEST(CompactIntrinsicsTest, DeltaFieldIdOverflowSetsError) {
  const std::vector<uint8_t> data = {
      static_cast<uint8_t>((1 << 4) | wire::kCompactI32)};
  auto r = makeReader(data);

  int16_t fieldId = -1;
  EXPECT_EQ(thrift_transcode_compact_read_field_header(&r, &fieldId, 32767), 0);
  EXPECT_EQ(fieldId, 0);
  EXPECT_NE(r.error, 0);
}

// A Compact bool is encoded in the header type nibble itself: the value is the
// type, and no separate value byte follows.
TEST(CompactIntrinsicsTest, BoolEncodedInTypeNibble) {
  for (bool value : {true, false}) {
    RoundTrip rt;
    auto w = rt.writer();
    thrift_transcode_compact_write_bool_field(&w, value ? 1 : 0, 1, 0);
    ASSERT_EQ(w.error, 0);
    EXPECT_EQ(thrift_transcode_cursor_bytes_written(&w), 1u); // no value byte

    auto r = rt.reader(w);
    int16_t got = 0;
    uint8_t ttype = thrift_transcode_compact_read_field_header(&r, &got, 0);
    EXPECT_EQ(
        ttype, value ? wire::kCompactBooleanTrue : wire::kCompactBooleanFalse);
    EXPECT_EQ(got, 1);
  }
}

// skip_field consumes a varint-encoded integer value, leaving the reader on the
// following byte.
TEST(CompactIntrinsicsTest, SkipVarintField) {
  RoundTrip rt;
  auto w = rt.writer();
  thrift_transcode_write_unsigned_varint(&w, 300); // two varint bytes
  thrift_transcode_write_byte_checked(&w, 0x7F);

  auto r = rt.reader(w);
  thrift_transcode_compact_skip_field(&r, wire::kCompactI32);
  ASSERT_EQ(r.error, 0);
  EXPECT_EQ(thrift_transcode_read_byte_checked(&r), 0x7F);
}

TEST(CompactIntrinsicsTest, SkipByteFieldRejectsTruncatedInput) {
  const std::vector<uint8_t> data;
  auto r = makeReader(data);

  thrift_transcode_compact_skip_field(&r, wire::kCompactByte);
  EXPECT_NE(r.error, 0);
  EXPECT_LE(r.readPos, r.readEnd);
}

TEST(CompactIntrinsicsTest, SkipFloatFieldRejectsTruncatedInput) {
  const std::vector<uint8_t> data = {0x00, 0x01, 0x02};
  auto r = makeReader(data);

  thrift_transcode_compact_skip_field(&r, wire::kCompactFloat);
  EXPECT_NE(r.error, 0);
  EXPECT_LE(r.readPos, r.readEnd);
}

// skip_field over a binary value consumes its varint length prefix and payload.
TEST(CompactIntrinsicsTest, SkipBinaryField) {
  const std::vector<uint8_t> payload = {'h', 'i', '!'};
  RoundTrip rt;
  auto w = rt.writer();
  thrift_transcode_write_varint_prefixed(&w, payload.data(), payload.size());
  thrift_transcode_write_byte_checked(&w, 0x7F);

  auto r = rt.reader(w);
  thrift_transcode_compact_skip_field(&r, wire::kCompactBinary);
  ASSERT_EQ(r.error, 0);
  EXPECT_EQ(thrift_transcode_read_byte_checked(&r), 0x7F);
}

TEST(CompactIntrinsicsTest, SkipBinaryFieldRejectsTruncatedPayload) {
  const std::vector<uint8_t> data = {0x03, 'h', 'i'};
  auto r = makeReader(data);

  thrift_transcode_compact_skip_field(&r, wire::kCompactBinary);
  EXPECT_NE(r.error, 0);
  EXPECT_LE(r.readPos, r.readEnd);
}

// A bool field carries no value bytes, so skipping it advances nothing.
TEST(CompactIntrinsicsTest, SkipBoolConsumesNothing) {
  RoundTrip rt;
  auto w = rt.writer();
  thrift_transcode_write_byte_checked(&w, 0x7F);

  auto r = rt.reader(w);
  thrift_transcode_compact_skip_field(&r, wire::kCompactBooleanTrue);
  ASSERT_EQ(r.error, 0);
  EXPECT_EQ(thrift_transcode_read_byte_checked(&r), 0x7F);
}

TEST(CompactIntrinsicsTest, SkipStructField) {
  RoundTrip rt;
  auto w = rt.writer();
  thrift_transcode_compact_write_field_header(
      &w, wire::kCompactI16, /*fieldId=*/1, /*prevFieldId=*/0);
  thrift_transcode_write_zigzag_varint(&w, 7);
  thrift_transcode_compact_write_stop(&w);
  thrift_transcode_write_byte_checked(&w, 0x7F);

  auto r = rt.reader(w);
  thrift_transcode_compact_skip_field(&r, wire::kCompactStruct);
  ASSERT_EQ(r.error, 0);
  EXPECT_EQ(thrift_transcode_read_byte_checked(&r), 0x7F);
}

TEST(CompactIntrinsicsTest, SkipStructFieldRejectsDeltaStopType) {
  const std::vector<uint8_t> data = {0x10, 0x7F};
  auto r = makeReader(data);

  thrift_transcode_compact_skip_field(&r, wire::kCompactStruct);
  EXPECT_NE(r.error, 0);
  EXPECT_LE(r.readPos, r.readEnd);
}

TEST(CompactIntrinsicsTest, SkipListField) {
  RoundTrip rt;
  auto w = rt.writer();
  writeCompactCollectionHeader(&w, wire::kCompactI16, 2);
  thrift_transcode_write_zigzag_varint(&w, 1);
  thrift_transcode_write_zigzag_varint(&w, 2);
  thrift_transcode_write_byte_checked(&w, 0x7F);

  auto r = rt.reader(w);
  thrift_transcode_compact_skip_field(&r, wire::kCompactList);
  ASSERT_EQ(r.error, 0);
  EXPECT_EQ(thrift_transcode_read_byte_checked(&r), 0x7F);
}

TEST(CompactIntrinsicsTest, SkipListFieldWithExtendedCount) {
  RoundTrip rt;
  auto w = rt.writer();
  writeCompactCollectionHeader(&w, wire::kCompactI16, 15);
  for (int i = 0; i < 15; ++i) {
    thrift_transcode_write_zigzag_varint(&w, i);
  }
  thrift_transcode_write_byte_checked(&w, 0x7F);

  auto r = rt.reader(w);
  thrift_transcode_compact_skip_field(&r, wire::kCompactList);
  ASSERT_EQ(r.error, 0);
  EXPECT_EQ(thrift_transcode_read_byte_checked(&r), 0x7F);
}

TEST(CompactIntrinsicsTest, SkipListFieldConsumesBoolValueBytes) {
  RoundTrip rt;
  auto w = rt.writer();
  writeCompactCollectionHeader(&w, wire::kCompactBooleanTrue, 2);
  thrift_transcode_write_byte_checked(&w, wire::kCompactBooleanTrue);
  thrift_transcode_write_byte_checked(&w, wire::kCompactBooleanFalse);
  thrift_transcode_write_byte_checked(&w, 0x7F);

  auto r = rt.reader(w);
  thrift_transcode_compact_skip_field(&r, wire::kCompactList);
  ASSERT_EQ(r.error, 0);
  EXPECT_EQ(thrift_transcode_read_byte_checked(&r), 0x7F);
}

TEST(CompactIntrinsicsTest, SkipMapField) {
  const std::vector<uint8_t> payload = {'o', 'k'};
  RoundTrip rt;
  auto w = rt.writer();
  writeCompactMapHeader(&w, wire::kCompactByte, wire::kCompactBinary, 2);
  thrift_transcode_write_byte_checked(&w, 1);
  thrift_transcode_write_varint_prefixed(&w, payload.data(), payload.size());
  thrift_transcode_write_byte_checked(&w, 2);
  thrift_transcode_write_varint_prefixed(&w, payload.data(), payload.size());
  thrift_transcode_write_byte_checked(&w, 0x7F);

  auto r = rt.reader(w);
  thrift_transcode_compact_skip_field(&r, wire::kCompactMap);
  ASSERT_EQ(r.error, 0);
  EXPECT_EQ(thrift_transcode_read_byte_checked(&r), 0x7F);
}

TEST(CompactIntrinsicsTest, SkipMapFieldRejectsTruncatedTypeByte) {
  const std::vector<uint8_t> data = {0x01};
  auto r = makeReader(data);

  thrift_transcode_compact_skip_field(&r, wire::kCompactMap);
  EXPECT_NE(r.error, 0);
}

TEST(CompactIntrinsicsTest, SkipStructFieldRejectsExcessiveDepth) {
  std::vector<uint8_t> data;
  data.reserve(70);
  for (int i = 0; i < 70; ++i) {
    data.push_back(static_cast<uint8_t>((1 << 4) | wire::kCompactStruct));
  }
  auto r = makeReader(data);

  thrift_transcode_compact_skip_field(&r, wire::kCompactStruct);
  EXPECT_NE(r.error, 0);
}

TEST(CompactIntrinsicsTest, SkipUnsupportedFieldTypeSetsError) {
  const std::vector<uint8_t> data = {0x00};
  auto r = makeReader(data);

  thrift_transcode_compact_skip_field(&r, 0xff);
  EXPECT_NE(r.error, 0);
}

} // namespace
