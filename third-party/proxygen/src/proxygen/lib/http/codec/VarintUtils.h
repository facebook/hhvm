/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/IOBufQueue.h>
#include <quic/codec/QuicInteger.h>

namespace proxygen {

constexpr size_t kDefaultVarintBufferGrowth = 32;

// Write a QUIC variable-length integer to buf, accumulating size and
// short-circuiting on error.
inline void writeVarint(folly::IOBufQueue& buf,
                        uint64_t value,
                        size_t& size,
                        bool& error) noexcept {
  if (error) {
    return;
  }
  folly::io::QueueAppender appender(&buf, kDefaultVarintBufferGrowth);
  auto appenderOp = [&](auto val) mutable {
    appender.writeBE(folly::tag<decltype(val)>, val);
  };
  auto res = quic::encodeQuicInteger(value, appenderOp);
  if (res.hasError()) {
    error = true;
  } else {
    size += *res;
  }
}

} // namespace proxygen
