/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef LIBMC_FBTRACE_DISABLE
#include <folly/fibers/FiberManager.h>

#include "fbtrace/libfbtrace/c/fbtrace.h"
#include "mcrouter/lib/carbon/RequestReplyUtil.h"
#include "mcrouter/lib/fbi/cpp/LogFailure.h"
#include "mcrouter/lib/mc/mc_fbtrace_info.h"
#endif

#include "mcrouter/lib/carbon/RequestCommon.h"
#include "mcrouter/lib/network/MessageHelpers.h"

namespace facebook {
namespace mcrouter {

#ifdef LIBMC_FBTRACE_DISABLE

inline bool traceCheckRateLimit() {
  return false;
}

inline bool traceShouldAlwaysEchoContext() {
  return false;
}

inline uint64_t traceGetCount() {
  return 0;
}

inline std::nullptr_t traceRequestReceived(
    const std::string& traceContext,
    folly::StringPiece requestType) {
  // Do nothing by default.
  return nullptr;
}

inline bool traceEchoOnlyRequestReceived(const std::string& traceContext) {
  // Do nothing by default.
  return false;
}

#else

// Fwd declaration
class TracingData;

// Start tracing for a request.
// NOTE: this function does not exist if LIBMC_FBTRACE_DISABLE is defined.
std::shared_ptr<TracingData> traceRequestReceived(
    const std::string& traceContext,
    folly::StringPiece requestType);

// Set up a tracer for reply-side echo only, without the heavyweight logging
// path (no cost counters, no scribe flush). Stores the tracer under the
// response-context key so getCurrentTracer()/replyImpl echoes it back. Returns
// true if a tracer was set up.
// NOTE: this function does not exist if LIBMC_FBTRACE_DISABLE is defined.
bool traceEchoOnlyRequestReceived(const std::string& traceContext);

#endif

} // namespace mcrouter
} // namespace facebook
