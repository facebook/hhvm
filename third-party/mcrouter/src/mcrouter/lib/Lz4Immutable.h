/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/IOBuf.h>

#include "mcrouter/lib/IovecCursor.h"

namespace facebook {
namespace memcache {

// Memory usage formula : N->2^N Bytes
// (examples: 12 -> 2KB ; 16 -> 32KB; 20 -> 512KB; etc.)
constexpr size_t kMemoryUsage = 14;
constexpr uint32_t kHashLog = kMemoryUsage - 2;
constexpr size_t kHashtableSize = 1 << kHashLog;

using Hashtable = std::array<uint16_t, kHashtableSize>;

/**
 * Internal state of Lz4Immutable.
 */
struct Lz4ImmutableState {
  Hashtable table;
  std::unique_ptr<folly::IOBuf> dictionary;
};

/**
 * A thread-safe version of LZ4 for 64 bit machines that compresses and
 * decompresses data using a static dictionary and works with chained IOBufs.
 */
class Lz4Immutable {
 public:
  /**
   * Builds Lz4Immutable.
   *
   * @param dictionary  Dictionary to use to compress the data.
   *                    The dictionary has to be between sizeof(size_t)
   *                    and 64 KB - otherwise it will throw!
   *
   * @throw std::invalid_argument If the dictionary is invalid.
   */
  explicit Lz4Immutable(std::unique_ptr<folly::IOBuf> dictionary);

  /**
   * Upper bound of compression size.
   *
   * @param size  Size of data to compress.
   */
  size_t compressBound(size_t size) const noexcept;

  /**
   * Compress data into the given buffer.
   *
   * @param iov       Array of iovec describing the input data.
   * @param iovcnt    Number of elements in 'iov'.
   * @param dest      Destination buffer to compress into.
   * @param destSize  Size of 'dest'. Should be large enough to hold the
   *                  compressed data or compression will fail.
   * @return          Size of the compressed data written to 'dest'.
   *
   * @throw std::invalid_argument   If the input is too large to be compressed
   *                                or the destination buffer is too small.
   */
  size_t compressInto(
      const struct iovec* iov,
      size_t iovcnt,
      void* dest,
      size_t destSize) const;

  /**
   * Compress the data.
   *
   * @param source  Data to compress.
   * @return        A newly allocated IOBuf with the compressed data.
   *
   * @throw std::invalid_argument   If the input is too large to be compressed.
   */
  std::unique_ptr<folly::IOBuf> compress(const folly::IOBuf& source) const;
  std::unique_ptr<folly::IOBuf> compress(const struct iovec* iov, size_t iovcnt)
      const;

  /**
   * Decompress the data.
   *
   * @param source            Compressed data to uncompress.
   * @param uncompressedSize  Original size (i.e. size of the data
   *                          before compression).
   * @return                  A newly allocated IOBuf with the uncompressed
   *                          data, or, in case of error, nullptr.
   */
  std::unique_ptr<folly::IOBuf> decompress(
      const folly::IOBuf& source,
      size_t uncompressedSize) const noexcept;
  std::unique_ptr<folly::IOBuf> decompress(
      const struct iovec* iov,
      size_t iovcnt,
      size_t uncompressedSize) const noexcept;

  // Read-only access to the immutable dictionary.
  const folly::IOBuf& dictionary() const {
    return *state_.dictionary;
  }

 private:
  // Compress 'source' into 'output' which has space for 'maxOutputSize' bytes.
  // NOTE: the caller must guarantee 'source' is not too large and
  // 'maxOutputSize' is large enough.
  size_t compressCommon(
      IovecCursor source,
      uint8_t* output,
      size_t maxOutputSize) const;

  const Lz4ImmutableState state_;
};

} // namespace memcache
} // namespace facebook
