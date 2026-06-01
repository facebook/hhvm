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

#include <folly/Range.h>

namespace apache::thrift::fast_thrift::frame {

/**
 * RSocket ERROR frame error codes.
 *
 * These match the RSocket protocol specification. Connection-level errors
 * (codes 0x0001-0x0102) use stream ID 0. Stream-level errors (codes 0x0201+)
 * use the stream ID of the affected stream.
 */
enum class ErrorCode : uint32_t {
  // Reserved
  RESERVED = 0x00000000,
  // The Setup frame is invalid for the server (stream ID MUST be 0)
  INVALID_SETUP = 0x00000001,
  // Some parameters specified by the client are unsupported (stream ID MUST be
  // 0)
  UNSUPPORTED_SETUP = 0x00000002,
  // The server rejected the setup (stream ID MUST be 0)
  REJECTED_SETUP = 0x00000003,
  // The server rejected the resume (stream ID MUST be 0)
  REJECTED_RESUME = 0x00000004,
  // The connection is being terminated, close immediately (stream ID MUST be
  // 0)
  CONNECTION_ERROR = 0x00000101,
  // The connection is being terminated, wait for streams (stream ID MUST be 0)
  CONNECTION_CLOSE = 0x00000102,
  // Application layer error (stream ID MUST be > 0)
  APPLICATION_ERROR = 0x00000201,
  // Request rejected, not processed (stream ID MUST be > 0)
  REJECTED = 0x00000202,
  // Request canceled, may have been processed (stream ID MUST be > 0)
  CANCELED = 0x00000203,
  // Invalid request (stream ID MUST be > 0)
  INVALID = 0x00000204,
};

inline folly::StringPiece toString(ErrorCode ec) {
  switch (ec) {
    case ErrorCode::RESERVED:
      return "RESERVED";
    case ErrorCode::INVALID_SETUP:
      return "INVALID_SETUP";
    case ErrorCode::UNSUPPORTED_SETUP:
      return "UNSUPPORTED_SETUP";
    case ErrorCode::REJECTED_SETUP:
      return "REJECTED_SETUP";
    case ErrorCode::REJECTED_RESUME:
      return "REJECTED_RESUME";
    case ErrorCode::CONNECTION_ERROR:
      return "CONNECTION_ERROR";
    case ErrorCode::CONNECTION_CLOSE:
      return "CONNECTION_CLOSE";
    case ErrorCode::APPLICATION_ERROR:
      return "APPLICATION_ERROR";
    case ErrorCode::REJECTED:
      return "REJECTED";
    case ErrorCode::CANCELED:
      return "CANCELED";
    case ErrorCode::INVALID:
      return "INVALID";
  }
  return "UNKNOWN";
}

} // namespace apache::thrift::fast_thrift::frame
