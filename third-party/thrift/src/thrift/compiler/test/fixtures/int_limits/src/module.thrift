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

include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

namespace java.swift test.fixtures.int_limits

struct Limits {
  1: i64 max_i64_field = 9223372036854775807;
  2: i64 min_i64_field = -9223372036854775808;
  3: i32 max_i32_field = 2147483647;
  4: i32 min_i32_field = -2147483648;
  5: i16 max_i16_field = 32767;
  6: i16 min_i16_field = -32768;
  7: byte max_byte_field = 127;
  8: byte min_byte_field = -128;
}

const i64 max_i64_const = 9223372036854775807;
const i64 min_i64_const = -9223372036854775808;
const i32 max_i32_const = 2147483647;
const i32 min_i32_const = -2147483648;
const i16 max_i16_const = 32767;
const i16 min_i16_const = -32768;
const byte max_byte_const = 127;
const byte min_byte_const = -128;
