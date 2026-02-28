/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "IovecCursor.h"

#include <algorithm>

namespace facebook {
namespace memcache {

IovecCursor::IovecCursor(const struct iovec* iov, size_t iovcnt)
    : iov_(iov), iovLength_(iovcnt), totalLength_(computeTotalLength()) {
  curBufLen_ = iovcnt > 0 ? iov_[iovIndex_].iov_len : 0;
  advanceBufferIfEmpty();
}

bool IovecCursor::hasDataAvailable() const {
  return (iovIndex_ < iovLength_);
}

void IovecCursor::advanceSlow(size_t bytes) {
  while (hasDataAvailable() && bytes > 0) {
    size_t toAdvance = std::min(bytes, curBufLen_);
    absolutePos_ += toAdvance;
    curBufPos_ += toAdvance;
    curBufLen_ -= toAdvance;
    bytes -= toAdvance;
    advanceBufferIfEmpty();
  }
}

void IovecCursor::retreatSlow(size_t bytes) {
  while (bytes > 0) {
    if (curBufPos_ == 0) {
      assert(iovIndex_ > 0);
      curBufPos_ = iov_[--iovIndex_].iov_len;
      curBufLen_ = 0;
    }
    size_t toRetreat = std::min(bytes, curBufPos_);
    absolutePos_ -= toRetreat;
    curBufPos_ -= toRetreat;
    curBufLen_ += toRetreat;
    bytes -= toRetreat;
  }
}

void IovecCursor::seek(size_t pos) {
  assert(pos <= totalLength());

  absolutePos_ = 0;
  iovIndex_ = 0;
  curBufPos_ = 0;
  curBufLen_ = iov_[iovIndex_].iov_len;

  return advance(pos);
}

void IovecCursor::peekInto(uint8_t* dest, size_t size) const {
  const uint8_t* cur =
      reinterpret_cast<uint8_t*>(iov_[iovIndex_].iov_base) + curBufPos_;
  size_t curLen = curBufLen_;
  size_t i = iovIndex_;

  while (size > 0) {
    size_t toCopy = std::min(size, curLen);
    std::memcpy(dest, cur, toCopy);
    dest += toCopy;
    size -= toCopy;
    if (size > 0) {
      ++i;
      cur = reinterpret_cast<uint8_t*>(iov_[i].iov_base);
      curLen = iov_[i].iov_len;
    }
  }
}

void IovecCursor::readInto(uint8_t* dest, size_t size) {
  peekInto(dest, size);
  advance(size);
}

void IovecCursor::advanceBufferIfEmpty() {
  while (hasDataAvailable() && (curBufLen_ == 0)) {
    ++iovIndex_;
    curBufPos_ = 0;
    if (iovIndex_ < iovLength_) {
      curBufLen_ = iov_[iovIndex_].iov_len;
    }
  }
}

/* static */ size_t IovecCursor::computeTotalLength(
    const struct iovec* iov,
    size_t iovcnt) {
  size_t len = 0;
  for (size_t i = 0; i < iovcnt; ++i) {
    len += iov[i].iov_len;
  }
  return len;
}

size_t IovecCursor::computeTotalLength() const {
  return computeTotalLength(iov_, iovLength_);
}

} // namespace memcache
} // namespace facebook
