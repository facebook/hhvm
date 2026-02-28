/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/futures/Future.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncSSLSocket.h>

namespace facebook {
namespace memcache {

/**
 * Utility method to offload SSL handshakes to a separate threadpool.
 * The result is a future for a connected socket which is returned on
 * on the original evb.
 */
folly::Future<folly::AsyncSocket::UniquePtr> connectSSLSocketWithAuxIO(
    folly::AsyncSSLSocket::UniquePtr socket,
    folly::SocketAddress address,
    int timeout,
    folly::SocketOptionMap options);

} // namespace memcache
} // namespace facebook
