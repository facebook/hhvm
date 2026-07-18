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

#include <thrift/lib/cpp2/transcode/Cursor.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace apache::thrift::transcode {
namespace {

void writeByte(TranscodeCursor& cursor, uint8_t value) {
  thrift_transcode_cursor_ensure_write(&cursor, 1);
  if (cursor.error != 0) {
    return;
  }
  *cursor.writePos++ = value;
}

template <size_t N>
TranscodeMutableByteRange segment(std::array<uint8_t, N>& bytes, size_t size) {
  return TranscodeMutableByteRange{bytes.data(), bytes.data() + size};
}

struct SegmentOutput {
  std::array<std::array<uint8_t, 16>, 12> segments{};
  size_t nextSegment = 1;
  bool failExtend = false;
  bool failFlush = false;
  bool blockExtend = false;
  bool blockFlush = false;
  bool invalidExtend = false;
  bool invalidFlush = false;
};

struct MallocOutput {
  ~MallocOutput() { free(buffer); }

  uint8_t* buffer = nullptr;
  size_t capacity = 0;
};

TranscodeStatus relocateMalloc(
    const TranscodeExtendRequest* request,
    TranscodeExtendResult* result,
    void* userData) {
  auto& output = *static_cast<MallocOutput*>(userData);
  const size_t written =
      static_cast<size_t>(request->writePoint - request->segment.begin);
  const size_t requested = written + request->minWritable;
  const size_t newCapacity = std::max(output.capacity * 2, requested);
  auto* next = static_cast<uint8_t*>(malloc(newCapacity));
  if (next == nullptr) {
    return TranscodeStatus::Error;
  }
  memcpy(next, request->segment.begin, written);
  free(output.buffer);
  output.buffer = next;
  output.capacity = newCapacity;
  result->kind = TranscodeExtendKind::RelocatedContiguous;
  result->segment = {next, next + newCapacity};
  return TranscodeStatus::Ok;
}

TranscodeStatus nextSegment(
    const TranscodeExtendRequest* /*request*/,
    TranscodeExtendResult* result,
    void* userData) {
  auto& output = *static_cast<SegmentOutput*>(userData);
  if (output.failExtend) {
    return TranscodeStatus::Error;
  }
  if (output.blockExtend) {
    return TranscodeStatus::Blocked;
  }
  if (output.invalidExtend) {
    result->kind = TranscodeExtendKind::NewSegment;
    result->segment = {output.segments[0].data(), output.segments[0].data()};
    return TranscodeStatus::Ok;
  }
  if (output.nextSegment >= output.segments.size()) {
    return TranscodeStatus::Error;
  }
  auto& bytes = output.segments[output.nextSegment++];
  result->kind = TranscodeExtendKind::NewSegment;
  result->segment = {bytes.data(), bytes.data() + bytes.size()};
  return TranscodeStatus::Ok;
}

TranscodeStatus nextSingleByteSegment(
    const TranscodeExtendRequest* /*request*/,
    TranscodeExtendResult* result,
    void* userData) {
  auto& output = *static_cast<SegmentOutput*>(userData);
  if (output.nextSegment >= output.segments.size()) {
    return TranscodeStatus::Error;
  }
  auto& bytes = output.segments[output.nextSegment++];
  result->kind = TranscodeExtendKind::NewSegment;
  result->segment = {bytes.data(), bytes.data() + 1};
  return TranscodeStatus::Ok;
}

TranscodeStatus relocateContiguous(
    const TranscodeExtendRequest* request,
    TranscodeExtendResult* result,
    void* userData) {
  auto& output = *static_cast<SegmentOutput*>(userData);
  auto& bytes = output.segments[1];
  memcpy(
      bytes.data(),
      request->segment.begin,
      static_cast<size_t>(request->writePoint - request->segment.begin));
  result->kind = TranscodeExtendKind::RelocatedContiguous;
  result->segment = {bytes.data(), bytes.data() + bytes.size()};
  return TranscodeStatus::Ok;
}

TranscodeStatus reuseTailroomOnFlush(
    const TranscodeFlushRequest* request,
    TranscodeExtendResult* result,
    void* userData) {
  auto& output = *static_cast<SegmentOutput*>(userData);
  if (output.failFlush) {
    return TranscodeStatus::Error;
  }
  if (output.blockFlush) {
    return TranscodeStatus::Blocked;
  }
  if (output.invalidFlush) {
    result->kind = TranscodeExtendKind::NewSegment;
    result->segment = {output.segments[0].data(), output.segments[0].data()};
    return TranscodeStatus::Ok;
  }
  result->kind = TranscodeExtendKind::NewSegment;
  result->segment = {request->flushPoint, request->segment.end};
  return TranscodeStatus::Ok;
}

TranscodeStatus rotateOnFlush(
    const TranscodeFlushRequest* /*request*/,
    TranscodeExtendResult* result,
    void* userData) {
  auto& output = *static_cast<SegmentOutput*>(userData);
  if (output.failFlush) {
    return TranscodeStatus::Error;
  }
  auto& bytes = output.segments[1];
  result->kind = TranscodeExtendKind::NewSegment;
  result->segment = {bytes.data(), bytes.data() + bytes.size()};
  return TranscodeStatus::Ok;
}

TEST(CursorTest, ProviderRelocationKeepsPatchPointsValid) {
  MallocOutput output;
  output.capacity = 4;
  output.buffer = static_cast<uint8_t*>(malloc(output.capacity));
  ASSERT_NE(output.buffer, nullptr);

  TranscodeCursor cursor{};
  thrift_transcode_cursor_init(
      &cursor,
      {nullptr, nullptr},
      {output.buffer, output.buffer + output.capacity},
      relocateMalloc,
      nullptr,
      &output);

  auto mark = thrift_transcode_cursor_mark(&cursor);
  thrift_transcode_cursor_skip(&cursor, 4);
  for (uint8_t i = 0; i < 8; ++i) {
    writeByte(cursor, i);
  }

  thrift_transcode_cursor_patch_i32_be(&cursor, mark, 8);

  ASSERT_EQ(cursor.error, 0);
  EXPECT_EQ(thrift_transcode_cursor_bytes_written(&cursor), 12);
  EXPECT_EQ(cursor.writeBuf[0], 0);
  EXPECT_EQ(cursor.writeBuf[1], 0);
  EXPECT_EQ(cursor.writeBuf[2], 0);
  EXPECT_EQ(cursor.writeBuf[3], 8);
}

TEST(CursorTest, EnsureWriteRotatesSegments) {
  SegmentOutput output;
  TranscodeCursor cursor{};
  thrift_transcode_cursor_init(
      &cursor,
      {nullptr, nullptr},
      segment(output.segments[0], 2),
      nextSegment,
      nullptr,
      &output);

  writeByte(cursor, 1);
  writeByte(cursor, 2);
  writeByte(cursor, 3);

  ASSERT_EQ(cursor.error, 0);
  EXPECT_EQ(thrift_transcode_cursor_bytes_written(&cursor), 3);
  EXPECT_EQ(output.segments[0][0], 1);
  EXPECT_EQ(output.segments[0][1], 2);
  EXPECT_EQ(output.segments[1][0], 3);
}

TEST(CursorTest, UnmarkedSegmentRotationDoesNotConsumePatchRecords) {
  SegmentOutput output;
  TranscodeCursor cursor{};
  thrift_transcode_cursor_init(
      &cursor,
      {nullptr, nullptr},
      segment(output.segments[0], 1),
      nextSingleByteSegment,
      nullptr,
      &output);

  for (uint8_t i = 0; i < THRIFT_TRANSCODE_MAX_SEGMENT_RECORDS + 2; ++i) {
    writeByte(cursor, i);
  }

  EXPECT_EQ(cursor.error, 0);
  EXPECT_EQ(cursor.segmentCount, 1);
}

TEST(CursorTest, PatchPointSurvivesSegmentRotation) {
  SegmentOutput output;
  TranscodeCursor cursor{};
  thrift_transcode_cursor_init(
      &cursor,
      {nullptr, nullptr},
      segment(output.segments[0], 2),
      nextSegment,
      nullptr,
      &output);

  auto mark = thrift_transcode_cursor_mark(&cursor);
  thrift_transcode_cursor_skip(&cursor, 1);
  writeByte(cursor, 2);
  writeByte(cursor, 3);

  thrift_transcode_cursor_patch_byte(&cursor, mark, 1);

  ASSERT_EQ(cursor.error, 0);
  EXPECT_EQ(output.segments[0][0], 1);
  EXPECT_EQ(output.segments[0][1], 2);
  EXPECT_EQ(output.segments[1][0], 3);
}

TEST(CursorTest, PatchPointSurvivesRelocatedContiguousSegment) {
  SegmentOutput output;
  TranscodeCursor cursor{};
  thrift_transcode_cursor_init(
      &cursor,
      {nullptr, nullptr},
      segment(output.segments[0], 4),
      relocateContiguous,
      nullptr,
      &output);

  auto mark = thrift_transcode_cursor_mark(&cursor);
  thrift_transcode_cursor_skip(&cursor, 4);
  writeByte(cursor, 9);

  thrift_transcode_cursor_patch_i32_be(&cursor, mark, 1);

  ASSERT_EQ(cursor.error, 0);
  EXPECT_EQ(output.segments[1][0], 0);
  EXPECT_EQ(output.segments[1][1], 0);
  EXPECT_EQ(output.segments[1][2], 0);
  EXPECT_EQ(output.segments[1][3], 1);
  EXPECT_EQ(output.segments[1][4], 9);
}

TEST(CursorTest, FlushCanReuseTailroom) {
  SegmentOutput output;
  TranscodeCursor cursor{};
  thrift_transcode_cursor_init(
      &cursor,
      {nullptr, nullptr},
      segment(output.segments[0], 8),
      nextSegment,
      reuseTailroomOnFlush,
      &output);

  writeByte(cursor, 1);
  writeByte(cursor, 2);
  thrift_transcode_cursor_flush(&cursor, 2);
  writeByte(cursor, 3);

  ASSERT_EQ(cursor.error, 0);
  EXPECT_EQ(thrift_transcode_cursor_bytes_written(&cursor), 3);
  EXPECT_EQ(output.segments[0][0], 1);
  EXPECT_EQ(output.segments[0][1], 2);
  EXPECT_EQ(output.segments[0][2], 3);
}

TEST(CursorTest, FlushCanRotateSegments) {
  SegmentOutput output;
  TranscodeCursor cursor{};
  thrift_transcode_cursor_init(
      &cursor,
      {nullptr, nullptr},
      segment(output.segments[0], 8),
      nextSegment,
      rotateOnFlush,
      &output);

  writeByte(cursor, 1);
  writeByte(cursor, 2);
  thrift_transcode_cursor_flush(&cursor, 1);
  writeByte(cursor, 3);

  ASSERT_EQ(cursor.error, 0);
  EXPECT_EQ(thrift_transcode_cursor_bytes_written(&cursor), 3);
  EXPECT_EQ(output.segments[0][0], 1);
  EXPECT_EQ(output.segments[0][1], 2);
  EXPECT_EQ(output.segments[0][2], 0);
  EXPECT_EQ(output.segments[1][0], 3);
}

TEST(CursorTest, CallbackFailuresLatchError) {
  SegmentOutput output;
  output.failExtend = true;

  TranscodeCursor cursor{};
  thrift_transcode_cursor_init(
      &cursor,
      {nullptr, nullptr},
      segment(output.segments[0], 1),
      nextSegment,
      nullptr,
      &output);

  writeByte(cursor, 1);
  writeByte(cursor, 2);

  EXPECT_NE(cursor.error, 0);
  EXPECT_EQ(thrift_transcode_cursor_bytes_written(&cursor), 1);

  SegmentOutput flushOutput;
  flushOutput.failFlush = true;
  thrift_transcode_cursor_init(
      &cursor,
      {nullptr, nullptr},
      segment(flushOutput.segments[0], 8),
      nextSegment,
      reuseTailroomOnFlush,
      &flushOutput);

  writeByte(cursor, 1);
  thrift_transcode_cursor_flush(&cursor, 1);

  EXPECT_NE(cursor.error, 0);
}

TEST(CursorTest, InvalidCallbackSegmentsLatchError) {
  SegmentOutput output;
  output.invalidExtend = true;

  TranscodeCursor cursor{};
  thrift_transcode_cursor_init(
      &cursor,
      {nullptr, nullptr},
      segment(output.segments[0], 1),
      nextSegment,
      nullptr,
      &output);

  writeByte(cursor, 1);
  writeByte(cursor, 2);

  EXPECT_NE(cursor.error, 0);

  SegmentOutput flushOutput;
  flushOutput.invalidFlush = true;
  thrift_transcode_cursor_init(
      &cursor,
      {nullptr, nullptr},
      segment(flushOutput.segments[0], 8),
      nextSegment,
      reuseTailroomOnFlush,
      &flushOutput);

  writeByte(cursor, 1);
  thrift_transcode_cursor_flush(&cursor, 1);

  EXPECT_NE(cursor.error, 0);
}

TEST(CursorTest, BlockedExtendSticksUntilResume) {
  SegmentOutput output;
  output.blockExtend = true;

  TranscodeCursor cursor{};
  thrift_transcode_cursor_init(
      &cursor,
      {nullptr, nullptr},
      segment(output.segments[0], 1),
      nextSegment,
      nullptr,
      &output);

  writeByte(cursor, 1);
  writeByte(cursor, 2);

  EXPECT_EQ(thrift_transcode_cursor_status(&cursor), TranscodeStatus::Blocked);
  EXPECT_EQ(thrift_transcode_cursor_bytes_written(&cursor), 1);

  output.blockExtend = false;
  writeByte(cursor, 2);
  EXPECT_EQ(thrift_transcode_cursor_status(&cursor), TranscodeStatus::Blocked);
  EXPECT_EQ(thrift_transcode_cursor_bytes_written(&cursor), 1);

  EXPECT_EQ(thrift_transcode_cursor_resume(&cursor), TranscodeStatus::Ok);
  writeByte(cursor, 2);

  ASSERT_EQ(thrift_transcode_cursor_status(&cursor), TranscodeStatus::Ok);
  ASSERT_EQ(cursor.error, 0);
  EXPECT_EQ(thrift_transcode_cursor_bytes_written(&cursor), 2);
  EXPECT_EQ(output.segments[0][0], 1);
  EXPECT_EQ(output.segments[1][0], 2);
}

TEST(CursorTest, BlockedFlushSticksUntilResume) {
  SegmentOutput output;
  output.blockFlush = true;

  TranscodeCursor cursor{};
  thrift_transcode_cursor_init(
      &cursor,
      {nullptr, nullptr},
      segment(output.segments[0], 8),
      nextSegment,
      reuseTailroomOnFlush,
      &output);

  writeByte(cursor, 1);
  EXPECT_EQ(
      thrift_transcode_cursor_flush(&cursor, 1), TranscodeStatus::Blocked);

  EXPECT_EQ(thrift_transcode_cursor_status(&cursor), TranscodeStatus::Blocked);
  EXPECT_EQ(thrift_transcode_cursor_bytes_written(&cursor), 1);

  output.blockFlush = false;
  EXPECT_EQ(
      thrift_transcode_cursor_flush(&cursor, 1), TranscodeStatus::Blocked);

  EXPECT_EQ(thrift_transcode_cursor_resume(&cursor), TranscodeStatus::Ok);
  EXPECT_EQ(thrift_transcode_cursor_flush(&cursor, 1), TranscodeStatus::Ok);
  writeByte(cursor, 2);

  ASSERT_EQ(thrift_transcode_cursor_status(&cursor), TranscodeStatus::Ok);
  ASSERT_EQ(cursor.error, 0);
  EXPECT_EQ(thrift_transcode_cursor_bytes_written(&cursor), 2);
  EXPECT_EQ(output.segments[0][0], 1);
  EXPECT_EQ(output.segments[0][1], 2);
}

} // namespace
} // namespace apache::thrift::transcode
