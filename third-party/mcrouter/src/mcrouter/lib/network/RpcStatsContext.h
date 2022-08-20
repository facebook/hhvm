/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/network/ServerLoad.h"

namespace facebook {
namespace memcache {

/*
 * Used to pass useful stats and information from AsyncMcClient back up to the
 * routing layer (DestinationRoute).
 *
 * Contains id of codec used for compressing reply, reply size before and after
 * compression. If no compression is used, then usedCodecId is zero. It also
 * contains information regarding the request which triggered the reply.
 */
struct RpcStatsContext {
  RpcStatsContext() = default;
  RpcStatsContext(
      uint32_t usedCodecId_,
      uint32_t replySizeBeforeCompression_,
      uint32_t replySizeAfterCompression_,
      ServerLoad serverLoad_)
      : usedCodecId(usedCodecId_),
        replySizeBeforeCompression(replySizeBeforeCompression_),
        replySizeAfterCompression(replySizeAfterCompression_),
        serverLoad(serverLoad_) {}

  uint32_t usedCodecId{0};
  uint32_t replySizeBeforeCompression{0};
  uint32_t replySizeAfterCompression{0};
  ServerLoad serverLoad{0};
  uint32_t requestBodySize{0};
};

} // namespace memcache
} // namespace facebook
