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
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <thrift/lib/cpp2/transcode/Cursor.h>

struct TranscodeJsonStringToken;

namespace apache::thrift::transcode {

// These helpers use schema metadata and are intentionally not JIT intrinsics.
bool thrift_transcode_parse_strict_i64(std::string_view text, int64_t& value);
bool thrift_transcode_parse_json_object_enum_key(
    std::string_view text,
    const std::vector<std::pair<int32_t, std::string>>* enumNames,
    int64_t& value);
int64_t thrift_transcode_parse_json_enum_value(
    TranscodeCursor* cursor,
    const std::vector<std::pair<int32_t, std::string>>* enumNames);
void thrift_transcode_json_skip_whitespace(TranscodeCursor* cursor);
// Does not skip whitespace. Returns 0 at EOF; delimiter callers should compare
// against the exact byte they expect.
uint8_t thrift_transcode_json_peek(TranscodeCursor* cursor);
// Skips leading JSON whitespace before consuming a value-terminated `null`.
bool thrift_transcode_json_consume_null(TranscodeCursor* cursor);
bool thrift_transcode_json_expect_byte(TranscodeCursor* cursor, uint8_t value);
bool thrift_transcode_read_json_object_key(
    TranscodeCursor* cursor, std::string& key);
std::string thrift_transcode_decode_json_string_token_to_string(
    TranscodeCursor* cursor, const TranscodeJsonStringToken& token);

} // namespace apache::thrift::transcode

extern "C" {

// JSON intrinsics are called directly by JIT-compiled kernels. Boolean results
// use uint8_t 0/1 values so the ABI does not depend on C++ bool representation.

struct TranscodeJsonStringToken {
  // Unquoted bytes in the input buffer. When `hasEscapes` is 0, this span is
  // also the decoded value.
  const uint8_t* begin;
  const uint8_t* end;
  // Non-zero when escaped bytes must be decoded before comparison or output.
  uint8_t hasEscapes;
};

// Reads a JSON integer token from the cursor and returns its int64 value.
int64_t thrift_transcode_parse_decimal_int(TranscodeCursor* cursor);

// Reads a JSON float token, including quoted NaN and infinity spellings.
double thrift_transcode_parse_decimal_float(TranscodeCursor* cursor);

// Reads a JSON true/false keyword and returns 1 or 0.
int64_t thrift_transcode_parse_bool_keyword(TranscodeCursor* cursor);

// Reads one JSON string, validates its escapes, and leaves decoded bytes
// materialized only when a later intrinsic asks for them.
uint8_t thrift_transcode_read_json_string_token(
    TranscodeCursor* cursor, TranscodeJsonStringToken* token);

// Compares a token's decoded bytes with `data[0..len)`.
uint8_t thrift_transcode_json_string_token_equals(
    const TranscodeJsonStringToken* token, const uint8_t* data, size_t len);

// Computes the byte length after JSON escape decoding.
uint8_t thrift_transcode_json_string_token_decoded_size(
    const TranscodeJsonStringToken* token, size_t* len);

// Decodes a JSON string token into a caller-provided buffer of exact size.
uint8_t thrift_transcode_decode_json_string_token(
    const TranscodeJsonStringToken* token, uint8_t* out, size_t len);

// Writes `value` as JSON decimal text.
void thrift_transcode_format_decimal_int(
    TranscodeCursor* cursor, int64_t value);

// Writes `value` as JSON decimal text, or a quoted non-finite spelling.
void thrift_transcode_format_decimal_float(
    TranscodeCursor* cursor, double value);

// Writes bytes as a quoted JSON string, escaping control bytes and quotes.
void thrift_transcode_format_escaped_string(
    TranscodeCursor* cursor, const uint8_t* data, size_t len);

// Writes bytes as quoted base64 JSON text.
void thrift_transcode_format_base64_string(
    TranscodeCursor* cursor, const uint8_t* data, size_t len);

// Writes the decoded bytes of a previously-read JSON string token.
size_t thrift_transcode_write_json_string_token(
    TranscodeCursor* cursor, const TranscodeJsonStringToken* token);

// Writes the decoded token bytes with a Thrift Binary string length prefix.
void thrift_transcode_write_json_string_token_i32_prefixed(
    TranscodeCursor* cursor, const TranscodeJsonStringToken* token);

// Writes the decoded token bytes with a varint string length prefix.
void thrift_transcode_write_json_string_token_varint_prefixed(
    TranscodeCursor* cursor, const TranscodeJsonStringToken* token);

// Decodes the token as base64 and writes a Thrift Binary string length prefix.
void thrift_transcode_write_json_base64_token_i32_prefixed(
    TranscodeCursor* cursor, const TranscodeJsonStringToken* token);

// Decodes the token as base64 and writes a varint string length prefix.
void thrift_transcode_write_json_base64_token_varint_prefixed(
    TranscodeCursor* cursor, const TranscodeJsonStringToken* token);

// Writes the original token bytes as a quoted JSON string.
void thrift_transcode_write_json_string_token_quoted(
    TranscodeCursor* cursor, const TranscodeJsonStringToken* token);

// Recursively skips one JSON value. This is for trusted JSON only until the
// strict scanner is added.
uint8_t thrift_transcode_skip_json_value(TranscodeCursor* cursor);

} // extern "C"
