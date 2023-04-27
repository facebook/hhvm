/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/HTTPRequestVerifier.h>
#include <proxygen/lib/http/codec/compress/HPACKConstants.h>
#include <proxygen/lib/http/codec/compress/HPACKHeaderName.h>

namespace proxygen {

class HTTPMessage;

class HeaderDecodeInfo {
 public:
  void init(bool isRequestIn,
            bool isRequestTrailers,
            bool validate,
            bool strictValidation,
            bool allowEmptyPath) {
    CHECK(!msg);
    msg.reset(new HTTPMessage());
    isRequest_ = isRequestIn;
    isRequestTrailers_ = isRequestTrailers;
    validate_ = validate;
    hasStatus_ = false;
    contentLength_ = folly::none;
    regularHeaderSeen_ = false;
    pseudoHeaderSeen_ = false;
    parsingError.clear();
    headerErrorValue.clear();
    decodeError = HPACK::DecodeError::NONE;
    strictValidation_ = strictValidation;
    allowEmptyPath_ = allowEmptyPath;
    verifier.reset(msg.get());
  }

  bool onHeader(const HPACKHeaderName& name, const folly::fbstring& value);

  void onHeadersComplete(HTTPHeaderSize decodedSize);

  bool hasStatus() const;

  // Change this to a map of decoded header blocks when we decide
  // to concurrently decode partial header blocks
  std::unique_ptr<HTTPMessage> msg;
  HTTPRequestVerifier verifier;
  std::string parsingError;
  std::string headerErrorValue;
  HPACK::DecodeError decodeError{HPACK::DecodeError::NONE};

 private:
  bool isRequest_{false};
  bool isRequestTrailers_{false};
  bool validate_{true};
  bool hasStatus_{false};
  bool regularHeaderSeen_{false};
  bool pseudoHeaderSeen_{false};
  // Default to false for now to match existing behavior
  bool strictValidation_{false};
  bool allowEmptyPath_{false};
  folly::Optional<uint32_t> contentLength_;
};

} // namespace proxygen
