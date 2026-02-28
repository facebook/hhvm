/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdint.h>
#include <string>
#include <vector>

namespace proxygen {

namespace HTTP3 {
enum ErrorCode : uint64_t {
  // HTTP/3 error codes from draft
  HTTP_NO_ERROR = 0x100,
  HTTP_GENERAL_PROTOCOL_ERROR = 0x101,
  HTTP_INTERNAL_ERROR = 0x102,
  HTTP_STREAM_CREATION_ERROR = 0x103,
  HTTP_CLOSED_CRITICAL_STREAM = 0x104,
  HTTP_FRAME_UNEXPECTED = 0x105,
  HTTP_FRAME_ERROR = 0x106,
  HTTP_EXCESSIVE_LOAD = 0x107,
  HTTP_ID_ERROR = 0x108,
  HTTP_SETTINGS_ERROR = 0x109,
  HTTP_MISSING_SETTINGS = 0x10A,
  HTTP_REQUEST_REJECTED = 0x10B,
  HTTP_REQUEST_CANCELLED = 0x10C,
  HTTP_INCOMPLETE_REQUEST = 0x10D,
  HTTP_MESSAGE_ERROR = 0x10E,
  HTTP_CONNECT_ERROR = 0x10F,
  HTTP_VERSION_FALLBACK = 0x110,
  // QPACK 0x200, all from draft
  HTTP_QPACK_DECOMPRESSION_FAILED = 0x200,
  HTTP_QPACK_ENCODER_STREAM_ERROR = 0x201,
  HTTP_QPACK_DECODER_STREAM_ERROR = 0x202,

  // Internal use only
  GIVEUP_ZERO_RTT = 0xF2
};
}
inline bool isQPACKError(HTTP3::ErrorCode err) {
  return err == HTTP3::ErrorCode::HTTP_QPACK_DECOMPRESSION_FAILED ||
         err == HTTP3::ErrorCode::HTTP_QPACK_ENCODER_STREAM_ERROR ||
         err == HTTP3::ErrorCode::HTTP_QPACK_DECODER_STREAM_ERROR;
}

std::string toString(HTTP3::ErrorCode code);
std::vector<HTTP3::ErrorCode> getAllHTTP3ErrorCodes();
} // namespace proxygen
