/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GFlags.h>
#include <memory>
#include <proxygen/lib/utils/StreamCompressor.h>
#include <proxygen/lib/utils/ZlibStreamDecompressor.h>
#include <zlib.h>

namespace folly {
class IOBuf;
}

FOLLY_GFLAGS_DECLARE_int64(zlib_compressor_buffer_growth);

namespace proxygen {

class ZlibStreamCompressor : public StreamCompressor {
 public:
  explicit ZlibStreamCompressor(CompressionType type, int level);

  ~ZlibStreamCompressor() override;

  void init();

  std::unique_ptr<folly::IOBuf> compress(const folly::IOBuf* in,
                                         bool trailer = true) override;

  int getStatus() {
    return status_;
  }

  bool hasError() override {
    return status_ != Z_OK && status_ != Z_STREAM_END;
  }

  bool finished() {
    return status_ == Z_STREAM_END;
  }

 private:
  CompressionType type_{CompressionType::NONE};
  int level_{Z_DEFAULT_COMPRESSION};
  z_stream zlibStream_;
  int status_{Z_OK};
  bool init_{false};
};
} // namespace proxygen
