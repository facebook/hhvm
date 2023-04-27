/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Format.h>

#if FOLLY_HAVE_LIBLZ4 && !defined(DISABLE_COMPRESSION)
#include "mcrouter/lib/Compression.h"
#include "mcrouter/lib/IOBufUtil.h"
#include "mcrouter/lib/IovecCursor.h"

#include <lz4.h>

namespace facebook {
namespace memcache {

class Lz4CompressionCodec : public CompressionCodec {
 public:
  Lz4CompressionCodec(
      std::unique_ptr<folly::IOBuf> dictionary,
      uint32_t id,
      FilteringOptions codecFilteringOptions,
      uint32_t codecCompressionLevel);

  std::unique_ptr<folly::IOBuf> compress(const struct iovec* iov, size_t iovcnt)
      final;
  std::unique_ptr<folly::IOBuf> uncompress(
      const struct iovec* iov,
      size_t iovcnt,
      size_t uncompressedLength = 0) final;

 private:
  struct Deleter {
    void operator()(LZ4_stream_t* stream) const {
      LZ4_freeStream(stream);
    }
  };
  static constexpr size_t kMaxDictionarySize = 64 * 1024;

  const std::unique_ptr<folly::IOBuf> dictionary_;
  std::unique_ptr<LZ4_stream_t, Deleter> lz4Stream_;
};

} // namespace memcache
} // namespace facebook
#endif // FOLLY_HAVE_LIBLZ4 && !defined(DISABLE_COMPRESSION)
