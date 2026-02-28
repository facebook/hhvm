/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/compress/HPACKContext.h>
#include <proxygen/lib/http/codec/compress/HPACKEncodeBuffer.h>
#include <proxygen/lib/http/codec/compress/HeaderIndexingStrategy.h>

namespace proxygen {

/**
 * Common encoder functionality between HPACK and QPACK
 */
class HPACKEncoderBase {
 public:
  explicit HPACKEncoderBase(bool huffman)
      : streamBuffer_(kBufferGrowth, huffman) {
    indexingStrat_ = HeaderIndexingStrategy::getDefaultInstance();
    // if huffman is false, the huffman limits don't matter
    streamBuffer_.setHuffmanLimits(indexingStrat_->getHuffmanLimits());
  }

  /**
   * Size of a new IOBuf which is added to the chain
   *
   * jemalloc will round up to 4k - overhead
   */
  static const uint32_t kBufferGrowth = 4000;

  void setHeaderTableSize(HeaderTable& table, uint32_t size) {
    if (size != table.capacity()) {
      CHECK(table.setCapacity(size));
      pendingContextUpdate_ = true;
    }
  }

  void setHeaderIndexingStrategy(const HeaderIndexingStrategy* indexingStrat) {
    indexingStrat_ = indexingStrat;
    if (indexingStrat_) {
      streamBuffer_.setHuffmanLimits(indexingStrat_->getHuffmanLimits());
    }
  }
  const HeaderIndexingStrategy* getHeaderIndexingStrategy() const {
    return indexingStrat_;
  }

 protected:
  uint32_t handlePendingContextUpdate(HPACKEncodeBuffer& buf,
                                      uint32_t tableCapacity);

  const HeaderIndexingStrategy* indexingStrat_;
  HPACKEncodeBuffer streamBuffer_;
  bool pendingContextUpdate_{false};
};

} // namespace proxygen
