/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

namespace facebook {
namespace memcache {

class McrouterOptions;

namespace mcrouter {

/**
 * Get mcrotuer stats prefix.
 */
std::string getStatPrefix(const McrouterOptions& opts);

/**
 * Get the full path of the client debug fifo (i.e. debug fifo that replicates
 * AsyncMcClient network traffic)..
 *
 * @param opts    Mcrouter options.
 * @return        Full path of the fifo.
 */
std::string getClientDebugFifoFullPath(const McrouterOptions& opts);

/**
 * Get the full path of the server debug fifo (i.e. debug fifo that replicates
 * AsyncMcServer network traffic)..
 *
 * @param opts    Mcrouter options.
 * @return        Full path of the fifo.
 */
std::string getServerDebugFifoFullPath(const McrouterOptions& opts);
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
