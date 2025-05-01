/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/CPortability.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp/Thrift.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace facebook::memcache {
namespace detail {
FOLLY_ATTR_WEAK std::shared_ptr<apache::thrift::RequestChannel>
getServiceRouterChannel(std::string_view serviceName);
}

template <class T>
T getServiceRouterClient(std::string_view serviceName) {
  XCHECK(detail::getServiceRouterChannel);
  return T(detail::getServiceRouterChannel(serviceName));
}

inline bool hasServiceRouter() {
  return !!detail::getServiceRouterChannel;
}
} // namespace facebook::memcache
