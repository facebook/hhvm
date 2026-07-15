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

// JSON protocol intrinsics — the complex operations only. Whitespace skip,
// structural chars, and field-name matching are generated inline by the
// codegen. Decimal int/float parse+format, string escape handling, and
// recursive value skipping are intrinsics because inlining them would bloat
// generated IR. The strict `true`/`false` reader is also an intrinsic: the JIT
// inlines a permissive first-byte check for its trusted (schema-fused)
// producer, but wire→wire JSON is untrusted, so the interpreter calls this
// validating parse.

extern "C" {

// ── Complex scalar reads ──

// Parse decimal integer from current position. Stops at non-digit.
int64_t thrift_transcode_parse_decimal_int(TranscodeCursor* cursor);

// Parse decimal float from current position (handles ., e, E, +, -).
double thrift_transcode_parse_decimal_float(TranscodeCursor* cursor);

// Parse a JSON `true`/`false` keyword, validating the whole token. Returns 1/0;
// latches an error on any other token or a short read. (The JIT inlines a
// permissive first-byte variant instead of calling this.)
int64_t thrift_transcode_parse_bool_keyword(TranscodeCursor* cursor);

// Parse a quoted string with escape handling (\n, \t, \", \\, \uXXXX).
// Writes the unescaped bytes into the write buffer area as scratch space
// and returns pointer + length. Cursor readPos advances past closing quote.
const uint8_t* thrift_transcode_parse_escaped_string(
    TranscodeCursor* cursor, size_t* len);

const uint8_t* thrift_transcode_parse_base64_string(
    TranscodeCursor* cursor, size_t* len);

// ── Complex scalar writes ──

// Write int64 as decimal text.
void thrift_transcode_format_decimal_int(
    TranscodeCursor* cursor, int64_t value);

// Write double as decimal text.
void thrift_transcode_format_decimal_float(
    TranscodeCursor* cursor, double value);

// Write a string with JSON escape handling (quotes + escapes). This preserves
// the input bytes as-is and does not validate UTF-8.
void thrift_transcode_format_escaped_string(
    TranscodeCursor* cursor, const uint8_t* data, size_t len);

void thrift_transcode_format_base64_string(
    TranscodeCursor* cursor, const uint8_t* data, size_t len);

// ── Structural ──

// Recursively skip one JSON value (object, array, string, number, bool, null).
// Used for unknown fields. This is complex because it handles nesting.
void thrift_transcode_skip_json_value(TranscodeCursor* cursor);

} // extern "C"
