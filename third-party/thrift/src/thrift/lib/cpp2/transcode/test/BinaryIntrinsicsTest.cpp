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

#include <thrift/lib/cpp2/transcode/BinaryIntrinsics.h>

#include <cstdint>
#include <vector>

#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/transcode/Intrinsics.h>
#include <thrift/lib/cpp2/transcode/WireType.h>

namespace {

namespace wire = apache::thrift::transcode::wire;

TranscodeCursor makeReader(const std::vector<uint8_t>& data) {
  static uint8_t output;
  TranscodeCursor cursor{};
  thrift_transcode_cursor_init(
      &cursor,
      {data.data(), data.data() + data.size()},
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

// A Binary field header round-trips its type byte and field id.
TEST(BinaryIntrinsicsTest, FieldHeaderRoundTrip) {
  for (int16_t fieldId :
       {int16_t{1}, int16_t{2}, int16_t{1000}, int16_t{32767}}) {
    RoundTrip rt;
    auto w = rt.writer();
    thrift_transcode_binary_write_field_header(
        &w, wire::kBinaryI32, fieldId, 0);
    ASSERT_EQ(w.error, 0);

    auto r = rt.reader(w);
    int16_t got = 0;
    EXPECT_EQ(
        thrift_transcode_binary_read_field_header(&r, &got, 0),
        wire::kBinaryI32);
    EXPECT_EQ(got, fieldId);
    EXPECT_EQ(r.error, 0);
  }
}

// The field id is laid out big-endian: type byte, then the id high byte first.
TEST(BinaryIntrinsicsTest, FieldIdIsBigEndian) {
  RoundTrip rt;
  auto w = rt.writer();
  thrift_transcode_binary_write_field_header(
      &w, wire::kBinaryString, 0x0102, 0);

  auto r = rt.reader(w);
  EXPECT_EQ(thrift_transcode_read_byte_checked(&r), wire::kBinaryString);
  EXPECT_EQ(thrift_transcode_read_byte_checked(&r), 0x01);
  EXPECT_EQ(thrift_transcode_read_byte_checked(&r), 0x02);
}

// A STOP marker reads back as T_STOP with a zeroed field id.
TEST(BinaryIntrinsicsTest, StopMarker) {
  RoundTrip rt;
  auto w = rt.writer();
  thrift_transcode_binary_write_stop(&w);

  auto r = rt.reader(w);
  int16_t fieldId = -1;
  EXPECT_EQ(
      thrift_transcode_binary_read_field_header(&r, &fieldId, 0),
      wire::kBinaryStop);
  EXPECT_EQ(fieldId, 0);
}

TEST(BinaryIntrinsicsTest, TruncatedFieldHeaderSetsError) {
  const std::vector<uint8_t> data = {wire::kBinaryI32, 0x01};
  auto r = makeReader(data);

  int16_t fieldId = -1;
  EXPECT_EQ(thrift_transcode_binary_read_field_header(&r, &fieldId, 0), 0);
  EXPECT_EQ(fieldId, 0);
  EXPECT_NE(r.error, 0);
}

// skip_field steps over a fixed-width value, leaving the reader on the byte
// that follows it.
TEST(BinaryIntrinsicsTest, SkipFixedWidthField) {
  RoundTrip rt;
  auto w = rt.writer();
  thrift_transcode_write_fixed32_be_checked(&w, 0xDEADBEEFu);
  thrift_transcode_write_byte_checked(&w, 0x7F);

  auto r = rt.reader(w);
  thrift_transcode_binary_skip_field(&r, wire::kBinaryI32);
  ASSERT_EQ(r.error, 0);
  EXPECT_EQ(thrift_transcode_read_byte_checked(&r), 0x7F);
}

TEST(BinaryIntrinsicsTest, SkipFixedWidthFieldRejectsTruncatedInput) {
  const std::vector<uint8_t> data = {0x01};
  auto r = makeReader(data);

  thrift_transcode_binary_skip_field(&r, wire::kBinaryI16);
  EXPECT_NE(r.error, 0);
  EXPECT_LE(r.readPos, r.readEnd);
}

// skip_field over a string consumes its i32 length prefix and payload.
TEST(BinaryIntrinsicsTest, SkipStringField) {
  const std::vector<uint8_t> payload = {'h', 'i', '!'};
  RoundTrip rt;
  auto w = rt.writer();
  thrift_transcode_write_i32_prefixed(&w, payload.data(), payload.size());
  thrift_transcode_write_byte_checked(&w, 0x7F);

  auto r = rt.reader(w);
  thrift_transcode_binary_skip_field(&r, wire::kBinaryString);
  ASSERT_EQ(r.error, 0);
  EXPECT_EQ(thrift_transcode_read_byte_checked(&r), 0x7F);
}

TEST(BinaryIntrinsicsTest, SkipStringFieldRejectsNegativeLength) {
  const std::vector<uint8_t> data = {0xff, 0xff, 0xff, 0xff};
  auto r = makeReader(data);

  thrift_transcode_binary_skip_field(&r, wire::kBinaryString);
  EXPECT_NE(r.error, 0);
}

TEST(BinaryIntrinsicsTest, SkipStringFieldRejectsTruncatedPayload) {
  const std::vector<uint8_t> data = {0x00, 0x00, 0x00, 0x03, 'h', 'i'};
  auto r = makeReader(data);

  thrift_transcode_binary_skip_field(&r, wire::kBinaryString);
  EXPECT_NE(r.error, 0);
  EXPECT_LE(r.readPos, r.readEnd);
}

TEST(BinaryIntrinsicsTest, SkipListField) {
  const std::vector<uint8_t> data = {
      wire::kBinaryI16, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x02, 0x7f};
  auto r = makeReader(data);

  thrift_transcode_binary_skip_field(&r, wire::kBinaryList);
  ASSERT_EQ(r.error, 0);
  EXPECT_EQ(thrift_transcode_read_byte_checked(&r), 0x7f);
}

TEST(BinaryIntrinsicsTest, SkipListFieldRejectsNegativeCount) {
  const std::vector<uint8_t> data = {wire::kBinaryI32, 0xff, 0xff, 0xff, 0xff};
  auto r = makeReader(data);

  thrift_transcode_binary_skip_field(&r, wire::kBinaryList);
  EXPECT_NE(r.error, 0);
}

TEST(BinaryIntrinsicsTest, SkipUnknownFieldTypeSetsError) {
  const std::vector<uint8_t> data = {0x00};
  auto r = makeReader(data);

  thrift_transcode_binary_skip_field(&r, 0xff);
  EXPECT_NE(r.error, 0);
}

TEST(BinaryIntrinsicsTest, SkipStructFieldRejectsExcessiveDepth) {
  std::vector<uint8_t> data;
  for (int i = 0; i < 70; ++i) {
    data.push_back(wire::kBinaryStruct);
    data.push_back(0x00);
    data.push_back(0x01);
  }
  auto r = makeReader(data);

  thrift_transcode_binary_skip_field(&r, wire::kBinaryStruct);
  EXPECT_NE(r.error, 0);
}

} // namespace
