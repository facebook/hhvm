/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Lz4ImmutableCompressionCodec.h"

namespace facebook {
namespace memcache {

Lz4ImmutableCompressionCodec::Lz4ImmutableCompressionCodec(
    std::unique_ptr<folly::IOBuf> dictionary,
    uint32_t id,
    FilteringOptions codecFilteringOptions,
    uint32_t codecCompressionLevel)
    : CompressionCodec(
          CompressionCodecType::LZ4Immutable,
          id,
          codecFilteringOptions,
          codecCompressionLevel),
      codec_(std::move(dictionary)) {}

std::unique_ptr<folly::IOBuf> Lz4ImmutableCompressionCodec::compress(
    const struct iovec* iov,
    size_t iovcnt) {
  return codec_.compress(iov, iovcnt);
}

std::unique_ptr<folly::IOBuf> Lz4ImmutableCompressionCodec::uncompress(
    const struct iovec* iov,
    size_t iovcnt,
    size_t uncompressedSize) {
  return codec_.decompress(iov, iovcnt, uncompressedSize);
}

} // namespace memcache
} // namespace facebook
