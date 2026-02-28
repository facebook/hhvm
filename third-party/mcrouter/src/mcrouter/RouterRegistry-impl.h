/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdexcept>

#include <folly/Conv.h>

#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/lib/network/gen/MemcacheServer.h"

/**
 * Finds the proper carbon api according to 'routerName', and calls 'func'
 * templated by the corresponding RouterInfo and RequestHandler.
 *
 * 'func' must be a function template, like the following:
 *
 * template <class RouterInfo, template <class> class RequestHandler>
 * void myFunc(...) {
 *   // ...
 * }
 *
 *
 * @param routerName  Name of the router (e.g. Memcache).
 * @param func        The function template that will be called.
 */
#define CALL_BY_ROUTER_NAME(routerName, func, ...)                        \
  do {                                                                    \
    if ((routerName) == facebook::memcache::MemcacheRouterInfo::name) {   \
      func<                                                               \
          facebook::memcache::MemcacheRouterInfo,                         \
          facebook::memcache::MemcacheRequestHandler>(__VA_ARGS__);       \
    } else {                                                              \
      throw std::invalid_argument(                                        \
          folly::to<std::string>("Invalid router name: ", (routerName))); \
    }                                                                     \
  } while (false);

#define CALL_BY_ROUTER_NAME_THRIFT(routerName, func, ...)               \
  do {                                                                  \
    throw std::invalid_argument(                                        \
        folly::to<std::string>("Invalid router name: ", (routerName))); \
  } while (false);
