/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>

namespace facebook {
namespace memcache {

class McrouterOptions;

namespace mcrouter {

class CarbonRouterInstanceBase;
class McrouterStandaloneOptions;

using StandalonePreRunCb =
    std::function<void(CarbonRouterInstanceBase& router)>;

extern thread_local size_t tlsWorkerThreadId;

/**
 * Spawns the standalone server and blocks until it's shutdown.
 *
 * @return True if server shut down cleanly, false if any errors occurred.
 */
template <class RouterInfo, template <class> class RequestHandler>
bool runServer(
    const McrouterOptions& mcrouterOpts,
    const McrouterStandaloneOptions& standaloneOpts,
    StandalonePreRunCb preRunCb = nullptr);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "Server-inl.h"
