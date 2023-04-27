/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

// We need access to zstd internals (to read frame headers etc.)
#ifndef ZSTD_STATIC_LINKING_ONLY
#define ZSTD_STATIC_LINKING_ONLY
#endif
#ifndef ZDICT_STATIC_LINKING_ONLY
#define ZDICT_STATIC_LINKING_ONLY
#endif

#include <memory>
#include <zdict.h>
#include <zstd.h>

#include <folly/Memory.h>

#include <proxygen/lib/utils/StreamDecompressor.h>

namespace proxygen {

class ZstdStreamDecompressor : public StreamDecompressor {
 public:
  explicit ZstdStreamDecompressor(bool reuseOutBuf = false);

  // May return nullptr on error / no output.
  std::unique_ptr<folly::IOBuf> decompress(const folly::IOBuf* in) override;

  bool hasError() override {
    return status_ == ZstdStatusType::ERROR;
  }

  // Note that this will return true anytime the stream is at a frame boundary.
  // The Zstd spec makes it clear that a response may be composed of multiple
  // concatenated frames. So this may return false positives. It should never
  // return a false negative, though.
  bool finished() override {
    return status_ == ZstdStatusType::FINISHED;
  }

 private:
  static void freeDCtx(ZSTD_DCtx* dctx);

  enum class ZstdStatusType : int { NONE, CONTINUE, ERROR, FINISHED };

  ZstdStatusType status_;

  const std::unique_ptr<ZSTD_DCtx,
                        folly::static_function_deleter<ZSTD_DCtx, freeDCtx>>
      dctx_;

  std::unique_ptr<folly::IOBuf> cachedIOBuf_; // For reuse when output is
                                              // 0-sized

  bool reuseOutBuf_; // Controls whether we may reuse the decompress outBuf
};
} // namespace proxygen
