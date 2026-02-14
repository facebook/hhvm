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

WtBufferedStreamData::PendingWrite::PendingWrite(
    std::unique_ptr<folly::IOBuf> data,
    proxygen::WebTransport::ByteEventCallback* callback,
    uint64_t offset,
    bool finFlag) noexcept
    : buf(folly::IOBufQueue::cacheChainLength()),
      deliveryCallback(callback),
      offset(offset),
      fin(finFlag) {
  buf.append(std::move(data)); // ok if nullptr
}

WtBufferedStreamData::FcRes WtBufferedStreamData::enqueue(
    std::unique_ptr<folly::IOBuf> data,
    bool fin,
    proxygen::WebTransport::ByteEventCallback* callback) noexcept {
  XCHECK(pendingWrites_.empty() || !pendingWrites_.back().fin)
      << "enqueue after fin";

  auto len = data ? data->computeChainDataLength() : 0;
  uint64_t offset = window_.getBufferedOffset() + (len ? len - 1 : 0);
  auto* lastWrite = pendingWrites_.empty() ? nullptr : &pendingWrites_.back();

  // If last write had no callback and no fin, coalesce
  if (lastWrite && !lastWrite->deliveryCallback) {
    lastWrite->buf.append(std::move(data));
    lastWrite->deliveryCallback = callback;
    lastWrite->offset = offset;
    lastWrite->fin = fin;
  } else {
    pendingWrites_.emplace_back(std::move(data), callback, offset, fin);
  }

  return window_.buffer(len);
}

WtBufferedStreamData::DequeueResult WtBufferedStreamData::dequeue(
    uint64_t atMost) noexcept {
  // min of maxBytes and how many bytes remaining in egress window
  atMost = std::min({atMost, window_.getAvailable()});
  DequeueResult res;
  if (atMost == 0 && !onlyFinPending()) {
    return res;
  }

  folly::IOBufQueue resQueue(folly::IOBufQueue::cacheChainLength());
  // Dequeue data from pending writes until we've dequeued atMost bytes
  // or completed a write that has a callback
  while (!pendingWrites_.empty() && !res.deliveryCallback) {
    auto& frontWrite = pendingWrites_.front();

    if (auto rem = atMost - resQueue.chainLength()) {
      resQueue.append(frontWrite.buf.splitAtMost(rem));
    }

    if (!frontWrite.buf.empty()) {
      break;
    }

    res.deliveryCallback = frontWrite.deliveryCallback;
    res.fin = frontWrite.fin;
    pendingWrites_.pop_front();
  }

  window_.commit(resQueue.chainLength());
  res.data = resQueue.move();
  return res;
}

bool WtBufferedStreamData::onlyFinPending() const {
  const auto* front =
      pendingWrites_.empty() ? nullptr : &pendingWrites_.front();
  return front && front->buf.empty() && front->fin;
}

void WtBufferedStreamData::clear(uint64_t id) noexcept {
  auto pendingWrites = std::move(pendingWrites_);
  for (auto& write : pendingWrites) {
    if (write.deliveryCallback) {
      write.deliveryCallback->onByteEventCanceled(id, write.offset);
    }
  }
}

}; // namespace proxygen::detail
