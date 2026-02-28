/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/ZlibStreamDecompressor.h>

#include <folly/io/Cursor.h>

using folly::IOBuf;

namespace proxygen {

void ZlibStreamDecompressor::init(CompressionType type) {
  DCHECK(type_ == CompressionType::NONE) << "Must be uninitialized";
  type_ = type;
  status_ = Z_OK;
  zlibStream_.zalloc = Z_NULL;
  zlibStream_.zfree = Z_NULL;
  zlibStream_.opaque = Z_NULL;
  zlibStream_.total_in = 0;
  zlibStream_.next_in = Z_NULL;
  zlibStream_.avail_in = 0;
  zlibStream_.avail_out = 0;
  zlibStream_.next_out = Z_NULL;

  DCHECK(type == CompressionType::DEFLATE || type == CompressionType::GZIP);
  auto windowBits =
      type_ == CompressionType::GZIP ? GZIP_WINDOW_BITS : DEFLATE_WINDOW_BITS;
  status_ = inflateInit2(&zlibStream_, windowBits);
}

ZlibStreamDecompressor::ZlibStreamDecompressor(
    CompressionType type,
    uint64_t zlib_decompressor_buffer_growth,
    uint64_t zlib_decompressor_buffer_minsize)
    : type_(CompressionType::NONE),
      decompressor_buffer_growth_(zlib_decompressor_buffer_growth),
      decompressor_buffer_minsize_(zlib_decompressor_buffer_minsize),
      status_(Z_OK) {
  init(type);
}

ZlibStreamDecompressor::~ZlibStreamDecompressor() {
  if (type_ != CompressionType::NONE) {
    status_ = inflateEnd(&zlibStream_);
  }
}

std::unique_ptr<IOBuf> ZlibStreamDecompressor::decompress(const IOBuf* in) {
  auto out = IOBuf::create(decompressor_buffer_growth_);
  auto appender = folly::io::Appender(out.get(), decompressor_buffer_growth_);

  const IOBuf* crtBuf = in;
  size_t offset = 0;
  while (true) {
    // Advance to the next IOBuf if necessary
    DCHECK_GE(crtBuf->length(), offset);
    if (crtBuf->length() == offset) {
      crtBuf = crtBuf->next();
      offset = 0;
      if (crtBuf == in) {
        // We hit the end of the IOBuf chain, and are done.
        break;
      }
    }

    if (status_ == Z_STREAM_END) {
      // we convert this into a stream error
      status_ = Z_STREAM_ERROR;
      // we should probably bump up a counter here
      LOG(ERROR) << "error uncompressing buffer: reached end of zlib data "
                    "before the end of the buffer";
      return nullptr;
    }

    // Ensure there is space in the output IOBuf
    appender.ensure(decompressor_buffer_minsize_);
    DCHECK_GT(appender.length(), 0);

    const size_t origAvailIn = crtBuf->length() - offset;
    zlibStream_.next_in = const_cast<uint8_t*>(crtBuf->data() + offset);
    zlibStream_.avail_in = origAvailIn;
    zlibStream_.next_out = appender.writableData();
    zlibStream_.avail_out = appender.length();
    status_ = inflate(&zlibStream_, Z_PARTIAL_FLUSH);
    if (status_ != Z_OK && status_ != Z_STREAM_END) {
      LOG(INFO) << "error uncompressing buffer: r=" << status_;
      return nullptr;
    }

    // Adjust the input offset ahead
    auto inConsumed = origAvailIn - zlibStream_.avail_in;
    offset += inConsumed;
    // Move output buffer ahead
    auto outMove = appender.length() - zlibStream_.avail_out;
    appender.append(outMove);
  }

  return out;
}

} // namespace proxygen
