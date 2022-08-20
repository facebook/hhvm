/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>

#include "mcrouter/lib/carbon/Result.h"
#include "mcrouter/lib/mc/protocol.h"
#include "mcrouter/lib/network/AccessPoint.h"

namespace facebook {
namespace mcrouter {

// Returns true if a rate limiting check passes and tracing can proceed.
bool traceCheckRateLimit();

// Returns the cumulative number of traces logged.
uint64_t traceGetCount();

} // namespace mcrouter
} // namespace facebook

#include "FBTrace-inl.h"
