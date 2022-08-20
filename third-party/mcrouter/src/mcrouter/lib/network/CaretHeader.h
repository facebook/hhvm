/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <utility>

#include <folly/Varint.h>

#include "mcrouter/lib/network/ServerLoad.h"

namespace facebook {
namespace memcache {

constexpr char kCaretMagicByte = '^';
constexpr size_t kMaxAdditionalFields = 6;
constexpr size_t kMaxHeaderLength = 1 /* magic byte */ +
    1 /* GroupVarint header (lengths of 4 ints) */ +
    4 * sizeof(uint32_t) /* body size, typeId, reqId, num additional fields */ +
    2 * kMaxAdditionalFields * folly::kMaxVarintLength64; /* key and value for
                                                          additional fields */

constexpr uint32_t kCaretConnectionControlReqId = 0;

struct CaretMessageInfo {
  uint32_t headerSize;
  uint32_t bodySize;
  uint32_t typeId;
  uint32_t reqId;

  // Additional fields
  std::pair<uint64_t, uint64_t> traceId{0, 0};
  uint64_t supportedCodecsFirstId{0};
  uint64_t supportedCodecsSize{0};
  uint64_t usedCodecId{0};
  uint64_t uncompressedBodySize{0};
  uint64_t dropProbability{0}; // Deprecated in version 37
  ServerLoad serverLoad{0};
};

enum class CaretAdditionalFieldType {
  TRACE_ID = 0,

  // Range of supportted codecs
  SUPPORTED_CODECS_FIRST_ID = 1,
  SUPPORTED_CODECS_SIZE = 2,

  // Id of codec used to compress the data.
  USED_CODEC_ID = 3,

  // Size of body after decompression.
  UNCOMPRESSED_BODY_SIZE = 4,

  // Drop Probability of each request.
  // Deprecated in version 37
  DROP_PROBABILITY = 5,

  // Node ID for trace
  TRACE_NODE_ID = 6,

  // Load on the server
  SERVER_LOAD = 7,
};

} // namespace memcache
} // namespace facebook
