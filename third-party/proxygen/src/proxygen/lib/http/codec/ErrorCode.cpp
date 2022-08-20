/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/ErrorCode.h>

#include <glog/logging.h>

namespace proxygen {

const uint8_t kMaxErrorCode = 12;

const char* getErrorCodeString(ErrorCode error) {
  switch (error) {
    case ErrorCode::NO_ERROR:
      return "NO_ERROR";
    case ErrorCode::PROTOCOL_ERROR:
      return "PROTOCOL_ERROR";
    case ErrorCode::INTERNAL_ERROR:
      return "INTERNAL_ERROR";
    case ErrorCode::FLOW_CONTROL_ERROR:
      return "FLOW_CONTROL_ERROR";
    case ErrorCode::SETTINGS_TIMEOUT:
      return "SETTINGS_TIMEOUT";
    case ErrorCode::STREAM_CLOSED:
      return "STREAM_CLOSED";
    case ErrorCode::FRAME_SIZE_ERROR:
      return "FRAME_SIZE_ERROR";
    case ErrorCode::REFUSED_STREAM:
      return "REFUSED_STREAM";
    case ErrorCode::CANCEL:
      return "CANCEL";
    case ErrorCode::COMPRESSION_ERROR:
      return "COMPRESSION_ERROR";
    case ErrorCode::CONNECT_ERROR:
      return "CONNECT_ERROR";
    case ErrorCode::ENHANCE_YOUR_CALM:
      return "ENHANCE_YOUR_CALM";
    case ErrorCode::INADEQUATE_SECURITY:
      return "INADEQUATE_SECURITY";
    case ErrorCode::HTTP_1_1_REQUIRED:
      return "HTTP_1_1_REQUIRED";
  }
  LOG(FATAL) << "Unreachable";
  return "";
}

} // namespace proxygen
