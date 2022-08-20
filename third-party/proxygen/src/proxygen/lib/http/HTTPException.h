/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Memory.h>
#include <folly/io/IOBufQueue.h>
#include <proxygen/lib/http/HTTP3ErrorCode.h>
#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/http/codec/ErrorCode.h>
#include <proxygen/lib/utils/Exception.h>

namespace proxygen {

HTTP3::ErrorCode toHTTP3ErrorCode(proxygen::ErrorCode err);

/**
 * This class encapsulates the various errors that can occur on an
 * http session. Errors can occur at various levels: the connection can
 * be closed for reads and/or writes, the message body may fail to parse,
 * or various protocol constraints may be violated.
 */
class HTTPException : public proxygen::Exception {
 public:
  /**
   * Indicates which direction of the data flow was affected by this
   * exception. For instance, if a class receives HTTPException(INGRESS),
   * then it should consider ingress callbacks finished (whether or not
   * the underlying transport actually shut down). Likewise for
   * HTTPException(EGRESS), the class should consider the write stream
   * shut down. INGRESS_AND_EGRESS indicates both directions are finished.
   */
  enum class Direction {
    INGRESS = 0,
    EGRESS,
    INGRESS_AND_EGRESS,
  };

  HTTPException(Direction dir, const std::string& msg);

  HTTPException(Direction dir, const char* msg);

  HTTPException(const HTTPException& ex);

  /**
   * Returns a string representation of this exception. This function is
   * intended for debugging and logging only. For the true exception
   * string, use what()
   */
  std::string describe() const;

  Direction getDirection() const {
    return dir_;
  }

  bool isIngressException() const {
    return dir_ == Direction::INGRESS || dir_ == Direction::INGRESS_AND_EGRESS;
  }

  bool isEgressException() const {
    return dir_ == Direction::EGRESS || dir_ == Direction::INGRESS_AND_EGRESS;
  }

  // Accessors for HTTP error codes
  bool hasHttpStatusCode() const {
    return (httpStatusCode_ != 0);
  }

  void setHttpStatusCode(uint32_t statusCode) {
    httpStatusCode_ = statusCode;
  }

  uint32_t getHttpStatusCode() const {
    return httpStatusCode_;
  }

  // Accessors for HTTP3 error codes
  bool hasHttp3ErrorCode() const {
    return http3ErrorCode_.has_value();
  }
  void setHttp3ErrorCode(HTTP3::ErrorCode errorCode) {
    http3ErrorCode_ = errorCode;
  }
  HTTP3::ErrorCode getHttp3ErrorCode() const;

  // Accessors for Codec specific status codes
  bool hasCodecStatusCode() const {
    return codecStatusCode_.has_value();
  }
  void setCodecStatusCode(ErrorCode statusCode) {
    codecStatusCode_ = statusCode;
  }
  ErrorCode getCodecStatusCode() const {
    CHECK(hasCodecStatusCode());
    return *codecStatusCode_;
  }

  // Accessors for errno
  bool hasErrno() const {
    return (errno_ != 0);
  }
  void setErrno(uint32_t e) {
    errno_ = e;
  }
  uint32_t getErrno() const {
    return errno_;
  }

  void setCurrentIngressBuf(std::unique_ptr<folly::IOBuf> buf) {
    currentIngressBuf_ = std::move(buf);
  }

  std::unique_ptr<folly::IOBuf> moveCurrentIngressBuf() {
    return std::move(currentIngressBuf_);
  }

  void setPartialMsg(std::unique_ptr<HTTPMessage> partialMsg) {
    partialMsg_ = std::move(partialMsg);
  }

  std::unique_ptr<HTTPMessage> movePartialMsg() {
    return std::move(partialMsg_);
  }

  HTTPMessage* getPartialMsg() const {
    return partialMsg_.get();
  }

 private:
  HTTP3::ErrorCode inferHTTP3ErrorCode() const;

  Direction dir_;
  uint32_t httpStatusCode_{0};
  folly::Optional<HTTP3::ErrorCode> http3ErrorCode_;
  folly::Optional<ErrorCode> codecStatusCode_;
  uint32_t errno_{0};
  // current ingress buffer, may be compressed
  std::unique_ptr<folly::IOBuf> currentIngressBuf_;
  // partial message that is being parsed
  std::unique_ptr<HTTPMessage> partialMsg_;
};

std::ostream& operator<<(std::ostream& os, const HTTPException& ex);

} // namespace proxygen
