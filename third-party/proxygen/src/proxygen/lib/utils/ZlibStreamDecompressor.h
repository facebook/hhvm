/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <proxygen/lib/utils/StreamDecompressor.h>
#include <zlib.h>

namespace folly {
class IOBuf;
}

namespace proxygen {

namespace {
constexpr uint64_t kZlibDecompressorBufferGrowthDefault = 480;
constexpr uint64_t kZlibDecompressorBufferMinsizeDefault = 64;

/**
 * These are misleading. See zlib.h for explanation of windowBits param. 31
 * implies a window log of 15 with enabled detection and decoding of the gzip
 * format.
 */
constexpr int GZIP_WINDOW_BITS = 31;
constexpr int DEFLATE_WINDOW_BITS = 15;
} // namespace

class ZlibStreamDecompressor : public StreamDecompressor {
 public:
  explicit ZlibStreamDecompressor(CompressionType type,
                                  uint64_t zlib_decompressor_buffer_growth =
                                      kZlibDecompressorBufferGrowthDefault,
                                  uint64_t zlib_decompressor_buffer_minsize =
                                      kZlibDecompressorBufferMinsizeDefault);

  ZlibStreamDecompressor() = default;

  ~ZlibStreamDecompressor() override;

  void init(CompressionType type);

  std::unique_ptr<folly::IOBuf> decompress(const folly::IOBuf* in) override;

  int getStatus() {
    return status_;
  }

  bool hasError() override {
    return status_ != Z_OK && status_ != Z_STREAM_END;
  }

  bool finished() override {
    return status_ == Z_STREAM_END;
  }

 private:
  CompressionType type_{CompressionType::NONE};
  uint64_t decompressor_buffer_growth_{kZlibDecompressorBufferGrowthDefault};
  uint64_t decompressor_buffer_minsize_{kZlibDecompressorBufferMinsizeDefault};
  z_stream zlibStream_;
  int status_{-1};
};
} // namespace proxygen
