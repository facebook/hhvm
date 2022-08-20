/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

namespace folly {
class IOBuf;
}

namespace proxygen {

enum class CompressionType : int { NONE, DEFLATE, GZIP, ZSTD };

/**
 * Abstract base class for stream decompressor implementations.
 */
class StreamDecompressor {
 public:
  virtual ~StreamDecompressor() = default;
  virtual std::unique_ptr<folly::IOBuf> decompress(const folly::IOBuf* in) = 0;
  virtual bool hasError() = 0;
  virtual bool finished() = 0;
};
} // namespace proxygen
