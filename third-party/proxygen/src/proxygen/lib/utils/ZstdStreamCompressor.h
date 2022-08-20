/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include <proxygen/lib/utils/StreamCompressor.h>

namespace folly {
class IOBuf;
namespace io {
class StreamCodec;
}
} // namespace folly

namespace proxygen {

class ZstdStreamCompressor : public StreamCompressor {
 public:
  /**
   * The Zstd content-coding for HTTP allows us to use multiple Zstd frames
   * in a single message, if we want. This gives us some freedom to choose how
   * to perform the compression.
   *
   * When independentChunks == false, this compressor produces a single frame,
   * maintaining the history of the stream across chunks, so that previous
   * content can be used to compress new chunks. This necessitates holding that
   * history in memory (the memory cost of which is worth considering if you
   * are holding many long-lived streams open).
   *
   * When independentChunks == true, each chunk is emitted as an independent
   * frame. This means they can't take advantage of previous chunks, so you
   * will get a worse compression ratio. However, no state needs to be stored
   * between chunks, so there's no memory footprint cost.
   */
  explicit ZstdStreamCompressor(int compressionLevel,
                                bool independentChunks = false);

  virtual ~ZstdStreamCompressor() override = default;

  virtual std::unique_ptr<folly::IOBuf> compress(const folly::IOBuf*,
                                                 bool last = true) override;

  virtual bool hasError() override {
    return error_;
  }

 private:
  folly::io::StreamCodec& getCodec();

  std::unique_ptr<folly::io::StreamCodec> codec_;
  const int compressionLevel_;
  const bool independent_;
  bool error_ = false;
};
} // namespace proxygen
