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

#include <cstdint>

#include <thrift/lib/cpp2/transcode/Cursor.h>

// Thrift Binary field framing intrinsics.

extern "C" {

// Read Binary field header: 1 byte TType + 2 bytes BE field ID.
// Returns T_STOP only for an actual STOP marker. Malformed or truncated input
// returns 0 with cursor->error latched.
// prevFieldId is unused (Binary has no delta encoding).
uint8_t thrift_transcode_binary_read_field_header(
    TranscodeCursor* cursor, int16_t* fieldId, int16_t prevFieldId);

// Write Binary field header: 1 byte TType + 2 bytes BE field ID.
void thrift_transcode_binary_write_field_header(
    TranscodeCursor* cursor,
    uint8_t ttype,
    int16_t fieldId,
    int16_t prevFieldId);

// Write Binary STOP marker (single 0x00 byte).
void thrift_transcode_binary_write_stop(TranscodeCursor* cursor);

// Skip a Binary field of the given TType.
void thrift_transcode_binary_skip_field(TranscodeCursor* cursor, uint8_t ttype);

} // extern "C"
