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

namespace apache {
namespace thrift {
namespace rocket {

// Commented-out values are not currently used/needed in Thrift, but may be
// in the future.
enum class FrameType : uint8_t {
  RESERVED = 0x00, // Reserved. Never transmitted over the wire.
  SETUP = 0x01,
  // LEASE = 0x02,
  KEEPALIVE = 0x03,
  REQUEST_RESPONSE = 0x04,
  REQUEST_FNF = 0x05,
  REQUEST_STREAM = 0x06,
  REQUEST_CHANNEL = 0x07,
  REQUEST_N = 0x08,
  CANCEL = 0x09,
  PAYLOAD = 0x0A,
  ERROR = 0x0B,
  METADATA_PUSH = 0x0C,
  // RESUME = 0x0D,
  // RESUME_OK = 0x0E,
  EXT = 0x3F,
};

enum class ExtFrameType : uint32_t {
  UNKNOWN = 0x00, // Never transmitted over the wire.
  // HEADERS_PUSH = 0xFB0,
  ALIGNED_PAGE = 0xFB1,
  // INTERACTION_TERMINATE = 0xFB2,
  // CUSTOM_ALLOC = 0xFB3,
};

} // namespace rocket
} // namespace thrift
} // namespace apache
