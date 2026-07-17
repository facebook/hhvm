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

// Protobuf field framing intrinsics, exposed with the uniform field-header
// signature the codegen uses for every protocol.

extern "C" {

// Reads a protobuf tag and adapts to uniform signature:
//   Returns wire_type + 1 as the "type info" byte (offset by 1 so 0 stays
//   reserved for end of message). Sets *fieldId.
//   Returns 0 when cursor is at end of message.
uint8_t thrift_transcode_proto_read_field_header(
    TranscodeCursor* cursor, int16_t* fieldId, int16_t prevFieldId);

// Writes a protobuf tag from uniform signature:
//   typeInfo = wire_type + 1 (the readers' offset form; 1 is subtracted to
//   recover the wire type), fieldId = field number. prevFieldId is ignored.
void thrift_transcode_proto_write_field_header(
    TranscodeCursor* cursor,
    uint8_t typeInfo,
    int16_t fieldId,
    int16_t prevFieldId);

// No-op for protobuf (messages don't have stop markers).
void thrift_transcode_proto_write_stop(TranscodeCursor* cursor);

// Skips a protobuf field payload using the same typeInfo byte returned by
// thrift_transcode_proto_read_field_header: wire_type + 1, with 0 reserved for
// end of message. Callers should not pass the raw protobuf wire type.
void thrift_transcode_proto_skip_field(
    TranscodeCursor* cursor, uint8_t typeInfo);

} // extern "C"
