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

#include <thrift/lib/cpp2/transcode/Cursor.h>

// Shared byte intrinsics used by protocol-specific framing code.
//
// `_checked` variants are safe standalone entry points: they validate the
// bytes or tailroom needed for that call and may latch `cursor->error`.
// `_unchecked` variants are for prevalidated paths: callers must already know
// the active buffer has enough contiguous space, and no cursor error may be
// pending. They never check bounds, tailroom, or call the output provider.
// After any checked call latches an error, the execution engine must stop at
// its next dispatch boundary.

extern "C" {

// Read an unsigned Compact/Protobuf varint.
uint64_t thrift_transcode_read_unsigned_varint(TranscodeCursor* cursor);

// Write an unsigned Compact/Protobuf varint.
void thrift_transcode_write_unsigned_varint(
    TranscodeCursor* cursor, uint64_t value);

// Read a Compact zigzag varint.
int64_t thrift_transcode_read_zigzag_varint(TranscodeCursor* cursor);

// Write a Compact zigzag varint.
void thrift_transcode_write_zigzag_varint(
    TranscodeCursor* cursor, int64_t value);

// Read one byte used by Binary and Compact framing.
uint8_t thrift_transcode_read_byte_unchecked(TranscodeCursor* cursor);
uint8_t thrift_transcode_read_byte_checked(TranscodeCursor* cursor);

// Write one byte used by Binary and Compact framing.
void thrift_transcode_write_byte_unchecked(
    TranscodeCursor* cursor, uint8_t value);
void thrift_transcode_write_byte_checked(
    TranscodeCursor* cursor, uint8_t value);

// Read a little-endian 32-bit fixed-width value used by Protobuf.
uint32_t thrift_transcode_read_fixed32_le_unchecked(TranscodeCursor* cursor);
uint32_t thrift_transcode_read_fixed32_le_checked(TranscodeCursor* cursor);

// Read a little-endian 64-bit fixed-width value used by Protobuf.
uint64_t thrift_transcode_read_fixed64_le_unchecked(TranscodeCursor* cursor);
uint64_t thrift_transcode_read_fixed64_le_checked(TranscodeCursor* cursor);

// Write a little-endian 32-bit fixed-width value used by Protobuf.
void thrift_transcode_write_fixed32_le_unchecked(
    TranscodeCursor* cursor, uint32_t value);
void thrift_transcode_write_fixed32_le_checked(
    TranscodeCursor* cursor, uint32_t value);

// Write a little-endian 64-bit fixed-width value used by Protobuf.
void thrift_transcode_write_fixed64_le_unchecked(
    TranscodeCursor* cursor, uint64_t value);
void thrift_transcode_write_fixed64_le_checked(
    TranscodeCursor* cursor, uint64_t value);

// Read a big-endian 16-bit fixed-width value used by Binary and Compact.
uint16_t thrift_transcode_read_fixed16_be_unchecked(TranscodeCursor* cursor);
uint16_t thrift_transcode_read_fixed16_be_checked(TranscodeCursor* cursor);

// Read a big-endian 32-bit fixed-width value used by Binary and Compact.
uint32_t thrift_transcode_read_fixed32_be_unchecked(TranscodeCursor* cursor);
uint32_t thrift_transcode_read_fixed32_be_checked(TranscodeCursor* cursor);

// Read a big-endian 64-bit fixed-width value used by Binary and Compact.
uint64_t thrift_transcode_read_fixed64_be_unchecked(TranscodeCursor* cursor);
uint64_t thrift_transcode_read_fixed64_be_checked(TranscodeCursor* cursor);

// Write a big-endian 16-bit fixed-width value used by Binary and Compact.
void thrift_transcode_write_fixed16_be_unchecked(
    TranscodeCursor* cursor, uint16_t value);
void thrift_transcode_write_fixed16_be_checked(
    TranscodeCursor* cursor, uint16_t value);

// Write a big-endian 32-bit fixed-width value used by Binary and Compact.
void thrift_transcode_write_fixed32_be_unchecked(
    TranscodeCursor* cursor, uint32_t value);
void thrift_transcode_write_fixed32_be_checked(
    TranscodeCursor* cursor, uint32_t value);

// Write a big-endian 64-bit fixed-width value used by Binary and Compact.
void thrift_transcode_write_fixed64_be_unchecked(
    TranscodeCursor* cursor, uint64_t value);
void thrift_transcode_write_fixed64_be_checked(
    TranscodeCursor* cursor, uint64_t value);

// Read a Compact/Protobuf varint-prefixed byte range. `len` is a size_t
// because it describes bytes addressable in the current process.
const uint8_t* thrift_transcode_read_varint_prefixed(
    TranscodeCursor* cursor, size_t* len);

// Write a Compact/Protobuf varint-prefixed byte range. `len` is checked before
// it is encoded as the uint64_t wire prefix.
void thrift_transcode_write_varint_prefixed(
    TranscodeCursor* cursor, const uint8_t* data, size_t len);

// Read a Thrift Binary i32-prefixed byte range. Negative wire lengths fail.
const uint8_t* thrift_transcode_read_i32_prefixed(
    TranscodeCursor* cursor, size_t* len);

// Write a Thrift Binary i32-prefixed byte range. Lengths above int32_t fail.
void thrift_transcode_write_i32_prefixed(
    TranscodeCursor* cursor, const uint8_t* data, size_t len);

// Copy bytes that are already framed by the caller.
void thrift_transcode_write_raw_bytes_unchecked(
    TranscodeCursor* cursor, const uint8_t* data, size_t len);
void thrift_transcode_write_raw_bytes_checked(
    TranscodeCursor* cursor, const uint8_t* data, size_t len);

} // extern "C"
