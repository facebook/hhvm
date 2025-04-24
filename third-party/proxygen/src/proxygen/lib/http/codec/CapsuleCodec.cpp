/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/logging/xlog.h>
#include <proxygen/lib/http/codec/CapsuleCodec.h>
#include <quic/codec/QuicInteger.h>

namespace proxygen {

void CapsuleCodec::onIngress(std::unique_ptr<folly::IOBuf> data, bool eom) {
  ingress_.append(std::move(data));
  size_t remainingLength = ingress_.chainLength();
  folly::io::Cursor cursor(ingress_.front());
  while (!connError_ && remainingLength > 0) {
    switch (parseState_) {
      case ParseState::CAPSULE_HEADER_TYPE: {
        auto type = quic::decodeQuicInteger(cursor);
        if (!type) {
          connError_ = ErrorCode::PARSE_UNDERFLOW;
          break;
        }
        curCapsuleType_ = type->first;
        remainingLength -= type->second;
        parseState_ = ParseState::CAPSULE_LENGTH;
        [[fallthrough]];
      }
      case ParseState::CAPSULE_LENGTH: {
        auto length = quic::decodeQuicInteger(cursor);
        if (!length) {
          connError_ = ErrorCode::PARSE_UNDERFLOW;
          break;
        }
        curCapsuleLength_ = length->first;
        remainingLength -= length->second;
        if (callback_) {
          callback_->onCapsule(curCapsuleType_, curCapsuleLength_);
        }
        if (!canParseCapsule(curCapsuleType_)) {
          parseState_ = ParseState::SKIP_CAPSULE;
          break;
        } else {
          parseState_ = ParseState::CAPSULE_PAYLOAD;
        }
        [[fallthrough]];
      }
      case ParseState::CAPSULE_PAYLOAD: {
        if (remainingLength < curCapsuleLength_) {
          connError_ = ErrorCode::PARSE_UNDERFLOW;
          break;
        }
        auto res = parseCapsule(cursor);
        if (res.hasError()) {
          connError_ = res.error();
          break;
        }
        parseState_ = ParseState::CAPSULE_HEADER_TYPE;
        remainingLength -= curCapsuleLength_;
        break;
      }
      case ParseState::SKIP_CAPSULE: {
        auto skipped = cursor.skipAtMost(curCapsuleLength_);
        curCapsuleLength_ -= skipped;
        remainingLength -= skipped;
        if (curCapsuleLength_ == 0) {
          parseState_ = ParseState::CAPSULE_HEADER_TYPE;
        }
        break;
      }
    }
  }
  if (connError_) {
    if (connError_.value() == ErrorCode::PARSE_UNDERFLOW && !eom) {
      ingress_.trimStart(ingress_.chainLength() - remainingLength);
      connError_.reset();
      return;
    } else {
      if (callback_) {
        callback_->onConnectionError(connError_.value());
      }
      XLOG(ERR) << "Connection error=" << uint32_t(*connError_);
    }
  }
  ingress_.move();
}

} // namespace proxygen
