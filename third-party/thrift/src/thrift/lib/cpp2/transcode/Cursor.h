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

#pragma once

#include <cstddef>
#include <cstdint>

// Segment records cover the active segment plus older segments that received a
// patch mark.
#define THRIFT_TRANSCODE_MAX_SEGMENT_RECORDS 8

// The transcode cursor and its buffer/memory management. JIT-compiled kernels
// and the interpreter share this state to track the current read/write
// positions across the Extract → Decode → Encode → Emit pipeline.

extern "C" {

// ─────────────────────────────────────────────────────────────────────────
// Cursor management
// ─────────────────────────────────────────────────────────────────────────

/**
 * An immutable contiguous range of bytes.
 */
struct TranscodeByteRange {
  // First byte in the range, or nullptr when the range is empty.
  const uint8_t* begin;
  // One past the last byte in the range.
  const uint8_t* end;
};

/**
 * A mutable contiguous range of bytes.
 */
struct TranscodeMutableByteRange {
  // First writable byte in the range.
  uint8_t* begin;
  // One past the last writable byte in the range.
  uint8_t* end;
};

/**
 * How the output segment was extended.
 * Used to determine the next write position.
 */
enum class TranscodeExtendKind : uint8_t {
  // The active segment kept the same begin pointer and gained more tailroom.
  InPlaceExtension,
  // Previously written bytes moved to a new contiguous allocation.
  RelocatedContiguous,
  // Output continues in a different segment; older segments stay patchable
  // until the cursor flushes past them.
  NewSegment,
};

/**
 * Request to extend the current output segment by at least `minWritable` bytes.
 * The callback may return a new segment or extend the current one in-place, see
 * `TranscodeExtendKind`.
 */
struct TranscodeExtendRequest {
  // Active output segment before the callback.
  TranscodeMutableByteRange segment;
  // First unwritten byte in `segment`.
  uint8_t* writePoint;
  // Required writable bytes at `writePoint` after the callback returns.
  size_t minWritable;
};

struct TranscodeFlushRequest {
  // Active output segment before the callback.
  TranscodeMutableByteRange segment;
  // Bytes before this point are committed and no longer patchable.
  uint8_t* flushPoint;
  // Required writable bytes after the callback returns.
  size_t minWritable;
};

struct TranscodeExtendResult {
  // The type of extension that occurred.
  TranscodeExtendKind kind;
  // InPlaceExtension keeps the request segment begin and resumes at the
  // request write point. RelocatedContiguous copies bytes before the request
  // write point to `segment.begin`. NewSegment copies no bytes and resumes at
  // `segment.begin`.
  TranscodeMutableByteRange segment;
};

/**
 * Called when the active segment lacks `request->minWritable` bytes at
 * `request->writePoint`.
 * Returning false latches an output error. Returning true requires a
 * result segment with enough tailroom at the resumed write point.
 * Every unflushed segment returned by the provider must stay alive and
 * mutable because open patch points are resolved by segment id and offset.
 */
using TranscodeExtendFn = bool (*)(
    const TranscodeExtendRequest* request,
    TranscodeExtendResult* result,
    void* userData);
/**
 * Called by `thrift_transcode_cursor_flush` after the cursor advances the
 * flushed watermark to `request->flushPoint`.
 * Returning false latches an output error.
 * Returning NewSegment with `{request->flushPoint, request->segment.end}`
 * reuses the old segment tailroom as a logical next segment.
 */
using TranscodeFlushFn = bool (*)(
    const TranscodeFlushRequest* request,
    TranscodeExtendResult* result,
    void* userData);

struct TranscodePatchPoint {
  // Absolute stream offset. Used for byte counts and flush validation.
  size_t streamOffset;
  // Offset inside `segmentId`; this keeps patching valid after relocation.
  size_t offsetInSegment;
  uint32_t segmentId;
  // Address at mark time. Segment records are authoritative after relocation.
  uint8_t* ptr;
};

struct TranscodeSegmentRecord {
  // Monotonic id stored in TranscodePatchPoint.
  uint32_t id;
  // Absolute stream offset. Used for byte counts and flush validation.
  size_t streamOffset;
  // Mutable segment retained because it is active or contains open patch
  // points.
  TranscodeMutableByteRange segment;
  // Unmarked records can be dropped when the segment stops being active.
  bool hasPatchPoint;
};

struct TranscodeCursor {
  // Next unread input byte and end of the input buffer.
  const uint8_t* readPos;
  const uint8_t* readEnd;
  // Active output segment and next write point within it.
  uint8_t* writeSegmentBegin;
  uint8_t* writePos;
  uint8_t* writeEnd;
  // Active-segment aliases kept for existing intrinsics and generated code.
  uint8_t* writeBuf;
  size_t writeCapacity;
  // Logical stream accounting for non-contiguous output.
  size_t totalBytesWrittenBeforeSegment;
  size_t flushedBytes;
  uint32_t currentSegmentId;
  // Active segment plus older segments with open patch points.
  size_t segmentCount;
  TranscodeSegmentRecord segmentRecords[THRIFT_TRANSCODE_MAX_SEGMENT_RECORDS];
  int64_t error; // 0 = ok, non-zero = error code (i64 for alignment)

  // Output-provider hooks. `extendFn` is required once tailroom is exhausted;
  // `flushFn` is optional when the provider does not need safe watermarks.
  TranscodeExtendFn extendFn;
  TranscodeFlushFn flushFn;
  void* userData;
};

// The output provider owns segment allocation and lifetime. The cursor keeps
// only enough metadata to preserve stream offsets and validate patch points.
void thrift_transcode_cursor_init(
    TranscodeCursor* cursor,
    TranscodeByteRange input,
    TranscodeMutableByteRange output,
    TranscodeExtendFn extendFn,
    TranscodeFlushFn flushFn,
    void* userData);

// Ensure at least `needed` bytes available in write buffer.
// May extend, relocate, or rotate the active segment.
void thrift_transcode_cursor_ensure_write(
    TranscodeCursor* cursor, size_t needed);

void thrift_transcode_cursor_flush(TranscodeCursor* cursor, size_t minWritable);

// Returns the number of bytes written so far.
size_t thrift_transcode_cursor_bytes_written(const TranscodeCursor* cursor);

// ─────────────────────────────────────────────────────────────────────────
// Cursor mark/patch (for deferred header writes)
// ─────────────────────────────────────────────────────────────────────────
//
// Used when a protocol needs information at the end of a section to write
// at the beginning (e.g., element count for Compact list header, byte
// length for Protobuf length-delimited blocks).
//
// Save the current write position.
TranscodePatchPoint thrift_transcode_cursor_mark(TranscodeCursor* cursor);

TranscodePatchPoint thrift_transcode_cursor_offset_patch_point(
    TranscodePatchPoint mark, size_t offset);

size_t thrift_transcode_cursor_bytes_since_mark(
    const TranscodeCursor* cursor, TranscodePatchPoint mark);

// Skip N bytes at the current write position (reserves space for later patch).
void thrift_transcode_cursor_skip(TranscodeCursor* cursor, size_t n);

// Patch a single byte at a previously marked offset.
void thrift_transcode_cursor_patch_byte(
    TranscodeCursor* cursor, TranscodePatchPoint mark, uint8_t value);

// Patch a varint at a previously marked offset, into a fixed number of
// reserved bytes (reservedBytes). The value is spread across all reservedBytes
// (every byte but the last carries a continuation bit) so the field always
// occupies exactly reservedBytes — an intentionally non-canonical, fixed-width
// varint. Returns false if the value doesn't fit in reservedBytes.
bool thrift_transcode_cursor_patch_varint(
    TranscodeCursor* cursor,
    TranscodePatchPoint mark,
    uint64_t value,
    size_t reservedBytes);

// Patch a big-endian i32 at a previously marked offset.
void thrift_transcode_cursor_patch_i32_be(
    TranscodeCursor* cursor, TranscodePatchPoint mark, int32_t value);

} // extern "C"

namespace apache::thrift::transcode::detail {

// Latch the first error onto the cursor; once set, every read/write becomes a
// no-op so failures propagate without per-call branching in the codegen.
inline void setError(TranscodeCursor* cursor, int64_t code) {
  if (cursor->error == 0) {
    cursor->error = code;
  }
}

// Are there at least `n` readable bytes remaining?
inline bool canRead(TranscodeCursor* cursor, size_t n) {
  if (cursor->error != 0) {
    return false;
  }
  // Underflow-safe: an earlier unchecked advance may have pushed readPos past
  // readEnd; never compute (readEnd - readPos) as size_t in that case, or it
  // wraps to ~SIZE_MAX and silently disarms every subsequent bounds check.
  if (cursor->readPos > cursor->readEnd ||
      static_cast<size_t>(cursor->readEnd - cursor->readPos) < n) {
    setError(cursor, 1); // read overrun
    return false;
  }
  return true;
}

} // namespace apache::thrift::transcode::detail

namespace apache::thrift::transcode {

inline void setCursorError(TranscodeCursor* cursor, int64_t code) {
  detail::setError(cursor, code);
}

inline bool cursorCanRead(TranscodeCursor* cursor, size_t n) {
  return detail::canRead(cursor, n);
}

} // namespace apache::thrift::transcode
