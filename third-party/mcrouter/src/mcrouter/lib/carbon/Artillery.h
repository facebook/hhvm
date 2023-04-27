/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>

namespace carbon {
namespace tracing {

/**
 * Serialized a traceContext in order to send it over the wire.
 */
std::pair<uint64_t, uint64_t> serializeTraceContext(
    const std::string& traceContext);

/**
 * Deserializes a traceContext after receiving it over the wire.
 */
std::string deserializeTraceContext(
    std::pair<uint64_t, uint64_t> serializedTraceId);

} // namespace tracing
} // namespace carbon

#include "Artillery-inl.h"
