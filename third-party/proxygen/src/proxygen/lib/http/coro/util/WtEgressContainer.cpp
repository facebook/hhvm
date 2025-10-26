/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/logging/xlog.h>
#include <proxygen/lib/http/coro/util/WtEgressContainer.h>

namespace proxygen::coro {

WtBufferedStreamData::FcRes WtBufferedStreamData::enqueue(
    std::unique_ptr<folly::IOBuf> data, bool fin) noexcept {
  XCHECK(!fin_) << "enqueue after fin";
  auto len = data ? data->computeChainDataLength() : 0;
  data_.append(std::move(data)); // ok if nullptr
  fin_ = fin;
  return window_.buffer(len);
}

WtBufferedStreamData::DequeueResult WtBufferedStreamData::dequeue(
    uint64_t atMost) noexcept {
  // min of maxBytes and how many bytes remaining in egress window
  atMost = std::min({atMost, window_.getAvailable(), data_.chainLength()});
  DequeueResult res;
  res.data = data_.splitAtMost(atMost);
  res.fin = data_.empty() && std::exchange(fin_, false);
  window_.commit(atMost);
  return res;
}

}; // namespace proxygen::coro
