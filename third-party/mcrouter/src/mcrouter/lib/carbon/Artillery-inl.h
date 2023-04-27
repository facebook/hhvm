/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/carbon/RequestReplyUtil.h"

#ifndef LIBMC_FBTRACE_DISABLE
#include "mcrouter/lib/carbon/facebook/ArtilleryUtil.h"
#endif

namespace carbon {
namespace tracing {

#ifdef LIBMC_FBTRACE_DISABLE

inline std::pair<uint64_t, uint64_t> serializeTraceContext(
    const std::string& traceContext) {
  return {0, 0};
}

inline std::string deserializeTraceContext(
    std::pair<uint64_t, uint64_t> serializedTraceId) {
  return "";
}

#else

inline std::pair<uint64_t, uint64_t> serializeTraceContext(
    const std::string& traceContext) {
  if (!traceContext.empty()) {
    return serializeTraceContextInternal(traceContext);
  }
  return {0, 0};
}

inline std::string deserializeTraceContext(
    std::pair<uint64_t, uint64_t> serializedTraceId) {
  if (serializedTraceId.first != 0 || serializedTraceId.second != 0) {
    return deserializeTraceContextInternal(serializedTraceId);
  }
  return "";
}

#endif

} // namespace tracing
} // namespace carbon
