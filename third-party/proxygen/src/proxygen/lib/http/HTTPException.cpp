/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <sstream>
#include <string>

#include <proxygen/lib/http/HTTPException.h>

namespace proxygen {

HTTP3::ErrorCode toHTTP3ErrorCode(proxygen::ErrorCode err) {
  switch (err) {
    case ErrorCode::NO_ERROR:
      return HTTP3::ErrorCode::HTTP_NO_ERROR;
    case ErrorCode::PROTOCOL_ERROR:
      return HTTP3::ErrorCode::HTTP_GENERAL_PROTOCOL_ERROR;
    case ErrorCode::INTERNAL_ERROR:
      return HTTP3::ErrorCode::HTTP_INTERNAL_ERROR;
    case ErrorCode::FLOW_CONTROL_ERROR:
      DCHECK(false) << "ErrorCode::FLOW_CONTROL_ERROR for QUIC";
#ifdef NDEBUG
      [[fallthrough]];
#endif
    case ErrorCode::SETTINGS_TIMEOUT: // maybe we should keep this?
    case ErrorCode::STREAM_CLOSED:
      return HTTP3::ErrorCode::HTTP_GENERAL_PROTOCOL_ERROR;
    case ErrorCode::FRAME_SIZE_ERROR:
      return HTTP3::ErrorCode::HTTP_FRAME_ERROR;
    case ErrorCode::REFUSED_STREAM:
      return HTTP3::ErrorCode::HTTP_REQUEST_REJECTED;
    case ErrorCode::CANCEL:
      return HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED;
    case ErrorCode::COMPRESSION_ERROR:
      return HTTP3::ErrorCode::HTTP_QPACK_DECOMPRESSION_FAILED;
    case ErrorCode::CONNECT_ERROR:
      return HTTP3::ErrorCode::HTTP_CONNECT_ERROR;
    case ErrorCode::ENHANCE_YOUR_CALM:
      return HTTP3::ErrorCode::HTTP_EXCESSIVE_LOAD;
    case ErrorCode::INADEQUATE_SECURITY:
    case ErrorCode::HTTP_1_1_REQUIRED:
    default:
      return HTTP3::ErrorCode::HTTP_GENERAL_PROTOCOL_ERROR;
  }
}

HTTPException::HTTPException(Direction dir, const std::string& msg)
    : Exception(msg), dir_(dir) {
}

HTTPException::HTTPException(Direction dir, const char* msg)
    : Exception(msg), dir_(dir) {
}

HTTPException::HTTPException(const HTTPException& ex)
    : Exception(static_cast<const Exception&>(ex)),
      dir_(ex.dir_),
      httpStatusCode_(ex.httpStatusCode_),
      http3ErrorCode_(ex.http3ErrorCode_),
      codecStatusCode_(ex.codecStatusCode_),
      errno_(ex.errno_) {
  if (ex.currentIngressBuf_) {
    currentIngressBuf_ = ex.currentIngressBuf_->clone();
  }
  if (ex.partialMsg_) {
    partialMsg_ = std::make_unique<HTTPMessage>(*ex.partialMsg_.get());
  }
}

HTTP3::ErrorCode HTTPException::inferHTTP3ErrorCode() const {
  if (hasHttpStatusCode()) {
    return HTTP3::ErrorCode::HTTP_NO_ERROR; // does this sound right?
  } else if (hasCodecStatusCode()) {
    return toHTTP3ErrorCode(getCodecStatusCode());
  }
  return HTTP3::ErrorCode::HTTP_GENERAL_PROTOCOL_ERROR;
}

HTTP3::ErrorCode HTTPException::getHttp3ErrorCode() const {
  if (hasHttp3ErrorCode()) {
    return *http3ErrorCode_;
  }
  return inferHTTP3ErrorCode();
}

std::string HTTPException::describe() const {
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

std::ostream& operator<<(std::ostream& os, const HTTPException& ex) {
  os << "what=\"" << ex.what()
     << "\", direction=" << static_cast<int>(ex.getDirection())
     << ", proxygenError=" << getErrorString(ex.getProxygenError())
     << ", codecStatusCode="
     << (ex.hasCodecStatusCode() ? getErrorCodeString(ex.getCodecStatusCode())
                                 : "-1")
     << ", httpStatusCode=" << ex.getHttpStatusCode();
  if (ex.hasHttp3ErrorCode()) {
    os << ", http3ErrorCode=" << toString(ex.getHttp3ErrorCode());
  }
  return os;
}

} // namespace proxygen
