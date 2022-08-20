/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/Compression.h"
#include "mcrouter/lib/Lz4Immutable.h"

namespace facebook {
namespace memcache {

class Lz4ImmutableCompressionCodec : public CompressionCodec {
 public:
  Lz4ImmutableCompressionCodec(
      std::unique_ptr<folly::IOBuf> dictionary,
      uint32_t id,
      FilteringOptions codecFilteringOptions,
      uint32_t codecCompressionLevel);

  std::unique_ptr<folly::IOBuf> compress(const struct iovec* iov, size_t iovcnt)
      final;

  std::unique_ptr<folly::IOBuf> uncompress(
      const struct iovec* iov,
      size_t iovcnt,
      size_t uncompressedSize) final;

 private:
  Lz4Immutable codec_;
};

} // namespace memcache
} // namespace facebook
