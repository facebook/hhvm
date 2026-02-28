/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/carbon/CarbonQueueAppender.h"

#include <cstring>

namespace carbon {

void CarbonQueueAppenderStorage::coalesce() {
  VLOG(4) << "Out of iovecs, coalescing in Caret message serialization";
  assert(nIovsUsed_ == kMaxIovecs);

  finalizeLastIovec();

  canUsePreviousIov_ = false;
  nIovsUsed_ = 1; // headerBuf_ always considered used
  resetStorageIdx();

  size_t newCapacity = 0;
  // coalesce() should always be triggered before writing header
  assert(headerOverlap_ == 0 && iovs_[0].iov_len == 0);
  for (size_t i = 1; i < kMaxIovecs; ++i) {
    newCapacity += iovs_[i].iov_len;
  }

  auto newBuf = folly::IOBuf(folly::IOBuf::CREATE, newCapacity);

  for (size_t i = 1; i < kMaxIovecs; ++i) {
    std::memcpy(newBuf.writableTail(), iovs_[i].iov_base, iovs_[i].iov_len);
    newBuf.append(iovs_[i].iov_len);
  }

  // Release old IOBufs and reset head to new large buffer
  head_ = std::move(newBuf);
  ++nIovsUsed_;
  iovs_[1] = {const_cast<uint8_t*>(head_->data()), head_->length()};
}

} // namespace carbon
