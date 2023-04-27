/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Expected.h>
#include <folly/Function.h>
#include <proxygen/lib/http/codec/compress/HPACKHeader.h>
#include <proxygen/lib/http/codec/compress/HPACKStreamingCallback.h>
#include <proxygen/lib/http/codec/compress/HeaderCodec.h>

namespace proxygen {

class TestStreamingCallback : public HPACK::StreamingCallback {
 public:
  void onHeader(const HPACKHeaderName& hname,
                const folly::fbstring& value) override {
    auto name = hname.get();
    headers.emplace_back(duplicate(name), name.size(), true, false);
    headers.emplace_back(duplicate(value), value.size(), true, false);
  }
  void onHeadersComplete(HTTPHeaderSize decodedSize,
                         bool /*acknowledge*/) override {
    decodedSize_ = decodedSize;
    if (headersCompleteCb) {
      headersCompleteCb();
    }
  }
  void onDecodeError(HPACK::DecodeError decodeError) override {
    error = decodeError;
  }

  void reset() {
    headers.clear();
    error = HPACK::DecodeError::NONE;
  }

  folly::Expected<HeaderDecodeResult, HPACK::DecodeError> getResult() {
    if (error == HPACK::DecodeError::NONE) {
      return HeaderDecodeResult{headers, 0};
    } else {
      return folly::makeUnexpected(error);
    }
  }

  bool hasError() const {
    return error != HPACK::DecodeError::NONE;
  }

  std::unique_ptr<std::vector<HPACKHeader>> hpackHeaders() const {
    CHECK(!hasError());
    auto result = std::make_unique<std::vector<HPACKHeader>>();
    for (size_t i = 0; i < headers.size(); i += 2) {
      result->emplace_back(headers[i].str, headers[i + 1].str);
    }

    return result;
  }

  compress::HeaderPieceList headers;
  HPACK::DecodeError error{HPACK::DecodeError::NONE};
  char* duplicate(const folly::fbstring& str) {
    char* res = CHECK_NOTNULL(new char[str.length() + 1]);
    memcpy(res, str.data(), str.length() + 1);
    return res;
  }

  folly::Function<void()> headersCompleteCb;
  HTTPHeaderSize decodedSize_;
};

} // namespace proxygen
