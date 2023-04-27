/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Lz4CompressionCodec.h"
#if FOLLY_HAVE_LIBLZ4 && !defined(DISABLE_COMPRESSION)

namespace facebook {
namespace memcache {

Lz4CompressionCodec::Lz4CompressionCodec(
    std::unique_ptr<folly::IOBuf> dictionary,
    uint32_t id,
    FilteringOptions codecFilteringOptions,
    uint32_t codecCompressionLevel)
    : CompressionCodec(
          CompressionCodecType::LZ4,
          id,
          codecFilteringOptions,
          codecCompressionLevel),
      dictionary_(std::move(dictionary)),
      lz4Stream_(LZ4_createStream()) {
  if (!lz4Stream_) {
    throw std::runtime_error("Failed to allocate LZ4_stream_t");
  }

  int res = LZ4_loadDict(
      lz4Stream_.get(),
      reinterpret_cast<const char*>(dictionary_->data()),
      dictionary_->length());
  if (res < 0 || static_cast<size_t>(res) != dictionary_->length()) {
    throw std::runtime_error(folly::sformat(
        "LZ4 codec: Failed to load dictionary. Return code: {}", res));
  }
}

std::unique_ptr<folly::IOBuf> Lz4CompressionCodec::compress(
    const struct iovec* iov,
    size_t iovcnt) {
  assert(iov);

  auto size = IovecCursor::computeTotalLength(iov, iovcnt);
  folly::IOBuf data = coalesceIovecs(iov, iovcnt, size);
  LZ4_stream_t lz4StreamCopy = *lz4Stream_;

  size_t compressBound = LZ4_compressBound(data.length());
  auto buffer = folly::IOBuf::create(compressBound);

  int compressedSize = LZ4_compress_fast_continue(
      &lz4StreamCopy,
      reinterpret_cast<const char*>(data.data()),
      reinterpret_cast<char*>(buffer->writableTail()),
      data.length(),
      compressBound,
      1);

  // compression is guaranteed to work as we use
  // LZ4_compressBound as destBuffer size.
  assert(compressedSize > 0);

  buffer->append(compressedSize);
  return buffer;
}

std::unique_ptr<folly::IOBuf> Lz4CompressionCodec::uncompress(
    const struct iovec* iov,
    size_t iovcnt,
    size_t uncompressedLength) {
  if (uncompressedLength == 0) {
    throw std::invalid_argument("LZ4 codec: uncompressed length required");
  }

  auto data =
      coalesceIovecs(iov, iovcnt, IovecCursor::computeTotalLength(iov, iovcnt));
  auto buffer = folly::IOBuf::create(uncompressedLength);
  int ret = LZ4_decompress_safe_usingDict(
      reinterpret_cast<const char*>(data.data()),
      reinterpret_cast<char*>(buffer->writableTail()),
      data.length(),
      buffer->tailroom(),
      reinterpret_cast<const char*>(dictionary_->data()),
      dictionary_->length());

  // Should either fail completely ...
  if (ret <= 0) {
    throw std::runtime_error("LZ4 codec: decompression returned invalid value");
  }

  auto const bytesWritten = static_cast<size_t>(ret);
  // or decompress everything.
  assert(bytesWritten == uncompressedLength);

  buffer->append(bytesWritten);
  return buffer;
}

} // namespace memcache
} // namespace facebook
#endif // FOLLY_HAVE_LIBLZ4 && !defined(DISABLE_COMPRESSION)
