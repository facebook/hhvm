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

#include <folly/lang/Bits.h>

#include <thrift/lib/cpp/protocol/TType.h>

// Protocol type-id constants used by the transcode wire intrinsics, in one
// place so the Compact and Binary framing code never hardcodes magic numbers.

namespace apache::thrift::transcode::wire {

// Thrift Binary field/element type bytes, taken from the canonical fbthrift
// enum so they can never drift from the protocol.
constexpr uint8_t kBinaryStop =
    folly::to_underlying(apache::thrift::protocol::TType::T_STOP);
constexpr uint8_t kBinaryBool =
    folly::to_underlying(apache::thrift::protocol::TType::T_BOOL);
constexpr uint8_t kBinaryByte =
    folly::to_underlying(apache::thrift::protocol::TType::T_BYTE);
constexpr uint8_t kBinaryDouble =
    folly::to_underlying(apache::thrift::protocol::TType::T_DOUBLE);
constexpr uint8_t kBinaryI16 =
    folly::to_underlying(apache::thrift::protocol::TType::T_I16);
constexpr uint8_t kBinaryI32 =
    folly::to_underlying(apache::thrift::protocol::TType::T_I32);
constexpr uint8_t kBinaryI64 =
    folly::to_underlying(apache::thrift::protocol::TType::T_I64);
constexpr uint8_t kBinaryString =
    folly::to_underlying(apache::thrift::protocol::TType::T_STRING);
constexpr uint8_t kBinaryStruct =
    folly::to_underlying(apache::thrift::protocol::TType::T_STRUCT);
constexpr uint8_t kBinaryMap =
    folly::to_underlying(apache::thrift::protocol::TType::T_MAP);
constexpr uint8_t kBinarySet =
    folly::to_underlying(apache::thrift::protocol::TType::T_SET);
constexpr uint8_t kBinaryList =
    folly::to_underlying(apache::thrift::protocol::TType::T_LIST);
constexpr uint8_t kBinaryFloat =
    folly::to_underlying(apache::thrift::protocol::TType::T_FLOAT);

// Thrift Compact field/element type nibbles. The canonical source
// (apache::thrift::detail::compact::Types in CompactProtocol-inl.h) is welded
// into the heavy CompactProtocol header, which this lightweight wire header
// must not pull in, so the values are mirrored here. WireTypeGuardTest
// static_asserts each of these against the canonical enum to catch drift.
constexpr uint8_t kCompactStop = 0;
constexpr uint8_t kCompactBooleanTrue = 1;
constexpr uint8_t kCompactBooleanFalse = 2;
constexpr uint8_t kCompactByte = 3;
constexpr uint8_t kCompactI16 = 4;
constexpr uint8_t kCompactI32 = 5;
constexpr uint8_t kCompactI64 = 6;
constexpr uint8_t kCompactDouble = 7;
constexpr uint8_t kCompactBinary = 8;
constexpr uint8_t kCompactList = 9;
constexpr uint8_t kCompactSet = 10;
constexpr uint8_t kCompactMap = 11;
constexpr uint8_t kCompactStruct = 12;
constexpr uint8_t kCompactFloat = 13;

} // namespace apache::thrift::transcode::wire
