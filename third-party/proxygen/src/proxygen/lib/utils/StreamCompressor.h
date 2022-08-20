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

/**
 * Interface to be implemented by all stream compressors
 */
class StreamCompressor {
 public:
  virtual ~StreamCompressor() {
  }

  virtual std::unique_ptr<folly::IOBuf> compress(const folly::IOBuf* in,
                                                 bool trailer = true) = 0;
  virtual bool hasError() = 0;
};

} // namespace proxygen
