/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/logging/xlog.h>
#include <proxygen/lib/http/webtransport/WtEgressContainer.h>

namespace proxygen::detail {

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
  atMost = std::min({atMost,
                     window_.getAvailable(),
                     static_cast<uint64_t>(data_.chainLength())});
  DequeueResult res;
  res.data = atMost == 0 ? nullptr : data_.splitAtMost(atMost);
  res.fin = data_.empty() && std::exchange(fin_, false);
  window_.commit(atMost);
  return res;
}

}; // namespace proxygen::detail
