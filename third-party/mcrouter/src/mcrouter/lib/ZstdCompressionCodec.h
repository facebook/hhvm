/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Format.h>

#if FOLLY_HAVE_LIBZSTD && !defined(DISABLE_COMPRESSION)
#include <zstd.h>

#include "mcrouter/lib/Compression.h"
#include "mcrouter/lib/IOBufUtil.h"
#include "mcrouter/lib/IovecCursor.h"

namespace facebook {
namespace memcache {

class ZstdCompressionCodec : public CompressionCodec {
 public:
  ZstdCompressionCodec(
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
  template <class T>
  using UPtr = std::unique_ptr<T, size_t (*)(T*)>;

  const std::unique_ptr<folly::IOBuf> dictionary_;
  int compressionLevel_{1};

  UPtr<ZSTD_CCtx> zstdCContext_;
  UPtr<ZSTD_DCtx> zstdDContext_;
  UPtr<ZSTD_CDict> zstdCDict_;
  UPtr<ZSTD_DDict> zstdDDict_;
};

} // namespace memcache
} // namespace facebook
#endif // FOLLY_HAVE_LIBZSTD && !defined(DISABLE_COMPRESSION)
