/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>

#define RETURN_IF_ERROR(err)                                       \
  if (err != ErrorCode::NO_ERROR) {                                \
    VLOG(4) << "Returning with error=" << getErrorCodeString(err); \
    return err;                                                    \
  }                                                                \
  static_assert(true, "semicolon required")

namespace proxygen {

// Error codes are 32-bit fields that are used in RST_STREAM and GOAWAY
// frames to convey the reasons for the stream or connection error.

// We only need <1 byte to represent it in memory
enum class ErrorCode : uint8_t {
  NO_ERROR = 0,
  PROTOCOL_ERROR = 1,
  INTERNAL_ERROR = 2,
  FLOW_CONTROL_ERROR = 3,
  SETTINGS_TIMEOUT = 4,
  STREAM_CLOSED = 5,
  FRAME_SIZE_ERROR = 6,
  REFUSED_STREAM = 7,
  CANCEL = 8,
  COMPRESSION_ERROR = 9,
  CONNECT_ERROR = 10,
  ENHANCE_YOUR_CALM = 11,
  INADEQUATE_SECURITY = 12,
  HTTP_1_1_REQUIRED = 13,
};

extern const uint8_t kMaxErrorCode;

/**
 * Returns a string representation of the error code.
 */
extern const char* getErrorCodeString(ErrorCode error);

} // namespace proxygen
