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

#include <folly/CppAttributes.h>
#include <folly/lang/Bits.h>

#include <array>
#include <cassert>
#include <cstring>

using apache::thrift::transcode::setCursorError;

namespace {

constexpr uint32_t kInitialSegmentId = 1;
constexpr int64_t kOutputError = 2;
constexpr int64_t kInvalidSegment = 13;
constexpr int64_t kSegmentStateError = 14;
constexpr int64_t kSegmentRecordLimit = 15;
constexpr int64_t kInvalidPatchPoint = 20;
constexpr int64_t kInvalidVarintPatch = 21;
constexpr int64_t kVarintPatchOverflow = 22;
constexpr int64_t kPatchBeforeFlush = 23;

size_t rangeSize(TranscodeMutableByteRange range) {
  return static_cast<size_t>(range.end - range.begin);
}

bool validRange(TranscodeMutableByteRange range, size_t minWritable) {
  return range.begin != nullptr && range.end != nullptr &&
      range.end >= range.begin && rangeSize(range) >= minWritable;
}

size_t bytesWrittenInSegment(const TranscodeCursor* cursor) {
  return static_cast<size_t>(cursor->writePos - cursor->writeSegmentBegin);
}

TranscodeSegmentRecord* FOLLY_NULLABLE
currentSegmentRecord(TranscodeCursor* cursor) {
  if (cursor->segmentCount == 0) {
    return nullptr;
  }
  auto* record = &cursor->segmentRecords[cursor->segmentCount - 1];
  if (record->id != cursor->currentSegmentId) {
    setCursorError(cursor, kSegmentStateError);
    return nullptr;
  }
  return record;
}

const TranscodeSegmentRecord* FOLLY_NULLABLE
findSegmentRecord(const TranscodeCursor* cursor, uint32_t id) {
  for (size_t i = 0; i < cursor->segmentCount; ++i) {
    if (cursor->segmentRecords[i].id == id) {
      return &cursor->segmentRecords[i];
    }
  }
  return nullptr;
}

void updateCurrentSegmentRecord(
    TranscodeCursor* cursor, uint8_t* begin, uint8_t* end) {
  auto* record = currentSegmentRecord(cursor);
  if (record == nullptr) {
    return;
  }
  record->streamOffset = cursor->totalBytesWrittenBeforeSegment;
  record->segment = {begin, end};
}

bool segmentFullyFlushed(
    const TranscodeSegmentRecord& record, size_t flushedBytes) {
  return record.streamOffset <= flushedBytes &&
      flushedBytes - record.streamOffset >= rangeSize(record.segment);
}

void dropInactiveSegmentRecords(
    TranscodeCursor* cursor, uint32_t currentSegmentId) {
  size_t writeIndex = 0;
  for (size_t readIndex = 0; readIndex < cursor->segmentCount; ++readIndex) {
    const auto& record = cursor->segmentRecords[readIndex];
    if (record.id != currentSegmentId &&
        (!record.hasPatchPoint ||
         segmentFullyFlushed(record, cursor->flushedBytes))) {
      continue;
    }
    if (writeIndex != readIndex) {
      cursor->segmentRecords[writeIndex] = record;
    }
    ++writeIndex;
  }
  cursor->segmentCount = writeIndex;
}

void dropInactiveSegmentRecords(TranscodeCursor* cursor) {
  dropInactiveSegmentRecords(cursor, cursor->currentSegmentId);
}

bool appendSegmentRecord(
    TranscodeCursor* cursor, uint8_t* begin, uint8_t* end) {
  dropInactiveSegmentRecords(cursor);
  if (cursor->segmentCount == THRIFT_TRANSCODE_MAX_SEGMENT_RECORDS) {
    setCursorError(cursor, kSegmentRecordLimit);
    return false;
  }
  cursor->segmentRecords[cursor->segmentCount++] = TranscodeSegmentRecord{
      cursor->currentSegmentId,
      cursor->totalBytesWrittenBeforeSegment,
      {begin, end},
      false,
  };
  return true;
}

void setActiveSegment(
    TranscodeCursor* cursor, uint8_t* begin, uint8_t* pos, uint8_t* end) {
  cursor->writeSegmentBegin = begin;
  cursor->writeBuf = begin;
  cursor->writePos = pos;
  cursor->writeEnd = end;
  cursor->writeCapacity = begin != nullptr && end != nullptr && end >= begin
      ? static_cast<size_t>(end - begin)
      : 0;
  if (cursor->segmentCount != 0) {
    updateCurrentSegmentRecord(cursor, begin, end);
  }
}

bool applyExtendResult(
    TranscodeCursor* cursor,
    const TranscodeExtendResult& result,
    size_t needed) {
  const size_t writtenInSegment = bytesWrittenInSegment(cursor);
  switch (result.kind) {
    case TranscodeExtendKind::InPlaceExtension: {
      if (result.segment.begin != cursor->writeSegmentBegin ||
          result.segment.end < cursor->writePos ||
          static_cast<size_t>(result.segment.end - cursor->writePos) < needed) {
        setCursorError(cursor, kInvalidSegment);
        return false;
      }
      setActiveSegment(
          cursor, result.segment.begin, cursor->writePos, result.segment.end);
      return true;
    }
    case TranscodeExtendKind::RelocatedContiguous: {
      if (!validRange(result.segment, writtenInSegment + needed)) {
        setCursorError(cursor, kInvalidSegment);
        return false;
      }
      setActiveSegment(
          cursor,
          result.segment.begin,
          result.segment.begin + writtenInSegment,
          result.segment.end);
      return true;
    }
    case TranscodeExtendKind::NewSegment: {
      if (!validRange(result.segment, needed)) {
        setCursorError(cursor, kInvalidSegment);
        return false;
      }
      const uint32_t nextSegmentId = cursor->currentSegmentId + 1;
      dropInactiveSegmentRecords(cursor, nextSegmentId);
      if (cursor->segmentCount == THRIFT_TRANSCODE_MAX_SEGMENT_RECORDS) {
        setCursorError(cursor, kSegmentRecordLimit);
        return false;
      }
      cursor->totalBytesWrittenBeforeSegment += writtenInSegment;
      cursor->currentSegmentId = nextSegmentId;
      cursor->segmentRecords[cursor->segmentCount++] = TranscodeSegmentRecord{
          cursor->currentSegmentId,
          cursor->totalBytesWrittenBeforeSegment,
          result.segment,
          false,
      };
      setActiveSegment(
          cursor,
          result.segment.begin,
          result.segment.begin,
          result.segment.end);
      return true;
    }
  }
  setCursorError(cursor, kInvalidSegment);
  return false;
}

uint8_t* FOLLY_NULLABLE resolvePatchPoint(
    TranscodeCursor* cursor, TranscodePatchPoint mark, size_t size) {
  if (cursor->error != 0) {
    return nullptr;
  }
  assert(
      mark.streamOffset >= cursor->flushedBytes &&
      "patch point is before the flushed watermark");
  if (mark.streamOffset < cursor->flushedBytes) {
    setCursorError(cursor, kPatchBeforeFlush);
    return nullptr;
  }
  if (mark.ptr == nullptr || size == 0) {
    setCursorError(cursor, kInvalidPatchPoint);
    return nullptr;
  }
  auto* record = findSegmentRecord(cursor, mark.segmentId);
  if (record == nullptr) {
    setCursorError(cursor, kInvalidPatchPoint);
    return nullptr;
  }
  const size_t segmentLength = rangeSize(record->segment);
  if (mark.offsetInSegment > segmentLength ||
      segmentLength - mark.offsetInSegment < size ||
      record->streamOffset + mark.offsetInSegment != mark.streamOffset) {
    setCursorError(cursor, kInvalidPatchPoint);
    return nullptr;
  }
  return record->segment.begin + mark.offsetInSegment;
}

} // namespace

extern "C" {

// ─────────────────────────────────────────────────────────────────────────
// Cursor management
// ─────────────────────────────────────────────────────────────────────────

void thrift_transcode_cursor_init(
    TranscodeCursor* cursor,
    TranscodeByteRange input,
    TranscodeMutableByteRange output,
    TranscodeExtendFn extendFn,
    TranscodeFlushFn flushFn,
    void* userData) {
  cursor->readPos = input.begin;
  cursor->readEnd = input.end;
  cursor->totalBytesWrittenBeforeSegment = 0;
  cursor->flushedBytes = 0;
  cursor->currentSegmentId = kInitialSegmentId;
  cursor->segmentCount = 0;
  if (validRange(output, 0)) {
    setActiveSegment(cursor, output.begin, output.begin, output.end);
    cursor->error = 0;
  } else {
    setActiveSegment(cursor, nullptr, nullptr, nullptr);
    cursor->error = kInvalidSegment;
  }
  if (cursor->error == 0) {
    appendSegmentRecord(cursor, output.begin, output.end);
  }
  cursor->extendFn = extendFn;
  cursor->flushFn = flushFn;
  cursor->userData = userData;
}

void thrift_transcode_cursor_ensure_write(
    TranscodeCursor* cursor, size_t needed) {
  if (cursor->error != 0) {
    return;
  }
  size_t available = static_cast<size_t>(cursor->writeEnd - cursor->writePos);
  if (available >= needed) {
    return;
  }
  if (cursor->extendFn) {
    TranscodeExtendRequest request{
        {cursor->writeSegmentBegin, cursor->writeEnd},
        cursor->writePos,
        needed,
    };
    TranscodeExtendResult result{};
    if (!cursor->extendFn(&request, &result, cursor->userData)) {
      setCursorError(cursor, kOutputError);
      return;
    }
    applyExtendResult(cursor, result, needed);
    return;
  }
  setCursorError(cursor, kOutputError);
}

void thrift_transcode_cursor_flush(
    TranscodeCursor* cursor, size_t minWritable) {
  if (cursor->error != 0) {
    return;
  }
  const size_t flushOffset = thrift_transcode_cursor_bytes_written(cursor);
  assert(flushOffset >= cursor->flushedBytes);
  cursor->flushedBytes = flushOffset;
  dropInactiveSegmentRecords(cursor);
  size_t available = static_cast<size_t>(cursor->writeEnd - cursor->writePos);
  if (!cursor->flushFn) {
    if (available < minWritable) {
      thrift_transcode_cursor_ensure_write(cursor, minWritable);
    }
    return;
  }
  TranscodeFlushRequest request{
      {cursor->writeSegmentBegin, cursor->writeEnd},
      cursor->writePos,
      minWritable,
  };
  TranscodeExtendResult result{};
  if (!cursor->flushFn(&request, &result, cursor->userData)) {
    setCursorError(cursor, kOutputError);
    return;
  }
  applyExtendResult(cursor, result, minWritable);
}

size_t thrift_transcode_cursor_bytes_written(const TranscodeCursor* cursor) {
  return cursor->totalBytesWrittenBeforeSegment + bytesWrittenInSegment(cursor);
}

// ─────────────────────────────────────────────────────────────────────────
// Cursor mark/patch
// ─────────────────────────────────────────────────────────────────────────

TranscodePatchPoint thrift_transcode_cursor_mark(TranscodeCursor* cursor) {
  if (cursor->error) {
    return {};
  }
  const size_t offsetInSegment = bytesWrittenInSegment(cursor);
  auto* record = currentSegmentRecord(cursor);
  if (record == nullptr) {
    return {};
  }
  record->hasPatchPoint = true;
  return TranscodePatchPoint{
      cursor->totalBytesWrittenBeforeSegment + offsetInSegment,
      offsetInSegment,
      cursor->currentSegmentId,
      cursor->writePos,
  };
}

TranscodePatchPoint thrift_transcode_cursor_offset_patch_point(
    TranscodePatchPoint mark, size_t offset) {
  mark.streamOffset += offset;
  mark.offsetInSegment += offset;
  if (mark.ptr != nullptr) {
    mark.ptr += offset;
  }
  return mark;
}

size_t thrift_transcode_cursor_bytes_since_mark(
    const TranscodeCursor* cursor, TranscodePatchPoint mark) {
  return thrift_transcode_cursor_bytes_written(cursor) - mark.streamOffset;
}

void thrift_transcode_cursor_skip(TranscodeCursor* cursor, size_t n) {
  if (cursor->error) {
    return;
  }
  thrift_transcode_cursor_ensure_write(cursor, n);
  if (cursor->error) {
    return;
  }
  // Zero-fill the reserved space
  memset(cursor->writePos, 0, n);
  cursor->writePos += n;
}

void thrift_transcode_cursor_patch_byte(
    TranscodeCursor* cursor, TranscodePatchPoint mark, uint8_t value) {
  auto* p = resolvePatchPoint(cursor, mark, 1);
  if (p == nullptr) {
    return;
  }
  *p = value;
}

bool thrift_transcode_cursor_patch_varint(
    TranscodeCursor* cursor,
    TranscodePatchPoint mark,
    uint64_t value,
    size_t reservedBytes) {
  if (cursor->error) {
    return false;
  }
  if (reservedBytes == 0 || reservedBytes > 10) {
    setCursorError(cursor, kInvalidVarintPatch);
    return false;
  }
  auto* p = resolvePatchPoint(cursor, mark, reservedBytes);
  if (p == nullptr) {
    return false;
  }
  std::array<uint8_t, 10> bytes{};
  for (size_t i = 0; i < reservedBytes - 1; ++i) {
    bytes[i] = static_cast<uint8_t>(value & 0x7F) | 0x80;
    value >>= 7;
  }
  if (value > 0x7F) {
    setCursorError(cursor, kVarintPatchOverflow);
    return false;
  }
  bytes[reservedBytes - 1] = static_cast<uint8_t>(value & 0x7F);
  memcpy(p, bytes.data(), reservedBytes);
  return true;
}

void thrift_transcode_cursor_patch_i32_be(
    TranscodeCursor* cursor, TranscodePatchPoint mark, int32_t value) {
  auto* p = resolvePatchPoint(cursor, mark, 4);
  if (p == nullptr) {
    return;
  }
  auto beVal = folly::Endian::big(static_cast<uint32_t>(value));
  memcpy(p, &beVal, 4);
}

} // extern "C"
