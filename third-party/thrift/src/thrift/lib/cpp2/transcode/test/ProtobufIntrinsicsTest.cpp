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

#include <thrift/lib/cpp2/transcode/ProtobufIntrinsics.h>

#include <cstdint>
#include <vector>

#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/transcode/Intrinsics.h>

namespace {

// The uniform field-header signature offsets the protobuf wire type by 1 so 0
// stays reserved for end-of-message. These are the "type info" bytes callers
// pass and readers return.
constexpr uint8_t kVarint = 1; // wire type 0
constexpr uint8_t k64Bit = 2; // wire type 1
constexpr uint8_t kLenDelim = 3; // wire type 2
constexpr uint8_t k32Bit = 6; // wire type 5

// Feed a cursor the bytes a writer just produced, so a read intrinsic decodes
// exactly what the matching write intrinsic encoded.
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

// A protobuf tag round-trips the field number and the (offset) wire type
// through the varint (field_number << 3 | wire_type) encoding.
TEST(ProtobufIntrinsicsTest, FieldHeaderRoundTrip) {
  for (auto [fieldId, typeInfo] : std::vector<std::pair<int16_t, uint8_t>>{
           {1, kVarint}, {1000, kLenDelim}, {5, k32Bit}, {2, k64Bit}}) {
    RoundTrip rt;
    auto w = rt.writer();
    thrift_transcode_proto_write_field_header(&w, typeInfo, fieldId, 0);
    ASSERT_EQ(w.error, 0);

    auto r = rt.reader(w);
    int16_t got = 0;
    EXPECT_EQ(thrift_transcode_proto_read_field_header(&r, &got, 0), typeInfo);
    EXPECT_EQ(got, fieldId);
  }
}

// There is no stop marker; a reader at end-of-input returns 0 with a zeroed
// field id.
TEST(ProtobufIntrinsicsTest, EndOfMessageReturnsZero) {
  RoundTrip rt;
  auto w = rt.writer(); // nothing written
  auto r = rt.reader(w);
  int16_t fieldId = -1;
  EXPECT_EQ(thrift_transcode_proto_read_field_header(&r, &fieldId, 0), 0);
  EXPECT_EQ(fieldId, 0);
}

TEST(ProtobufIntrinsicsTest, ReadFieldHeaderRejectsMalformedTag) {
  for (const auto& data : std::vector<std::vector<uint8_t>>{
           {0x80}, // truncated varint
           {0x00}, // field number 0
           {0x01}, // field number 0, non-zero wire type
           {0x0B}, // unsupported SGROUP wire type
       }) {
    auto r = makeReader(data);
    int16_t fieldId = -1;

    EXPECT_EQ(thrift_transcode_proto_read_field_header(&r, &fieldId, 0), 0);
    EXPECT_EQ(fieldId, 0);
    EXPECT_NE(r.error, 0);
  }
}

TEST(ProtobufIntrinsicsTest, ReadFieldHeaderRejectsFieldIdOutOfRange) {
  RoundTrip rt;
  auto w = rt.writer();
  thrift_transcode_write_unsigned_varint(
      &w, (static_cast<uint64_t>(32768) << 3) | 0);
  ASSERT_EQ(w.error, 0);

  auto r = rt.reader(w);
  int16_t fieldId = -1;
  EXPECT_EQ(thrift_transcode_proto_read_field_header(&r, &fieldId, 0), 0);
  EXPECT_EQ(fieldId, 0);
  EXPECT_NE(r.error, 0);
}

// write_stop is a no-op: protobuf messages have no stop marker, so it emits
// nothing.
TEST(ProtobufIntrinsicsTest, WriteStopEmitsNothing) {
  RoundTrip rt;
  auto w = rt.writer();
  thrift_transcode_proto_write_stop(&w);
  EXPECT_EQ(thrift_transcode_cursor_bytes_written(&w), 0u);
}

TEST(ProtobufIntrinsicsTest, WriteFieldHeaderRejectsMalformedTag) {
  for (auto [fieldId, typeInfo] : std::vector<std::pair<int16_t, uint8_t>>{
           {0, kVarint}, {-1, kVarint}, {1, 0}, {1, 4}}) {
    RoundTrip rt;
    auto w = rt.writer();

    thrift_transcode_proto_write_field_header(&w, typeInfo, fieldId, 0);

    EXPECT_NE(w.error, 0);
    EXPECT_EQ(thrift_transcode_cursor_bytes_written(&w), 0u);
  }
}

// skip_field steps over each wire-type payload, leaving the reader on the byte
// that follows.
TEST(ProtobufIntrinsicsTest, SkipFixedWidthRejectsTruncatedInput) {
  for (uint8_t typeInfo : {k32Bit, k64Bit}) {
    const std::vector<uint8_t> data = {0x01};
    auto r = makeReader(data);

    thrift_transcode_proto_skip_field(&r, typeInfo);

    EXPECT_NE(r.error, 0);
    EXPECT_LE(r.readPos, r.readEnd);
  }
}

TEST(ProtobufIntrinsicsTest, SkipLengthDelimitedRejectsTruncatedPayload) {
  const std::vector<uint8_t> data = {0x03, 'h', 'i'};
  auto r = makeReader(data);

  thrift_transcode_proto_skip_field(&r, kLenDelim);

  EXPECT_NE(r.error, 0);
  EXPECT_LE(r.readPos, r.readEnd);
}

TEST(ProtobufIntrinsicsTest, SkipRejectsUnsupportedWireType) {
  for (uint8_t typeInfo : {0, 4, 5, 7, 8}) {
    const std::vector<uint8_t> data = {0x7F};
    auto r = makeReader(data);

    thrift_transcode_proto_skip_field(&r, typeInfo);

    EXPECT_NE(r.error, 0);
    EXPECT_EQ(r.readPos, data.data());
  }
}

TEST(ProtobufIntrinsicsTest, SkipByWireType) {
  {
    RoundTrip rt;
    auto w = rt.writer();
    thrift_transcode_write_unsigned_varint(&w, 300); // varint payload
    thrift_transcode_write_byte_checked(&w, 0x7F);
    auto r = rt.reader(w);
    thrift_transcode_proto_skip_field(&r, kVarint);
    ASSERT_EQ(r.error, 0);
    EXPECT_EQ(thrift_transcode_read_byte_checked(&r), 0x7F);
  }
  {
    const std::vector<uint8_t> payload = {'h', 'i', '!'};
    RoundTrip rt;
    auto w = rt.writer();
    thrift_transcode_write_varint_prefixed(&w, payload.data(), payload.size());
    thrift_transcode_write_byte_checked(&w, 0x7F);
    auto r = rt.reader(w);
    thrift_transcode_proto_skip_field(&r, kLenDelim);
    ASSERT_EQ(r.error, 0);
    EXPECT_EQ(thrift_transcode_read_byte_checked(&r), 0x7F);
  }
  {
    RoundTrip rt;
    auto w = rt.writer();
    thrift_transcode_write_fixed32_le_checked(&w, 0x12345678u);
    thrift_transcode_write_byte_checked(&w, 0x7F);
    auto r = rt.reader(w);
    thrift_transcode_proto_skip_field(&r, k32Bit);
    ASSERT_EQ(r.error, 0);
    EXPECT_EQ(thrift_transcode_read_byte_checked(&r), 0x7F);
  }
  {
    RoundTrip rt;
    auto w = rt.writer();
    thrift_transcode_write_fixed64_le_checked(&w, 0x1122334455667788ull);
    thrift_transcode_write_byte_checked(&w, 0x7F);
    auto r = rt.reader(w);
    thrift_transcode_proto_skip_field(&r, k64Bit);
    ASSERT_EQ(r.error, 0);
    EXPECT_EQ(thrift_transcode_read_byte_checked(&r), 0x7F);
  }
}

} // namespace
