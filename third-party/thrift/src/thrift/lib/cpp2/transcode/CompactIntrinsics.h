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

// Thrift Compact field framing intrinsics.

extern "C" {

// Returns T_STOP only for an actual STOP marker. Malformed or truncated input
// returns 0 with cursor->error latched.
uint8_t thrift_transcode_compact_read_field_header(
    TranscodeCursor* cursor, int16_t* fieldId, int16_t prevFieldId);

void thrift_transcode_compact_write_field_header(
    TranscodeCursor* cursor,
    uint8_t ttype,
    int16_t fieldId,
    int16_t prevFieldId);

void thrift_transcode_compact_write_stop(TranscodeCursor* cursor);

// Fused bool field write: writes a Compact field header with the bool value
// encoded in the ttype byte (CT_BOOLEAN_TRUE=1 or CT_BOOLEAN_FALSE=2).
// No separate value bytes are written.
void thrift_transcode_compact_write_bool_field(
    TranscodeCursor* cursor,
    uint8_t boolValue,
    int16_t fieldId,
    int16_t prevFieldId);

// Skip the value for a field whose Compact header has already been read.
// Boolean field values are carried in the header ttype and consume no bytes.
void thrift_transcode_compact_skip_field(
    TranscodeCursor* cursor, uint8_t ttype);

} // extern "C"
