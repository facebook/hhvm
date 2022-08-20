/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestMcAsciiParserUtil.h"

#include <folly/io/IOBuf.h>

namespace facebook {
namespace memcache {

std::vector<std::vector<size_t>> genChunkedDataSets(
    size_t length,
    size_t maxPieceSize) {
  std::vector<std::vector<std::vector<size_t>>> m(length + 1);
  for (size_t i = 1; i <= std::min(length, maxPieceSize); ++i) {
    m[i].push_back({i});
  }
  for (size_t i = 2; i <= length; ++i) {
    for (size_t piece = 1; piece <= std::min(i, maxPieceSize); ++piece) {
      for (const auto& split : m[i - piece]) {
        m[i].push_back(split);
        m[i].back().push_back(piece);
      }
    }
  }
  return m[length];
}

size_t chunkedDataSetsCnt(size_t length, size_t maxPieceSize) {
  std::vector<size_t> m(length + 1, 0);
  for (size_t i = 1; i <= std::min(length, maxPieceSize); ++i) {
    m[i] = 1;
  }
  for (size_t i = 2; i <= length; ++i) {
    for (size_t piece = 1; piece <= std::min(i, maxPieceSize); ++piece) {
      m[i] += m[i - piece];
      // do not overflow
      m[i] = std::min(m[i], 1UL << 28);
    }
  }
  return m[length];
}

std::unique_ptr<folly::IOBuf> chunkData(
    folly::IOBuf data,
    const std::vector<size_t>& pieces) {
  data.coalesce();
  size_t start = 0;
  std::unique_ptr<folly::IOBuf> buffer;
  for (const auto& piece : pieces) {
    auto p = folly::IOBuf::copyBuffer(data.data() + start, piece);
    if (start == 0) {
      buffer = std::move(p);
    } else {
      buffer->prependChain(std::move(p));
    }
    start += piece;
  }
  return buffer;
}
} // namespace memcache
} // namespace facebook
