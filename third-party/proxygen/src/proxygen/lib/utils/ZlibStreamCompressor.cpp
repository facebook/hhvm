/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/ZlibStreamCompressor.h>

#include <folly/io/Cursor.h>
#include <folly/lang/Bits.h>

using folly::IOBuf;

// IOBuf uses 24 bytes of data for bookeeping purposes, so requesting for 4073
// bytes of data will be rounded up to an allocation of 1 page.
FOLLY_GFLAGS_DEFINE_int64(
    zlib_compressor_buffer_growth,
    2024,
    "The buffer growth size to use during IOBuf zlib deflation");

namespace proxygen {

namespace {

std::unique_ptr<IOBuf> addOutputBuffer(z_stream* stream, uint32_t length) {
  CHECK_EQ(stream->avail_out, 0);

  auto buf = IOBuf::create(length);
  buf->append(buf->capacity());

  stream->next_out = buf->writableData();
  stream->avail_out = buf->length();

  return buf;
}

int deflateHelper(z_stream* stream, IOBuf* out, int flush) {
  if (stream->avail_out == 0) {
    out->prependChain(
        addOutputBuffer(stream, FLAGS_zlib_compressor_buffer_growth));
  }

  return deflate(stream, flush);
}
} // namespace

void ZlibStreamCompressor::init() {
  if (init_) {
    return;
  }
  init_ = true;

  status_ = Z_OK;

  zlibStream_.zalloc = Z_NULL;
  zlibStream_.zfree = Z_NULL;
  zlibStream_.opaque = Z_NULL;
  zlibStream_.total_in = 0;
  zlibStream_.next_in = Z_NULL;
  zlibStream_.avail_in = 0;
  zlibStream_.avail_out = 0;
  zlibStream_.next_out = Z_NULL;

  DCHECK(level_ == Z_DEFAULT_COMPRESSION ||
         (level_ >= Z_NO_COMPRESSION && level_ <= Z_BEST_COMPRESSION))
      << "Invalid Zlib compression level. level=" << level_;

  switch (type_) {
    case CompressionType::GZIP: {
      auto windowBits = type_ == CompressionType::GZIP ? GZIP_WINDOW_BITS
                                                       : DEFLATE_WINDOW_BITS;
      status_ = deflateInit2(&zlibStream_,
                             level_,
                             Z_DEFLATED,
                             windowBits,
                             MAX_MEM_LEVEL,
                             Z_DEFAULT_STRATEGY);
    } break;
    case CompressionType::DEFLATE:
      status_ = deflateInit(&zlibStream_, level_);
      break;
    default:
      DCHECK(false) << "Unsupported zlib compression type.";
      break;
  }

  if (status_ != Z_OK) {
    LOG(ERROR) << "error initializing zlib stream. r=" << status_;
  }
}

ZlibStreamCompressor::ZlibStreamCompressor(CompressionType type, int level)
    : type_(type), level_(level) {
}

ZlibStreamCompressor::~ZlibStreamCompressor() {
  if (init_) {
    status_ = deflateEnd(&zlibStream_);
  }
}

// Compress an IOBuf chain. Compress can be called multiple times and the
// Zlib stream will be synced after each call. trailer must be set to
// true on the final compression call.
std::unique_ptr<IOBuf> ZlibStreamCompressor::compress(const IOBuf* in,
                                                      bool trailer) {
  init();
  auto bufferLength = FLAGS_zlib_compressor_buffer_growth;

  auto out = addOutputBuffer(&zlibStream_, bufferLength);

  for (auto& range : *in) {
    uint64_t remaining = range.size();
    uint64_t written = 0;
    while (remaining) {
      uint32_t step = remaining;
      zlibStream_.next_in = const_cast<uint8_t*>(range.data() + written);
      zlibStream_.avail_in = step;
      remaining -= step;
      written += step;

      while (zlibStream_.avail_in != 0) {
        status_ = deflateHelper(&zlibStream_, out.get(), Z_NO_FLUSH);
        if (status_ != Z_OK) {
          DLOG(FATAL) << "Deflate failed: " << zlibStream_.msg;
          return nullptr;
        }
      }
    }
  }

  if (trailer) {
    do {
      status_ = deflateHelper(&zlibStream_, out.get(), Z_FINISH);
    } while (status_ == Z_OK);

    if (status_ != Z_STREAM_END) {
      DLOG(FATAL) << "Deflate failed: " << zlibStream_.msg;
      return nullptr;
    }
  } else {
    do {
      status_ = deflateHelper(&zlibStream_, out.get(), Z_SYNC_FLUSH);
    } while (zlibStream_.avail_out == 0);

    if (status_ != Z_OK) {
      DLOG(FATAL) << "Deflate failed: " << zlibStream_.msg;
      return nullptr;
    }
  }

  out->prev()->trimEnd(zlibStream_.avail_out);

  zlibStream_.next_out = Z_NULL;
  zlibStream_.avail_out = 0;

  return out;
}
} // namespace proxygen
