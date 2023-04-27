/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Compression.h"

#include <memory>

#include <folly/Format.h>
#include <folly/Portability.h>
#include <folly/io/IOBuf.h>

#include "mcrouter/lib/IovecCursor.h"
#include "mcrouter/lib/Lz4CompressionCodec.h"
#include "mcrouter/lib/Lz4ImmutableCompressionCodec.h"
#include "mcrouter/lib/ZstdCompressionCodec.h"

namespace facebook {
namespace memcache {

CompressionCodec::CompressionCodec(
    CompressionCodecType type,
    uint32_t id,
    FilteringOptions codecFilteringOptions,
    uint32_t codecCompressionLevel)
    : type_(type),
      id_(id),
      filteringOptions_(codecFilteringOptions),
      compressionLevel_(codecCompressionLevel) {}

std::unique_ptr<folly::IOBuf> CompressionCodec::compress(
    const folly::IOBuf& data) {
  auto iov = data.getIov();
  return compress(iov.data(), iov.size());
}
std::unique_ptr<folly::IOBuf> CompressionCodec::compress(
    const void* data,
    size_t len) {
  struct iovec iov;
  iov.iov_base = const_cast<void*>(data);
  iov.iov_len = len;
  return compress(&iov, 1);
}

std::unique_ptr<folly::IOBuf> CompressionCodec::uncompress(
    const folly::IOBuf& data,
    size_t uncompressedLength) {
  auto iov = data.getIov();
  return uncompress(iov.data(), iov.size(), uncompressedLength);
}
std::unique_ptr<folly::IOBuf> CompressionCodec::uncompress(
    const void* data,
    size_t len,
    size_t uncompressedLength) {
  struct iovec iov;
  iov.iov_base = const_cast<void*>(data);
  iov.iov_len = len;
  return uncompress(&iov, 1, uncompressedLength);
}

namespace {

std::unique_ptr<folly::IOBuf> wrapIovec(
    const struct iovec* iov,
    size_t iovcnt) {
  if (iovcnt == 0) {
    return nullptr;
  }

  auto head = folly::IOBuf::wrapBuffer(iov[0].iov_base, iov[0].iov_len);
  for (size_t i = iovcnt - 1; i > 0; --i) {
    head->appendChain(
        folly::IOBuf::wrapBuffer(iov[i].iov_base, iov[i].iov_len));
  }
  return head;
}

/************************
 * No Compression Codec *
 ************************/
class NoCompressionCodec : public CompressionCodec {
 public:
  NoCompressionCodec(
      std::unique_ptr<folly::IOBuf> /* dictionary */,
      uint32_t id,
      FilteringOptions codecFilteringOptions,
      uint32_t codecCompressionLevel)
      : CompressionCodec(
            CompressionCodecType::NO_COMPRESSION,
            id,
            codecFilteringOptions,
            codecCompressionLevel) {}

  std::unique_ptr<folly::IOBuf> compress(const struct iovec* iov, size_t iovcnt)
      final {
    return wrapIovec(iov, iovcnt);
  }
  std::unique_ptr<folly::IOBuf> uncompress(
      const struct iovec* iov,
      size_t iovcnt,
      size_t /* uncompressedLength */ = 0) final {
    return wrapIovec(iov, iovcnt);
  }
};

} // anonymous namespace

/*****************************
 * Compression Codec Factory *
 *****************************/
std::unique_ptr<CompressionCodec> createCompressionCodec(
    CompressionCodecType type,
    std::unique_ptr<folly::IOBuf> dictionary,
    uint32_t id,
    FilteringOptions codecFilteringOptions,
    uint32_t codecCompressionLevel) {
  switch (type) {
    case CompressionCodecType::NO_COMPRESSION:
      return std::make_unique<NoCompressionCodec>(
          std::move(dictionary),
          id,
          codecFilteringOptions,
          codecCompressionLevel);
    case CompressionCodecType::LZ4:
#if FOLLY_HAVE_LIBLZ4 && !defined(DISABLE_COMPRESSION)
      return std::make_unique<Lz4CompressionCodec>(
          std::move(dictionary),
          id,
          codecFilteringOptions,
          codecCompressionLevel);
#else
      LOG(ERROR) << "LZ4 is not available. Returning nullptr.";
      return nullptr;
#endif // FOLLY_HAVE_LIBLZ4 && !defined(DISABLE_COMPRESSION)
    case CompressionCodecType::LZ4Immutable:
      return std::make_unique<Lz4ImmutableCompressionCodec>(
          std::move(dictionary),
          id,
          codecFilteringOptions,
          codecCompressionLevel);
    case CompressionCodecType::ZSTD:
#if FOLLY_HAVE_LIBLZ4 && !defined(DISABLE_COMPRESSION)
      return std::make_unique<ZstdCompressionCodec>(
          std::move(dictionary),
          id,
          codecFilteringOptions,
          codecCompressionLevel);
#else
      LOG(ERROR) << "ZSTD is not available. Returning nullptr.";
      return nullptr;
#endif // FOLLY_HAVE_LIBZSTD && !defined(DISABLE_COMPRESSION)
  }
  return nullptr;
}

} // namespace memcache
} // namespace facebook
