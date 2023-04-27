/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <sys/uio.h>
#include <limits>
#include <memory>

namespace folly {
class IOBuf;
}

namespace facebook {
namespace memcache {

/**
 * Types of the codecs available.
 */
enum class CompressionCodecType {
  // Does not compress.
  // Thread-safe.
  // Doesn't need uncompressed size.
  NO_COMPRESSION = 0,

  // Use LZ4 compression.
  // Not thread-safe.
  // Requires uncompressed size.
  LZ4 = 1,

  // Use ZSTD compression.
  // Not thread-safe.
  // Requires compression level and uncompressed size.
  ZSTD = 2,

  // Use LZ4 compression.
  // Thread-safe.
  LZ4Immutable = 3,
};

/**
 * Options that are used to find the best compression codec
 * for the particular reply.
 */
struct FilteringOptions {
  /**
   * Minimum data size that the codec will try compressing.
   */
  uint32_t minCompressionThreshold{0};
  /**
   * Maximum data size that the codec will try compressing.
   */
  uint32_t maxCompressionThreshold{std::numeric_limits<uint32_t>::max()};
  /**
   * If we have dictionary built on data with only specific operation type
   */
  size_t typeId{0};
  /**
   * Variable to check if the codec is enabled.
   */
  bool isEnabled{true};

  FilteringOptions() {}

  FilteringOptions(
      uint32_t minThreshold,
      uint32_t maxThreshold,
      size_t codecTypeId,
      bool isCodecEnabled)
      : minCompressionThreshold(minThreshold),
        maxCompressionThreshold(maxThreshold),
        typeId(codecTypeId),
        isEnabled(isCodecEnabled) {}
};

/**
 * Dictionary-based compression codec.
 */
class CompressionCodec {
 public:
  virtual ~CompressionCodec() {}

  /**
   * Compress data.
   *
   * @param iov     Iovec array containing the data to compress.
   * @param iovcnt  Size of the array.
   * @return        Compressed data.
   *
   * @throw std::runtime_error    On compression error.
   * @throw std::bad_alloc        On error to allocate output buffer.
   */
  virtual std::unique_ptr<folly::IOBuf> compress(
      const struct iovec* iov,
      size_t iovcnt) = 0;
  std::unique_ptr<folly::IOBuf> compress(const folly::IOBuf& data);
  std::unique_ptr<folly::IOBuf> compress(const void* data, size_t len);

  /**
   * Uncompress data.
   *
   * @param iov     Iovec array containing the data to uncompress.
   * @param iovcnt  Size of the array.
   * @return        Uncompressed data.
   *
   * @throw std::invalid_argument If the codec expects uncompressedLength,
   *                              but 0 is provided.
   * @throw std::runtime_error    On uncompresion error.
   * @throw std::bad_alloc        On error to allocate output buffer.
   */
  virtual std::unique_ptr<folly::IOBuf> uncompress(
      const struct iovec* iov,
      size_t iovcnt,
      size_t uncompressedLength = 0) = 0;
  std::unique_ptr<folly::IOBuf> uncompress(
      const folly::IOBuf& data,
      size_t uncompressedLength = 0);
  std::unique_ptr<folly::IOBuf>
  uncompress(const void* data, size_t len, size_t uncompressedLength = 0);

  /**
   * Return the codec's type.
   */
  CompressionCodecType type() const {
    return type_;
  }

  /**
   * Return the id of this codec.
   */
  uint32_t id() const {
    return id_;
  }

  /**
   * Return the compression level used by this codec.
   */
  uint32_t compressionLevel() const {
    return compressionLevel_;
  }

  /**
   * Return the filtering options used by this codec.
   */
  FilteringOptions filteringOptions() const {
    return filteringOptions_;
  }

 protected:
  /**
   * Builds the compression codec
   *
   * @param type                   Compression algorithm to use.
   * @param id                     Id of the codec. This is merely
   *                               informative - it has no impact in the
   *                               behavior of the codec.
   * @param codecFilteringOptions  Filtering options are needed to be able
   *                               to find the best codec for reply compression.
   * @param codecCompressionLevel  Compression level used by the codec.
   */
  CompressionCodec(
      CompressionCodecType type,
      uint32_t id,
      FilteringOptions codecFilteringOptions,
      uint32_t codecCompressionLevel);

 private:
  const CompressionCodecType type_;
  const uint32_t id_;
  const FilteringOptions filteringOptions_;
  const uint32_t compressionLevel_;
};

/**
 * Creates a compression codec with a given pre-defined dictionary.
 *
 * @param type                   Type of the codec.
 * @param dictionary             Dictionary to compress/uncompress data.
 * @param id                     Id of the codec. This is merely informative -
 *                               it has no impact in the behavior of the codec.
 * @param codecFilteringOptions  Filtering options are needed to be able
 *                               to find the best codec for reply compression.
 * @param codecCompressionLevel  Compression level used by the codec.
 *
 * @throw std::runtime_error     On any error to create the codec.
 */
std::unique_ptr<CompressionCodec> createCompressionCodec(
    CompressionCodecType type,
    std::unique_ptr<folly::IOBuf> dictionary,
    uint32_t id,
    FilteringOptions codecFilteringOptions = FilteringOptions(),
    uint32_t codecCompressionLevel = 1);

} // namespace memcache
} // namespace facebook
