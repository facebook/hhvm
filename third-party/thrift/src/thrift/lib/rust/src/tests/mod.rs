/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

// Test fixtures borrowed from protocol_test.go
const BOOL_VALUES: [bool; 5] = [false, true, false, false, true];
const BYTE_VALUES: [i8; 7] = [117i8, 0, 1, 32, 127, -128, 44];
const INT16_VALUES: [i16; 8] = [459i16, 0, 1, -1, -128, 127, 32767, -32768];
const INT32_VALUES: [i32; 9] = [459i32, 0, 1, -1, -128, 127, 32767, 2147483647, -2147483535];
const INT64_VALUES: [i64; 13] = [
    459i64,
    0,
    1,
    -1,
    -128,
    127,
    32767,
    2147483647,
    -2147483535,
    34359738481,
    -35184372088719,
    -9223372036854775808,
    9223372036854775807,
];
const FLOAT_VALUES: [f32; 14] = [
    459.3f32,
    0.0,
    -1.0,
    1.0,
    0.5,
    0.3333,
    3.14159,
    1.537e-38,
    1.673e25,
    6.02214179e23,
    -6.02214179e23,
    ::std::f32::INFINITY,
    ::std::f32::NEG_INFINITY,
    ::std::f32::NAN,
];
const DOUBLE_VALUES: [f64; 14] = [
    459.3f64,
    0.0,
    -1.0,
    1.0,
    0.5,
    0.3333,
    3.14159,
    1.537e-38,
    1.673e25,
    6.02214179e23,
    -6.02214179e23,
    ::std::f64::INFINITY,
    ::std::f64::NEG_INFINITY,
    ::std::f64::NAN,
];

mod binary;
mod compact;
mod simplejson;
