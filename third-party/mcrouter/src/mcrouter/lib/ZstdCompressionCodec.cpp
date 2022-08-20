/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ZstdCompressionCodec.h"

#if FOLLY_HAVE_LIBZSTD && !defined(DISABLE_COMPRESSION)

namespace facebook {
namespace memcache {

ZstdCompressionCodec::ZstdCompressionCodec(
    std::unique_ptr<folly::IOBuf> dictionary,
    uint32_t id,
    FilteringOptions codecFilteringOptions,
    uint32_t codecCompressionLevel)
    : CompressionCodec(
          CompressionCodecType::ZSTD,
          id,
          codecFilteringOptions,
          codecCompressionLevel),
      dictionary_(std::move(dictionary)),
      compressionLevel_(codecCompressionLevel),
      zstdCContext_(ZSTD_createCCtx(), ZSTD_freeCCtx),
      zstdDContext_(ZSTD_createDCtx(), ZSTD_freeDCtx),
      zstdCDict_(
          ZSTD_createCDict(
              reinterpret_cast<const char*>(dictionary_->data()),
              dictionary_->length(),
              compressionLevel_),
          ZSTD_freeCDict),
      zstdDDict_(
          ZSTD_createDDict(
              reinterpret_cast<const char*>(dictionary_->data()),
              dictionary_->length()),
          ZSTD_freeDDict) {
  if (zstdCDict_ == nullptr || zstdDDict_ == nullptr) {
    throw std::runtime_error("ZSTD codec: Failed to load dictionary.");
  }

  if (zstdCContext_ == nullptr || zstdDContext_ == nullptr) {
    throw std::runtime_error("ZSTD codec: Failed to create context.");
  }
}

std::unique_ptr<folly::IOBuf> ZstdCompressionCodec::compress(
    const struct iovec* iov,
    size_t iovcnt) {
  assert(iov);
  folly::IOBuf data =
      coalesceIovecs(iov, iovcnt, IovecCursor::computeTotalLength(iov, iovcnt));
  auto bytes = data.coalesce();
  size_t compressBound = ZSTD_compressBound(bytes.size());
  auto buffer = folly::IOBuf::create(compressBound);

  size_t const compressedSize = ZSTD_compress_usingCDict(
      zstdCContext_.get(),
      buffer->writableTail(),
      compressBound,
      bytes.data(),
      bytes.size(),
      zstdCDict_.get());

  if (ZSTD_isError(compressedSize)) {
    throw std::runtime_error(folly::sformat(
        "ZSTD codec: Failed to compress. Error: {}",
        ZSTD_getErrorName(compressedSize)));
  }

  buffer->append(compressedSize);
  return buffer;
}

std::unique_ptr<folly::IOBuf> ZstdCompressionCodec::uncompress(
    const struct iovec* iov,
    size_t iovcnt,
    size_t uncompressedLength) {
  folly::IOBuf data =
      coalesceIovecs(iov, iovcnt, IovecCursor::computeTotalLength(iov, iovcnt));
  auto bytes = data.coalesce();
  auto buffer = folly::IOBuf::create(uncompressedLength);

  size_t const bytesWritten = ZSTD_decompress_usingDDict(
      zstdDContext_.get(),
      buffer->writableTail(),
      buffer->capacity(),
      bytes.data(),
      bytes.size(),
      zstdDDict_.get());

  if (ZSTD_isError(bytesWritten)) {
    throw std::runtime_error(folly::sformat(
        "ZSTD codec: decompression returned invalid value. Error: {} ",
        ZSTD_getErrorName(bytesWritten)));
  }

  assert(bytesWritten == uncompressedLength);

  buffer->append(bytesWritten);
  return buffer;
}

} // namespace memcache
} // namespace facebook
#endif // FOLLY_HAVE_LIBZSTD && !defined(DISABLE_COMPRESSION)
