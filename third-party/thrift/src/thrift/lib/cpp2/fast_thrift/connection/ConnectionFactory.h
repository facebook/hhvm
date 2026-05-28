/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <functional>
#include <type_traits>
#include <utility>

#include <folly/io/async/AsyncTransport.h>

namespace apache::thrift::fast_thrift::connection {

/**
 * Connection — anything the connection layer is willing to store and tear
 * down. The connection layer drives lifecycle in three phases:
 *
 *   start() — begin reading. May synchronously dispatch the first request
 *             inline (e.g. when bytes were already buffered at the
 *             transport, as with a post-StopTLS handoff). The connection
 *             layer calls this only after the accept hook has fired AND
 *             the connection has been registered in its bookkeeping, so
 *             any synchronous close fired from start() finds the entry.
 *   drain() — initiate graceful shutdown; returns immediately. The
 *             connection sends any peer-disconnect signal, lets in-flight
 *             work complete, and fires its close callback when done.
 *   close() — forceful, synchronous teardown. Used for connections that
 *             didn't drain in time.
 *
 * The close callback is set once via setCloseCallback at install time,
 * and fires exactly once when the connection has fully torn down. The
 * connection layer uses it to remove the entry from its bookkeeping.
 */
template <typename C>
concept Connection = requires(C& c, std::function<void()> cb) {
  c.start();
  c.close();
  c.drain();
  c.setCloseCallback(std::move(cb));
};

/**
 * ConnectionFactory — anything that, given a ready transport, produces a
 * Connection. The returned type is up to the factory; the connection layer
 * doesn't care, as long as it satisfies the Connection concept.
 */
template <typename F>
concept ConnectionFactory =
    requires(F& f, folly::AsyncTransport::UniquePtr socket) {
      { f.getConnection(std::move(socket)) };
    } &&
    Connection<std::decay_t<decltype(std::declval<F&>().getConnection(
        std::declval<folly::AsyncTransport::UniquePtr>()))>>;

/**
 * The connection type produced by a given factory.
 */
template <ConnectionFactory F>
using FactoryConnectionType =
    std::decay_t<decltype(std::declval<F&>().getConnection(
        std::declval<folly::AsyncTransport::UniquePtr>()))>;

} // namespace apache::thrift::fast_thrift::connection
