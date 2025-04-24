/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <folly/Expected.h>
#include <folly/Optional.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <iomanip>
#include <iostream>

namespace proxygen {

class CapsuleCodec {
 public:
  enum class ErrorCode : uint32_t {
    PARSE_UNDERFLOW = 0x0,
    PARSE_ERROR = 0x1,
  };

  class Callback {
   public:
    virtual ~Callback() = default;

    virtual void onCapsule(uint64_t /*capsuleType*/,
                           uint64_t /*capsuleLength*/) {
    }
    virtual void onConnectionError(ErrorCode error) = 0;
  };

  explicit CapsuleCodec(Callback* callback = nullptr)
      : parseState_(ParseState::CAPSULE_HEADER_TYPE),
        connError_(folly::none),
        callback_(callback) {
  }
  virtual ~CapsuleCodec() = default;

  void onIngress(std::unique_ptr<folly::IOBuf> data, bool eom);

  void setCallback(Callback* callback) {
    callback_ = callback;
  }

 protected:
  virtual bool canParseCapsule(uint64_t /*capsuleType*/) {
    return false;
  }

  virtual folly::Expected<folly::Unit, ErrorCode> parseCapsule(
      folly::io::Cursor& cursor) {
    // Override for capsule specific parsing functions
    // Return folly::none if parsing is successful, or an error code if it fails
    return folly::makeUnexpected(ErrorCode::PARSE_ERROR);
  }

  uint64_t curCapsuleType_{std::numeric_limits<uint64_t>::max()};
  uint64_t curCapsuleLength_{0};

 private:
  folly::IOBufQueue ingress_{folly::IOBufQueue::cacheChainLength()};
  enum class ParseState : uint8_t {
    CAPSULE_HEADER_TYPE,
    CAPSULE_LENGTH,
    CAPSULE_PAYLOAD,
    SKIP_CAPSULE,
  };
  ParseState parseState_{};
  folly::Optional<ErrorCode> connError_;

 protected:
  Callback* callback_{nullptr};
};

} // namespace proxygen
