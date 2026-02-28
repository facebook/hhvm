/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <sys/uio.h>
#include <cassert>
#include <cstdint>

#include <folly/Likely.h>

namespace facebook {
namespace memcache {

/**
 * Simple cursor of iovecs.
 */
class IovecCursor {
 public:
  /**
   * Builds an IovecCursor.
   *
   * @param iov     Array of iovec.
   * @param iovcnt  Size of iovcnt (must be greater than 0).
   */
  IovecCursor(const struct iovec* iov, size_t iovcnt);

  /**
   * Computes the total size of iovec.
   */
  static size_t computeTotalLength(const struct iovec* iov, size_t iovcnt);

  /**
   * Tells whether there is data available to read.
   */
  bool hasDataAvailable() const;

  /**
   * Read sizeof(T) bytes from the current position of this cursor.
   * Does not advance the cursor.
   * NOTE: There must be at least sizeof(T) bytes available.
   */
  template <class T>
  T peek() const;

  /**
   * Read "size" bytes from the current position of this cursor.
   * Does not advance the cursor.
   * NOTE: There must be at least "size" bytes available.
   *
   * @param dest    Destination buffer.
   * @param size    Number of bytes to read.
   */
  void peekInto(uint8_t* dest, size_t size) const;

  /**
   * Similar to peek(), but advances the cursor.
   */
  template <class T>
  T read();

  /**
   * Similar to peekInto(), but advances the cursor.
   */
  void readInto(uint8_t* dest, size_t size);

  /**
   * Get the total length of this cursor (i.e. the sum of the length of
   * all iovecs).
   */
  inline size_t totalLength() const {
    return totalLength_;
  }

  /**
   * Get the position of this cursor, starting from position 0 of the
   * first iovec.
   */
  inline size_t tell() const {
    return absolutePos_;
  }

  /**
   * Set the position of this cursor, starting from position 0 of the
   * first iovec.
   */
  void seek(size_t pos);

  /**
   * Advance cursor by the given number of bytes.
   * NOTE: User is responsible to check if arguments are valid.
   */
  inline void advance(size_t bytes) {
    assert(bytes <= totalLength() - tell());

    if (FOLLY_LIKELY(bytes < curBufLen_)) {
      absolutePos_ += bytes;
      curBufPos_ += bytes;
      curBufLen_ -= bytes;
      return;
    }
    advanceSlow(bytes);
  }

  /**
   * Retreat cursor by the given number of bytes.
   * NOTE: User is responsible to check if arguments are valid.
   */
  inline void retreat(size_t bytes) {
    assert(bytes <= tell());

    if (FOLLY_LIKELY(bytes <= curBufPos_)) {
      absolutePos_ -= bytes;
      curBufPos_ -= bytes;
      curBufLen_ += bytes;
      return;
    }
    retreatSlow(bytes);
  }

 private:
  const struct iovec* iov_;
  const size_t iovLength_;
  const size_t totalLength_;
  size_t iovIndex_{0}; // This might point just past iov_;
  size_t curBufPos_{0}; // Invariant: This never points past the buffer.
  size_t curBufLen_{0}; // This is kept for optimization during peek()/read().
  size_t absolutePos_{0};

  void advanceSlow(size_t bytes);
  void retreatSlow(size_t bytes);
  void advanceBufferIfEmpty();
  size_t computeTotalLength() const;
};

} // namespace memcache
} // namespace facebook

#include "IovecCursor-inl.h"
