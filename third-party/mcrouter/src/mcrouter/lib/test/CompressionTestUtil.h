/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>
#include <unordered_map>

#include "mcrouter/lib/CompressionCodecManager.h"

namespace facebook {
namespace memcache {
namespace test {

// Utility function used to create a random string of length size.
std::string createBinaryData(size_t size);

// Static CodecConfig map that can be used in tests to initialize a
// CompressionCodecManager.
std::unordered_map<uint32_t, CodecConfigPtr> testCodecConfigs();

} // namespace test
} // namespace memcache
} // namespace facebook
